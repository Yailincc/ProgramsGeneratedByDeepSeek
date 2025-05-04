import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

public class PasswordGenerator {
    private static final String CHAR_LOWER = "abcdefghijklmnopqrstuvwxyz";
    private static final String CHAR_UPPER = CHAR_LOWER.toUpperCase();
    private static final String NUMBER = "0123456789";
    private static final String SPECIAL_CHARS = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    private static final String PASSWORD_ALLOW = CHAR_LOWER + CHAR_UPPER + NUMBER + SPECIAL_CHARS;
    private static final SecureRandom random = new SecureRandom();
    
    public static void main(String[] args) {
        int length = 12; // password length
        System.out.println("Generated password: " + generatePassword(length));
    }
    
    public static String generatePassword(int length) {
        if (length < 4) {
            throw new IllegalArgumentException("Password length must be at least 4");
        }
        
        // Ensure at least one character from each category
        String password = getRandomChar(CHAR_LOWER) 
                            + getRandomChar(CHAR_UPPER) 
                            + getRandomChar(NUMBER) 
                            + getRandomChar(SPECIAL_CHARS);
        
        // Fill the rest with random characters
        for (int i = 4; i < length; i++) {
            password += getRandomChar(PASSWORD_ALLOW);
        }
        
        // Shuffle the characters to randomize their positions
        return shuffleString(password);
    }
    
    private static String getRandomChar(String charSet) {
        int randomIndex = random.nextInt(charSet.length());
        return String.valueOf(charSet.charAt(randomIndex));
    }
    
    private static String shuffleString(String input) {
        List<String> characters = Arrays.asList(input.split(""));
        Collections.shuffle(characters);
        return characters.stream().collect(Collectors.joining());
    }
}
