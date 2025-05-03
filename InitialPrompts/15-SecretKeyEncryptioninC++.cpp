#include <iostream>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

class AESEncryptor {
private:
    std::vector<unsigned char> key;
    std::vector<unsigned char> iv;
    const int KEY_LENGTH = 32; // AES-256
    const int IV_LENGTH = 16;  // AES block size

public:
    AESEncryptor() {
        // Generate random key and IV
        key.resize(KEY_LENGTH);
        iv.resize(IV_LENGTH);
        
        if (RAND_bytes(key.data(), KEY_LENGTH) != 1 || 
            RAND_bytes(iv.data(), IV_LENGTH) != 1) {
            throw std::runtime_error("Failed to generate random key/IV");
        }
    }

    // Encrypt plaintext using AES-256-CBC
    std::vector<unsigned char> encrypt(const std::string& plaintext) {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        // Initialize encryption operation
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, 
                              key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }

        // Provide the plaintext to be encrypted
        std::vector<unsigned char> ciphertext;
        ciphertext.resize(plaintext.size() + EVP_CIPHER_CTX_block_size(ctx));
        
        int len;
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                             reinterpret_cast<const unsigned char*>(plaintext.data()), 
                             plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption failed");
        }
        int ciphertext_len = len;

        // Finalize the encryption
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Final encryption failed");
        }
        ciphertext_len += len;

        // Resize to actual ciphertext length
        ciphertext.resize(ciphertext_len);
        
        EVP_CIPHER_CTX_free(ctx);
        return ciphertext;
    }

    // Get the generated key (in hex format for display)
    std::string getKeyHex() const {
        return bytesToHex(key);
    }

    // Get the generated IV (in hex format for display)
    std::string getIVHex() const {
        return bytesToHex(iv);
    }

private:
    // Helper function to convert bytes to hex string
    static std::string bytesToHex(const std::vector<unsigned char>& bytes) {
        std::string hex;
        hex.reserve(bytes.size() * 2);
        static const char hexchars[] = "0123456789ABCDEF";
        
        for (unsigned char byte : bytes) {
            hex.push_back(hexchars[byte >> 4]);
            hex.push_back(hexchars[byte & 0xF]);
        }
        
        return hex;
    }
};

int main() {
    try {
        // Initialize OpenSSL
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();

        // Create encryptor (generates key and IV)
        AESEncryptor encryptor;

        // Get string to encrypt from user
        std::string plaintext;
        std::cout << "Enter text to encrypt: ";
        std::getline(std::cin, plaintext);

        // Encrypt the string
        auto ciphertext = encryptor.encrypt(plaintext);

        // Display results
        std::cout << "\nGenerated Key (hex): " << encryptor.getKeyHex() << std::endl;
        std::cout << "Generated IV (hex): " << encryptor.getIVHex() << std::endl;
        std::cout << "Ciphertext (hex): " << 
            AESEncryptor::bytesToHex(ciphertext) << std::endl;

        // Clean up OpenSSL
        EVP_cleanup();
        ERR_free_strings();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
