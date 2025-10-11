#include <iostream>
#include <sqlite3.h>
#include <string>
#include <sstream>

// Forward declarations for stub functions and main logic
int RPC_call_B1_credit(const std::string& account, int amount);
int RPC_call_B1_debit(const std::string& account, int amount);
int RPC_call_B2_credit(const std::string& account, int amount);
int RPC_call_B2_debit(const std::string& account, int amount);

// Callback function for sqlite3_exec 
static int callback(void *data, int argc, char **argv, char **azColName) {
    // We expect one row, one column (bank_id)
    if (argc > 0 && argv[0]) {
        *static_cast<std::string*>(data) = argv[0];
    }
    return 0;
}

// Helper function to check if column exists in a table
bool column_exists(sqlite3* db, const std::string& table, const std::string& column) {
    sqlite3_stmt* stmt = nullptr;
    std::string pragma_sql = "PRAGMA table_info(" + table + ");";
    bool found = false;

    if (sqlite3_prepare_v2(db, pragma_sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* col_name = sqlite3_column_text(stmt, 1); // Column name is at index 1
            if (col_name && column == reinterpret_cast<const char*>(col_name)) {
                found = true;
                break;
            }
        }
    } else {
        std::cerr << "Failed to prepare statement to check columns.\n";
    }
    sqlite3_finalize(stmt);
    return found;
}

// Open database
int open_db(sqlite3** db) {
    int rc = sqlite3_open("banks.db", db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(*db) << std::endl;
        return rc;
    }

    // Create accounts table if not exists (without amount column for initial backwards compatibility)
    const char* create_table_sql =
        "CREATE TABLE IF NOT EXISTS accounts ("
        "account_num TEXT PRIMARY KEY, "
        "bank_id TEXT NOT NULL"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(*db, create_table_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error during table creation: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return rc;
    }

    // Check if 'amount' column exists, add it if not
    if (!column_exists(*db, "accounts", "amount")) {
        const char* add_column_sql = "ALTER TABLE accounts ADD COLUMN amount INTEGER DEFAULT 0;";
        rc = sqlite3_exec(*db, add_column_sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error adding amount column: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return rc;
        }
    }

    // Insert initial dummy data with amount 0 if not exist
    const char* insert_sql =
        "INSERT OR IGNORE INTO accounts (account_num, bank_id, amount) VALUES ('A12345', 'BANK1', 0);"
        "INSERT OR IGNORE INTO accounts (account_num, bank_id, amount) VALUES ('B12345', 'BANK2', 0);";

    rc = sqlite3_exec(*db, insert_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        // Non-fatal: likely already populated
        sqlite3_free(errMsg);
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

// Helper for get_account_amount
static int get_amount_callback(void* data, int argc, char** argv, char** azColName) {
    if (argc > 0 && argv[0]) {
        *static_cast<int*>(data) = std::stoi(argv[0]);
    }
    return 0;
}

// Return the amount of $ in account
int get_account_amount(sqlite3* db, const std::string& account) {
    int amount = -1;  // -1 indicates error or not found
    std::string sql = "SELECT amount FROM accounts WHERE account_num = '" + account + "';";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), get_amount_callback, &amount, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error getting amount: " << (errMsg ? errMsg : "unknown error") << std::endl;
        sqlite3_free(errMsg);
        return -1;
    }
    return amount;
}

// Update the local database after successful RPC
int update_account_amount(sqlite3* db, const std::string& account, int new_amount) {
    std::string sql = "UPDATE accounts SET amount = " + std::to_string(new_amount) + " WHERE account_num = '" + account + "';";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error updating amount: " << (errMsg ? errMsg : "unknown error") << std::endl;
        sqlite3_free(errMsg);
        return rc;
    }
    return 0;
}

// Business logic~ CREDIT
int VB_credit(sqlite3* db, const std::string& account, int amount) {
    std::string bank = get_bank_for_account(db, account);
    if (bank.empty()) {
	std::cout << "Account not found or DB error" << std::endl; 
        return -1; // Account not found or DB error
    }
    if (bank == "BANK1") {
        return RPC_call_B1_credit(account, amount);
    } else if (bank == "BANK2") {
        return RPC_call_B2_credit(account, amount);
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
            // For a same-bank transfer, debit account1 and credit account2 
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
            return debit_result; // Debit faileds
        }

        // Credit account2
        if (bank2 == "BANK1") {
            credit_result = RPC_call_B1_credit(account2, amount);
        } else if (bank2 == "BANK2") {
            credit_result = RPC_call_B2_credit(account2, amount);
        } else {
            // Debit succeeded, but bank2 is unknown.
            return -4; // Unknown bank2 ID after successful debit
        }

        if (credit_result != 0) {
            // Debit succeeded, but credit failed.
            return -5; // Partial failure (debit succeeded, credit failed)
        }
        
        return 0; // Success
    }
}


// Stub RPC call functions implementations to simulate network calls to an actual banking system
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
        std::cerr << "FAILURE Usage for credit/debit: " << argv[0] << " <credit|debit> <account> <amount>\n";
        std::cerr << "FAILURE Usage for transfer: " << argv[0] << " transfer <account1> <account2> <amount>\n";
        return 1;
    }

    std::string command = argv[1];
    sqlite3* db;
    
    if (open_db(&db) != SQLITE_OK) return 1;

    int result = -1;

    if (command == "credit" || command == "debit") {
    std::string account = argv[2];
    int input_amount = 0;
    try {
        input_amount = std::stoi(argv[3]);
    } catch (...) {
        std::cerr << "FAILURE Invalid amount: " << argv[3] << std::endl;
        close_db(db);
        return 1;
    }

    int current_amount = get_account_amount(db, account);
    if (current_amount < 0) {
        std::cerr << "FAILURE Account not found or error reading amount" << std::endl;
        close_db(db);
        return 1;
    }
    std::cout << "Initial amount in " << account << ": " << current_amount << std::endl;

    if (command == "debit" && current_amount < input_amount) {
        std::cerr << "FAILURE Insufficient funds: cannot debit " << input_amount << " from " << current_amount << std::endl;
        close_db(db);
        return 1;
    }

    int result = -1;
    if (command == "credit") {
        result = VB_credit(db, account, input_amount);
        if (result == 0) {
            current_amount += input_amount;
        }
    } else if (command == "debit") {
        result = VB_debit(db, account, input_amount);
        if (result == 0) {
            current_amount -= input_amount;
        }
    }

    if (result == 0) {
        if (update_account_amount(db, account, current_amount) != 0) {
            std::cerr << "FAILURE to update local DB after RPC" << std::endl;
            close_db(db);
            return 1;
        }
        std::cout << command << " operation SUCCESS on account " << account << " amount " << input_amount << std::endl;
        std::cout << "Current amount in " << account << ": " << current_amount << std::endl;
    } else {
        std::cout << command << " operation FAILURE with error code: " << result << std::endl;
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
    int input_amount;
    try {
        input_amount = std::stoi(argv[4]);
    } catch (...) {
        std::cerr << "Invalid amount: " << argv[4] << std::endl;
        close_db(db);
        return 1;
    }

    int amount1 = get_account_amount(db, account1);
    int amount2 = get_account_amount(db, account2);
    if (amount1 < 0 || amount2 < 0) {
        std::cerr << "Error reading accounts\n";
        close_db(db);
        return 1;
    }
    std::cout << "Initial amount in " << account1 << ": " << amount1 << std::endl;
    std::cout << "Initial amount in " << account2 << ": " << amount2 << std::endl;

    if (amount1 < input_amount) {
        std::cerr << "Transfer FAILURE: Insufficient funds in " << account1 << " to transfer " << input_amount << std::endl;
        close_db(db);
        return 1;
    }

    int result = VB_transfer(db, account1, account2, input_amount);
    if (result == 0) {
        amount1 -= input_amount;
        amount2 += input_amount;

        if (update_account_amount(db, account1, amount1) != 0 ||
            update_account_amount(db, account2, amount2) != 0) {
            std::cerr << "FAILURE to update local DB amounts after RPC transfer" << std::endl;
            close_db(db);
            return 1;
        }
        std::cout << "Transfer SUCCESS from " << account1 << " to " << account2 << " amount " << input_amount << std::endl;
        std::cout << "Current amount in " << account1 << ": " << amount1 << std::endl;
        std::cout << "Current amount in " << account2 << ": " << amount2 << std::endl;
    } else {
        std::cout << "Transfer FAILURE with error code: " << result << std::endl;
    }
}
}
