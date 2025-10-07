#include <iostream>

#include <sqlite3.h>

#include <string>

int main() {

    sqlite3* db;

    char* errMsg = 0;

    int rc;

    // Open database

    rc = sqlite3_open("test.db", &db);

    if (rc) {

        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;

        return 1;

    } else {

        std::cout << "Opened database successfully" << std::endl;

    }

    // Create table

    std::string sqlCreateTable = "CREATE TABLE IF NOT EXISTS USERS(ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL, AGE INT NOT NULL);";

    rc = sqlite3_exec(db, sqlCreateTable.c_str(), 0, 0, &errMsg);

    if (rc != SQLITE_OK) {

        std::cerr << "SQL error: " << errMsg << std::endl;

        sqlite3_free(errMsg);

    } else {

        std::cout << "Table created successfully" << std::endl;

    }

    // Insert data

    std::string sqlInsert = "INSERT INTO USERS (ID,NAME,AGE) VALUES (1, 'Alice', 30);";

    rc = sqlite3_exec(db, sqlInsert.c_str(), 0, 0, &errMsg);

    if (rc != SQLITE_OK) {

        std::cerr << "SQL error: " << errMsg << std::endl;

        sqlite3_free(errMsg);

    } else {

        std::cout << "Records created successfully" << std::endl;

    }

    // Query data

    std::string sqlSelect = "SELECT * from USERS;";

    sqlite3_stmt* stmt;

    rc = sqlite3_prepare_v2(db, sqlSelect.c_str(), -1, &stmt, 0);

    if (rc == SQLITE_OK) {

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {

            int id = sqlite3_column_int(stmt, 0);

            const unsigned char* name = sqlite3_column_text(stmt, 1);

            int age = sqlite3_column_int(stmt, 2);

            std::cout << "ID = " << id << ", NAME = " << name << ", AGE = " << age << std::endl;

        }

        sqlite3_finalize(stmt);

    } else {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;

    }

    // Close database

    sqlite3_close(db);

    std::cout << "Database closed" << std::endl;

    return 0;

}
