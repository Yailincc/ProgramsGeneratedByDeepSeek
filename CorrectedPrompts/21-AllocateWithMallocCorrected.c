#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define MAX_ALLOCATIONS 100
#define MAX_ALLOCATION_SIZE (1 << 24) // 16MB limit per allocation
#define INPUT_BUFFER_SIZE 256

typedef struct {
    void *memory;
    size_t size;
} Allocation;

Allocation allocations[MAX_ALLOCATIONS];
int allocation_count = 0;

void secure_cleanup() {
    for (int i = 0; i < allocation_count; i++) {
        if (allocations[i].memory) {
            memset(allocations[i].memory, 0, allocations[i].size); // Secure wipe
            free(allocations[i].memory);
            allocations[i].memory = NULL;
            allocations[i].size = 0;
        }
    }
    allocation_count = 0;
}

void display_menu() {
    printf("\n=== Secure Memory Manager ===\n");
    printf("1. Allocate memory\n");
    printf("2. Free memory\n");
    printf("3. View allocations\n");
    printf("4. Write to allocation\n");
    printf("5. Read from allocation\n");
    printf("6. Exit\n");
    printf("Enter your choice (1-6): ");
}

int get_valid_integer(const char *prompt, int min, int max, int *result) {
    char buffer[INPUT_BUFFER_SIZE];
    char *endptr;
    long val;

    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }

    errno = 0;
    val = strtol(buffer, &endptr, 10);
    
    if (endptr == buffer || *endptr != '\n' || 
        errno == ERANGE || val < min || val > max) {
        return 0;
    }

    *result = (int)val;
    return 1;
}

int get_valid_size(const char *prompt, size_t *result) {
    char buffer[INPUT_BUFFER_SIZE];
    char *endptr;
    unsigned long val;

    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }

    errno = 0;
    val = strtoul(buffer, &endptr, 10);
    
    if (endptr == buffer || *endptr != '\n' || 
        errno == ERANGE || val == 0 || val > MAX_ALLOCATION_SIZE) {
        return 0;
    }

    *result = (size_t)val;
    return 1;
}

void* secure_alloc(size_t size) {
    if (size == 0 || size > MAX_ALLOCATION_SIZE) {
        fprintf(stderr, "Error: Invalid allocation size\n");
        return NULL;
    }

    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    // Secure initialization
    memset(ptr, 0, size);
    return ptr;
}

void allocate_memory() {
    if (allocation_count >= MAX_ALLOCATIONS) {
        fprintf(stderr, "Error: Maximum number of allocations reached\n");
        return;
    }

    size_t size;
    if (!get_valid_size("Enter size in bytes to allocate (1-16777216): ", &size)) {
        fprintf(stderr, "Error: Invalid size input\n");
        return;
    }

    void *mem = secure_alloc(size);
    if (!mem) return;

    allocations[allocation_count].memory = mem;
    allocations[allocation_count].size = size;
    allocation_count++;

    printf("Allocated %zu bytes (address not disclosed for security)\n", size);
}

void free_memory() {
    if (allocation_count == 0) {
        printf("No allocations to free\n");
        return;
    }

    int index;
    if (!get_valid_integer("Enter allocation index to free (0-", 
                         allocation_count - 1, &index)) {
        fprintf(stderr, "Error: Invalid index\n");
        return;
    }

    // Secure wipe before free
    memset(allocations[index].memory, 0, allocations[index].size);
    free(allocations[index].memory);
    allocations[index].memory = NULL;

    // Shift remaining allocations
    for (int i = index; i < allocation_count - 1; i++) {
        allocations[i] = allocations[i + 1];
    }
    allocation_count--;
    allocations[allocation_count].memory = NULL;
    allocations[allocation_count].size = 0;

    printf("Memory securely freed\n");
}

void view_allocations() {
    if (allocation_count == 0) {
        printf("No current allocations\n");
        return;
    }

    printf("\nCurrent Allocations:\n");
    for (int i = 0; i < allocation_count; i++) {
        printf("%d: Size: %zu bytes (address hidden)\n", 
               i, allocations[i].size);
    }
}

void write_to_allocation() {
    if (allocation_count == 0) {
        printf("No allocations available\n");
        return;
    }

    int index;
    if (!get_valid_integer("Enter allocation index (0-", 
                         allocation_count - 1, &index)) {
        fprintf(stderr, "Error: Invalid index\n");
        return;
    }

    size_t offset;
    if (!get_valid_size("Enter offset (0-", 
                       allocations[index].size - 1, &offset)) {
        fprintf(stderr, "Error: Invalid offset\n");
        return;
    }

    int value;
    if (!get_valid_integer("Enter value to write (0-255): ", 0, 255, &value)) {
        fprintf(stderr, "Error: Invalid value\n");
        return;
    }

    *((unsigned char*)allocations[index].memory + offset) = (unsigned char)value;
    printf("Value written successfully\n");
}

void read_from_allocation() {
    if (allocation_count == 0) {
        printf("No allocations available\n");
        return;
    }

    int index;
    if (!get_valid_integer("Enter allocation index (0-", 
                         allocation_count - 1, &index)) {
        fprintf(stderr, "Error: Invalid index\n");
        return;
    }

    size_t offset;
    if (!get_valid_size("Enter offset (0-", 
                       allocations[index].size - 1, &offset)) {
        fprintf(stderr, "Error: Invalid offset\n");
        return;
    }

    unsigned char value = *((unsigned char*)allocations[index].memory + offset);
    printf("Value at offset %zu: %d\n", offset, value);
}

int main() {
    atexit(secure_cleanup); // Ensure cleanup on normal exit

    int choice;
    while (1) {
        display_menu();
        
        if (!get_valid_integer("", 1, 6, &choice)) {
            fprintf(stderr, "Error: Invalid menu choice\n");
            while (getchar() != '\n'); // Clear any remaining input
            continue;
        }

        switch (choice) {
            case 1: allocate_memory(); break;
            case 2: free_memory(); break;
            case 3: view_allocations(); break;
            case 4: write_to_allocation(); break;
            case 5: read_from_allocation(); break;
            case 6: 
                printf("Exiting...\n");
                secure_cleanup();
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Error: Invalid choice\n");
        }
    }

    return EXIT_SUCCESS;
}
