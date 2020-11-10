#include "apocalypse_binary_heap.h"

inline uint32_t GetLeftChildIndex(uint32_t ParentIndex)
{
	return 2 * ParentIndex + 1;
}

inline uint32_t GetRightChildIndex(uint32_t ParentIndex)
{
	return 2 * ParentIndex + 2;
}

inline uint32_t GetParentIndex(uint32_t ChildIndex)
{
	uint32_t Evened = ChildIndex;
	Evened &= 0xFFFFFFFE;
	return Evened / 2;
}

inline void SwapEntries(heap_entry* Entry1, heap_entry* Entry2)
{
	heap_entry Temp = *Entry1;
	*Entry1 = *Entry2;
	*Entry2 = Temp;
}

void MinInsert(heap* Heap, uint32_t Key, uint32_t DataIndex)
{
	ASSERT(Heap->MaxEntries > Heap->EntryCount);
	uint32_t CurrentIndex = Heap->EntryCount;
	Heap->EntryCount++;
	heap_entry* NewEntry = Heap->Entries + CurrentIndex;
	NewEntry->Key = Key;
	NewEntry->DataIndex = DataIndex;

	uint32_t ParentIndex = GetParentIndex(CurrentIndex);
	while(CurrentIndex > 0)
	{
		heap_entry* Parent = Heap->Entries + ParentIndex;
		heap_entry* Current = Heap->Entries + CurrentIndex;
		if(Current->Key < Parent->Key)
		{
			SwapEntries(Current, Parent);
			CurrentIndex = ParentIndex;
			ParentIndex = GetParentIndex(CurrentIndex);
		}
		else
		{
			break;
		}
	}
}

inline uint32_t PeekKey(heap* Heap)
{
	return Heap->Entries[0].Key;
}

uint32_t ExtractMinRoot(heap* Heap)
{
	ASSERT(Heap->EntryCount > 0);

	heap_entry* Root = &Heap->Entries[0];
	uint32_t Result = Root->DataIndex;
	Heap->EntryCount--;
	*Root = Heap->Entries[Heap->EntryCount]; 
	
	uint32_t CurrentIndex = 0;
	uint32_t LeftChildIndex = 1;
	uint32_t RightChildIndex = 2;
	while(LeftChildIndex < Heap->EntryCount)
	{
		heap_entry* Current = Heap->Entries + CurrentIndex;
		heap_entry* LeftChild = Heap->Entries + LeftChildIndex;
		heap_entry* RightChild = Heap->Entries + RightChildIndex;

		if(Current->Key > LeftChild->Key)
		{
			SwapEntries(Current, LeftChild);
			CurrentIndex = LeftChildIndex;
			LeftChildIndex = GetLeftChildIndex(CurrentIndex);
			RightChildIndex = GetRightChildIndex(CurrentIndex);
		}
		else if(
			RightChildIndex < Heap->EntryCount && 
			Current->Key > RightChild->Key
		)
		{
			SwapEntries(Current, RightChild);
			CurrentIndex = RightChildIndex;
			LeftChildIndex = GetLeftChildIndex(CurrentIndex);
			RightChildIndex = GetRightChildIndex(CurrentIndex);
		}
		else
		{
			break;
		}
	}

	return Result;
}

inline bool IsEmpty(heap* Heap)
{
	return Heap->EntryCount == 0;
} 

void TestHeap()
{
	heap_entry Entries[256];
	heap Heap = {};
	Heap.Entries = Entries;
	Heap.MaxEntries = 256;

	MinInsert(&Heap, 1, 0);
	MinInsert(&Heap, 1, 1);
	MinInsert(&Heap, 1, 2);
	MinInsert(&Heap, 0, 3);
	MinInsert(&Heap, 1, 4);
	MinInsert(&Heap, 0, 5);
	ExtractMinRoot(&Heap);
	MinInsert(&Heap, 0, 3);
	ExtractMinRoot(&Heap);
	ExtractMinRoot(&Heap);
	ExtractMinRoot(&Heap);
	ExtractMinRoot(&Heap);
	ExtractMinRoot(&Heap);
	ExtractMinRoot(&Heap);
}