#ifndef __MEMORYCHUNK
#define __MEMORYCHUNK

#include <cstdint>

struct MemoryChunk
{
	MemoryChunk(MemoryChunk&) = delete;
	MemoryChunk(void* data = nullptr)
		: m_data(data)
		, m_avaliableContiguousChunks(0u)
		, m_usedChunks(0u)
		, m_used(false)
		, m_nextChunk(nullptr)
		, m_previousChunk(nullptr)
	{}

	~MemoryChunk()
	{}

	void* m_data;
	uint32_t m_avaliableContiguousChunks;
	uint32_t m_usedChunks;
	bool m_used;

	MemoryChunk* m_nextChunk;
	MemoryChunk* m_previousChunk;
};

#endif // !__MEMORYCHUNK