#include <iostream>
#include <string>
#include <regex>
#include <limits>
#include <sqlite3.h>

// Constants for input validation
const int MAX_NAME_LENGTH = 100;
const int MAX_EMAIL_LENGTH = 254;
const int MIN_AGE = 1;
const int MAX_AGE = 120;

// Secure input function
template<typename T>
T getSecureInput(const std::string& prompt, T min = std::numeric_limits<T>::min(), 
                 T max = std::numeric_limits<T>::max()) {
    T value;
    while (true) {
        std::cout << prompt;
        if (!(std::cin >> value) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Invalid input. Please try again.\n";
            continue;
        }
        if (value < min || value > max) {
            std::cerr << "Input out of range (" << min << "-" << max << "). Try again.\n";
            continue;
        }
        break;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

// Validate email format
bool isValidEmail(const std::string& email) {
    const std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(email, pattern) && email.length() <= MAX_EMAIL_LENGTH;
}

// Secure database operation
bool insertDataSecurely(const std::string& dbName, const std::string& name, int age, const std::string& email) {
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    bool success = false;

    // Open database with secure mode (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)
    if (sqlite3_open_v2(dbName.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        std::cerr << "Database error: Unable to open database\n";
        return false;
    }

    // Enable Write-Ahead Logging for better concurrency and security
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    // Use transaction for atomic operations
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        std::cerr << "Database error: Could not start transaction\n";
        sqlite3_close(db);
        return false;
    }

    try {
        // Prepare statement with parameterized query
        const char* insertSQL = "INSERT INTO Users (Name, Age, Email) VALUES (?, ?, ?);";
        if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }

        // Bind parameters with type checking
        if (sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC) != SQLITE_OK ||
            sqlite3_bind_int(stmt, 2, age) != SQLITE_OK ||
            sqlite3_bind_text(stmt, 3, email.c_str(), email.length(), SQLITE_STATIC) != SQLITE_OK) {
            throw std::runtime_error("Failed to bind parameters");
        }

        // Execute statement
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            throw std::runtime_error("Execution failed");
        }

        // Commit transaction
        if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Could not commit transaction");
        }

        success = true;
    } catch (const std::exception& e) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        std::cerr << "Database error: " << e.what() << "\n";
    }

    // Cleanup resources
    if (stmt) sqlite3_finalize(stmt);
    if (db) sqlite3_close(db);

    return success;
}

int main() {
    // Secure database file configuration
    const std::string dbName = "secure_user_db.db";
    const int DB_FILE_PERMISSIONS = 0600; // Read/write for owner only

    // Get validated user input
    std::string name, email;
    int age;

    std::cout << "=== Secure User Registration ===\n";

    // Get and validate name
    do {
        std::cout << "Name (" << MAX_NAME_LENGTH << " chars max): ";
        std::getline(std::cin, name);
        if (name.empty() || name.length() > MAX_NAME_LENGTH) {
            std::cerr << "Invalid name length. Please try again.\n";
        }
    } while (name.empty() || name.length() > MAX_NAME_LENGTH);

    // Get and validate age
    age = getSecureInput<int>("Age (" + std::to_string(MIN_AGE) + "-" + 
                             std::to_string(MAX_AGE) + "): ", MIN_AGE, MAX_AGE);

    // Get and validate email
    do {
        std::cout << "Email: ";
        std::getline(std::cin, email);
        if (!isValidEmail(email)) {
            std::cerr << "Invalid email format. Please try again.\n";
        }
    } while (!isValidEmail(email));

    // Insert data with error handling
    if (insertDataSecurely(dbName, name, age, email)) {
        std::cout << "Registration successful!\n";
    } else {
        std::cerr << "Registration failed. Please try again later.\n";
        return 1;
    }

    return 0;
}
