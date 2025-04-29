#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Platform-specific headers */
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

/* Secure temporary file creation with automatic cleanup */
FILE* create_secure_tempfile(char **final_path) {
    FILE *fp = NULL;
    
#ifdef _WIN32
    /* Windows secure implementation */
    TCHAR temp_path[MAX_PATH];
    TCHAR temp_file[MAX_PATH];
    
    if (!GetTempPath(MAX_PATH, temp_path)) {
        fprintf(stderr, "GetTempPath failed (%lu)\n", GetLastError());
        return NULL;
    }
    
    if (!GetTempFileName(temp_path, TEXT("TMP"), 0, temp_file)) {
        fprintf(stderr, "GetTempFileName failed (%lu)\n", GetLastError());
        return NULL;
    }
    
    /* Convert to multibyte if needed */
    #ifdef UNICODE
    char narrow_path[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, temp_file, -1, narrow_path, MAX_PATH, NULL, NULL);
    *final_path = strdup(narrow_path);
    #else
    *final_path = strdup(temp_file);
    #endif
    
    fp = fopen(*final_path, "wb+TD");  // T=temp, D=delete on close
    if (!fp) {
        free(*final_path);
        *final_path = NULL;
    }
    
#else
    /* POSIX secure implementation */
    char template[] = "/tmp/securetemp-XXXXXX"; // Consider using $XDG_RUNTIME_DIR instead
    int fd = -1;
    
    // Create with restrictive permissions (rw-------)
    mode_t old_umask = umask(S_IRWXO | S_IRWXG);
    fd = mkstemp(template);
    umask(old_umask);
    
    if (fd == -1) {
        fprintf(stderr, "mkstemp failed: %s\n", strerror(errno));
        return NULL;
    }
    
    // Immediately unlink (file will be deleted when closed)
    if (unlink(template) == -1) {
        fprintf(stderr, "unlink failed: %s\n", strerror(errno));
        close(fd);
        return NULL;
    }
    
    *final_path = strdup(template);
    fp = fdopen(fd, "w+");
    if (!fp) {
        free(*final_path);
        *final_path = NULL;
        close(fd);
    }
#endif

    return fp;
}

/* Secure cleanup function */
void secure_cleanup(FILE **fp, char **path) {
    if (*fp) {
        // Securely erase contents before closing
        fflush(*fp);
        #ifndef _WIN32
        if (ftruncate(fileno(*fp), 0) == -1) {
            fprintf(stderr, "Warning: Failed to truncate temp file: %s\n", strerror(errno));
        }
        #endif
        fclose(*fp);
        *fp = NULL;
    }
    
    if (*path) {
        // Double-check deletion (Windows may need this)
        #ifdef _WIN32
        DeleteFileA(*path);
        #endif
        free(*path);
        *path = NULL;
    }
}

int main() {
    FILE *temp_file = NULL;
    char *temp_path = NULL;
    char buffer[256];
    
    /* 1. Create secure temp file */
    temp_file = create_secure_tempfile(&temp_path);
    if (!temp_file) {
        fprintf(stderr, "Failed to create secure temporary file\n");
        return EXIT_FAILURE;
    }
    
    printf("Created secure temp file (path: %s)\n", temp_path ? temp_path : "(anonymous)");
    
    /* 2. Write data securely */
    const char *sensitive_data = "This is confidential temporary data";
    size_t data_len = strlen(sensitive_data);
    
    if (fwrite(sensitive_data, 1, data_len, temp_file) != data_len) {
        fprintf(stderr, "Failed to write all data to temp file\n");
        secure_cleanup(&temp_file, &temp_path);
        return EXIT_FAILURE;
    }
    
    /* 3. Read back data */
    rewind(temp_file);
    if (!fgets(buffer, sizeof(buffer), temp_file)) {
        fprintf(stderr, "Failed to read from temp file\n");
        secure_cleanup(&temp_file, &temp_path);
        return EXIT_FAILURE;
    }
    
    printf("Read from temp file: %s\n", buffer);
    
    /* 4. Secure cleanup */
    secure_cleanup(&temp_file, &temp_path);
    
    /* 5. Verify cleanup */
    if (temp_path) {
        FILE *test = fopen(temp_path, "r");
        if (test) {
            fprintf(stderr, "SECURITY WARNING: Temp file still exists after cleanup!\n");
            fclose(test);
            remove(temp_path);
        }
    }
    
    return EXIT_SUCCESS;
}
