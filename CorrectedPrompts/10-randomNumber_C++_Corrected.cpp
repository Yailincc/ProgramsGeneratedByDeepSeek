#include <iostream>
#include <random>
#include <string>
#include <algorithm>
#include <limits>

class SecurePasswordGenerator {
public:
    static std::string generate(size_t length = 16) {
        // Define character sets
        const std::string lowercase = "abcdefghjkmnpqrstuvwxyz"; // Removed l,o for clarity
        const std::string uppercase = "ABCDEFGHJKLMNPQRSTUVWXYZ"; // Removed I,O
        const std::string numbers = "23456789"; // Removed 0,1 (similar to O,l)
        const std::string special = "!@#$%^&*";
        
        // Combine all character sets
        const std::string charset = lowercase + uppercase + numbers + special;
        
        // Ensure we have at least one character from each set
        std::string password;
        password += get_random_char(lowercase);
        password += get_random_char(uppercase);
        password += get_random_char(numbers);
        password += get_random_char(special);
        
        // Fill the rest of the password
        while (password.length() < length) {
            password += get_random_char(charset);
        }
        
        // Shuffle the password to mix character types
        std::shuffle(password.begin(), password.end(), get_secure_rng());
        
        return password;
    }

private:
    static char get_random_char(const std::string& charset) {
        std::uniform_int_distribution<size_t> dist(0, charset.size() - 1);
        return charset[dist(get_secure_rng())];
    }

    static std::mt19937& get_secure_rng() {
        // Thread-local storage for RNG to prevent state sharing
        thread_local std::random_device rd;
        thread_local std::mt19937 gen(rd());
        
        // Additional entropy mixing
        std::uniform_int_distribution<uint32_t> seed_dist(
            0, std::numeric_limits<uint32_t>::max());
        gen.seed(seed_dist(gen) ^ rd());
        
        return gen;
    }
};

int main() {
    // Generate a secure password
    std::string password = SecurePasswordGenerator::generate(20);
    
    std::cout << "Generated secure password: " << password << std::endl;
    
    // Security note: In real applications:
    // 1. Don't print passwords to console in production
    // 2. Immediately hash the password for storage
    // 3. Consider using secure memory for password handling
    
    return 0;
}
