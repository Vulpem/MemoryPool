#include "MemoryChunk.h"
#include "MemoryPool.h"

#include <assert.h>
#include <math.h>
#include <fstream>
#include <algorithm>

MemoryPool::MemoryPool(uint32_t chunkSizeInBytes, uint32_t chunkCount)
	: m_firstChunk(nullptr)
	, m_freeSlotMarkers()
	, m_dirtyFreeSlotMarkers(0u)
	, m_chunkCount(chunkCount)
	, m_chunkSize(chunkSizeInBytes)
	, m_pool(nullptr)
{
	m_firstChunk = new MemoryChunk[m_chunkCount];
	m_pool = new byte[GetPoolSize()];
	m_cursor = m_firstChunk;

	//Initializing all chunks to their default values
	for (uint32_t chunkN = 0; chunkN < m_chunkCount; ++chunkN)
	{
		MemoryChunk* chunkPtr = m_firstChunk + chunkN;
		chunkPtr->m_usedChunks = 0u;
		chunkPtr->m_avaliableContiguousChunks = 0;
		//Pointer to the alloted piece of memory from the pool
		chunkPtr->m_data = m_pool + (GetChunkSize() * chunkN);
		chunkPtr->m_chunkN = chunkN;
	}

	m_freeSlotMarkers.reserve(m_chunkCount / 5);
	//Adding a marker at the start of the pool as the first "free" spot avaliable
	AddFreeSlotMarker(m_firstChunk);
	m_firstChunk->m_avaliableContiguousChunks = GetChunkCount();

}

MemoryPool::~MemoryPool()
{
	delete[] m_pool;
	delete[] m_firstChunk;
}

PoolAllocation MemoryPool::Alloc(uint32_t bytes)
{
	assert(bytes != 0);

	//Amount of chunks required
	uint32_t chunksOccupied = ChunksToFit(bytes);

	//Find the first slot big enough to fit our data
	MemoryChunk* headChunk = FindSlotFor(chunksOccupied);

	//We're guaranteed that this chunk is free and has more than *bytes* of free space
	MemoryChunk* headChunk = m_freeSlotMarkers[freeSlotIndex];

	//Mark this chunk as the first of a slot, and how many chunks it manages
	headChunk->m_usedChunks = chunksOccupied;
	headChunk->m_used = true;
	//Minus one, because "chunks occupied" already includes the current chunk
	MemoryChunk* endCHunk = headChunk + chunksOccupied - 1;
	endCHunk->m_used = true;

	//We only need to mark as "used" the first and last chunks of the used slot.
	//No one should request intermediary slots if all behaves as expected

	//If the chunk following the reserved memory is free, move the "free marker" pointer to there
	if (IsLastChunk(endCHunk) == false && (endCHunk + 1)->IsUsed() == false)
	{
		m_freeSlotMarkers[freeSlotIndex] = (endCHunk + 1);
		(endCHunk + 1)->m_avaliableContiguousChunks = headChunk->m_avaliableContiguousChunks - chunksOccupied;
	}
	//Else, nullify the "free marker"
	else
	{
		NullifyFreeSlotMarker(freeSlotIndex);
	}
	headChunk->m_avaliableContiguousChunks = 0;

	return PoolAllocation(headChunk);
}

void MemoryPool::Free(PoolAllocation& toFree)
{
	if (toFree.IsValid())
	{
		MemoryChunk* firstChunk = toFree.chunk;
		//Mark the "PoolAllocation" as invalid
		toFree.chunk = nullptr;
		if (firstChunk->IsUsed() == true && firstChunk->m_usedChunks != 0)
		{
			//Minus one, because "usedChunks" already includes the first one
			MemoryChunk* lastChunk = firstChunk + firstChunk->m_usedChunks - 1;
			assert(lastChunk->IsUsed() == true && (lastChunk->m_usedChunks == 0 || firstChunk->m_usedChunks == 1));

		m_freeChunks += chunk->m_usedChunks;

			//If the chunk following the last chunk was "free", it will have been marked as a "free slot start"
			//We need to remove that marker since it's no longer the start, and replace it by the new "first chunk" of the slot
			if (IsLastChunk(lastChunk) == false && (lastChunk+1)->m_avaliableContiguousChunks != 0)
			{
				std::vector<MemoryChunk*>::iterator followingFreeSlot = std::find(m_freeSlotMarkers.begin(), m_freeSlotMarkers.end(), lastChunk + 1);
				assert(followingFreeSlot != m_freeSlotMarkers.end());

		UpdateAvaliableContiguousChunks(lastChunk);

				//If this is the first chunk or the previous chunks are already used, we need to mark this as a "start" of a free slot
				if (IsFirstChunk(firstChunk) || (firstChunk-1)->IsUsed() == true)
				{
					*followingFreeSlot = firstChunk;
				}
				//If the chunk previous to "firstChunk" is not used, we can nullify the "slot marker" and we'll need to update the "avaliable chunks" of the marker this chunks now belong to
				else
				{
					NullifyFreeSlotMarker(followingFreeSlot);

					m_freeSlotMarkers[FindPreceedingSlotMarker(firstChunk)]->m_avaliableContiguousChunks += firstChunk->m_avaliableContiguousChunks;
					firstChunk->m_avaliableContiguousChunks = 0;
				}
			}
			//If the chunk following the reserved slot is occupied, we'll need to create a new marker or update the previous one
			else
			{
				//If this is the first chunk or the previous chunks are already used, we need to mark this as a "start" of a free slot
				if ( IsFirstChunk(firstChunk) || (firstChunk-1)->IsUsed() == true)
				{
					AddFreeSlotMarker(firstChunk);
					//Since the chunk following the last chunk was used, this means this slot is as big as the space we released
					firstChunk->m_avaliableContiguousChunks = firstChunk->m_usedChunks;
				}
				else
				{
					m_freeSlotMarkers[FindPreceedingSlotMarker(firstChunk)]->m_avaliableContiguousChunks += firstChunk->m_usedChunks;
				}
			}

			firstChunk->m_usedChunks = 0u;
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

void MemoryPool::Clear()
{
	m_cursor = m_firstChunk;
	m_freeChunks = m_chunkCount;

	MemoryChunk* chunk = m_firstChunk;
	do {
		chunk->m_used = false;
		chunk->m_usedChunks = 0;
		chunk++;
	} while (IsLastChunk(chunk) == false);
	m_firstChunk->m_avaliableContiguousChunks = GetChunkCount();
}

inline uint32_t MemoryPool::GetPoolSize() const
{
	return m_chunkCount * m_chunkSize;
}

inline uint32_t MemoryPool::GetChunkSize() const
{
	return m_chunkSize;
}

inline uint32_t MemoryPool::GetChunkCount() const
{
	return m_chunkCount;
}

uint32_t MemoryPool::GetFreeChunks() const
{
	uint32_t ret = 0u;
	std::for_each(m_freeSlotMarkers.begin(), m_freeSlotMarkers.end() - m_dirtyFreeSlotMarkers,
		[&ret](MemoryChunk* freeSlot)
	{
		ret += freeSlot->m_avaliableContiguousChunks;
	});
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

void MemoryPool::DumpChunksToFile(const std::string& fileName, const std::string& identifier, bool append) const
{
	std::ofstream file;
	if(append)
		file.open(fileName.c_str(), std::ofstream::out | std::ios::app | std::ofstream::binary);
	else
		file.open(fileName.c_str(), std::ofstream::out | std::ofstream::binary);

	if (file.good())
	{
		file << std::endl << " | " << identifier.c_str()
			<< " | Chunk count: " << GetChunkCount()
			<< " |  Pool size: " << GetPoolSize()
			<< " |" << std::endl;

		MemoryChunk* chunk = m_firstChunk;
		bool inUsedMemory = false;
		do {
			if (chunk->IsUsed() && chunk->m_usedChunks != 0)
			{
				file << "|<" << chunk->m_usedChunks << "--";
				inUsedMemory = true;
			}

			file.write((char*)chunk->m_data, GetChunkSize());

			if (inUsedMemory == true && chunk->IsUsed() == true)
			{
				file << ">|";
				inUsedMemory = false;
			}

			file << "|";
			chunk++;
		} while (IsLastChunk(chunk) == false);

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
		file << " |  Chunk size: " << GetChunkSize()
			<< "  |  Chunk count: " << GetChunkCount()
			<< "  |  Pool size: " << GetPoolSize()
			<< " |" << std::endl << std::endl;

		do {
			file << " | Chunk " << chunk->m_chunkN
				<< "\t| Used: " << (chunk->IsUsed() ? "true" : "false")
				<< "\t| Avaliable: " << chunk->m_avaliableContiguousChunks
				<< "\t| Used chunks: " << chunk->m_usedChunks << " |";

			if (chunk == m_cursor)
			{
				file << " <-- Cursor points here";
			}

			file  << std::endl;
			file << "   ";
			file.write((char*)chunk->m_data, GetChunkSize());
			file << std::endl;
			chunk++;
		} while (IsLastChunk(chunk) == false);

	}
	file.close();
}

MemoryChunk* MemoryPool::FindSlotFor(uint32_t requiredChunks)
{
	uint32_t ret = (uint32_t)m_freeSlotMarkers.size() - m_dirtyFreeSlotMarkers;
	for (std::vector<MemoryChunk*>::const_reverse_iterator freeSlot = m_freeSlotMarkers.rbegin() + m_dirtyFreeSlotMarkers;
		freeSlot != m_freeSlotMarkers.crend(); freeSlot++)
	{
		ret--;
		if ((*freeSlot)->m_avaliableContiguousChunks >= requiredChunks)
			return ret;
	}

	return m_cursor;
}

inline uint32_t MemoryPool::ChunksToFit(uint32_t bytesOfSpace) const
{
	//Rounding up the division
	return (bytesOfSpace + m_chunkSize - 1) / m_chunkSize;
}

void MemoryPool::UpdateAvaliableContiguousChunks(MemoryChunk* chunk) const
{
	if (chunk)
	{
		uint32_t avaliableChunks = 1u;
		if (chunk->m_nextChunk && chunk->m_nextChunk->Used() == false)
			avaliableChunks = chunk->m_nextChunk->m_avaliableContiguousChunks + 1u;

		while (chunk && chunk->Used() == false)
		{
			chunk->m_avaliableContiguousChunks = avaliableChunks++;
			chunk = chunk->m_previousChunk;
		}
	}
}

uint32_t MemoryPool::FindPreceedingSlotMarker(MemoryChunk* chunk) const
{
	assert(m_freeSlotMarkers.size() > m_dirtyFreeSlotMarkers);
	uint32_t candidate = INVALID_CHUNK_ID;
	uint32_t candidateDistance = UINT32_MAX;
	for (int32_t index = m_freeSlotMarkers.size() - 1 - m_dirtyFreeSlotMarkers; index >= 0; --index)
	{
		if (chunk->m_chunkN > m_freeSlotMarkers[index]->m_chunkN &&
			chunk->m_chunkN - m_freeSlotMarkers[index]->m_chunkN < candidateDistance)
		{
			candidateDistance = chunk->m_chunkN - m_freeSlotMarkers[index]->m_chunkN;
			candidate = index;
		}
	}
	assert(candidate != INVALID_CHUNK_ID);
	return candidate;
}

inline bool MemoryPool::IsFirstChunk(MemoryChunk* chunk) const
{
	return chunk->m_chunkN == 0;
}

inline bool MemoryPool::IsLastChunk(MemoryChunk* chunk) const
{
	return chunk->m_chunkN == m_chunkCount - 1;
}

void MemoryPool::NullifyFreeSlotMarker(uint32_t index)
{
	uint32_t moves = 0u;

void MemoryPool::NullifyFreeSlotMarker(std::vector<MemoryChunk*>::iterator it)
{
	uint32_t lastUsedMarker = (uint32_t)(m_freeSlotMarkers.size()) - m_dirtyFreeSlotMarkers - 1;
	(*it) = m_freeSlotMarkers[lastUsedMarker];
	m_freeSlotMarkers[lastUsedMarker] = nullptr;
	m_dirtyFreeSlotMarkers++;
}

uint32_t MemoryPool::AdvanceCursor()
{
	uint32_t moves = 0u;

	moves = (m_cursor->Used() ? m_cursor->m_usedChunks : m_cursor->m_avaliableContiguousChunks);

	m_cursor += moves - 1;
	if(m_cursor->m_nextChunk)
		m_cursor = m_cursor->m_nextChunk;
	else
		m_cursor = m_firstChunk;

	return moves;
}
