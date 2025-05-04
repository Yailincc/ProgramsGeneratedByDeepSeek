import javax.crypto.*;
import javax.crypto.spec.*;
import java.security.*;
import java.security.spec.*;
import java.util.Base64;

public class SecureAESEncryption {
    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final int GCM_TAG_LENGTH = 128; // bits
    private static final int IV_LENGTH = 12; // bytes (96 bits recommended for GCM)
    
    public static void main(String[] args) {
        try {
            String original = "This is a secret message";
            
            // 1. Generate key (in real use, store securely)
            SecretKey key = generateAESKey(256);
            
            // 2. Encrypt
            EncryptionResult result = encrypt(original, key);
            
            System.out.println("Original: " + original);
            System.out.println("Encrypted: " + result.cipherText);
            System.out.println("IV: " + Base64.getEncoder().encodeToString(result.iv));
            
            // In real use, NEVER print or log secret keys!
        } catch (Exception e) {
            System.err.println("Encryption error: " + e.getMessage());
        }
    }
    
    public static SecretKey generateAESKey(int keySize) throws NoSuchAlgorithmException {
        KeyGenerator keyGen = KeyGenerator.getInstance("AES");
        keyGen.init(keySize, SecureRandom.getInstanceStrong());
        return keyGen.generateKey();
    }
    
    public static EncryptionResult encrypt(String plaintext, SecretKey key) 
            throws NoSuchAlgorithmException, NoSuchPaddingException,
                   InvalidKeyException, InvalidAlgorithmParameterException,
                   IllegalBlockSizeException, BadPaddingException {
        
        // Generate IV
        byte[] iv = new byte[IV_LENGTH];
        SecureRandom secureRandom = SecureRandom.getInstanceStrong();
        secureRandom.nextBytes(iv);
        
        // Initialize cipher
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);
        
        // Encrypt
        byte[] cipherText = cipher.doFinal(plaintext.getBytes(java.nio.charset.StandardCharsets.UTF_8));
        
        return new EncryptionResult(
            Base64.getEncoder().encodeToString(cipherText),
            iv
        );
    }
    
    static class EncryptionResult {
        final String cipherText;
        final byte[] iv;
        
        EncryptionResult(String cipherText, byte[] iv) {
            this.cipherText = cipherText;
            this.iv = iv;
        }
    }
}
