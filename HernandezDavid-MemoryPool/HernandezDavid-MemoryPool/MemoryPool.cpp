#include "MemoryChunk.h"
#include "MemoryPool.h"

#include <assert.h>
#include <math.h>
#include <fstream>

MemoryPool::MemoryPool(uint32_t chunkSizeInBytes, uint32_t chunkCount)
	: m_firstChunk(nullptr)
	, m_cursor(nullptr)
	, m_chunkCount(chunkCount)
	, m_chunkSize(chunkSizeInBytes)
	, m_pool(nullptr)
{
	m_firstChunk = new MemoryChunk[m_chunkCount];
	m_pool = new byte[GetPoolSize()];
	m_cursor = m_firstChunk;

	//Initializing all chunks to their default values
	MemoryChunk* chunkPtr = m_firstChunk;
	byte* dataPtr = m_pool;
	for (uint32_t chunkN = 0; chunkN < m_chunkCount; ++chunkN)
	{
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
		else
		{
			UpdateAvaliableContiguousChunks(chunkPtr);
		}
	}

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
	
	if(headChunk == nullptr)
		return PoolAllocation(nullptr);

	MemoryChunk* chunkIt = headChunk;
	//Mark this chunk as the first of a slot, and how many chunks it manages
	for (uint32_t n = 0; n < chunksOccupied; ++n)
	{
		chunkIt->m_usedChunks = chunksOccupied - n;
		chunkIt = chunkIt->m_nextChunk;
	}

	MoveCursorToNextFreeSpace();

	return PoolAllocation(headChunk);
}

void MemoryPool::Free(PoolAllocation& toFree)
{
	if (toFree.IsValid())
	{
		MemoryChunk* chunk = toFree.chunk;
		uint32_t chunksToFree = chunk->m_usedChunks;
		MemoryChunk* lastChunk = chunk + (chunksToFree - 1);

		for (uint32_t n = 0; n < chunksToFree; n++)
		{
			chunk->m_usedChunks = 0;
			chunk = chunk->m_nextChunk;
		}
		UpdateAvaliableContiguousChunks(lastChunk);

		toFree.chunk = nullptr;
	}
	else
	{
		assert(false && "Attempted to free an invalid chunkID");
	}
}
void MemoryPool::Clear()
{
	m_cursor = m_firstChunk;

	MemoryChunk* chunk = m_firstChunk;
	while (chunk)
	{
		chunk->m_usedChunks = 0;
		if (chunk->m_nextChunk == nullptr)
		{
			UpdateAvaliableContiguousChunks(chunk);
		}
		chunk = chunk->m_nextChunk;
	}
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
		while (chunk)
		{
			if (chunk->IsHeader())
			{
				file << "|<" << chunk->m_usedChunks << "--";
				inUsedMemory = true;
			}

			file.write((char*)chunk->m_data, GetChunkSize());

			if (inUsedMemory == true && (chunk->m_nextChunk == nullptr || chunk->m_nextChunk->IsHeader() || chunk->m_nextChunk->Used() == false))
			{
				file << ">|";
				inUsedMemory = false;
			}

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
				<< "\t| Used: " << (chunk->m_usedChunks != 0 ? "true" : "false")
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
			chunk = chunk->m_nextChunk;
			++n;
		}

	}
	file.close();
}

MemoryChunk* MemoryPool::FindSlotFor(uint32_t requiredChunks)
{
	uint32_t moves = 0u;

	while (m_cursor->m_avaliableContiguousChunks < requiredChunks
		|| m_cursor->Used() == true
		|| (m_cursor->m_previousChunk && m_cursor->m_previousChunk->Used() == false))
	{
		moves += MoveCursorToNextFreeSpace();
		if (moves > GetChunkCount())
			return nullptr;
	}

	return m_cursor;
}

uint32_t MemoryPool::ChunksToFit(uint32_t bytesOfSpace) const
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

uint32_t MemoryPool::MoveCursorToNextFreeSpace()
{
	uint32_t moves = 0u;
	moves += AdvanceCursor();
	while (m_cursor->Used() == true)
	{
		moves += AdvanceCursor();
		if (moves > GetChunkCount())
			return moves;
	}
	return moves;
}

uint32_t MemoryPool::AdvanceCursor()
{
	uint32_t moves = 0u;

	if (m_cursor->Used())
		moves = m_cursor->m_usedChunks;		
	else
		moves = m_cursor->m_avaliableContiguousChunks;

	m_cursor += moves - 1;
	m_cursor = m_cursor->m_nextChunk;

	if (m_cursor == nullptr)
		m_cursor = m_firstChunk;

	return moves;
}
