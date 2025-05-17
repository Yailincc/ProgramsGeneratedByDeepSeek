#include <iostream>
#include <string>
#include <algorithm>

std::string removeBackslashes(const std::string& input) {
    std::string result;
    // Reserve space to optimize memory allocation
    result.reserve(input.length());
    
    // Copy all characters except backslashes
    for (char c : input) {
        if (c != '\\') {
            result += c;
        }
    }
    
    return result;
}

int main() {
    std::string userInput;
    
    // Get input from user
    std::cout << "Enter a string with backslashes: ";
    std::getline(std::cin, userInput);
    
    // Remove backslashes
    std::string cleanedString = removeBackslashes(userInput);
    
    // Output the result
    std::cout << "String after removing backslashes:\n";
    std::cout << cleanedString << std::endl;
    
    return 0;
}
