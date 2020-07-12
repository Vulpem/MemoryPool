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
		, m_nextChunk(nullptr)
		, m_previousChunk(nullptr)
	{}

	~MemoryChunk()
	{}

	bool Used() { return m_usedChunks != 0; }
	bool IsHeader() { return Used() && (m_previousChunk == nullptr || m_usedChunks >= m_previousChunk->m_usedChunks); }

	void* m_data;
	uint32_t m_avaliableContiguousChunks;
	uint32_t m_usedChunks;

	MemoryChunk* m_nextChunk;
	MemoryChunk* m_previousChunk;
};

#endif // !__MEMORYCHUNK