#include <iostream>
#include <sqlite3.h>
#include <string>


// Open database
int open_db(sqlite3** db) {
	/*
	 * Fill this out
	 */
	return SQLITE_OK;
}

// Close database
void close_db(sqlite3* db) {
	sqlite3_close(db);
}

// Get bank id
std::string get_bank_for_account(sqlite3* db, const std::string& account) {
	std::strring bank_id;

	return bank_id;

// Business logic~ CREDIT
int VB_credit(sqlite3* db, const std::string& account, int amount) {
	std::string bank = get_bank_for_account(db, account);
	if (bank.empty()) {
		return -1;
	}
	if (bank == "BANK1") {
		return RPC_call_B1_credit(account, amount);
	} else if (bank == "BANK2") {
		return RPC_call+B2_credit(account, amount);
	}
	else {
		return -1;
	}

// Business logic: debit
int VB_debit(sqlite3* db, const std::string& account, int amount) {
	/*
	 * implement business logic for debit
	 * /
}

// Business logic: transfer
int VB_transfer(sqlite3* db, const std::string& account1, const std::string& account2, int amount) {
	std::string bank1 = get_bank_for_account(db, account1);
	std::string bank2 = get_bank_for_account(db, account2);

	if (bank1.empty() || bank2.empty()) {
		return -1;
	}
	/*
	 * COMPLETE business logic: transfer
	 */
}


// Stub RPC call functions
int RPC_call_B1_credit(const std::string& account, int amount);
int RPC_call_B1_debit(const std::string& account, int amount);
int RPC_call_B2_credit(const std::string& account, int amount);
int RPC_call_B2_debit(const std::string& account, int amount);

//  Usage: ./main  <command> <account> <amount>;
int main(int argc, char* argv[]) {
	if (argc < 4) {
		std::cerr << "Usage: " << argv[0] << " <command> <account> <amount>\n";
		return 1;
	}

	std::string command = argv[1];
	std::string account = argv[2];
	int amount = std::stoi(argv[3]);

	sqlite3* db;
	if (open_db(&db) != SQLITE_OK) return 1;

	int result = -1;

	if (command == "credit") {
		result = VB_credit(db, account, amount);
	}
	else if (command == "debit") {
		result = VB_debit(db, account, amount);
	}
	else {
		std::cerr << "Invalid command: " << command << "\n";
		close_db(db);
		return 1;
	}

	if (result == 0) {
		std::cout << command << " operation successful on account " << account << " amount " << amount << std::endl;
	}
	else {
		std::cout << command << " operation failed with error code: " << result << std::endl;
	}

	close_db(db);
	return 0;
}

