#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Convert a raw pointer address to a AmphoraHeap block, index addressing format, returns the block */
int Amphora_HeapPtrToBlkIdxV1(void *ptr, int *blk, int *idx);
/* Convert an AmphoraHeap block, index address to a raw pointer */
void *Amphora_HeapBlkIdxToPtrV1(int blk, int idx);
/* Dump the contents of a heap memory block to the console */
void Amphora_HeapDumpBlockV1(uint8_t blk);
/* Read the value of a specific address on the heap */
uint8_t Amphora_HeapPeekV1(uint8_t blk, uint16_t idx);
/* Write a value to a specific address on the heap */
void Amphora_HeapPokeV1(uint8_t blk, uint16_t idx, uint8_t val);
#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H */
