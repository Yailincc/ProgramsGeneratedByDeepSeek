import javax.crypto.*;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.*;
import java.util.Base64;

public class AESStringEncryption {

    public static void main(String[] args) throws Exception {
        String originalString = "This is a secret message";
        
        // 1. Generate AES key
        SecretKey secretKey = generateAESKey(256); // 256-bit key
        
        // 2. Generate Initialization Vector (IV)
        byte[] iv = generateIV();
        
        // 3. Encrypt the string
        String encryptedString = encrypt(originalString, secretKey, iv);
        
        // 4. Display results
        System.out.println("Original: " + originalString);
        System.out.println("AES Key (Base64): " + Base64.getEncoder().encodeToString(secretKey.getEncoded()));
        System.out.println("IV (Base64): " + Base64.getEncoder().encodeToString(iv));
        System.out.println("Encrypted: " + encryptedString);
    }
    
    // Generate AES key of specified size (128, 192, or 256 bits)
    public static SecretKey generateAESKey(int keySize) throws NoSuchAlgorithmException {
        KeyGenerator keyGenerator = KeyGenerator.getInstance("AES");
        keyGenerator.init(keySize);
        return keyGenerator.generateKey();
    }
    
    // Generate Initialization Vector (IV)
    public static byte[] generateIV() {
        byte[] iv = new byte[16]; // 16 bytes for AES
        new SecureRandom().nextBytes(iv);
        return iv;
    }
    
    // Encrypt a string using AES/CBC/PKCS5Padding
    public static String encrypt(String input, SecretKey key, byte[] iv) throws Exception {
        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        IvParameterSpec ivSpec = new IvParameterSpec(iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, ivSpec);
        
        byte[] encryptedBytes = cipher.doFinal(input.getBytes("UTF-8"));
        return Base64.getEncoder().encodeToString(encryptedBytes);
    }
}
