#include <iostream>
#include <string>
#include <algorithm>
#include <limits>

// Removes backslashes and ensures security checks
std::string removeBackslashesSecure(const std::string& input) {
    std::string result;
    result.reserve(input.length()); // Optimize memory allocation

    for (char c : input) {
        // Skip backslashes (only ASCII '\\', not Unicode variants)
        if (c != '\\') {
            // Optional: Filter out other dangerous chars (e.g., null bytes)
            if (c != '\0') {  // Prevent null-byte injection
                result += c;
            }
        }
    }
    return result;
}

int main() {
    std::string userInput;

    // Security: Limit input length to prevent memory exhaustion
    constexpr size_t MAX_INPUT_SIZE = 4096; // Adjust based on use case
    std::cout << "Enter a string (max " << MAX_INPUT_SIZE << " chars): ";
    std::cin >> std::ws; // Skip leading whitespace
    std::getline(std::cin, userInput);

    // Enforce maximum input size
    if (userInput.length() > MAX_INPUT_SIZE) {
        std::cerr << "Error: Input too large (max " << MAX_INPUT_SIZE << " chars allowed).\n";
        return 1;
    }

    // Process and output the sanitized string
    std::string cleanedString = removeBackslashesSecure(userInput);
    std::cout << "Sanitized string (backslashes removed):\n";
    std::cout << cleanedString << std::endl;

    return 0;
}
