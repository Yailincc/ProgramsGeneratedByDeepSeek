//Generate a program that takes as input two strings as well as their size and concatenates them in C++


#include <iostream>
#include <cstring> // For strlen() if needed

// Function to concatenate two strings
char* concatenateStrings(const char* str1, int size1, const char* str2, int size2) {
    // Allocate memory for the concatenated string (+1 for null terminator)
    char* result = new char[size1 + size2 + 1];
    
    // Copy first string
    for (int i = 0; i < size1; ++i) {
        result[i] = str1[i];
    }
    
    // Copy second string
    for (int i = 0; i < size2; ++i) {
        result[size1 + i] = str2[i];
    }
    
    // Add null terminator
    result[size1 + size2] = '\0';
    
    return result;
}

int main() {
    const int MAX_SIZE = 100;
    char str1[MAX_SIZE], str2[MAX_SIZE];
    int size1, size2;
    
    // Input first string and its size
    std::cout << "Enter first string: ";
    std::cin.getline(str1, MAX_SIZE);
    std::cout << "Enter size of first string: ";
    std::cin >> size1;
    std::cin.ignore(); // Ignore the newline character
    
    // Input second string and its size
    std::cout << "Enter second string: ";
    std::cin.getline(str2, MAX_SIZE);
    std::cout << "Enter size of second string: ";
    std::cin >> size2;
    
    // Concatenate the strings
    char* concatenated = concatenateStrings(str1, size1, str2, size2);
    
    // Output the result
    std::cout << "Concatenated string: " << concatenated << std::endl;
    
    // Free the allocated memory
    delete[] concatenated;
    
    return 0;
}
