
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_ALLOCATIONS 100  // Prevent excessive memory allocation

typedef struct {
    void *memory;
    size_t size;
} Allocation;

Allocation allocations[MAX_ALLOCATIONS];
int allocation_count = 0;

void display_menu() {
    printf("\nMemory Allocation Program\n");
    printf("1. Allocate memory\n");
    printf("2. Free memory\n");
    printf("3. View allocations\n");
    printf("4. Write to allocation\n");
    printf("5. Read from allocation\n");
    printf("6. Exit\n");
    printf("Enter your choice: ");
}

void* safe_malloc(size_t size) {
    if (size == 0 || size > SIZE_MAX / 2) {
        fprintf(stderr, "Error: Invalid size requested\n");
        return NULL;
    }

    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    // Initialize memory to zero for security
    memset(ptr, 0, size);
    return ptr;
}

void allocate_memory() {
    if (allocation_count >= MAX_ALLOCATIONS) {
        printf("Error: Maximum number of allocations reached\n");
        return;
    }

    size_t size;
    printf("Enter size in bytes to allocate: ");
    if (scanf("%zu", &size) != 1 || size == 0) {
        printf("Error: Invalid size\n");
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    void *mem = safe_malloc(size);
    if (mem == NULL) {
        return;
    }

    allocations[allocation_count].memory = mem;
    allocations[allocation_count].size = size;
    allocation_count++;

    printf("Allocated %zu bytes at address %p\n", size, mem);
}

void free_memory() {
    if (allocation_count == 0) {
        printf("No allocations to free\n");
        return;
    }

    int index;
    printf("Enter allocation index to free (0-%d): ", allocation_count - 1);
    if (scanf("%d", &index) != 1 || index < 0 || index >= allocation_count) {
        printf("Error: Invalid index\n");
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    free(allocations[index].memory);
    allocations[index].memory = NULL;

    // Shift remaining allocations down
    for (int i = index; i < allocation_count - 1; i++) {
        allocations[i] = allocations[i + 1];
    }
    allocation_count--;

    printf("Memory freed successfully\n");
}

void view_allocations() {
    if (allocation_count == 0) {
        printf("No current allocations\n");
        return;
    }

    printf("\nCurrent Allocations:\n");
    for (int i = 0; i < allocation_count; i++) {
        printf("%d: Address: %p, Size: %zu bytes\n", 
               i, allocations[i].memory, allocations[i].size);
    }
}

void write_to_allocation() {
    if (allocation_count == 0) {
        printf("No allocations available\n");
        return;
    }

    int index;
    printf("Enter allocation index (0-%d): ", allocation_count - 1);
    if (scanf("%d", &index) != 1 || index < 0 || index >= allocation_count) {
        printf("Error: Invalid index\n");
        while (getchar() != '\n');
        return;
    }

    size_t offset;
    printf("Enter offset (0-%zu): ", allocations[index].size - 1);
    if (scanf("%zu", &offset) != 1 || offset >= allocations[index].size) {
        printf("Error: Invalid offset\n");
        while (getchar() != '\n');
        return;
    }

    int value;
    printf("Enter value to write (0-255): ");
    if (scanf("%d", &value) != 1 || value < 0 || value > 255) {
        printf("Error: Invalid value\n");
        while (getchar() != '\n');
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
    printf("Enter allocation index (0-%d): ", allocation_count - 1);
    if (scanf("%d", &index) != 1 || index < 0 || index >= allocation_count) {
        printf("Error: Invalid index\n");
        while (getchar() != '\n');
        return;
    }

    size_t offset;
    printf("Enter offset (0-%zu): ", allocations[index].size - 1);
    if (scanf("%zu", &offset) != 1 || offset >= allocations[index].size) {
        printf("Error: Invalid offset\n");
        while (getchar() != '\n');
        return;
    }

    unsigned char value = *((unsigned char*)allocations[index].memory + offset);
    printf("Value at offset %zu: %d\n", offset, value);
}

int main() {
    int choice;

    while (1) {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        switch (choice) {
            case 1:
                allocate_memory();
                break;
            case 2:
                free_memory();
                break;
            case 3:
                view_allocations();
                break;
            case 4:
                write_to_allocation();
                break;
            case 5:
                read_from_allocation();
                break;
            case 6:
                // Free all remaining allocations before exit
                for (int i = 0; i < allocation_count; i++) {
                    free(allocations[i].memory);
                }
                printf("Program exiting. All memory freed.\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }

        while (getchar() != '\n'); // Clear input buffer after each operation
    }

    return 0;
}
