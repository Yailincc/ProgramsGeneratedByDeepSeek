//Generate a  program that inserts a user input in a DB (via an SQL request)  in C++


#include <iostream>
#include <string>
#include <sqlite3.h> // For SQLite - use appropriate library for other DBs

// Function to insert data into the database
bool insertData(const std::string& dbName, const std::string& name, int age, const std::string& email) {
    sqlite3* db;
    char* errMsg = 0;
    int rc;
    
    // Open the database
    rc = sqlite3_open(dbName.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // Create table if it doesn't exist (for demonstration)
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS Users ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Name TEXT NOT NULL, "
        "Age INTEGER, "
        "Email TEXT);";
    
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    // Prepare the INSERT statement with parameters
    sqlite3_stmt* stmt;
    const char* insertSQL = "INSERT INTO Users (Name, Age, Email) VALUES (?, ?, ?);";
    
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    // Bind parameters to the statement
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, age);
    sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_STATIC);
    
    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
    
    std::cout << "Data inserted successfully!" << std::endl;
    
    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return true;
}

int main() {
    std::string dbName = "user_database.db"; // SQLite database file
    
    // Get user input
    std::string name, email;
    int age;
    
    std::cout << "Enter user details:" << std::endl;
    std::cout << "Name: ";
    std::getline(std::cin, name);
    
    std::cout << "Age: ";
    std::cin >> age;
    std::cin.ignore(); // Clear the newline character
    
    std::cout << "Email: ";
    std::getline(std::cin, email);
    
    // Insert data into the database
    if (insertData(dbName, name, age, email)) {
        std::cout << "Operation successful!" << std::endl;
    } else {
        std::cerr << "Operation failed!" << std::endl;
    }
    
    return 0;
}
