import javax.net.ssl.*;
import java.io.*;
import java.net.*;
import java.security.*;
import java.util.Date;

public class SecureCalendarClient {
    private static final String TRUSTSTORE = "client_trust.jks";
    private static final String TRUSTSTORE_PASS = "cl13nt!";

    public static void main(String[] args) throws Exception {
        // 1. Setup TLS
        SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
        KeyStore ts = KeyStore.getInstance("JKS");
        try (InputStream is = new FileInputStream(TRUSTSTORE)) {
            ts.load(is, TRUSTSTORE_PASS.toCharArray());
        }
        
        TrustManagerFactory tmf = TrustManagerFactory.getInstance("PKIX");
        tmf.init(ts);
        sslContext.init(null, tmf.getTrustManagers(), null);

        // 2. Connect and send
        try (SSLSocket socket = (SSLSocket) 
            sslContext.getSocketFactory().createSocket("localhost", PORT);
             ObjectOutputStream oos = new ObjectOutputStream(socket.getOutputStream())) {
            
            socket.setSoTimeout(5_000); // 5s timeout
            
            // Create and send event
            CalendarEvent event = new CalendarEvent(
                new Date(), 
                "Secure Team Meeting"
            );
            oos.writeObject(event);
            System.out.println("📤 Event sent successfully");
        }
    }
}
