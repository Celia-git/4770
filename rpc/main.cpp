#include <iostream>
#include <sqlite3.h>
#include <string>
#include <sstream>

// Forward declarations for stub functions and main logic
// Note: RPC_call_B2_credit in the original code had a typo: RPC_call+B2_credit
int RPC_call_B1_credit(const std::string& account, int amount);
int RPC_call_B1_debit(const std::string& account, int amount);
int RPC_call_B2_credit(const std::string& account, int amount);
int RPC_call_B2_debit(const std::string& account, int amount);

// Callback function for sqlite3_exec (not strictly needed for single-value select, but good practice for select queries)
static int callback(void *data, int argc, char **argv, char **azColName) {
    // We expect one row, one column (bank_id)
    if (argc > 0 && argv[0]) {
        *static_cast<std::string*>(data) = argv[0];
    }
    return 0;
}

// Open database
int open_db(sqlite3** db) {
    // Open a database file named "banks.db"
    // This file is expected to contain a table that maps account to bank ID.
    int rc = sqlite3_open("banks.db", db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(*db) << std::endl;
        // The sqlite3_close() called in main won't hurt, even if open failed, but it's cleaner to handle it here.
        // If sqlite3_open fails, it may or may not allocate the db handle. If it did, it should be closed.
        // We'll return the error and let main's check handle the cleanup or early exit.
    } else {
        // Optional: Create a dummy table for testing if it doesn't exist
        const char* sql = "CREATE TABLE IF NOT EXISTS accounts (account_num TEXT PRIMARY KEY, bank_id TEXT NOT NULL);";
        // Optional: Insert dummy data
        const char* insert_sql = 
            "INSERT OR IGNORE INTO accounts VALUES ('12345678', 'BANK1');"
            "INSERT OR IGNORE INTO accounts VALUES ('87654321', 'BANK2');";
        
        char *zErrMsg = 0;
        int create_rc = sqlite3_exec(*db, sql, 0, 0, &zErrMsg);
        if (create_rc != SQLITE_OK) {
            std::cerr << "SQL error during table creation: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        
        int insert_rc = sqlite3_exec(*db, insert_sql, 0, 0, &zErrMsg);
        if (insert_rc != SQLITE_OK) {
            // This is non-fatal for operation if the table is already populated
            sqlite3_free(zErrMsg);
        }
    }
    return rc;
}

// Close database
void close_db(sqlite3* db) {
    sqlite3_close(db);
}

// Get bank id
std::string get_bank_for_account(sqlite3* db, const std::string& account) {
    std::string bank_id;
    char *zErrMsg = 0;
    
    // SQL query to find the bank_id for the given account number
    std::stringstream ss;
    ss << "SELECT bank_id FROM accounts WHERE account_num = '" << account << "';";
    std::string sql = ss.str();

    // The callback function will populate the bank_id string if a row is found
    int rc = sqlite3_exec(db, sql.c_str(), callback, &bank_id, &zErrMsg);

    if (rc != SQLITE_OK) {
        // std::cerr << "SQL error in get_bank_for_account: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return ""; // Return empty on error
    }

    return bank_id;
}

// Business logic~ CREDIT
int VB_credit(sqlite3* db, const std::string& account, int amount) {
    std::string bank = get_bank_for_account(db, account);
    if (bank.empty()) {
        return -1; // Account not found or DB error
    }
    if (bank == "BANK1") {
        return RPC_call_B1_credit(account, amount);
    } else if (bank == "BANK2") {
        return RPC_call_B2_credit(account, amount); // Fixed typo from RPC_call+B2_credit
    }
    else {
        return -2; // Unknown bank ID
    }
}

// Business logic: debit
int VB_debit(sqlite3* db, const std::string& account, int amount) {
    std::string bank = get_bank_for_account(db, account);
    if (bank.empty()) {
        return -1; // Account not found or DB error
    }
    if (bank == "BANK1") {
        return RPC_call_B1_debit(account, amount);
    } else if (bank == "BANK2") {
        return RPC_call_B2_debit(account, amount);
    }
    else {
        return -2; // Unknown bank ID
    }
}

// Business logic: transfer
int VB_transfer(sqlite3* db, const std::string& account1, const std::string& account2, int amount) {
    std::string bank1 = get_bank_for_account(db, account1);
    std::string bank2 = get_bank_for_account(db, account2);

    if (bank1.empty() || bank2.empty()) {
        return -1; // One or both accounts not found or DB error
    }

    // Case 1: Transfer between accounts in the same bank
    if (bank1 == bank2) {
        if (bank1 == "BANK1") {
            // For a same-bank transfer, debit account1 and credit account2 (must be atomic in a real system)
            int debit_result = RPC_call_B1_debit(account1, amount);
            if (debit_result != 0) return debit_result;
            return RPC_call_B1_credit(account2, amount);
        } else if (bank1 == "BANK2") {
            int debit_result = RPC_call_B2_debit(account1, amount);
            if (debit_result != 0) return debit_result;
            return RPC_call_B2_credit(account2, amount);
        } else {
            return -2; // Unknown bank ID
        }
    }
    // Case 2: Transfer between different banks (Cross-bank transfer)
    else {
        // A simple cross-bank transfer logic (debit then credit, without distributed transaction guarantee)
        int debit_result = -3; // Initialize with a temporary error code
        int credit_result = -3;
        
        // Debit account1
        if (bank1 == "BANK1") {
            debit_result = RPC_call_B1_debit(account1, amount);
        } else if (bank1 == "BANK2") {
            debit_result = RPC_call_B2_debit(account1, amount);
        } else {
            return -2; // Unknown bank1 ID
        }
        
        if (debit_result != 0) {
            return debit_result; // Debit failed
        }

        // Credit account2
        if (bank2 == "BANK1") {
            credit_result = RPC_call_B1_credit(account2, amount);
        } else if (bank2 == "BANK2") {
            credit_result = RPC_call_B2_credit(account2, amount);
        } else {
            // CRITICAL: Debit succeeded, but bank2 is unknown. Need manual reconciliation in a real system.
            // For this exercise, we just return an error.
            return -4; // Unknown bank2 ID after successful debit
        }

        if (credit_result != 0) {
            // CRITICAL: Debit succeeded, but credit failed. Need manual reconciliation in a real system.
            // For this exercise, we return a specific error to indicate this partial failure.
            return -5; // Partial failure (debit succeeded, credit failed)
        }
        
        return 0; // Success
    }
}


// Stub RPC call functions implementations
// These simulate network calls to the respective bank's systems
int RPC_call_B1_credit(const std::string& account, int amount) {
    // std::cout << "RPC_call_B1_credit: account " << account << " amount " << amount << std::endl;
    // Simulate success
    return 0;
}
int RPC_call_B1_debit(const std::string& account, int amount) {
    // std::cout << "RPC_call_B1_debit: account " << account << " amount " << amount << std::endl;
    // Simulate success
    return 0;
}
int RPC_call_B2_credit(const std::string& account, int amount) {
    // std::cout << "RPC_call_B2_credit: account " << account << " amount " << amount << std::endl;
    // Simulate success
    return 0;
}
int RPC_call_B2_debit(const std::string& account, int amount) {
    // std::cout << "RPC_call_B2_debit: account " << account << " amount " << amount << std::endl;
    // Simulate success
    return 0;
}


// Usage: ./main <command> <account> <amount>
// Usage for transfer: ./main transfer <account1> <account2> <amount>
int main(int argc, char* argv[]) {
    // Modified argument check for credit/debit/transfer
    if (argc < 4) {
        std::cerr << "Usage for credit/debit: " << argv[0] << " <credit|debit> <account> <amount>\n";
        std::cerr << "Usage for transfer: " << argv[0] << " transfer <account1> <account2> <amount>\n";
        return 1;
    }

    std::string command = argv[1];
    sqlite3* db;
    
    if (open_db(&db) != SQLITE_OK) return 1;

    int result = -1;

    if (command == "credit" || command == "debit") {
        if (argc < 4) {
            std::cerr << "Missing arguments for " << command << "\n";
            close_db(db);
            return 1;
        }
        std::string account = argv[2];
        int amount;
        try {
            amount = std::stoi(argv[3]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid amount: " << argv[3] << "\n";
            close_db(db);
            return 1;
        }

        if (command == "credit") {
            result = VB_credit(db, account, amount);
        }
        else if (command == "debit") {
            result = VB_debit(db, account, amount);
        }
        
        if (result == 0) {
            std::cout << command << " operation successful on account " << account << " amount " << amount << std::endl;
        }
        else {
            std::cout << command << " operation failed with error code: " << result << std::endl;
        }
    }
    else if (command == "transfer") {
        if (argc < 5) {
            std::cerr << "Missing arguments for transfer\n";
            close_db(db);
            return 1;
        }
        std::string account1 = argv[2];
        std::string account2 = argv[3];
        int amount;
        try {
            amount = std::stoi(argv[4]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid amount: " << argv[4] << "\n";
            close_db(db);
            return 1;
        }

        result = VB_transfer(db, account1, account2, amount);

        if (result == 0) {
            std::cout << "transfer operation successful from " << account1 << " to " << account2 << " amount " << amount << std::endl;
        }
        else {
            std::cout << "transfer operation failed with error code: " << result << std::endl;
        }
    }
    else {
        std::cerr << "Invalid command: " << command << "\n";
        close_db(db);
        return 1;
    }

    close_db(db);
    return (result == 0) ? 0 : 1;
}