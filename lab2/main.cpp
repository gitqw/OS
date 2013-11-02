/*
	main.cpp
	Test client
	laboratory work #2
*/

#include "allocator.h"
#include <cstdio>

int main(char** args) {
	void* arr[5];			
	arr[0] = mem_alloc(20);
	arr[1] = mem_alloc(20);
	arr[2] = mem_alloc(20);
	arr[3] = mem_alloc(20);
	arr[4] = mem_alloc(30);
	void* virtual_page = mem_alloc(5000);
	mem_dump();
	mem_free(arr[0]);
	mem_free(arr[4]);
	mem_dump();
	mem_realloc(virtual_page, 10000);
	mem_realloc(arr[1], 40);
	mem_dump();
	getchar();
	return 0;
}