import java.io.*;
import java.net.*;

public class CalendarEventServer {
    private final int port;

    public CalendarEventServer(int port) {
        this.port = port;
    }

    public void start() {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server started. Waiting for connections on port " + port + "...");

            while (true) {
                try (Socket clientSocket = serverSocket.accept();
                     ObjectInputStream ois = new ObjectInputStream(clientSocket.getInputStream())) {

                    System.out.println("Client connected: " + clientSocket.getInetAddress());
                    CalendarEvent receivedEvent = (CalendarEvent) ois.readObject();
                    System.out.println("Received event: " + receivedEvent);

                    // Process the event (could pass to another class)
                    processEvent(receivedEvent);

                } catch (ClassNotFoundException e) {
                    System.err.println("Error: Received object is not a CalendarEvent");
                } catch (IOException e) {
                    System.err.println("Error handling client: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
        }
    }

    private void processEvent(CalendarEvent event) {
        System.out.println("\n[Processing Event]");
        System.out.println("Date: " + event.getDate());
        System.out.println("Description: " + event.getEventDescription());
    }

    public static void main(String[] args) {
        CalendarEventServer server = new CalendarEventServer(12345);
        server.start();
    }
}
