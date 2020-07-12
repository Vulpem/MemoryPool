#ifndef __POOLALLOCATION
#define __POOLALLOCATION

struct MemoryChunk;

class PoolAllocation
{
public:
	PoolAllocation(MemoryChunk* referencedChunk = nullptr)
		: chunk(referencedChunk)
	{}
	bool IsValid() const;
	void* GetData() const;
private:
	friend class MemoryPool;
	MemoryChunk* chunk;
};

#endif // !__POOLALLOCATION