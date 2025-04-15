#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For unlink() on POSIX systems

int main() {
    FILE *temp_file;
    char buffer[100];
    
    // Create a temporary file
    temp_file = tmpfile();
    if (temp_file == NULL) {
        perror("Error creating temporary file");
        exit(EXIT_FAILURE);
    }
    
    printf("Temporary file created successfully\n");
    
    // Write some data to the temporary file
    const char *data = "This is temporary data that will be deleted automatically";
    fprintf(temp_file, "%s\n", data);
    printf("Data written to temporary file: %s\n", data);
    
    // Rewind to read what we wrote
    rewind(temp_file);
    
    // Read data from the temporary file
    if (fgets(buffer, sizeof(buffer), temp_file) {
        printf("Data read from temporary file: %s", buffer);
    } else {
        perror("Error reading from temporary file");
    }
    
    // The file is automatically deleted when closed or program exits
    fclose(temp_file);
    
    // Alternative method using tmpnam (less secure)
    /*
    char temp_name[L_tmpnam];
    tmpnam(temp_name);
    printf("Temporary filename: %s\n", temp_name);
    temp_file = fopen(temp_name, "w+");
    // ... use the file ...
    unlink(temp_name); // Delete the file
    fclose(temp_file);
    */
    
    return 0;
}
