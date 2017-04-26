
/***************************************************
mcsh memory subsystem declarations and functions.
Last modified: 4/6/2013
***************************************************/

//defines for the number of each memory size
#define NUM_XS 1000
#define NUM_S 400
#define NUM_M 150
#define NUM_L 70
#define NUM_XL 30

//defines for sizes of each memory size
#define SIZE_XS 30
#define SIZE_S 50
#define SIZE_M 120
#define SIZE_L 250
#define SIZE_XL 800

//number of different sizes being used
#define NUM_SIZES 5

#define FALSE 0
#define TRUE 1

#include <string.h>
#include "memory.h"

//availability arrays
char axs[NUM_XS];
char as[NUM_S];
char am[NUM_M];
char al[NUM_L];
char axl[NUM_XL];

//actual space allocated
char spaceXS[NUM_XS * SIZE_XS];
char spaceS[NUM_S * SIZE_S];
char spaceM[NUM_M * SIZE_M];
char spaceL[NUM_L * SIZE_L];
char spaceXL[NUM_XL * SIZE_XL];

int first = 1; //used to flag if first init

typedef struct
{
	int num;
	int size;
	char *start;
	char *end;
	char *available;
} tMemory;

tMemory chunks[NUM_SIZES];

/************************
initializeMemory(void)
input: no input
output: no output
Initializes all of the memory used for the program.
************************/
void initializeMemory(void)
{
	first = 0; //flips flag so memory is only init'd one time.
	chunks[0].num = NUM_XS;
	chunks[0].size = SIZE_XS;
	chunks[0].start = spaceXS;
	chunks[0].end = (spaceXS + NUM_XS * SIZE_XS - 1);
	memset(axs, TRUE, sizeof(axs));
	chunks[0].available = axs;

	chunks[1].num = NUM_S;
	chunks[1].size = SIZE_S;
	chunks[1].start = spaceS;
	chunks[1].end = (spaceS + NUM_S * SIZE_S - 1);
	memset(as, TRUE, sizeof(as));
	chunks[1].available = as;

	chunks[2].num = NUM_M;
	chunks[2].size = SIZE_M;
	chunks[2].start = spaceM;
	chunks[2].end = (spaceM + NUM_M * SIZE_M - 1);
	memset(am, TRUE, sizeof(am));
	chunks[2].available = am;

	chunks[3].num = NUM_L;
	chunks[3].size = SIZE_L;
	chunks[3].start = spaceL;
	chunks[3].end = (spaceL + NUM_L * SIZE_L - 1);
	memset(al, TRUE, sizeof(al));
	chunks[3].available = al;

	chunks[4].num = NUM_XL;
	chunks[4].size = SIZE_XL;
	chunks[4].start = spaceXL;
	chunks[4].end = (spaceXL + NUM_XL * SIZE_XL - 1);
	memset(axl, TRUE, sizeof(axl));
	chunks[4].available = axl;
}

/************************
mcshMalloc(int)
input: int of requested size
output: pointer to that block of memory
Determines which size block to use, then:
	-sets availability of that block to false
	-returns the pointer to the block
************************/
void *mcshMalloc(int size)
{
	int i, j;
	void *retVal = NULL; //will default to NULL if none can be allocated
	if (first)			 //checks if it needs to init
	{
		initializeMemory();
	}
	for (i = 0; i < NUM_SIZES; i++)
	{
		if (size <= chunks[i].size) //seeing if size will fit in this block size
		{
			for (j = 0; j < chunks[i].num; j++)
			{
				if (chunks[i].available[j]) //if so, looks for first available spot
				{
					retVal = (void *)(chunks[i].start + j * chunks[i].size); //changes return to that pointer
					chunks[i].available[j] = FALSE;							 //sets availability to false
					break;
				}
			}
			if (j < chunks[i].num)
				break;
		}
	}
	return retVal;
}

/************************
mcshCalloc(int)
input: int of requested size
output: pointer to that block of memory
Mallocs a block of requested size, 0's out the memory space, and returns the pointer
************************/
void *mcshCalloc(int size)
{
	void *retVal;
	retVal = mcshMalloc(size);
	if (retVal != NULL)
	{
		memset(retVal, 0, sizeof(retVal));
	}
	return retVal;
}

/************************
mcshFree(pointer)
input: pointer to memory block
output: no output
Searches for memory address. If found, will set availability to TRUE, thus "freeing"
the memory for other use in the program.
************************/
void mcshFree(void *ptr)
{
	int i, j;
	for (i = 0; i < NUM_SIZES; i++)
	{
		if (((void *)chunks[i].start <= ptr) && (ptr <= (void *)chunks[i].end))
		{
			j = (ptr - (void *)chunks[i].start) / chunks[i].size;
			chunks[i].available[j] = TRUE;
			break;
		}
	}
}

/************************
mcshRealloc(pointer, int)
input: pointer to block of memory, int of requested size
output: pointer to that block of memory
Will deal with new memory size and return the pointer to the new memory block.
************************/
void *mcshRealloc(void *ptr, int size)
{
	void *retVal;
	int i;

	if (ptr == NULL)			   //if there is no requested pointer
		retVal = mcshMalloc(size); //mallocs and returns that value
	else if (size == 0)			   //if size is zero, you gotta free that space
	{
		mcshFree(ptr);
		retVal = NULL;
	}
	else
	{
		for (i = 0; i < NUM_SIZES; i++)
		{
			if (((void *)chunks[i].start <= ptr) && (ptr <= (void *)chunks[i].end)) //checks to see if the pointer is within each memory group
			{
				if (size == chunks[i].size)		//if the new and old sizes are the same
					retVal = ptr;				//does nothing and returns the same pointer
				else if (size < chunks[i].size) //if a smaller size is requested
				{
					retVal = mcshMalloc(size); //malloc the smaller size
					if (retVal == NULL)		   //if that fails
					{
						retVal = ptr; //just return the existing pointer
						break;
					}
					memcpy(retVal, ptr, size); //copies over the contents
					mcshFree(ptr);			   //and free's the current pointer
				}
				else if (size > chunks[i].size) //if a larger size
				{
					retVal = mcshMalloc(size); //mallocs new size
					memcpy(retVal, ptr, size); //copies content
					mcshFree(ptr);			   //and frees old pointer
				}
			}
		}
	}
	return retVal;
}
