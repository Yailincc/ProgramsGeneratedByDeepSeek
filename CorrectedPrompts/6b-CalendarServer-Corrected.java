import javax.net.ssl.*;
import java.io.*;
import java.net.*;
import java.security.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class SecureCalendarServer {
    private static final int PORT = 12345;
    private static final int MAX_CONNECTIONS = 50;
    private static final String KEYSTORE_PATH = "server_keystore.p12";
    private static final String KEYSTORE_PASSWORD = "Keystore@Pass123";
    private static final AtomicInteger activeConnections = new AtomicInteger(0);

    public static void main(String[] args) {
        System.setProperty("javax.net.ssl.keyStore", KEYSTORE_PATH);
        System.setProperty("javax.net.ssl.keyStorePassword", KEYSTORE_PASSWORD);
        System.setProperty("jdk.tls.server.protocols", "TLSv1.3");

        ExecutorService threadPool = Executors.newFixedThreadPool(MAX_CONNECTIONS);
        
        try (SSLServerSocket serverSocket = createServerSocket()) {
            System.out.println("🔐 Secure Calendar Server started on port " + PORT);
            
            while (true) {
                try {
                    if (activeConnections.get() >= MAX_CONNECTIONS) {
                        Thread.sleep(100);
                        continue;
                    }
                    
                    SSLSocket clientSocket = (SSLSocket) serverSocket.accept();
                    activeConnections.incrementAndGet();
                    threadPool.execute(new ClientHandler(clientSocket));
                } catch (SSLHandshakeException e) {
                    System.err.println("SSL Handshake failed: " + e.getMessage());
                }
            }
        } catch (Exception e) {
            System.err.println("Server fatal error: " + e.getMessage());
        } finally {
            threadPool.shutdown();
        }
    }

    private static SSLServerSocket createServerSocket() throws Exception {
        SSLServerSocketFactory factory = (SSLServerSocketFactory) SSLServerSocketFactory.getDefault();
        SSLServerSocket socket = (SSLServerSocket) factory.createServerSocket(PORT);
        socket.setEnabledCipherSuites(new String[] {
            "TLS_AES_256_GCM_SHA384",
            "TLS_CHACHA20_POLY1305_SHA256"
        });
        socket.setNeedClientAuth(true);
        socket.setSoTimeout(30000);
        return socket;
    }

    private static class ClientHandler implements Runnable {
        private final SSLSocket clientSocket;

        ClientHandler(SSLSocket clientSocket) {
            this.clientSocket = clientSocket;
        }

        @Override
        public void run() {
            try (clientSocket;
                 ObjectInputStream ois = new SecureObjectInputStream(
                     new BufferedInputStream(
                         new LimitedInputStream(clientSocket.getInputStream(), 1048576)))) {
                
                clientSocket.setSoTimeout(10000);
                Object received = ois.readObject();
                
                if (!(received instanceof CalendarDate)) {
                    throw new SecurityException("Invalid object type received");
                }
                
                CalendarDate dateEntry = (CalendarDate) received;
                System.out.println("📅 Received valid entry: " + dateEntry);
                
            } catch (Exception e) {
                System.err.println("⚠️ Error handling client: " + e.getMessage());
            } finally {
                activeConnections.decrementAndGet();
            }
        }
    }
}
