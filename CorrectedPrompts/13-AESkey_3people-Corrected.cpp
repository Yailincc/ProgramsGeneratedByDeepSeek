#include <iostream>
#include <vector>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <openssl/err.h>
#include <iomanip>
#include <sstream>
#include <sodium.h>

// Secure memory cleanup
struct ZeroizeGuard {
    std::vector<unsigned char>& data;
    explicit ZeroizeGuard(std::vector<unsigned char>& d) : data(d) {}
    ~ZeroizeGuard() {
        sodium_memzero(data.data(), data.size());
    }
};

class CryptoError : public std::runtime_error {
public:
    explicit CryptoError(const std::string& msg) : std::runtime_error(msg + " (OpenSSL: " + ERR_error_string(ERR_get_error(), nullptr) + ")") {}
};

std::vector<unsigned char> generate_secure_key(int key_length, const std::string& user_context) {
    std::vector<unsigned char> key(key_length);
    ZeroizeGuard guard(key); // Ensures cleanup
    
    // Use HKDF with system entropy for better key derivation
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
    if (!pctx) throw CryptoError("HKDF context creation failed");
    
    if (EVP_PKEY_derive_init(pctx) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw CryptoError("HKDF init failed");
    }

    // Get system entropy for salt
    std::vector<unsigned char> salt(32);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        EVP_PKEY_CTX_free(pctx);
        throw CryptoError("Salt generation failed");
    }

    // Set HKDF parameters
    if (EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) <= 0 ||
        EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt.data(), salt.size()) <= 0 ||
        EVP_PKEY_CTX_set1_hkdf_key(pctx, "", 0) <= 0 || // Empty key since we want pure entropy
        EVP_PKEY_CTX_add1_hkdf_info(pctx, reinterpret_cast<const unsigned char*>(user_context.data()), user_context.size()) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw CryptoError("HKDF parameter setup failed");
    }

    // Derive the key
    size_t outlen = key.size();
    if (EVP_PKEY_derive(pctx, key.data(), &outlen) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw CryptoError("Key derivation failed");
    }

    EVP_PKEY_CTX_free(pctx);
    return key; // Guard will clean up when copied
}

std::string bytes_to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

int main() {
    if (sodium_init() == -1) {
        std::cerr << "Error: libsodium initialization failed\n";
        return 1;
    }

    const int key_length = 32; // 256-bit AES key
    const std::string users[] = {"user1@domain", "user2@domain", "user3@domain"};
    const int num_users = sizeof(users)/sizeof(users[0]);

    try {
        std::cout << "Generating secure AES-256 keys...\n\n";

        // Generate and store keys securely
        std::vector<std::vector<unsigned char>> user_keys;
        for (int i = 0; i < num_users; ++i) {
            auto key = generate_secure_key(key_length, users[i] + std::to_string(time(nullptr)));
            user_keys.push_back(key);
            
            // In real use, you would:
            // 1. Store in secure enclave/HSM
            // 2. Or encrypt with KMS before storage
            // 3. Never print to console in production!
            std::cout << "Key for " << users[i] << ": " 
                     << bytes_to_hex(key) << "\n";
            
            // Immediately clear the displayed key from memory
            sodium_memzero(key.data(), key.size());
        }

        std::cout << "\nKey generation complete. Ensure secure storage!\n";
        
        // Secure cleanup
        for (auto& key : user_keys) {
            sodium_memzero(key.data(), key.size());
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Security Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
