#ifndef MCSH_MEMORY_H
#define MCSH_MEMORY_H

/***************************************************
mcsh memory subsystem declarations and functions.
Last modified: 4/6/13
***************************************************/

/************************
initializeMemory(void)
input: no input
output: no output
Initializes all of the memory used for the program.
************************/
void initializeMemory(void);

/************************
mcshMalloc(int)
input: int of requested size
output: pointer to that block of memory
Determines which size block to use, then:
	-sets availability of that block to false
	-returns the pointer to the block
************************/
void *mcshMalloc(int size);

/************************
mcshCalloc(int)
input: int of requested size
output: pointer to that block of memory
Mallocs a block of requested size, 0's out the memory space, and returns the pointer
************************/
void *mcshCalloc(int size);

/************************
mcshFree(pointer)
input: pointer to memory block
output: no output
Searches for memory address. If found, will set availability to TRUE, thus "freeing"
the memory for other use in the program.
************************/
void mcshFree(void *ptr);

/************************
mcshRealloc(pointer, int)
input: pointer to block of memory, int of requested size
output: pointer to that block of memory
Will deal with new memory size and return the pointer to the new memory block.
************************/
void *mcshRealloc(void *ptr, int size);

#endif
