#ifndef __MEMORYPOOL
#define __MEMORYPOOL

#include "PoolPtr.h"

#include <vector>
#include <cstdint>
#include <string>
#include <mutex>

#define INVALID_CHUNK_ID UINT32_MAX

typedef unsigned char byte;

struct MemoryChunk;

/*
Glossary
- Memory pool: The class that owns the reserved memory and manages the chunks
- Chunk: Struct in charge of a small piece of the pool memory
- Slot: Group of chunks used together when memory required > chunk size.
	Free slots are the entirety of contiguous free chunks.
	Used slots are the amount of chunks taken by a single allocation.
- Marker: Marks the start of a free slot
*/
class MemoryPool
{
public:
	MemoryPool(MemoryPool&) = delete;
	//Less/bigger chunks will result in a quicker execution, but more memory overhead
	//More/smaller chunks will result in slower execution, but less memory overhead
	MemoryPool(uint32_t chunkSizeInBytes, uint32_t chunkCount);
	~MemoryPool();

	//Allocate *bytes* space in the pool of uninitialized memory
	//PoolPtr will point at the first byte of the stored memory
	PoolPtr<byte> Alloc(uint32_t bytes);

	//Allocate enough space for *amount* instances of *type* class
	//Constructor will be called on all of them
	template<class type>
	PoolPtr<type> Alloc(uint32_t amount = 1);

	//Release previously allocated memory
	//Will fail if PoolPtr isn't allocated, is allocated on a diferent pool or was already freed
	template<class type>
	void Free(PoolPtr<type>& toFree);

	//DEBUG FUNCTION
	//Empties the pool and releases all memory, rendering all created pointers unusable
	void Clear();

	//Returns pool size un bytes
	inline uint32_t GetPoolSize() const;
	//Returns chunk size in bytes
	inline uint32_t GetChunkSize() const;
	inline uint32_t GetChunkCount() const;

	//Returns the amount of free chunks in the pool
	//If memory is fragmented, may not be representative of max alloc size avaliable
	uint32_t GetFreeChunks() const;
	//Returns the amount of used chunks
	uint32_t GetUsedChunks() const;


	//Appends a dump of the raw content of the pool into a file.
	//Identifier is just a string to be added before the dump
	void DumpMemoryToFile(const std::string& fileName, const std::string& identifier = "") const;

	//Appends a formatted dump of the contents of the pool into a file.
	//Identifier is just a string to be added before the dump
	//Chunks are separated by |
	//Start of a used chunk is marked by |<X- , where X is the amount of chunks
	//		linked to that allocation
	//End of a used chunk is marked by >|
	void DumpChunksToFile(const std::string& fileName, const std::string& identifier = "") const;

	//Appends detailed information of the state of the pool into a file.
	//Identifier is just a string to be added before the dump
	//Every chunk information and content is displayed independantly
	void DumpDetailedDebugChunksToFile(const std::string& fileName, const std::string& identifier = "") const;

private:
	//Release the memory this chunk is holding
	//Will fail if the chunk is not from this pool or this chunk is not the first in a used slot
	void Free(MemoryChunk* toFree);
	//Find a free slot with at least *requiredChunks* of contiguous avaliable chunks
	uint32_t FindSlotFor(uint32_t requiredChunks) const;
	//Calculate the amount of chunks needed to fit *bytesOfSpace*
	uint32_t ChunksToFit(uint32_t bytesOfSpace) const;
	//Add a new Free slot marker onto the chunk
	void AddFreeSlotMarker(MemoryChunk* chunk);
	//Find the "Free slot marker" inmediately preceeding the requested chunk
	//Will assert if no slot is avaliable for the requested chunk
	uint32_t FindPreceedingSlotMarker(MemoryChunk* chunk) const;

	inline bool IsFirstChunk(MemoryChunk* chunk) const;
	inline bool IsLastChunk(MemoryChunk* chunk) const;

	//Erase the slot marker with the passed index
	void NullifyFreeSlotMarker(uint32_t index);
	//Erase the slot marker passed
	void NullifyFreeSlotMarker(std::vector<MemoryChunk*>::iterator it);
	
	bool IsChunkMarkedAsFreeSlotStart(MemoryChunk* chunk) const;

private:
	MemoryChunk* m_firstChunk;

	std::vector<MemoryChunk*> m_freeSlotMarkers;
	uint32_t m_dirtyFreeSlotMarkers;

	uint32_t m_chunkCount;
	uint32_t m_chunkSize;

	mutable std::mutex m_mutex;

	byte* m_pool;
};

template<class type>
inline PoolPtr<type> MemoryPool::Alloc(uint32_t amount)
{
	PoolPtr<type> ret(Alloc(sizeof(type) * amount).m_chunk);
	if (ret.IsValid())
	{
		type* chunkData = ret.GetData();
		for (uint32_t n = 0; n < amount; n++)
		{
			//Calling constructor of "type" with a placement new
			new(chunkData) type();
			chunkData ++;
		}
	}
	return ret;
}

template<class type>
inline void MemoryPool::Free(PoolPtr<type>& toFree)
{
	if(toFree.IsValid())
		Free(toFree.m_chunk);
	//Mark as invalid the released PoolPtr
	toFree.m_chunk = nullptr;
}

#endif // !__MEMORYCHUNK