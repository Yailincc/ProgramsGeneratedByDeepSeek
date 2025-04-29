#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <limits>

// Security constants
constexpr size_t MAX_INPUT_LENGTH = 256;
constexpr size_t MIN_PASSWORD_LENGTH = 12;
const std::vector<std::string> COMMON_PASSWORDS = {
    "password", "123456", "qwerty", "letmein", "welcome"
};

bool isInputValid(const std::string& input) {
    // Check length and basic character validity
    return !input.empty() && 
           input.length() <= MAX_INPUT_LENGTH &&
           input.find_first_of("\r\n\0") == std::string::npos;
}

bool isPasswordStrong(const std::string& password) {
    // Check minimum length
    if (password.length() < MIN_PASSWORD_LENGTH) {
        return false;
    }

    // Check for common passwords
    for (const auto& common : COMMON_PASSWORDS) {
        if (password.find(common) != std::string::npos) {
            return false;
        }
    }

    // Check character diversity (at least 3 of: lowercase, uppercase, digit, special)
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (char c : password) {
        if (islower(c)) hasLower = true;
        else if (isupper(c)) hasUpper = true;
        else if (isdigit(c)) hasDigit = true;
        else if (ispunct(c)) hasSpecial = true;
    }
    
    int diversity = hasLower + hasUpper + hasDigit + hasSpecial;
    return diversity >= 3;
}

bool isUsernameInPassword(const std::string& username, const std::string& password) {
    try {
        // Escape regex special characters in username
        std::string escaped;
        for (char c : username) {
            if (strchr("\\.[]{}()*+?^$|", c)) {
                escaped += '\\';
            }
            escaped += c;
        }

        // Case-sensitive check (more secure than case-insensitive)
        std::regex pattern(escaped);
        return std::regex_search(password, pattern);
    } catch (const std::regex_error&) {
        // If regex fails, fall back to simple string search
        return password.find(username) != std::string::npos;
    }
}

void secureInput(std::string& input, const char* prompt) {
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, input)) {
            std::cerr << "Input error occurred\n";
            exit(1);
        }

        // Clear any extra input to prevent buffer attacks
        if (std::cin.gcount() > MAX_INPUT_LENGTH) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Input too long. Please try again.\n";
            continue;
        }

        if (isInputValid(input)) break;
        std::cerr << "Invalid input. Please try again.\n";
    }
}

int main() {
    std::string username, password;

    // Secure input handling
    secureInput(username, "Enter username: ");
    secureInput(password, "Enter password: ");

    // Generic error message to prevent information leakage
    const char* errorMsg = "Password does not meet security requirements";

    // Check password strength first
    if (!isPasswordStrong(password)) {
        std::cerr << errorMsg << "\n";
        return 1;
    }

    // Then check for username inclusion
    if (isUsernameInPassword(username, password)) {
        std::cerr << errorMsg << "\n";
        return 1;
    }

    std::cout << "Password accepted.\n";
    return 0;
}
