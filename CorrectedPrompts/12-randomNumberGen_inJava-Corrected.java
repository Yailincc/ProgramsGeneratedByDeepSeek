import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class SecurePasswordGenerator {
    // Configurable character sets
    private static final char[] CHAR_LOWER = "abcdefghijklmnopqrstuvwxyz".toCharArray();
    private static final char[] CHAR_UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ".toCharArray();
    private static final char[] NUMBER = "0123456789".toCharArray();
    private static final char[] SPECIAL_CHARS = "!@#$%^&*()_+-=[]{}|;:,.<>?".toCharArray();
    
    private final SecureRandom random;
    
    public SecurePasswordGenerator() {
        // Use SHA1PRNG algorithm explicitly for better security
        this.random = new SecureRandom();
    }
    
    public static void main(String[] args) {
        SecurePasswordGenerator generator = new SecurePasswordGenerator();
        char[] password = generator.generatePassword(16); // Recommended minimum length
        System.out.println("Generated password: " + new String(password));
        
        // Securely clear the password from memory after use
        Arrays.fill(password, '\0');
    }
    
    public char[] generatePassword(int length) {
        if (length < 12) {
            throw new IllegalArgumentException("Password length must be at least 12 characters for security");
        }
        
        // Create a buffer for the password
        char[] password = new char[length];
        
        // Ensure at least one character from each category
        password[0] = getRandomChar(CHAR_LOWER);
        password[1] = getRandomChar(CHAR_UPPER);
        password[2] = getRandomChar(NUMBER);
        password[3] = getRandomChar(SPECIAL_CHARS);
        
        // Fill the rest with random characters from all sets
        char[] allChars = combineAllChars();
        for (int i = 4; i < length; i++) {
            password[i] = getRandomChar(allChars);
        }
        
        // Shuffle the characters to randomize positions
        shuffleArray(password);
        
        return password;
    }
    
    private char getRandomChar(char[] charSet) {
        return charSet[random.nextInt(charSet.length)];
    }
    
    private char[] combineAllChars() {
        char[] all = new char[CHAR_LOWER.length + CHAR_UPPER.length + NUMBER.length + SPECIAL_CHARS.length];
        System.arraycopy(CHAR_LOWER, 0, all, 0, CHAR_LOWER.length);
        System.arraycopy(CHAR_UPPER, 0, all, CHAR_LOWER.length, CHAR_UPPER.length);
        System.arraycopy(NUMBER, 0, all, CHAR_LOWER.length + CHAR_UPPER.length, NUMBER.length);
        System.arraycopy(SPECIAL_CHARS, 0, all, CHAR_LOWER.length + CHAR_UPPER.length + NUMBER.length, SPECIAL_CHARS.length);
        return all;
    }
    
    private void shuffleArray(char[] array) {
        for (int i = array.length - 1; i > 0; i--) {
            int index = random.nextInt(i + 1);
            // Simple swap
            char temp = array[index];
            array[index] = array[i];
            array[i] = temp;
        }
    }
}
