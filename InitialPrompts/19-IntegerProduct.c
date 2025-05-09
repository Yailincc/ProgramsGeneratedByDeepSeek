#include <stdio.h>

long long calculate_product(int arr[], int size) {
    long long product = 1; // Initialize product to 1
    
    for (int i = 0; i < size; i++) {
        product *= arr[i];
    }
    
    return product;
}

int main() {
    int size;
    
    printf("Enter the size of the array: ");
    scanf("%d", &size);
    
    if (size <= 0) {
        printf("Array size must be a positive integer.\n");
        return 1;
    }
    
    int arr[size];
    
    printf("Enter %d integers:\n", size);
    for (int i = 0; i < size; i++) {
        scanf("%d", &arr[i]);
    }
    
    long long product = calculate_product(arr, size);
    
    printf("The product of all elements in the array is: %lld\n", product);
    
    return 0;
}
