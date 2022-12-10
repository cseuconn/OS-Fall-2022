#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

void test() {
    
    clock_t start, end;
    start = clock();
    int max = 100000;

    for(int n = 100; n <= max; n *= 10) {
        int** x = malloc(sizeof(int*)*n);
        for(int i = 0; i < n; i++) {
            x[i] = malloc(sizeof(int));
        }
        for(int i = 0; i < n; i++) {
            free(x[i]);
        }
        free(x);
        end = clock();
        printf("for %d took %lf seconds using malloc\n", n, 
            ((double)(end-start)) / CLOCKS_PER_SEC);
    }

    start = clock();

    for(int n = 100; n <= max; n *= 10) {
        int** x = malloc(sizeof(int*)*n);
        for(int i = 0; i < n; i++) {
            x[i] = mymalloc(sizeof(int));
        }
        for(int i = 0; i < n; i++) {
            myfree(x[i]);
        }
        free(x);
        end = clock();
        printf("for %d took %lf seconds using mymalloc\n", n, ((double)(end-start)) / CLOCKS_PER_SEC);
    }
    
}


int main() {
    // printf("size of int: %lu\n", sizeof(int)); // 4 bytes
    // printf("size of void*: %lu\n", sizeof(void*)); // 8 bytes
    mymalloc_init();

    test();

    // destroy
    mymalloc_destroy();
}