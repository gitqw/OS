/*
	allocator.h
	laboratory work #2
*/

#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <cstddef>

// description of a block page (page devided into blocks)
typedef struct BlockHeader {					// header of a free block
	BlockHeader* nextHeader;
} BlockHeader;

typedef struct BlockPageHeader {
    BlockHeader* nextFreeBlock;
	size_t block_size;							// size of a free block
} BlockPageHeader;

// struct for list items in each class
typedef struct ClassItem {
	ClassItem* next;
	BlockPageHeader* firstBlockHeader;
} ClassItem;

// struct for a block class
typedef struct BlockClassesNode {				// node for list of classes of the blocks
	size_t block_size;							// size of a block of the class
	BlockClassesNode* next;
	ClassItem* firstItem;
} BlockClassesNode;


// page type
typedef struct Page {
} Page;

const size_t default_buffer_size = 0x8000;		// 0x20000000 = 0.5 Gb  0x64000 - 100 pages

// size of a page header (page must be devided into blocks to possess header)
const size_t pageHeader_size = sizeof(BlockPageHeader);
const size_t page_size = 0x1000;				// 4096 bytes
const size_t page_quant = default_buffer_size / page_size;
extern void* memory_start;
extern Page* pages[page_quant];						// pointers to each page
extern bool isMalloced;								// whether memory pool for the allocator is created
extern BlockClassesNode* firstClass;

// allocates block of memory of "size" size
void* mem_alloc(size_t size);

// reallocate block of memory by address addr to "size" size
void* mem_realloc(void* addr, size_t size);

// deallocate block of memory by addr
void mem_free(void* addr);

// show current memory status
void mem_dump();

// aligns size by the measure of sizeof(void*) bytes
size_t allign_size(size_t size);

// returns the first index of page array, which 
// starts free sequence of pagesNeeded number of pages
int findPageSequence(size_t pagesNeeded);		

// makes new page a block page of block_size blocks
void* createBlockPage(size_t block_size);

// deletes one block from its page by the addr adress
void deleteBlock(BlockPageHeader* pageHeader, void* addr);
#endif