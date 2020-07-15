#include "MemoryChunk.h"
#include "MemoryPool.h"

#include <assert.h>
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
	assert(chunkSizeInBytes != 0 && chunkCount != 0);

	m_firstChunk = new MemoryChunk[m_chunkCount];
	m_pool = new MP_byte[GetPoolSize()];
	
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
	//Checking there are no dangling PoolPtrs and that all of them have been freed
	//If there is a single free slot marker and the end of the pool is clean, no reserved memory is left
	// --- Commenting this assert will still ensure there are no leaks, but may lead to invalid pointers
	assert(m_freeSlotMarkers.size() - m_dirtyFreeSlotMarkers == 1
		&& m_freeSlotMarkers[0] == m_firstChunk
		&& (m_firstChunk + GetChunkCount() - 1)->IsUsed() == false);

	delete[] m_pool;
	delete[] m_firstChunk;
}

void* MemoryPool::Alloc(uint32_t bytes)
{
	//Amount of chunks required
	uint32_t chunksOccupied = ChunksToFit(bytes);

	//Find the first slot big enough to fit our data
	uint32_t freeSlotIndex = FindSlotFor(chunksOccupied);
	if (freeSlotIndex == INVALID_CHUNK_ID)
		return nullptr;

	//We're guaranteed that this chunk is free and has more than *bytes* of free space
	MemoryChunk* headChunk = m_freeSlotMarkers[freeSlotIndex];

	//Mark this chunk as the first of a slot, and how many chunks it manages
	headChunk->m_usedChunks = chunksOccupied;
	headChunk->m_used = true;
	//Minus one, because "chunks occupied" already includes the current chunk
	MemoryChunk* endCHunk = headChunk + chunksOccupied - 1;
	//We only need to mark as "used" the first and last chunks of the used slot.
	//No one should request intermediary slots if all behaves as expected
	endCHunk->m_used = true;

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

	return headChunk->m_data;
}

void MemoryPool::FreeInternal(void* toFree)
{
	assert(toFree != nullptr && "Cannot free a nullptr");

	const uint32_t chunkN = ((MP_byte*)toFree - m_pool) / m_chunkSize;

	assert(chunkN >= 0 && chunkN < m_chunkCount && "Pointer to free does not point to this pool");

	MemoryChunk* firstChunkToFree = m_firstChunk + chunkN;

	//Checking ToFree is a valid pointer and it belongs to this specific pool
	if (firstChunkToFree->IsHeader())
	{
		//Minus one, because "usedChunks" already includes the first one
		MemoryChunk* lastChunk = firstChunkToFree + firstChunkToFree->m_usedChunks - 1;
		assert(lastChunk->IsUsed() == true && (lastChunk->m_usedChunks == 0 || firstChunkToFree->m_usedChunks == 1));

		firstChunkToFree->m_used = false;
		lastChunk->m_used = false;

		//If the chunk following the last chunk was "free", it will have been marked as a "free slot start"
		//We need to remove that marker since it's no longer the start, and replace it by the new "first chunk" of the slot
		if (IsLastChunk(lastChunk) == false && (lastChunk + 1)->m_avaliableContiguousChunks != 0)
		{
			std::vector<MemoryChunk*>::iterator followingFreeSlot = std::find(m_freeSlotMarkers.begin(), m_freeSlotMarkers.end(), lastChunk + 1);
			assert(followingFreeSlot != m_freeSlotMarkers.end());

			//Take note of how many contiguous chunks are avaliable starting on "firstChunk"
			firstChunkToFree->m_avaliableContiguousChunks = (*followingFreeSlot)->m_avaliableContiguousChunks + firstChunkToFree->m_usedChunks;
			(*followingFreeSlot)->m_avaliableContiguousChunks = 0;

			//If this is the first chunk or the previous chunks are already used, we need to mark this as a "start" of a free slot
			if (IsFirstChunk(firstChunkToFree) || (firstChunkToFree - 1)->IsUsed() == true)
			{
				*followingFreeSlot = firstChunkToFree;
			}
			//If the chunk previous to "firstChunk" is not used, we can nullify the "slot marker" and we'll need to update the "avaliable chunks" of the marker this chunks now belong to
			else
			{
				NullifyFreeSlotMarker(followingFreeSlot);

				m_freeSlotMarkers[FindPreceedingSlotMarker(firstChunkToFree)]->m_avaliableContiguousChunks += firstChunkToFree->m_avaliableContiguousChunks;
				firstChunkToFree->m_avaliableContiguousChunks = 0;
			}
		}
		//If the chunk following the reserved slot is occupied, we'll need to create a new marker or update the previous one
		else
		{
			//If this is the first chunk or the previous chunks are already used, we need to mark this as a "start" of a free slot
			if (IsFirstChunk(firstChunkToFree) || (firstChunkToFree - 1)->IsUsed() == true)
			{
				AddFreeSlotMarker(firstChunkToFree);
				//Since the chunk following the last chunk was used, this means this slot is as big as the space we released
				firstChunkToFree->m_avaliableContiguousChunks = firstChunkToFree->m_usedChunks;
			}
			else
			{
				m_freeSlotMarkers[FindPreceedingSlotMarker(firstChunkToFree)]->m_avaliableContiguousChunks += firstChunkToFree->m_usedChunks;
			}
		}

		firstChunkToFree->m_usedChunks = 0u;
	}
	else
	{
		if (firstChunkToFree->m_usedChunks == 0)
			assert(false && "Attempted to free a chunk which is not the first of an allocated slot");
		else if (firstChunkToFree->IsUsed() == false)
			assert(false && "Attempted to free an unused/unhandled chunk");
		else
			assert(false && "Unhandled error");
	}
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

		bool inUsedMemory = false;
		for(uint32_t n = 0; n < m_chunkCount; n++)
		{
			MemoryChunk* chunk = m_firstChunk + n;
			if (chunk->IsUsed() && chunk->m_usedChunks != 0)
			{
				file << "|<" << chunk->m_usedChunks << "- ";
				inUsedMemory = true;
			}

			file.write((char*)chunk->m_data, GetChunkSize());

			if (inUsedMemory == true && chunk->IsUsed() == true && chunk->m_usedChunks == 0)
			{
				file << ">|";
				inUsedMemory = false;
			}				

			file << "|";
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

		file << " |  Chunk size: " << GetChunkSize()
			<< "  |  Chunk count: " << GetChunkCount()
			<< "  |  Pool size: " << GetPoolSize()
			<< " |" << std::endl << std::endl;

		for (uint32_t n = 0; n < m_chunkCount; n++)
		{
			MemoryChunk* chunk = m_firstChunk + n;

			file << " | Chunk " << chunk->m_chunkN
				<< "\t| Used: " << (chunk->IsUsed() ? "true" : "false")
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
		}

	}
	file.close();
}

uint32_t MemoryPool::FindSlotFor(uint32_t requiredChunks) const
{
	uint32_t ret = (uint32_t)m_freeSlotMarkers.size() - m_dirtyFreeSlotMarkers;
	for (std::vector<MemoryChunk*>::const_reverse_iterator freeSlot = m_freeSlotMarkers.rbegin() + m_dirtyFreeSlotMarkers;
		freeSlot != m_freeSlotMarkers.crend(); freeSlot++)
	{
		ret--;
		if ((*freeSlot)->m_avaliableContiguousChunks >= requiredChunks)
			return ret;
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
	//All "dirty" or "nullptr" free markers should be at the end of the array
	if (m_dirtyFreeSlotMarkers != 0)
	{
		m_freeSlotMarkers[m_freeSlotMarkers.size() - m_dirtyFreeSlotMarkers] = chunk;
		m_dirtyFreeSlotMarkers--;
	}
	else
	{
		m_freeSlotMarkers.push_back(nullptr);
		m_freeSlotMarkers[m_freeSlotMarkers.size() - m_dirtyFreeSlotMarkers - 1] = chunk;
	}
}

uint32_t MemoryPool::FindPreceedingSlotMarker(MemoryChunk* chunk) const
{
	assert(m_freeSlotMarkers.size() > m_dirtyFreeSlotMarkers);
	uint32_t candidate = INVALID_CHUNK_ID;
	uint32_t candidateDistance = UINT32_MAX;
	for (int32_t index = (uint32_t)m_freeSlotMarkers.size() - 1u - m_dirtyFreeSlotMarkers; index >= 0; --index)
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
	NullifyFreeSlotMarker(m_freeSlotMarkers.begin() + index);
}

void MemoryPool::NullifyFreeSlotMarker(std::vector<MemoryChunk*>::iterator it)
{
	uint32_t lastUsedMarker = (uint32_t)(m_freeSlotMarkers.size()) - m_dirtyFreeSlotMarkers - 1;
	(*it) = m_freeSlotMarkers[lastUsedMarker];
	m_freeSlotMarkers[lastUsedMarker] = nullptr;
	m_dirtyFreeSlotMarkers++;
}

bool MemoryPool::IsChunkMarkedAsFreeSlotStart(MemoryChunk* chunk) const
{
	if (chunk == nullptr)
		return false;
	return chunk->m_avaliableContiguousChunks != 0;
}