//Generate a program that generates AES keys to communicate securely to 3 different users in C++


#include <iostream>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>

// Function to generate a random AES key of specified length
std::vector<unsigned char> generate_aes_key(int key_length) {
    std::vector<unsigned char> key(key_length);
    
    if (RAND_bytes(key.data(), key_length) != 1) {
        throw std::runtime_error("Failed to generate random key");
    }
    
    return key;
}

// Function to convert binary data to hex string for display
std::string bytes_to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (unsigned char byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    
    return ss.str();
}

int main() {
    // Initialize OpenSSL's random number generator
    RAND_poll();
    
    const int key_length = 256 / 8; // 256-bit AES key (32 bytes)
    const int num_users = 3;
    
    try {
        std::cout << "Generating AES-256 keys for " << num_users << " users...\n\n";
        
        // Generate keys for each user
        std::vector<std::vector<unsigned char>> user_keys;
        for (int i = 0; i < num_users; ++i) {
            user_keys.push_back(generate_aes_key(key_length));
        }
        
        // Display the generated keys
        for (int i = 0; i < num_users; ++i) {
            std::cout << "Key for User " << (i + 1) << ": " 
                      << bytes_to_hex(user_keys[i]) << "\n";
        }
        
        std::cout << "\nKey generation complete. Store these keys securely!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
