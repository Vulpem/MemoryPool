#ifndef __MEMORYPOOL
#define __MEMORYPOOL

#include "PoolAllocation.h"

#include <vector>
#include <cstdint>
#include <string>

#define INVALID_CHUNK_ID UINT32_MAX

#define DEFAULT_BLOCKSIZE 32
#define DEFAULT_NUMBLOCKS 50

typedef unsigned char byte;

struct MemoryChunk;

/*
Memory pool: The class that owns the reserved memory and manages the chunks
Chunk: Class in charge of a small piece of the pool's memory
Slot: Group of chunks used together when memory required > chunk size
Free marker: Marks the start of a free slot, a collection of chunks avaliable to reserve
*/

class MemoryPool
{
public:
	MemoryPool(MemoryPool&) = delete;
	//-chunkSizeInBytes is the size of the chunks that manage the memory. Smaller values will make the pool slower, bigger values will
	//    mean more memory overhead (less bad)
	//-chunkCount is amount of chunks that will be reserved
	MemoryPool(uint32_t chunkSizeInBytes, uint32_t chunkCount);
	~MemoryPool();

	//Allocate *bytes* space in the pool of uninitialized memory
	PoolAllocation Alloc(uint32_t bytes);

	//Allocate enough space for *amount* instances of *type* class and call the constructor
	template<class type>
	PoolAllocation Alloc(uint32_t amount = 1);

	//Release memory
	void Free(PoolAllocation& toFree);

	//DEBUG FUNCTION
	//Empties the pool and releases all memory, rendering all created pointers unusable
	void Clear();

	inline uint32_t GetPoolSize() const;
	inline uint32_t GetChunkSize() const;
	inline uint32_t GetChunkCount() const;

	const void* GetRawPool() const;
	//Dump all the contents of the pool into a text file
	void DumpMemoryToFile(const std::string& fileName, const std::string& identifier = "") const;
	//Dump all the contents of the pool into a text file with 0 on unused memory and simplified chunk information
	void DumpChunksToFile(const std::string& fileName, const std::string& identifier = "", bool append = true) const;
	//Dump each chunk individually with details and its contents into a text file
	void DumpDetailedDebugChunksToFile(const std::string& fileName, const std::string& identifier = "") const;

private:
	//Find a slot with at least *requiredChunks* of contiguous avaliable chunks
	MemoryChunk* FindSlotFor(uint32_t requiredChunks);
	//Calculate the amount of chunks needed to fit *bytesOfSpace*
	uint32_t ChunksToFit(uint32_t bytesOfSpace) const;

	//Update the "m_avaliableChunks" variable of the MemoryChunk and all the previous ones
	void UpdateAvaliableContiguousChunks(MemoryChunk* chunk) const;

	uint32_t MoveCursorToNextFreeSpace();
	uint32_t AdvanceCursor();

private:
	MemoryChunk* m_firstChunk;

	MemoryChunk* m_cursor;

	uint32_t m_chunkCount;
	uint32_t m_chunkSize;

	byte* m_pool;

	uint32_t m_freeChunks;
	uint32_t m_biggestKnownChunk;
};

template<class type>
inline PoolAllocation MemoryPool::Alloc(uint32_t amount)
{
	PoolAllocation ret = Alloc(sizeof(type) * amount);
	if (ret.IsValid())
	{
		type* chunkData = (type*)ret.GetData();
		for (uint32_t n = 0; n < amount; n++)
		{
			//Calling consttructor of "type" with a placement new
			new(chunkData) type();
			chunkData += 1;
		}
	}
	return ret;
}

#endif // !__MEMORYCHUNK