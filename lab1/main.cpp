/*
	Test client for memory allocator
*/

#include <cstdio>
#include "allocator.h"

int main(void) {
	void* arr[5];			// pointers to memory blocks
	for (int i = 0; i < 5; i++) {
		arr[i] = mem_alloc(8);
	}
	mem_dump();
	mem_free(arr[1]);
	mem_free(arr[2]);
	mem_free(arr[3]);
	mem_dump();
	mem_realloc(arr[0], 280);
	mem_dump();
	getchar();
    return 0;
}