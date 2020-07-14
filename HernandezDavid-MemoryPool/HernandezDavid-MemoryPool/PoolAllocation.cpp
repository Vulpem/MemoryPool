#include "MemoryChunk.h"
#include "PoolAllocation.h"

bool PoolAllocation::IsValid() const
{
	return chunk != nullptr && chunk->m_usedChunks != 0;
}

void* PoolAllocation::GetData() const
{
	if(IsValid())
		return chunk->m_data;
	return nullptr;
}
