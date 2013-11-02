#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allocator.h"

Header* head = NULL;
Header* tail = NULL;

void* mem_alloc(size_t size) {
	// size needed in memory pool
    const size_t real_size = align_size(size) + header_size;
	if (default_buffer_size < 20) {
		return NULL;
	}
    if (head == NULL) {			// no list created yet
        head = (Header*)malloc(default_buffer_size);
        if (head == NULL) {
            return NULL;
        } else {
			// add tail and head to the list
			tail = (Header*)((size_t)head + default_buffer_size - header_size);
			tail->size = header_size;
			tail->next = NULL;
            head->next = tail;
            head->size = header_size;
        }
    } 

	// add new node to the list
    for (Header* cur = head; cur < tail; cur = cur->next) {
		Header* free_start = (Header*)((size_t)cur + cur->size);
		Header* free_end = cur->next;

		// space between two adjcacent nodes in the list
        size_t allowed_size = (char*)free_end - (char*)free_start;
		if (allowed_size >= real_size) {			// if there are enough space available
			Header* new_block = free_start;
			new_block->next = cur->next;
			new_block->size = real_size;
			cur->next = new_block;
			if (new_block->next == tail) {
			}
			// +1 adds header size
			return (void*) (new_block + 1);
		}
    }
	return NULL;
}

void mem_free(void* addr) {
	Header* block_start = (Header*)addr - 1;
	Header* cur;
	for (cur = head; cur->next && cur->next != block_start; cur = cur->next) { }
	// delete node from the list, if it exist
	if (cur->next) cur->next = block_start->next;
}

void* mem_realloc (void* addr, size_t size) {
	if (addr == NULL) return mem_alloc(size);		// due to assignment specifications

	size_t alligned_size = align_size(size);	
	Header* block_start = (Header*)addr - 1;		 // header of the block to be reallocated

	// size of a new block
	size_t data_size = block_start->size < alligned_size + header_size ? 
						block_start->size : alligned_size + header_size;

	Header* free_start = (Header*)((size_t)block_start + data_size);
	Header* free_end = block_start->next;

	// space between two adjacent nodes
    size_t allowed_size = (char*)free_end - (char*)free_start;

	// enough space available, so no reallocation needed
	if (allowed_size + block_start->size - header_size >= alligned_size) {
		block_start->size = alligned_size + header_size;
		return addr;
	} else {	// reallocate block of memory to a new place
		void* result = mem_alloc(size);			// new address
		if (result) {
			memcpy(result, addr, size);			// copy all data to a new place
			mem_free(addr);						// free unused memory
			return result;
		}
		else return NULL;
	}

}

void mem_dump() {
	if (head == NULL) {					// no list created
		printf("Memory is free.\n\n");
		return;
	}
	int sum_size = 0;					// all memory size, to detect memory leaks, if any

	// go through the list
	for (Header* cur = head; cur != NULL && cur->next!=NULL; cur = cur->next) {
		Header* free_start = (Header*)((size_t)cur + cur->size);
		Header* free_end = cur->next;
        size_t allowed_size = (char*)free_end - (char*)free_start;
		printf("Size: %10d Status: full.\n", cur->size);
		sum_size += cur->size;
		if (allowed_size > 0) {
			printf("Size: %10d Status: empty.\n", allowed_size);
			sum_size += allowed_size;
		}
	}
	sum_size += header_size;
	printf("Size: %10d Status: full.\n", header_size);
	printf("Full memory size: %6d\n\n", sum_size);
}

size_t align_size(size_t size) {
    return size + (min_size - size % min_size) % min_size;
}
