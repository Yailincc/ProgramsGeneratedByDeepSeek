from Crypto.Cipher import AES
from Crypto.Protocol.KDF import PBKDF2
from Crypto.Random import get_random_bytes
from Crypto.Hash import SHA256
import base64

# Get user inputs
password = input("Enter password: ").encode()
plaintext = input("Enter plaintext: ").encode('utf-8')  # Explicit encoding

# Generate parameters
salt = get_random_bytes(16)  # 16-byte salt
nonce = get_random_bytes(12)  # 12-byte nonce for GCM (recommended size)

# Key derivation with stronger parameters (600,000 iterations)
key = PBKDF2(
    password=password,
    salt=salt,
    dkLen=32,  # AES-256 key
    count=600_000,  # Increased iteration count
    hmac_hash_module=SHA256
)

# Encrypt with AES-GCM (authenticated encryption)
cipher = AES.new(key, AES.MODE_GCM, nonce=nonce)
ciphertext, tag = cipher.encrypt_and_digest(plaintext)

# Package data with versioning (prepended 0x01 = version 1)
VERSION = b'\x01'  # Version identifier for future compatibility
encrypted_data = VERSION + salt + nonce + tag + ciphertext

# Base64 encode for storage/transmission
encrypted_b64 = base64.b64encode(encrypted_data).decode('utf-8')

print("Encrypted data (base64):", encrypted_b64)
