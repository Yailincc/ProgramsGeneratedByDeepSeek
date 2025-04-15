import secrets
import sys
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
import os

def generate_secure_random(bits=256, use_kdf=False, salt=None):
    """
    Generate a cryptographically secure random number.
    
    Args:
        bits (int): Bit-length of the random number (default: 256).
        use_kdf (bool): If True, applies PBKDF2-HMAC for additional hardening.
        salt (bytes): Optional salt for KDF (if not provided, a random one is generated).
    
    Returns:
        int: A secure random number.
    """
    if bits < 128:
        raise ValueError("For security, use at least 128 bits.")
    
    # Generate raw secure random bytes
    random_bytes = secrets.token_bytes(bits // 8)
    
    # Optionally strengthen with PBKDF2-HMAC (if use_kdf=True)
    if use_kdf:
        if salt is None:
            salt = os.urandom(16)  # 128-bit random salt
        
        kdf = PBKDF2HMAC(
            algorithm=hashes.SHA256(),
            length=bits // 8,
            salt=salt,
            iterations=100000,  # High iteration count for security
        )
        random_bytes = kdf.derive(random_bytes)
    
    # Convert to integer
    secure_number = int.from_bytes(random_bytes, byteorder='big')
    return secure_number

if __name__ == "__main__":
    try:
        # Generate a 256-bit secure random number (with KDF hardening)
        secure_num = generate_secure_random(bits=256, use_kdf=True)
        print(f"[✓] Secure Random Number (256-bit): {secure_num}")
        
        # Example: Generate a 6-digit OTP (without KDF for speed)
        otp = secrets.randbelow(10**6)
        print(f"[✓] Secure 6-digit OTP: {otp:06d}")
    except Exception as e:
        print(f"[✗] Security Error: {e}", file=sys.stderr)
        sys.exit(1)
