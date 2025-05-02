import javax.net.ssl.*;
import java.io.*;
import java.net.*;
import java.security.*;
import java.util.Date;

public class SecureCalendarClient {
    private static final String TRUSTSTORE_PATH = "client_truststore.p12";
    private static final String TRUSTSTORE_PASSWORD = "Truststore@Pass123";
    private static final String HOSTNAME = "localhost";

    public static void main(String[] args) {
        System.setProperty("javax.net.ssl.trustStore", TRUSTSTORE_PATH);
        System.setProperty("javax.net.ssl.trustStorePassword", TRUSTSTORE_PASSWORD);
        System.setProperty("jdk.tls.client.protocols", "TLSv1.3");

        try {
            SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
            try (SSLSocket socket = (SSLSocket) factory.createSocket(HOSTNAME, PORT);
                 ObjectOutputStream oos = new ObjectOutputStream(
                     new BufferedOutputStream(socket.getOutputStream()))) {
                
                socket.setSoTimeout(5000);
                socket.setEnabledCipherSuites(new String[] {
                    "TLS_AES_256_GCM_SHA384",
                    "TLS_CHACHA20_POLY1305_SHA256"
                });
                
                CalendarDate dateEntry = new CalendarDate(
                    new Date(),
                    "Quarterly Financial Review"
                );
                
                oos.writeObject(dateEntry);
                oos.flush();
                System.out.println("✉️ Successfully sent calendar entry");
            }
        } catch (Exception e) {
            System.err.println("Client error: " + e.getMessage());
        }
    }
}

class SecureObjectInputStream extends ObjectInputStream {
    private static final Set<String> ALLOWED_CLASSES = Set.of(
        "CalendarDate",
        "java.util.Date",
        "java.lang.String"
    );

    public SecureObjectInputStream(InputStream in) throws IOException {
        super(in);
    }

    @Override
    protected Class<?> resolveClass(ObjectStreamClass desc) 
        throws IOException, ClassNotFoundException {
        if (!ALLOWED_CLASSES.contains(desc.getName())) {
            throw new InvalidClassException("Unauthorized deserialization attempt", desc.getName());
        }
        return super.resolveClass(desc);
    }
}

class LimitedInputStream extends FilterInputStream {
    private final long maxSize;
    private long bytesRead;

    public LimitedInputStream(InputStream in, long maxSize) {
        super(in);
        this.maxSize = maxSize;
    }

    @Override
    public int read() throws IOException {
        int data = super.read();
        if (data != -1 && ++bytesRead > maxSize) {
            throw new IOException("Input size exceeded limit of " + maxSize + " bytes");
        }
        return data;
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        int read = super.read(b, off, len);
        if (read != -1) {
            bytesRead += read;
            if (bytesRead > maxSize) {
                throw new IOException("Input size exceeded limit of " + maxSize + " bytes");
            }
        }
        return read;
    }
}
