
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define TEMP_FILE_TEMPLATE "/tmp/sensitive_data_XXXXXX"
#define MAX_DATA_SIZE 4096

// Function to securely wipe memory
void secure_wipe(void *ptr, size_t len) {
    volatile char *p = ptr;
    while (len--) *p++ = 0;
}

// Function to create a secure temporary file
int create_secure_tempfile(char *template) {
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("Error creating temporary file");
        return -1;
    }

    // Set restrictive permissions (rw-------)
    if (fchmod(fd, S_IRUSR | S_IWUSR) == -1) {
        perror("Error setting file permissions");
        close(fd);
        return -1;
    }

    return fd;
}

// Function to securely write data to file
int secure_write_data(int fd, const char *data, size_t len) {
    if (len > MAX_DATA_SIZE) {
        fprintf(stderr, "Data size exceeds maximum allowed\n");
        return -1;
    }

    ssize_t written = write(fd, data, len);
    if (written == -1) {
        perror("Error writing to file");
        return -1;
    }

    if ((size_t)written != len) {
        fprintf(stderr, "Incomplete write operation\n");
        return -1;
    }

    // Ensure data is physically written to disk
    if (fsync(fd) == -1) {
        perror("Error syncing file");
        return -1;
    }

    return 0;
}

// Function to securely delete temporary file
int secure_delete_tempfile(const char *filename, int fd) {
    if (unlink(filename) == -1) {
        perror("Error deleting temporary file");
        return -1;
    }

    if (close(fd) == -1) {
        perror("Error closing file descriptor");
        return -1;
    }

    return 0;
}

int main() {
    char temp_filename[256] = TEMP_FILE_TEMPLATE;
    char sensitive_data[MAX_DATA_SIZE];
    int fd;

    printf("Secure Temporary File Handler\n");
    printf("-----------------------------\n");

    // Get sensitive data from user
    printf("Enter sensitive data (max %d chars): ", MAX_DATA_SIZE-1);
    if (fgets(sensitive_data, MAX_DATA_SIZE, stdin) == NULL) {
        fprintf(stderr, "Error reading input\n");
        return EXIT_FAILURE;
    }

    // Remove newline if present
    size_t data_len = strlen(sensitive_data);
    if (data_len > 0 && sensitive_data[data_len-1] == '\n') {
        sensitive_data[data_len-1] = '\0';
        data_len--;
    }

    // Create secure temporary file
    fd = create_secure_tempfile(temp_filename);
    if (fd == -1) {
        return EXIT_FAILURE;
    }

    printf("Created secure temporary file: %s\n", temp_filename);

    // Write data to file
    if (secure_write_data(fd, sensitive_data, data_len) == -1) {
        secure_delete_tempfile(temp_filename, fd);
        return EXIT_FAILURE;
    }

    printf("Data written securely to temporary file\n");

    // Securely wipe the sensitive data from memory
    secure_wipe(sensitive_data, MAX_DATA_SIZE);

    // Demonstrate reading back (in a real application, you'd process the data here)
    printf("\nContents of temporary file (for demonstration):\n");
    
    // Seek to beginning of file
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Error seeking in file");
        secure_delete_tempfile(temp_filename, fd);
        return EXIT_FAILURE;
    }

    char buffer[MAX_DATA_SIZE];
    ssize_t bytes_read = read(fd, buffer, MAX_DATA_SIZE-1);
    if (bytes_read == -1) {
        perror("Error reading from file");
        secure_delete_tempfile(temp_filename, fd);
        return EXIT_FAILURE;
    }
    buffer[bytes_read] = '\0';
    printf("%s\n", buffer);

    // Securely delete the temporary file when done
    if (secure_delete_tempfile(temp_filename, fd) {
        return EXIT_FAILURE;
    }

    printf("\nTemporary file securely deleted\n");

    return EXIT_SUCCESS;
}
