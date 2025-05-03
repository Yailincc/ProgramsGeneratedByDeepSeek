#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

const string ENCRYPTED_FILE = "data.enc";
const string KEY_FILE = "key.salt";
const int ITERATIONS = 100000;
const int KEY_LENGTH = 32; // AES-256
const int IV_LENGTH = 16;

class SecureString : public string {
public:
    ~SecureString() {
        secure_wipe();
    }

    void secure_wipe() {
        if (!empty()) {
            memset(&(*this)[0], 0, size());
        }
    }
};

void handle_openssl_error() {
    cerr << "OpenSSL error: " << ERR_error_string(ERR_get_error(), nullptr) << endl;
    exit(1);
}

void generate_random(unsigned char* buffer, int length) {
    if (RAND_bytes(buffer, length) != 1) {
        handle_openssl_error();
    }
}

vector<unsigned char> derive_key(const SecureString& password, const unsigned char* salt) {
    vector<unsigned char> key(KEY_LENGTH);
    
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                         salt, PKCS5_SALT_LEN,
                         ITERATIONS, EVP_sha256(),
                         KEY_LENGTH, key.data()) != 1) {
        handle_openssl_error();
    }
    
    return key;
}

void encrypt_and_store(const SecureString& password) {
    // Generate random salt and IV
    unsigned char salt[PKCS5_SALT_LEN];
    unsigned char iv[IV_LENGTH];
    generate_random(salt, PKCS5_SALT_LEN);
    generate_random(iv, IV_LENGTH);

    // Derive encryption key
    auto key = derive_key(password, salt);

    // Set up encryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_openssl_error();

    try {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                              key.data(), iv) != 1) {
            handle_openssl_error();
        }

        // Write salt and IV to key file
        ofstream key_file(KEY_FILE, ios::binary);
        key_file.write(reinterpret_cast<char*>(salt), PKCS5_SALT_LEN);
        key_file.write(reinterpret_cast<char*>(iv), IV_LENGTH);
        key_file.close();

        // Encrypt and write data
        ofstream data_file(ENCRYPTED_FILE, ios::binary);
        vector<unsigned char> ciphertext(password.length() + EVP_MAX_BLOCK_LENGTH);
        int len;
        
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                            reinterpret_cast<const unsigned char*>(password.c_str()),
                            password.length()) != 1) {
            handle_openssl_error();
        }
        
        int ciphertext_len = len;
        
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            handle_openssl_error();
        }
        
        ciphertext_len += len;
        data_file.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext_len);
        data_file.close();

        // Set file permissions
        fs::permissions(ENCRYPTED_FILE, fs::perms::owner_read | fs::perms::owner_write);
        fs::permissions(KEY_FILE, fs::perms::owner_read | fs::perms::owner_write);
    }
    catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    
    EVP_CIPHER_CTX_free(ctx);
}

SecureString decrypt_and_retrieve() {
    ifstream key_file(KEY_FILE, ios::binary);
    ifstream data_file(ENCRYPTED_FILE, ios::binary);
    
    if (!key_file || !data_file) {
        throw runtime_error("No stored credentials found");
    }

    // Read salt and IV
    unsigned char salt[PKCS5_SALT_LEN];
    unsigned char iv[IV_LENGTH];
    key_file.read(reinterpret_cast<char*>(salt), PKCS5_SALT_LEN);
    key_file.read(reinterpret_cast<char*>(iv), IV_LENGTH);

    // Get master password for decryption
    SecureString master_password;
    cout << "Enter decryption password: ";
    getline(cin, master_password);

    // Derive decryption key
    auto key = derive_key(master_password, salt);

    // Set up decryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_openssl_error();

    try {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                              key.data(), iv) != 1) {
            handle_openssl_error();
        }

        // Read encrypted data
        vector<unsigned char> ciphertext(
            (istreambuf_iterator<char>(data_file)),
            istreambuf_iterator<char>()
        );

        vector<unsigned char> plaintext(ciphertext.size() + IV_LENGTH);
        int len;
        
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                            ciphertext.data(), ciphertext.size()) != 1) {
            handle_openssl_error();
        }
        
        int plaintext_len = len;
        
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            throw runtime_error("Decryption failed - wrong password?");
        }
        
        plaintext_len += len;
        plaintext.resize(plaintext_len);

        SecureString result(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
        memset(plaintext.data(), 0, plaintext.size());
        return result;
    }
    catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    
    EVP_CIPHER_CTX_free(ctx);
}

int main() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    try {
        while (true) {
            cout << "\nSecure Password Manager\n";
            cout << "1. Store password\n";
            cout << "2. Retrieve password\n";
            cout << "3. Exit\n";
            cout << "Enter choice: ";

            int choice;
            cin >> choice;
            cin.ignore();

            if (choice == 1) {
                SecureString password;
                cout << "Enter password to store: ";
                getline(cin, password);
                encrypt_and_store(password);
                cout << "Password stored securely!\n";
            }
            else if (choice == 2) {
                auto password = decrypt_and_retrieve();
                cout << "Decrypted password: " << password << endl;
                password.secure_wipe();
            }
            else if (choice == 3) {
                break;
            }
        }
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    EVP_cleanup();
    ERR_free_strings();
    return 0;
}
