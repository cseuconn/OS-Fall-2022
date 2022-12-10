#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mymalloc.h"


void mymalloc_init() {
    // set up the bins for the memory manager
    memoryManager.miscSzBin.size = -1;
    memoryManager.miscSzBin.head = NULL;
    memoryManager.miscSzBin.tail = NULL;
    // printf("bin[miscSz] = %d\n", memoryManager.miscSzBin.size);

    int nBins;
    for(nBins = 0; (int)(getpagesize() / pow(2, nBins) - sizeof(int)*2) > (int)sizeof(void*)*2; nBins++) {
        // printf("%d > %d = %d\n", (int)(getpagesize() / pow(2, nBins) - sizeof(int)*2), (int)sizeof(void*), (int)(getpagesize() / pow(2, nBins) - sizeof(int)*2) > (int)sizeof(void*));
    }
    // printf("nBins: %d\n", nBins);
    memoryManager.bins = malloc(sizeof(bin) * nBins); // use actual malloc to keep track of bins, which we can have different numbers of
    memoryManager.nBins = nBins;
    // set the size for the highest bin to -1 and let it keep track of blocks bigger than a page
    // memoryManager.bins[nBins-1].size = -1;
    for(int i = nBins-1; i >= 0; i--) {
        // printf("nBins: %d\n", nBins);
        int sz = getpagesize() / pow(2, i) - sizeof(int)*2;
        memoryManager.bins[i].size = sz;
        memoryManager.bins[i].head = NULL;
        memoryManager.bins[i].tail = NULL;
        // printf("bin[%d] = %d\n", nBins-i, sz);
    }

    // for(int i = 0; i < nBins; i++) {
    //     printf("bin[%d] = %d\n", i, memoryManager.bins[i].size);
    // }
}

void mymalloc_destroy() {
    free(memoryManager.bins);
}

void* get_prev_chunk(void* chunk) {
    return *(void**)(chunk+sizeof(int));
}

void* get_next_chunk(void* chunk) {
    return *(void**)(chunk+sizeof(int)+sizeof(void*));
}

void set_prev_chunk(void* chunk, void* prev_chunk) {
    *(void**)(chunk+sizeof(int)) = prev_chunk;
}

void set_next_chunk(void* chunk, void* next_chunk) {
    *(void**)(chunk+sizeof(int)+sizeof(void*)) = next_chunk;
}

void* mem_of_chunk(void* chunk) {
    return (chunk+sizeof(int));
}

void mark_chunk_valid(void* chunk) {
    // multiple the size by -1 to make it valid  (valid if negative)
    *(int*)chunk *= -1;
}

void mark_chunk_invalid(void* chunk) {
    // multiple the size by -1 to make it invalid (valid if negative)
    *(int*)chunk *= -1;
}

int get_chunk_size(void* chunk) {
    return *(int*)(chunk);
}

int get_size_from_payload(void* ptr) {
    return *(int*)(ptr-sizeof(int));
}

int get_total_chunk_size(void* chunk) {
    return get_chunk_size(chunk) + sizeof(int)*2;
}

int get_split_size(void* chunk) {
    return get_total_chunk_size(chunk) / 2 - sizeof(int) * 2;
}

int get_payload_size(int size) {
    return size - sizeof(int) * 2;
}

void* get_chunk_from_payload(void* ptr) {
    return ptr - sizeof(int);
}

void print_bins() {
    void* chunk;
    for(int i = 0; i < memoryManager.nBins; i++) {
        // printf("bin[%d] (%d)\n", i, memoryManager.bins[i].size);
        chunk = memoryManager.bins[i].head;
        while(chunk) {
            // printf("\t%p (size = %d)\n", chunk, get_chunk_size(chunk));
            chunk = get_next_chunk(chunk);
        }
    }
}

bin* find_bin(int size) {
    if(size > memoryManager.bins[0].size)
        return &memoryManager.miscSzBin;
    int i;
    for(i = memoryManager.nBins-1; i >= 0 && memoryManager.bins[i].size < size; i--) {
        // printf("size of bin[%d] (%d) < %d = %d\n", i, memoryManager.bins[i].size, size, memoryManager.bins[i].size < size);
    }
    // printf("\n");
    return &memoryManager.bins[i];
}

void* create_chunk(void* ptr, int size) {
    // set the first 4 bytes of the chunk to its size
    *(int*)ptr = size;
    // set the last 4 bytes of the chunk to its size
    *(int*)(ptr+size+sizeof(int)) = size;
    
    return ptr;
}

void add_chunk(void* chunk, int size) {
    // find the bin
    bin* b = find_bin(size);
    // printf("adding a chunk to bin of size %d\n", b->size);
    
    // set the pointer to the previous chunk in the bin
    set_prev_chunk(chunk, b->tail);
    // set the pointer to the next chunk in the bin to NULL
    set_next_chunk(chunk, NULL);

    if(b->tail) {
        set_next_chunk(b->tail, chunk);
        set_prev_chunk(chunk, b->tail);
    }
    else {
        b->head = chunk;
    }
     // update the tail to point to this chunk
    b->tail = chunk;
}

// only use to allocate memory that is too big for the largest bin size
void* alloc_new_mem(int size) {
    int nPages = (size+1) / getpagesize();
    // allocate 4 new pages using mmap
    void* p = mmap(0, nPages * getpagesize(), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    // printf("allocated pointer: %p\n", p);
    // sort them into chunks/bins
    create_chunk(p, get_payload_size(size));
    add_chunk(p, get_payload_size(size));

    return p;
}

void alloc_four_pages() {
    // allocate 4 new pages using mmap
    void* p = mmap(0, getpagesize()*4, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    // printf("allocated pointer: %p", p);
    if(p > memoryManager.highestAddress)
        memoryManager.highestAddress = p;
    // sort them into chunks/bins
    for(int i = 0; i < 4; i++) {
        create_chunk(p + i * getpagesize(), get_payload_size(getpagesize()));
        add_chunk(p + i * getpagesize(), get_payload_size(getpagesize()));
    }
}

// removes a chunk from the head of the bin
void* get_chunk(bin* b) {
    void* ptr = NULL;
    // try to find a chunk and remove it from the bin
    if(b->head != NULL && b->tail != NULL) {
        // printf("trying to remove a chunk...\n");
        ptr = b->head;
        void* next_chunk = get_next_chunk(b->head);
        // printf("next chunk: %p\n", next_chunk);
        if(next_chunk != NULL) {
            b->head = next_chunk;
            set_prev_chunk(b->head, NULL);
        }
        else {
            b->head = NULL;
            b->tail = NULL;
        }
        // mark chunk as in use
        mark_chunk_valid(ptr);
        // printf("successfully found a chunk from bin of size: %d\n", b->size);
    }

    return ptr;
}

void remove_chunk(void* chunk, int size) {
    bin* b = find_bin(size);
    void* cur_chunk = b->head;
    while(cur_chunk) {
        if(cur_chunk == chunk) {
            void* prev = get_prev_chunk(cur_chunk);
            void* next = get_next_chunk(cur_chunk);
            if(prev) {
                set_next_chunk(prev, next);
            }
            else {
                b->head = next;
            }
            if(next) {
                set_prev_chunk(next, prev);
            }
            else {
                b->tail = prev;
            }
        }
        cur_chunk = get_next_chunk(cur_chunk);
    }
}

void* split_chunk(void* chunk, int goalSz) {
    // need to mark the original chunk as invalid, which was marked valid when removed from its bin
    mark_chunk_invalid(chunk);
    void* left = chunk;
    void* right;
    int sz = get_chunk_size(left);
    int splitSz = 0;
    while(sz != goalSz) {
        splitSz = get_split_size(left);
        // printf("splitSz: %d\n", splitSz);
        right = create_chunk(left+get_total_chunk_size(left)/2, splitSz);
        left = create_chunk(left, splitSz);
        // add_chunk(left, splitSz);
        add_chunk(right, splitSz);
        sz = get_chunk_size(left);
    }
    // mark chunk as valid
    // printf("successfully split chunk into size [%d] = %d\n", goalSz, get_chunk_size(left));
    mark_chunk_valid(left);
    // printf("size of chunk from split chunk: %d\n", get_chunk_size(left));
    return left;
}

void* find_chunk(int size) {
    bin* b = find_bin(size);
    // printf("found chunk of size (%d) would fit best for (%d)\n", b->size, size);
    void* chunk = get_chunk(b);
    void* chunk_to_split = chunk;
    for(int sz = size * 2; chunk == NULL && sz < memoryManager.bins[0].size; sz *= 2) {
        // find a bigger bin with a chunk and split it to form a bin of this size
        // split a chunk of a bigger bin to make a bin of this size
        chunk_to_split = get_chunk(find_bin(sz));
        if(chunk_to_split != NULL) {
            chunk = split_chunk(chunk_to_split, b->size);
        }
    }
    return chunk;
}

void* mymalloc(int size) {
    // find the smallest bin that will fit size
    // if there aren't any bins that are small enough, break a larger one up
    // reconfigure the bins

    void* chunk;
    // if the size requested is bigger than the biggest bin, allocate that memory directly
    if(size > memoryManager.bins[0].size) {
        chunk = get_chunk(&memoryManager.miscSzBin);
        // printf("got a chunk from the big bins\n");
        if(chunk == NULL) {
            chunk = alloc_new_mem(size);
            // printf("had to allocate big enough memory\n");
        }
    }
    else {
        // printf("trying to allocate a chunk...\n");
        chunk = find_chunk(size);
        // need to allocate new memory if couldn't find a chunk
        if(chunk == NULL) {
            // printf("allocated four pages bc not enough memory existed\n");
            alloc_four_pages();
            chunk = find_chunk(size);
        }

    }
    // print_bins();
    return mem_of_chunk(chunk);
}

void* find_buddy(void* chunk) {
    int sz = get_chunk_size(chunk);
    // printf("chunk is (%p): %d\n", chunk, (int)chunk % (sz * 2) == 0);
    // printf("chunk size: %d\n", get_total_chunk_size(chunk));
    if((int)(memoryManager.highestAddress - chunk) % (sz * 2) == 0) {
        // left half
        return chunk + get_total_chunk_size(chunk);
    }
    else {
        // right half
        return chunk - get_total_chunk_size(chunk);
    }
}

void merge_chunks(void* chunk) {
    void* buddy = find_buddy(chunk);
    // printf("buddy for %p is %p\n", chunk, buddy);

    void* left;
    void* new_chunk;
    // printf("chunk size buddy (%d); chunk size chunk: (%d)\n", get_chunk_size(buddy), get_chunk_size(chunk));
    while(buddy && get_chunk_size(buddy) == get_chunk_size(chunk)) {
        // printf("going to merge a chunk\n");
        remove_chunk(buddy, get_chunk_size(buddy));
        left = (chunk < buddy) ? chunk : buddy;
        new_chunk = create_chunk(left, get_chunk_size(chunk) * 2);
        add_chunk(new_chunk, get_chunk_size(chunk));
        buddy = find_buddy(new_chunk);
        // printf("buddy for %p is %p\n", chunk, buddy);
    }
}

// need to give memory back when possible
void myfree(void* ptr) {
    // set the chunk to invalid
    // add it to the bin corresponding to its size
    // merge the chunk with a chunk next to it if needed
    void* chunk = get_chunk_from_payload(ptr);
    // printf("freeing chunk of size (%d)\n", get_chunk_size(chunk));
    mark_chunk_invalid(chunk);
    // printf("freeing chunk of size (%d)\n", get_chunk_size(chunk));
    // maybe do merge_chunks instead of add chunk
    merge_chunks(chunk);
    // add_chunk(chunk, get_chunk_size(chunk));

    // print_bins();
}