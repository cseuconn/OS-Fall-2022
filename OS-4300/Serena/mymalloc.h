#ifndef _MY_MALLOC_H
#define _MY_MALLOC_H

#include <unistd.h>


typedef struct bin {
    int size;
    void* head;
    void* tail;
} bin;

typedef struct memMan {
    void* highestAddress;
    void* lowestAddress;

    bin miscSzBin;

    bin* bins;
    int nBins;
} memMan;

memMan memoryManager;


void mymalloc_init();

void mymalloc_destroy();

void* alloc_new_mem(int size);

void alloc_four_pages();

void* get_prev_chunk(void* chunk);

void* get_next_chunk(void* chunk);

void set_prev_chunk(void* chunk, void* prev_chunk);

void set_next_chunk(void* chunk, void* next_chunk);

void* mem_of_chunk(void* chunk);

void make_chunk_valid(void* chunk);

int get_chunk_size(void* chunk);

int get_size_from_payload(void* ptr);

void* mymalloc(int size);

void myfree(void* ptr);

#endif