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
		, m_chunkN(0u)
	{}

	~MemoryChunk()
	{}

	inline bool IsUsed() const { return m_used; }
	inline bool IsHeader() const { return IsUsed() && m_usedChunks != 0; }

	void* m_data;
	uint32_t m_avaliableContiguousChunks;
	uint32_t m_usedChunks;

	uint32_t m_chunkN;
};

#endif // !__MEMORYCHUNK