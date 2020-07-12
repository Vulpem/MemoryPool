#include "MemoryChunk.h"
#include "MemoryPool.h"

#include <assert.h>
#include <math.h>
#include <fstream>

MemoryPool::MemoryPool(uint32_t poolSize, uint32_t chunkSize)
	: m_firstChunk(nullptr)
	, m_lastChunk(nullptr)
	, m_freeSlotMarkers()
	, m_dirtyFreeSlotMarkers(0u)
	, m_chunkCount(0u)
	, m_chunkSize(chunkSize)
	, m_pool(nullptr)
	, m_lastAllocatedChunk(nullptr)
{
	m_chunkCount = ChunksToFit(poolSize);

	m_firstChunk = new MemoryChunk[m_chunkCount];
	m_pool = new byte[GetPoolSize()];
	
	//Initializing all chunks to their default values
	MemoryChunk* chunkPtr = m_firstChunk;
	byte* dataPtr = m_pool;
	for (uint32_t chunkN = 0; chunkN < m_chunkCount; ++chunkN)
	{
		chunkPtr->m_avaliableContiguousChunks = GetChunkCount() - chunkN;
		chunkPtr->m_usedChunks = 0u;
		//Pointer to the alloted piece of memory from the pool
		chunkPtr->m_data = dataPtr;
		dataPtr += m_chunkSize;

		if (chunkN < m_chunkCount - 1u)
		{
			chunkPtr->m_nextChunk = chunkPtr + 1;
			chunkPtr->m_nextChunk->m_previousChunk = chunkPtr;
			chunkPtr = chunkPtr->m_nextChunk;
		}
	}

	m_lastChunk = chunkPtr;

	m_freeSlotMarkers.reserve(m_chunkCount / 25);
	//Adding a marker at the start of the pool as the first "free" spot avaliable
	AddFreeSlotMarker(m_firstChunk);

}

MemoryPool::~MemoryPool()
{
	delete[] m_pool;
	delete[] m_firstChunk;
}

PoolAllocation MemoryPool::Alloc(uint32_t bytes)
{
	//Amount of chunks required
	uint32_t chunksOccupied = ChunksToFit(bytes);

	//Find the first slot big enough to fit our data
	uint32_t freeSlotIndex = FindSlotFor(chunksOccupied);
	if (freeSlotIndex == INVALID_CHUNK_ID)
		return PoolAllocation(nullptr);

	//We're guaranteed that this chunk is free and has more than *bytes* of free space
	MemoryChunk* chunk = m_freeSlotMarkers[freeSlotIndex];
	
	m_lastAllocatedChunk = chunk;

	//Currently we're always allocating on the "start" of a free slot.
	//Previous chunks should be used / null
	//if (startingChunk->m_previousChunk)
	//	startingChunk->m_previousChunk->UpdateAvaliableSize(m_chunkSize);

	//Mark this chunk as the first of a slot, and how many chunks it manages
	chunk->m_usedChunks = chunksOccupied;
	//Mark them all as used
	for (uint32_t n = 0; n < chunksOccupied; ++n)
	{
		chunk->m_used = true;
		chunk = chunk->m_nextChunk;
	}

	//If the chunk following the reserved memory is free, move the "free marker" pointer to there
	if (chunk && chunk->m_used == false)
		m_freeSlotMarkers[freeSlotIndex] = chunk;
	//Else, nullify the "free marker"
	else
	{
		m_freeSlotMarkers[freeSlotIndex] = nullptr;
		m_dirtyFreeSlotMarkers++;
	}

	return PoolAllocation(m_lastAllocatedChunk);
}

void MemoryPool::Free(PoolAllocation& toFree)
{
	if (toFree.chunk != nullptr)
	{
		MemoryChunk* firstChunk = toFree.chunk;
		if (firstChunk->m_used == true && firstChunk->m_usedChunks != 0)
		{
			MemoryChunk* chunk = firstChunk;
			//Mark all the chunks as unused
			for (uint32_t n = 0; n < firstChunk->m_usedChunks; n++)
			{
				//If this number wasn't 0 we'd be erasing the "First" chunk of another reserved slot
				assert(chunk->m_usedChunks == 0 || chunk == firstChunk);
				chunk->m_used = false;
				chunk = chunk->m_nextChunk;
			}
			firstChunk->m_usedChunks = 0;

			if (chunk == nullptr)
				chunk = m_lastChunk;
			else
				chunk = chunk->m_previousChunk;

			UpdateChunkAvaliableSize(chunk);

			//If the chunk following the last chunk was "free", it will be marked as a "free slot start"
			//We need to remove that marker since it's no longer the start
			if (chunk->m_nextChunk && chunk->m_nextChunk->m_used == false)
			{
				auto it = std::find(m_freeSlotMarkers.begin(), m_freeSlotMarkers.end(), chunk->m_nextChunk);
				assert(it != m_freeSlotMarkers.end());
				*it = nullptr;
				m_dirtyFreeSlotMarkers++;
			}

			//If chunk before "firstChunk" is free, no need to add a "Free Slot" pointer
			//If it is occupied or this was the first chunk, mark this spot as a new "starting point" of free chunks
			MemoryChunk* previousChunk = firstChunk->m_previousChunk;
			if (previousChunk == nullptr || previousChunk->m_used == true)
			{
				AddFreeSlotMarker(firstChunk);
			}
			toFree.chunk = nullptr;
		}
		else
		{
			assert(false && "attempted to free an invalid chunk");
		}
	}
	else
	{
		assert(false && "Attempted to free an invalid chunkID");
	}
}

PoolAllocation MemoryPool::GetLastAllocation() const
{
	return PoolAllocation(m_lastAllocatedChunk);
}

void MemoryPool::Clear()
{
	m_freeSlotMarkers.clear();
	m_freeSlotMarkers.push_back(m_firstChunk);
	m_dirtyFreeSlotMarkers = 0u;

	m_lastAllocatedChunk = nullptr;

	MemoryChunk* chunk = m_firstChunk;
	while (chunk)
	{
		chunk->m_used = false;
		chunk->m_usedChunks = 0;
		chunk = chunk->m_nextChunk;
	}
	UpdateChunkAvaliableSize(m_lastChunk);
}

uint32_t MemoryPool::GetPoolSize() const
{
	return m_chunkCount * m_chunkSize;
}

uint32_t MemoryPool::GetChunkSize() const
{
	return m_chunkSize;
}

uint32_t MemoryPool::GetChunkCount() const
{
	return m_chunkCount;
}

uint32_t MemoryPool::GetFreeChunks() const
{
	uint32_t ret = 0u;
	for (auto freeSlot : m_freeSlotMarkers)
	{
		MemoryChunk* chunk = freeSlot;
		while (chunk && chunk->m_used == false)
		{
			ret++;
			chunk = chunk->m_nextChunk;
		}
	}
	return ret;
}

uint32_t MemoryPool::GetUsedChunks() const
{
	return GetChunkCount() - GetFreeChunks();
}

const void* MemoryPool::GetRawPool() const
{
	return m_pool;
}

void MemoryPool::DumpMemoryToFile(const std::string& fileName, const std::string& identifier) const
{
	std::ofstream file;
	file.open(fileName.c_str(), std::ofstream::out | std::ios::app | std::ofstream::binary);

	if (file.good())
	{
		file << std::endl << " - " << identifier.c_str() << + "---------------------------" << std::endl;
		file.write((char*)m_pool, GetPoolSize());
	}
	file.close();
}

void MemoryPool::DumpChunksToFile(const std::string& fileName, const std::string& identifier) const
{
	std::ofstream file;
	file.open(fileName.c_str(), std::ofstream::out | std::ios::app | std::ofstream::binary);

	if (file.good())
	{
		file << std::endl << " | " << identifier.c_str()
			<< " | Chunk count: " << GetChunkCount()
			<< " |  Pool size: " << GetPoolSize()
			<< " |" << std::endl;

		MemoryChunk* chunk = m_firstChunk;

		while (chunk)
		{
			if (chunk->m_used && chunk->m_usedChunks != 0)
					file << "|<" << chunk->m_usedChunks << "--";

			file.write((char*)chunk->m_data, GetChunkSize());

			if (chunk->m_used && (chunk->m_nextChunk == nullptr || chunk->m_nextChunk->m_used == false || chunk->m_nextChunk->m_usedChunks != 0))
				file << ">|";

			file << "|";
			chunk = chunk->m_nextChunk;
		}

	}
	file.close();
}

void MemoryPool::DumpDetailedDebugChunksToFile(const std::string& fileName, const std::string& identifier) const
{
	std::ofstream file;
	file.open(fileName.c_str(), std::ofstream::out | std::ios::app | std::ofstream::binary);

	if (file.good())
	{
		file << std::endl << " - " << identifier.c_str() << +"---------------------------" << std::endl;

		MemoryChunk* chunk = m_firstChunk;
		uint32_t n = 0u;
		file << " |  Chunk size: " << GetChunkSize()
			<< "  |  Chunk count: " << GetChunkCount()
			<< "  |  Pool size: " << GetPoolSize()
			<< " |" << std::endl << std::endl;

		while (chunk)
		{
			file << " | Chunk " << n
				<< "\t| Used: " << (chunk->m_used ? "true" : "false")
				<< "\t| Avaliable: " << chunk->m_avaliableContiguousChunks
				<< "\t| Used chunks: " << chunk->m_usedChunks << " |";

			if (std::find(m_freeSlotMarkers.begin(), m_freeSlotMarkers.end(), chunk) != m_freeSlotMarkers.end())
			{
				file << " <-- Marked as free slot start";
			}

			file  << std::endl;
			file << "   ";
			file.write((char*)chunk->m_data, GetChunkSize());
			file << std::endl;
			chunk = chunk->m_nextChunk;
			++n;
		}

	}
	file.close();
}

uint32_t MemoryPool::FindSlotFor(uint32_t requiredChunks) const
{
	uint32_t ret = m_freeSlotMarkers.size() - 1u;
	for (std::vector<MemoryChunk*>::const_reverse_iterator freeChunks = m_freeSlotMarkers.rbegin();
		freeChunks != m_freeSlotMarkers.rend(); freeChunks++)
	{
		if ((*freeChunks) && (*freeChunks)->m_avaliableContiguousChunks >= requiredChunks)
		{
			assert((*freeChunks)->m_used == false);
			return ret;
		}
		--ret;
	}
	return INVALID_CHUNK_ID;
}

uint32_t MemoryPool::ChunksToFit(uint32_t bytesOfSpace) const
{
	//Rounding up the division
	return (bytesOfSpace + m_chunkSize - 1) / m_chunkSize;
}

void MemoryPool::AddFreeSlotMarker(MemoryChunk* chunk)
{
	//If there's at least one "null" marker, find it and use it to store the new marker
	if (m_dirtyFreeSlotMarkers)
	{
		auto reused = std::find(m_freeSlotMarkers.begin(), m_freeSlotMarkers.end(), nullptr);
		assert(reused != m_freeSlotMarkers.end());
		*reused = chunk;
		m_dirtyFreeSlotMarkers--;
	}
	else
		m_freeSlotMarkers.push_back(chunk);
}

void MemoryPool::UpdateChunkAvaliableSize(MemoryChunk* chunk) const
{
	while (chunk && chunk->m_used == false)
	{
		if (chunk->m_nextChunk && chunk->m_nextChunk->m_used == false)
			chunk->m_avaliableContiguousChunks = chunk->m_nextChunk->m_avaliableContiguousChunks + 1;
		else
			chunk->m_avaliableContiguousChunks = 1;

		chunk = chunk->m_previousChunk;
	}
}