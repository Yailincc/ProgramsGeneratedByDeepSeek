#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <sodium.h>

class SecureAESEncryptor {
private:
    std::vector<unsigned char> key;
    std::vector<unsigned char> iv;
    static constexpr int KEY_LENGTH = 32; // AES-256
    static constexpr int IV_LENGTH = 12;  // GCM recommended IV size (96 bits)
    static constexpr int TAG_LENGTH = 16; // GCM authentication tag

    // Secure memory cleanup
    void secure_clean(std::vector<unsigned char>& data) {
        if (!data.empty()) {
            sodium_memzero(data.data(), data.size());
        }
    }

public:
    SecureAESEncryptor() {
        if (sodium_init() < 0) {
            throw std::runtime_error("Libsodium initialization failed");
        }

        // Secure key and IV generation
        key.resize(KEY_LENGTH);
        iv.resize(IV_LENGTH);
        
        if (RAND_bytes(key.data(), KEY_LENGTH) != 1) {
            secure_clean(key);
            throw std::runtime_error("Failed to generate random key");
        }
        
        if (RAND_bytes(iv.data(), IV_LENGTH) != 1) {
            secure_clean(key);
            secure_clean(iv);
            throw std::runtime_error("Failed to generate random IV");
        }
    }

    ~SecureAESEncryptor() {
        secure_clean(key);
        secure_clean(iv);
    }

    // Encrypt plaintext using AES-256-GCM (authenticated encryption)
    std::vector<unsigned char> encrypt(const std::string& plaintext) {
        if (plaintext.empty()) {
            throw std::runtime_error("Empty plaintext");
        }

        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(
            EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
        
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }

        // Initialize encryption operation
        if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, 
                              key.data(), iv.data()) != 1) {
            throw std::runtime_error("Failed to initialize encryption");
        }

        // Provide the plaintext to be encrypted
        std::vector<unsigned char> ciphertext;
        ciphertext.resize(plaintext.size() + EVP_CIPHER_CTX_block_size(ctx.get()));
        
        int len;
        if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, 
                             reinterpret_cast<const unsigned char*>(plaintext.data()), 
                             plaintext.size()) != 1) {
            throw std::runtime_error("Encryption failed");
        }
        int ciphertext_len = len;

        // Finalize the encryption and get the authentication tag
        unsigned char tag[TAG_LENGTH];
        if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) {
            sodium_memzero(tag, TAG_LENGTH);
            throw std::runtime_error("Final encryption failed");
        }
        ciphertext_len += len;

        // Get the authentication tag
        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_LENGTH, tag) != 1) {
            sodium_memzero(tag, TAG_LENGTH);
            throw std::runtime_error("Failed to get authentication tag");
        }

        // Resize to actual ciphertext length and append tag
        ciphertext.resize(ciphertext_len);
        ciphertext.insert(ciphertext.end(), tag, tag + TAG_LENGTH);
        
        sodium_memzero(tag, TAG_LENGTH);
        return ciphertext;
    }

    // Get the generated key (base64 encoded for secure display)
    std::string getKeyBase64() const {
        return bytesToBase64(key);
    }

    // Get the generated IV (base64 encoded for secure display)
    std::string getIVBase64() const {
        return bytesToBase64(iv);
    }

private:
    // Helper function to convert bytes to base64 string
    static std::string bytesToBase64(const std::vector<unsigned char>& bytes) {
        if (bytes.empty()) return "";
        
        std::unique_ptr<BIO, decltype(&BIO_free_all)> b64(BIO_new(BIO_f_base64()), BIO_free_all);
        BIO* mem = BIO_new(BIO_s_mem());
        BIO_push(b64.get(), mem);
        
        BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
        BIO_write(b64.get(), bytes.data(), bytes.size());
        BIO_flush(b64.get());
        
        char* data;
        long len = BIO_get_mem_data(mem, &data);
        return std::string(data, len);
    }

    // Delete copy operations for security
    SecureAESEncryptor(const SecureAESEncryptor&) = delete;
    SecureAESEncryptor& operator=(const SecureAESEncryptor&) = delete;
};

int main() {
    try {
        // Initialize OpenSSL
        OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS | 
                           OPENSSL_INIT_ADD_ALL_CIPHERS |
                           OPENSSL_INIT_ADD_ALL_DIGESTS, nullptr);

        // Create encryptor (generates key and IV)
        SecureAESEncryptor encryptor;

        // Get string to encrypt from user
        std::string plaintext;
        std::cout << "Enter text to encrypt: ";
        std::getline(std::cin, plaintext);

        // Encrypt the string
        auto ciphertext = encryptor.encrypt(plaintext);

        // Securely display results (truncated for safety)
        std::cout << "\nGenerated Key (first 8 chars of base64): " 
                  << encryptor.getKeyBase64().substr(0, 8) << "..." << std::endl;
        std::cout << "Generated IV (first 8 chars of base64): " 
                  << encryptor.getIVBase64().substr(0, 8) << "..." << std::endl;
        std::cout << "Ciphertext length: " << ciphertext.size() << " bytes" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Security Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
