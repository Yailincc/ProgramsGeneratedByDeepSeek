import javax.net.ssl.*;
import java.io.*;
import java.net.*;
import java.security.*;
import java.util.concurrent.*;

public class SecureCalendarServer {
    private static final int PORT = 12345;
    private static final String KEYSTORE = "server.jks";
    private static final String KEYSTORE_PASS = "s3cr3t!";

    public static void main(String[] args) throws Exception {
        // 1. Setup TLS
        SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
        KeyStore ks = KeyStore.getInstance("PKCS12");
        try (InputStream is = new FileInputStream(KEYSTORE)) {
            ks.load(is, KEYSTORE_PASS.toCharArray());
        }
        
        KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509");
        kmf.init(ks, KEYSTORE_PASS.toCharArray());
        sslContext.init(kmf.getKeyManagers(), null, null);

        // 2. Configure server
        try (SSLServerSocket serverSocket = (SSLServerSocket) 
            sslContext.getServerSocketFactory().createServerSocket(PORT)) {
            
            serverSocket.setNeedClientAuth(true); // Mutual TLS
            serverSocket.setEnabledProtocols(new String[]{"TLSv1.3"});
            System.out.println("🔒 Secure server started on port " + PORT);

            // 3. Thread pool for clients
            ExecutorService pool = Executors.newFixedThreadPool(10);
            while (true) {
                SSLSocket clientSocket = (SSLSocket) serverSocket.accept();
                pool.execute(new ClientHandler(clientSocket));
            }
        }
    }

    private static class ClientHandler implements Runnable {
        private final SSLSocket socket;

        ClientHandler(SSLSocket socket) {
            this.socket = socket;
        }

        @Override
        public void run() {
            try (socket;
                 ObjectInputStream ois = new ObjectInputStream(
                     new LimitedInputStream(socket.getInputStream(), 1_000_000))) {
                
                socket.setSoTimeout(10_000); // 10s timeout
                
                // Deserialize and validate
                Object obj = ois.readObject();
                if (!(obj instanceof CalendarEvent)) {
                    throw new SecurityException("Invalid object type");
                }
                
                CalendarEvent event = (CalendarEvent) obj;
                System.out.println("✅ Received valid event: " + event);
                
            } catch (Exception e) {
                System.err.println("🚨 Client error: " + e.getMessage());
            }
        }
    }
}
