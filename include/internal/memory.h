#ifndef MEMORY_INTERNAL_H
#define MEMORY_INTERNAL_H

#include "../memory.h"

#define MAGIC 0xEFBE

#define AMPHORA_MEM_CATEGORIES	\
	X(MEM_UNASSIGNED)	\
	X(MEM_RENDERABLE)	\
	X(MEM_IMAGE)		\
	X(MEM_EMITTER)		\
	X(MEM_PARTICLE)		\
	X(MEM_TILEMAPS)		\
	X(MEM_STRING)		\
	X(MEM_HASHTABLE)	\
	X(MEM_MISC)		\
	X(MEM_META)

#define AMPHORA_HEAP_SIZE 0x10000
#define AMPHORA_NUM_MEM_BLOCKS 0x100 /* Must be <= 0x100 */

typedef uint8_t AmphoraMemBlock[AMPHORA_HEAP_SIZE];

typedef enum
{
#define X(cat) cat,
	AMPHORA_MEM_CATEGORIES
#undef X
	MEM_COUNT
} AmphoraMemBlockCategory;

struct
amphora_mem_allocation_header_t
{
	uint16_t magic;
	uint8_t scope; /* unimplemented */
	uint8_t free : 1;
	uint8_t large : 1; /* unimplemented */
	uint16_t off_b;
	uint16_t off_f;
}; /* This struct must be exactly 8 bytes */

struct amphora_mem_block_metadata_t
{
	uint16_t largest_free;
	uint16_t allocations;
	AmphoraMemBlockCategory category;
	uint8_t corrupted : 1;
	uint8_t recovery_count : 3;
};

int Amphora_InitHeap(void);
void Amphora_DestroyHeap(void);
void *Amphora_HeapAlloc(size_t size, AmphoraMemBlockCategory category);
void *Amphora_HeapAllocFrame(size_t size);
void *Amphora_HeapRealloc(void *ptr, size_t size, AmphoraMemBlockCategory category);
void *Amphora_HeapCalloc(size_t num, size_t size, AmphoraMemBlockCategory category);
char *Amphora_HeapStrdup(const char *str);
char *Amphora_HeapStrdupFrame(const char *str);
void Amphora_HeapFree(void *ptr);
void Amphora_HeapClearFrameHeap(void);
uint32_t Amphora_HeapHousekeeping(uint32_t ms);

#endif /* MEMORY_INTERNAL_H */
