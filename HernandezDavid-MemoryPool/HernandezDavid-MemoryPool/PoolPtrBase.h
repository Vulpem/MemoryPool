#ifndef __POOLPTRBASE
#define __POOLPTRBASE

struct MemoryChunk;

class PoolPtrBase
{
public:
	PoolPtrBase(MemoryChunk* referencedChunk)
		: m_chunk(referencedChunk)
	{}

	bool IsValid() const;
protected:
	void* GetRawData();
	const void* GetRawData() const;
private:
	friend class MemoryPool;
	MemoryChunk* m_chunk;
};

#endif // !__POOLPTRBASE