#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#define MAX_ALLOWED_SIZE 100000  // Prevent excessive memory usage

// Safely computes the product with overflow checks
long long safe_product(int arr[], size_t size) {
    long long product = 1;

    for (size_t i = 0; i < size; i++) {
        // Check for potential overflow before multiplying
        if (arr[i] != 0) {
            if (llabs(product) > LLONG_MAX / llabs(arr[i])) {
                fprintf(stderr, "Error: Integer overflow detected\n");
                exit(EXIT_FAILURE);
            }
        }
        product *= arr[i];
    }

    return product;
}

// Secure input function for integers
int safe_input_int(const char *prompt, int *value) {
    char buffer[256];
    char *endptr;
    long temp;

    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;  // Input failure
    }

    errno = 0;
    temp = strtol(buffer, &endptr, 10);
    
    // Check for conversion errors
    if (endptr == buffer || *endptr != '\n' || errno == ERANGE ||
        temp < INT_MIN || temp > INT_MAX) {
        return 0;  // Invalid input
    }

    *value = (int)temp;
    return 1;
}

int main() {
    size_t size;
    int *arr = NULL;

    // Get array size with validation
    printf("Enter the size of the array (1-%zu): ", MAX_ALLOWED_SIZE);
    if (scanf("%zu", &size) != 1 || size == 0 || size > MAX_ALLOWED_SIZE) {
        fprintf(stderr, "Error: Invalid array size\n");
        return EXIT_FAILURE;
    }
    while (getchar() != '\n');  // Clear input buffer

    // Allocate memory securely
    arr = malloc(size * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    // Get array elements securely
    for (size_t i = 0; i < size; i++) {
        char prompt[50];
        snprintf(prompt, sizeof(prompt), "Enter element %zu: ", i + 1);
        
        if (!safe_input_int(prompt, &arr[i])) {
            fprintf(stderr, "Error: Invalid input for element %zu\n", i + 1);
            free(arr);
            return EXIT_FAILURE;
        }
    }

    // Calculate and print product
    long long product = safe_product(arr, size);
    printf("The product of all elements is: %lld\n", product);

    // Clean up
    free(arr);
    return EXIT_SUCCESS;
}
