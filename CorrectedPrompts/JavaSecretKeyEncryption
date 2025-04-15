import javax.crypto.*;
import javax.crypto.spec.GCMParameterSpec;
import java.security.*;
import java.util.Base64;
import java.util.HashSet;
import java.util.Set;

public class SecureAESEncryption {

    // Track used IVs/nonces to prevent reuse
    private static final Set<String> usedNonces = new HashSet<>();
    private static final int GCM_IV_LENGTH = 12; // 12 bytes recommended for GCM
    private static final int GCM_TAG_LENGTH = 16; // 128-bit authentication tag

    public static void main(String[] args) throws Exception {
        String plaintext = "This is a sensitive message requiring protection";
        
        // 1. Generate AES key
        SecretKey secretKey = generateAESKey(256);
        
        // 2. Generate and validate unique nonce (IV for GCM)
        byte[] nonce = generateNonce();
        
        // 3. Encrypt with authentication
        String[] encryptionResults = encryptWithAuth(plaintext, secretKey, nonce);
        String ciphertext = encryptionResults[0];
        String authTag = encryptionResults[1];
        
        // Output results
        System.out.println("Original: " + plaintext);
        System.out.println("AES Key: " + base64Encode(secretKey.getEncoded()));
        System.out.println("Nonce: " + base64Encode(nonce));
        System.out.println("Ciphertext: " + ciphertext);
        System.out.println("Auth Tag: " + authTag);
    }

    public static SecretKey generateAESKey(int keySize) throws NoSuchAlgorithmException {
        KeyGenerator keyGen = KeyGenerator.getInstance("AES");
        keyGen.init(keySize, SecureRandom.getInstanceStrong());
        return keyGen.generateKey();
    }

    public static byte[] generateNonce() throws NoSuchAlgorithmException {
        byte[] nonce = new byte[GCM_IV_LENGTH];
        SecureRandom secureRandom = SecureRandom.getInstanceStrong();
        
        // Ensure unique nonce
        String nonceBase64;
        do {
            secureRandom.nextBytes(nonce);
            nonceBase64 = base64Encode(nonce);
        } while (usedNonces.contains(nonceBase64));
        
        usedNonces.add(nonceBase64);
        return nonce;
    }

    public static String[] encryptWithAuth(String plaintext, SecretKey key, byte[] nonce) throws Exception {
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, nonce);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);
        
        byte[] encrypted = cipher.doFinal(plaintext.getBytes("UTF-8"));
        
        // Extract authentication tag (last GCM_TAG_LENGTH bytes)
        byte[] authTag = new byte[GCM_TAG_LENGTH];
        System.arraycopy(encrypted, encrypted.length - GCM_TAG_LENGTH, authTag, 0, GCM_TAG_LENGTH);
        
        // Ciphertext without the tag
        byte[] ciphertext = new byte[encrypted.length - GCM_TAG_LENGTH];
        System.arraycopy(encrypted, 0, ciphertext, 0, ciphertext.length);
        
        return new String[] {
            base64Encode(ciphertext),
            base64Encode(authTag)
        };
    }

    private static String base64Encode(byte[] bytes) {
        return Base64.getEncoder().encodeToString(bytes);
    }
}
