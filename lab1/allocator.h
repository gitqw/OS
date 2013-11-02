#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <cstddef>

// header of every used memory block
typedef struct Header {
    struct Header* next;		// next header
    size_t size;				// size of used space (without the header)
} Header;

// pointers to the start and end of the list,
// which connects used memory blocks' headers
extern Header* head;
extern Header* tail;

const size_t default_buffer_size = 0x100000;	// 1Mb up to 2^32-1 ~ 4Gb
const size_t header_size = sizeof(Header);
const size_t min_size = sizeof(void*);

// allocates size bytes of memory, returns pointer to the
// beginning of a block or NULL, if impossible to allocate
void* mem_alloc(size_t size);				

// copies data from addr to a new place if needed,
// returns pointer to the beginning of a block with new size
// or NULL, if impossible to reallocate
void* mem_realloc(void* addr, size_t size);

// releases memory by addr
void mem_free(void* addr);

// shows current status of memory
void mem_dump();

// aligns size by the measure of sizeof(void*) bytes
// e.g. allign_size(7) = 8, when sizeof(void*) = 4
size_t align_size(size_t size);

#endif