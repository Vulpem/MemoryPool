#include "MemoryChunk.h"
#include "PoolAllocation.h"

bool PoolAllocation::IsValid() const
{
	return chunk != nullptr;
}

void* PoolAllocation::GetData() const
{
	if(chunk && chunk->m_used && chunk->m_usedChunks != 0)
		return chunk->m_data;
	return nullptr;
}
