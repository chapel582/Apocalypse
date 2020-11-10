#ifndef APOCALYPSE_HEAP_H

#include <stdint.h>

struct heap_entry
{
	uint32_t Key;
	uint32_t DataIndex;
};

struct heap
{
	heap_entry* Entries;
	uint32_t MaxEntries;
	uint32_t EntryCount;
};

void MinInsert(heap* Heap, uint32_t Key, uint32_t DataIndex);
uint32_t ExtractMinRoot(heap* Heap);

// TODO: remove TestHeap from non-internal builds
void TestHeap();

#define APOCALYPSE_HEAP_H
#endif