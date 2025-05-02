import java.io.*;
import java.net.*;
import java.util.*;

public class CalendarEventSender {
    public static void main(String[] args) {
        try (Socket socket = new Socket("localhost", 12345);
             ObjectOutputStream oos = new ObjectOutputStream(socket.getOutputStream())) {
            
            CalendarEvent event = new CalendarEvent(new Date(), "Team Meeting");
            oos.writeObject(event);
            System.out.println("Event sent successfully");
            
        } catch (IOException e) {
            System.err.println("Error sending event: " + e.getMessage());
        }
    }
}
