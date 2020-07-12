#include "MemoryChunk.h"
#include "PoolAllocation.h"

bool PoolAllocation::IsValid() const
{
	return chunk != nullptr && chunk->IsHeader();
}

void* PoolAllocation::GetData() const
{
	if(chunk && chunk->Used() && chunk->m_usedChunks != 0)
		return chunk->m_data;
	return nullptr;
}
