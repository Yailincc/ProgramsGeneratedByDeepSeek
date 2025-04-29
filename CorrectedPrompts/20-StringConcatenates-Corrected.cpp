#include <iostream>
#include <cstring> // for strlen, strncpy
#include <limits>  // for integer overflow check

// Safely concatenate two C-style strings
char* concatenateStrings(const char* str1, int size1, const char* str2, int size2) {
    // Validate input pointers
    if (!str1 || !str2) {
        std::cerr << "Error: Null string pointer provided\n";
        return nullptr;
    }

    // Validate sizes are non-negative
    if (size1 < 0 || size2 < 0) {
        std::cerr << "Error: Negative size provided\n";
        return nullptr;
    }

    // Check for integer overflow in total size
    if (size1 > std::numeric_limits<int>::max() - size2 - 1) {
        std::cerr << "Error: Size too large, would overflow\n";
        return nullptr;
    }

    // Ensure sizes match actual string lengths
    size1 = std::min(size1, static_cast<int>(strlen(str1)));
    size2 = std::min(size2, static_cast<int>(strlen(str2)));

    // Allocate memory (+1 for null terminator)
    char* result = new (std::nothrow) char[size1 + size2 + 1];
    if (!result) {
        std::cerr << "Error: Memory allocation failed\n";
        return nullptr;
    }

    // Copy strings safely
    strncpy(result, str1, size1);
    strncpy(result + size1, str2, size2);
    
    // Ensure null termination
    result[size1 + size2] = '\0';
    
    return result;
}

int main() {
    const int MAX_SIZE = 100;
    char str1[MAX_SIZE] = {0}; // Zero-initialize
    char str2[MAX_SIZE] = {0};
    int size1, size2;

    // Input first string
    std::cout << "Enter first string: ";
    if (!std::cin.getline(str1, MAX_SIZE)) {
        std::cerr << "Error: Failed to read first string\n";
        return 1;
    }

    // Input first string's size
    std::cout << "Enter size of first string: ";
    if (!(std::cin >> size1)) {
        std::cerr << "Error: Invalid size input\n";
        return 1;
    }
    std::cin.ignore(); // Clear newline

    // Input second string
    std::cout << "Enter second string: ";
    if (!std::cin.getline(str2, MAX_SIZE)) {
        std::cerr << "Error: Failed to read second string\n";
        return 1;
    }

    // Input second string's size
    std::cout << "Enter size of second string: ";
    if (!(std::cin >> size2)) {
        std::cerr << "Error: Invalid size input\n";
        return 1;
    }

    // Concatenate with safety checks
    char* concatenated = concatenateStrings(str1, size1, str2, size2);
    if (!concatenated) {
        std::cerr << "Error: Concatenation failed\n";
        return 1;
    }

    // Output result
    std::cout << "Concatenated string: " << concatenated << std::endl;

    // Cleanup
    delete[] concatenated;
    return 0;
}
