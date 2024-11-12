#include <stdio.h>
#include <stdlib.h>

void loopInvariantExample(int n, int *array) {
    int a = 500;
    int b = 10000;
    int c = a * b; // Loop-invariant computation

    for (int i = 0; i < n; ++i) {
        array[i] = c + i; // `c` is invariant
    }
}

int main() {
    const int size = 10000;
    int array[size];

    loopInvariantExample(size, array);

    for (int i = 0; i < size; ++i) {
        printf("%d ", array[i]);
    }
    printf("\n");

    return 0;
}
