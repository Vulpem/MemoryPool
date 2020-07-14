#include "MemoryChunk.h"
#include "PoolPtrBase.h"

bool PoolPtrBase::IsValid() const
{
	return m_chunk && m_chunk->m_usedChunks != 0;
}

void* PoolPtrBase::GetRawData()
{
	if (IsValid())
		return m_chunk->m_data;
	return nullptr;
}

const void* PoolPtrBase::GetRawData() const
{
	if (IsValid())
		return m_chunk->m_data;
	return nullptr;
}