/*
	allocator.cpp
	laboratory work #2
*/

#include "allocator.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

BlockClassesNode* firstClass = NULL;
bool isMalloced = false;
Page* pages[page_quant];
void* memory_start;

void* mem_alloc(size_t size) {
	size_t real_size = allign_size(size);
	if (!isMalloced) {
		isMalloced = true;
		memory_start = (Page*)malloc(default_buffer_size);
		if (memory_start == NULL) return NULL;					// can't allocate memory
	}

	if (real_size <= page_size/2 - pageHeader_size) {			// page is devided into blocks of the same size
		BlockPageHeader* blockHeader = NULL;					// block with a free space is available
		BlockClassesNode* node;
		BlockClassesNode* prevNode = NULL;
		for (node = firstClass; node != NULL; node = node->next) {
			if (node->block_size == real_size) {
				blockHeader = node->firstItem->firstBlockHeader;
				break;
			}
			prevNode = node;
		}

		if (node == NULL) {									// new class addition
			if (firstClass == NULL) {						// if no class exists, then create it,
															// add new class Item, which points to a new
															// page header
				firstClass = new BlockClassesNode;
				ClassItem* firstItem = new ClassItem;
				firstItem->next = NULL;
				firstClass->firstItem = firstItem;
				void* startAdress = createBlockPage(real_size);
				if (startAdress == NULL) return NULL;		// no pages available
				firstClass->block_size = real_size;
				firstClass->next = NULL;
				firstClass->firstItem->firstBlockHeader = (BlockPageHeader*)startAdress;
				return (void*)((size_t)startAdress + pageHeader_size);
			} else {										// at least one class exist,
															// but no class with needed block size
				node = new BlockClassesNode;
				ClassItem* firstItem = new ClassItem;
				firstItem->next = NULL;
				node->firstItem = firstItem;
				prevNode->next = node;
				void* startAdress = createBlockPage(real_size);
				if (startAdress == NULL) return NULL;		// no pages available
				node->block_size = real_size;
				node->next = NULL;
				node->firstItem->firstBlockHeader = (BlockPageHeader*)startAdress;
				return (void*)((size_t)startAdress + pageHeader_size);
			}

		} else {											// class found
			BlockHeader* freeBlock = blockHeader->nextFreeBlock;
			if (freeBlock == NULL) return NULL;
			if (freeBlock->nextHeader == NULL) {
				BlockHeader* temp = (BlockHeader*)((size_t)blockHeader->nextFreeBlock + real_size);
				blockHeader->nextFreeBlock = temp;
				temp->nextHeader = NULL;
			} else {
				blockHeader->nextFreeBlock = freeBlock->nextHeader;
			}

			// no more space in the page after block adding
			if ((size_t)blockHeader + page_size - (size_t)freeBlock - real_size < real_size) {
				ClassItem* item;
				ClassItem* prevItem = NULL;
				// delete block header from its item in the class
				for (ClassItem* item = node->firstItem; item != NULL; item = item->next) {
					if (item->firstBlockHeader == blockHeader) {
						if (prevItem == NULL) {
							node->firstItem = item->next;
						} else { 
							prevItem->next = item->next;
						}
						break;
					}
					prevItem = item;
				}
				if (node->firstItem == NULL) {
					if (prevNode == NULL) firstClass = NULL;
					else prevNode->next = node->next;
				}
			} 
			return freeBlock;
		}

		} else {												// virtual page consists of a few blocks
		size_t pagesNeeded = (size_t)ceil((double)real_size / (double)page_size);
		int firstFreePage = findPageSequence(pagesNeeded);
		if (firstFreePage == -1) {
			return NULL;
		}

		void *startAdress = (void*)((size_t)memory_start + firstFreePage*page_size);
		for (size_t i = firstFreePage; i < firstFreePage + pagesNeeded; i++) {
			pages[i] = (Page*)startAdress;
		}
		return startAdress;
	}
}

void mem_free(void* addr) {
	//address is out of memory bounds
	if (addr < memory_start || (size_t)addr > (size_t)memory_start + page_quant*page_size) return;
	size_t spaceDistance = (size_t)addr - (size_t)memory_start;
	int pageIndex = spaceDistance / page_size;
	bool isBlockPage = true;
	// page is a block page only if adress points not to the start of a new page
	if (spaceDistance % page_size == 0) isBlockPage = false;

	if (isBlockPage) {							// delete one block from the (pageIndex+1)-th page
		BlockPageHeader* pageHeader = (BlockPageHeader*) pages[pageIndex];
		size_t block_size = pageHeader->block_size;
		BlockClassesNode* classNode;				// classNode
		BlockClassesNode* prevClassNode = NULL;

		// try to find a class, where the block belongs to
		for (classNode = firstClass; classNode != NULL; classNode = classNode->next) { 
			if (classNode->block_size == block_size) break;
			prevClassNode = classNode;
		}

		// no class for the block found, which means that new class needed
		if (classNode == NULL) {
			BlockClassesNode* newClassNode = new BlockClassesNode;
			newClassNode->block_size = block_size;
			newClassNode->next = NULL;
			ClassItem* newClassItem = new ClassItem;
			newClassItem->firstBlockHeader = pageHeader;
			newClassItem->next = NULL;
			newClassNode->firstItem = newClassItem;
			if (firstClass == NULL) {				// no classes exist
				firstClass = newClassNode;
			} else { 
				prevClassNode->next = newClassNode;
			}
			// delete the block
			deleteBlock(pageHeader, addr);
		} else {
			bool oneBlockOnPage = false;
			size_t usedSpace = (size_t)pageHeader->nextFreeBlock - (size_t)pageHeader - pageHeader_size;
			for (BlockHeader* bHeader = pageHeader->nextFreeBlock; bHeader->nextHeader != NULL; bHeader = bHeader->nextHeader) { 
				usedSpace += (size_t)bHeader->nextHeader - (size_t)bHeader;
			}
			if (usedSpace == block_size) oneBlockOnPage = true;
			if (oneBlockOnPage) {					// only one block in the page
				pages[pageIndex] = NULL;			// mark the page as free
				ClassItem* curClassItem;
				ClassItem* prevClassItem = NULL;
				// seek for a class item, the block belongs to
				for (curClassItem = classNode->firstItem
					; curClassItem->next != NULL
					; curClassItem = curClassItem->next) { 
						if (curClassItem->firstBlockHeader == pageHeader) break;
						prevClassItem = curClassItem;
				}

				// delete class item of the block
				if (prevClassItem) prevClassItem->next = curClassItem->next;
				else {
					classNode->firstItem = NULL;			// class has no items
					// at least two classes exist
					if (prevClassNode) prevClassNode->next = classNode->next;
					// only one class exists
					else firstClass = NULL;
				}
			} else {
				// delete the block, create new header if needed 
				deleteBlock(pageHeader, addr);
			}
		}

	} else {										// delete all pages, given to user as one virtual page
		for (int i = pageIndex + 1; i < page_quant && pages[pageIndex] == pages[i]; i++) { 
			pages[i] = NULL;
		}
		pages[pageIndex] = NULL;
	}

}

void deleteBlock(BlockPageHeader* pageHeader, void* addr) { 
	size_t block_size = pageHeader->block_size;
	BlockHeader* bHeader;
	BlockHeader* prevHeader = NULL;
	// look for header of the previous to current free block
	for (bHeader = (BlockHeader*)pageHeader; (void*)bHeader < addr; bHeader = bHeader->nextHeader) { 
		prevHeader = bHeader;
	}

	// new node for released block of memory
	BlockHeader* newHeader = (BlockHeader*)addr;

	// block is before some free block, so delete all free block
	// node and add new to addr instead
	if ((size_t)addr + block_size == (size_t)bHeader) {
		newHeader->nextHeader = bHeader->nextHeader;
		prevHeader->nextHeader = newHeader;
	// just add new node
	} else if ((size_t)prevHeader + block_size != (size_t)addr) { 
		newHeader->nextHeader = prevHeader->nextHeader;
		prevHeader->nextHeader = newHeader;
	}
}

void* mem_realloc(void* addr, size_t size) {
	if (addr < memory_start || (size_t)addr > (size_t)memory_start + page_quant*page_size) return NULL;
	size_t real_size = allign_size(size);
	void* newAddr = mem_alloc(real_size);
	if (newAddr) { 
		memcpy(newAddr, addr, real_size);
		mem_free(addr);
		return newAddr;
	} else {
		return NULL;
	}
}

int findPageSequence(size_t pagesNeeded) {
	int freeCounter = 0;					// quantity of sequential free pages
	int firstFreePageInd = 0;
	for (int i = 0; i < page_quant; i++) {
		if (freeCounter == pagesNeeded) break;
		if (pages[i] == NULL) {				// page is free
			freeCounter++;				
		} else {							// page is full or used
			freeCounter = 0;
			firstFreePageInd = i + 1;
		}
	}

	if (freeCounter == pagesNeeded) {
		return firstFreePageInd;
	} else {
		return -1;
	}
}

void* createBlockPage(size_t block_size) {
	int freePageIndex = findPageSequence(1);					// seek for 1 free page
	if (freePageIndex == -1) return NULL;

	// calculate adress of a new page
	void *startAdress = (void*)((size_t)memory_start + freePageIndex*page_size);
	pages[freePageIndex] = (Page*)startAdress;					// mark page as used

	// operations with page header
	BlockPageHeader* pageHeader = (BlockPageHeader*)startAdress;
	pageHeader->block_size = block_size;

	// next free header
	BlockHeader* freeHeader = (BlockHeader*)((size_t)startAdress + pageHeader_size + block_size);
	pageHeader->nextFreeBlock = freeHeader;
	freeHeader->nextHeader = NULL;
	return startAdress;
}

void mem_dump() {
	for (int i = 0; i < page_quant; i++) {
		printf("Page #%2d", i + 1);
		if (pages[i] == NULL) {
			printf(" : free\n");
		} else {
			BlockClassesNode* node;
			bool isDevided = false;
			for (node = firstClass; node != NULL; node = node->next) {
				for (ClassItem* item = node->firstItem; item != NULL; item = item->next) { 
					if ((void*)(item->firstBlockHeader) == pages[i]) { 		// page consists of blocks
						
						isDevided = true;
						break;
					}
					if (isDevided) break;
				}
			}
			if (!isDevided) {
				printf(" : full\n");
				continue;
			}
			printf(" : devided");
			size_t freeSpace = 0;
			int blockNumb = 1;
			BlockPageHeader* header = (BlockPageHeader*)pages[i];
			BlockHeader* bHeader;
			BlockHeader* prevBHeader = (BlockHeader*)((size_t)header + pageHeader_size);
			printf(", block size: %d\n", header->block_size);
			for (bHeader = header->nextFreeBlock; bHeader!= NULL; bHeader = bHeader->nextHeader) {
				size_t usedSpace = sizeof(BlockHeader*)*(size_t)(bHeader - prevBHeader);
				if (usedSpace) {
					if (bHeader != header->nextFreeBlock) usedSpace -= header->block_size;
					int blocksQuant = usedSpace / header->block_size;
					for (int i = 0; i < blocksQuant; i++) {
						printf("   block #%2d: used\n", blockNumb);
						blockNumb++;
					}
				} else freeSpace += header->block_size;
				if (bHeader->nextHeader != NULL) printf("   block #%2d: free\n", blockNumb);
				blockNumb++;
				prevBHeader = bHeader;
			}
			freeSpace = freeSpace + (size_t)header + page_size - (size_t)prevBHeader;
			if (freeSpace != 0)
				printf("   free space available: %5d\n", freeSpace);
		}
	}
	printf("\n\n\n");
}

size_t allign_size(size_t size) {
	size_t min_size = sizeof(void*);
    return size + (min_size - size % min_size) % min_size;
}


