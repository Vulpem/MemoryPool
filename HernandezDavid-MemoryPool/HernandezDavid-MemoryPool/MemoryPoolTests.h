#ifndef __MEMPOOLTESTS
#define __MEMPOOLTESTS

#include "MemoryPool/MemoryPool.h"

#include <string>
#include <assert.h>

#define DEFAULT_CHUNK_SIZE 32
#define DEFAULT_CHUNK_COUNT 512
#define DEFAULT_TEST_COUNT 1000
#define DEFAULT_TEST_TICKS 1000

class MemoryPool;

class PoolTests
{
	enum Allocator : unsigned int
	{
		Pool = 0u,
		Malloc = 1u,
		New = 2u,
		END = 3u
	};

	struct testStructSmall
	{
		testStructSmall() : a{ 's','m','a','l','l' } {}
		char a[5];
	};

	struct testStructLarge
	{
		testStructLarge() :a{ 'b','i','g','_','t','e','s','t' } {}
		char a[8];
	};

public:
	PoolTests(uint32_t chunkSize, uint32_t chunkCount, uint32_t testCount = 1000u, uint32_t testTicks = 1000u);

	void RunAllTests() const;

	void InitResultsFile() const;

	void PoolBasicFunctionality() const;

	void ComparativeRandomTests() const;
	void ComparativeSimpleTests() const;

	template<class type>
	type* Alloc(MemoryPool& pool, Allocator allocatorType, uint32_t amount = 1) const;

	template<class type>
	void Free(MemoryPool& pool, Allocator allocatorType, type* ptr) const;

	uint32_t m_testCount;
	uint32_t m_testTicks;
	std::string m_outputFile;

private:
	uint32_t m_chunkSize;
	uint32_t m_chunkCount;
};

#endif // !__MEMPOOLTESTS

template<class type>
type* PoolTests::Alloc(MemoryPool& pool, PoolTests::Allocator allocatorType, uint32_t amount) const
{
	switch (allocatorType)
	{
	case PoolTests::Pool:
		return pool.Alloc<type>(amount);
	case PoolTests::Malloc:
		return (type*)malloc(sizeof(type) * amount);
	case PoolTests::New:
		return new type[amount];
	default:
		assert(false && "Undefined type"); break;
	}
	return nullptr;
}

template<class type>
void PoolTests::Free(MemoryPool& pool, PoolTests::Allocator allocatorType, type* ptr) const
{
	switch (allocatorType)
	{
	case PoolTests::Pool:
		pool.Free(ptr); break;
	case PoolTests::Malloc:
		free(ptr); break;
	case PoolTests::New:
		delete[] ptr; break;
	default:
		assert(false && "Undefined type"); break;
	}
}
