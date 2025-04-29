//Generate a pseudorandom number to serve as password in C++

#include <iostream>
#include <random>
#include <string>
#include <algorithm>

std::string generate_password(int length = 12) {
    // Define the character set for the password
    const std::string charset =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    // Random number generator setup
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, charset.size() - 1);
    
    std::string password;
    password.reserve(length);
    
    // Generate random characters
    for (int i = 0; i < length; ++i) {
        password += charset[distrib(gen)];
    }
    
    return password;
}

int main() {
    // Generate a 16-character password
    std::string password = generate_password(16);
    
    std::cout << "Generated password: " << password << std::endl;
    
    return 0;
}
