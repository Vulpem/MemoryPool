#ifndef __MEMPOOLTESTS
#define __MEMPOOLTESTS

#include <string>

#define DEFAULT_CHUNK_SIZE 32
#define DEFAULT_CHUNK_COUNT 512
#define DEFAULT_SIMPLE_TEST_COUNT 1000
#define DEFAULT_RANDOM_TEST_COUNT 1000
#define DEFAULT_TEST_TICKS 1000
#define DEFAULT_DUMP_OUTPUT_FILE "MemoryPoolDumps.txt"

class MemoryPool;

class PoolTests
{
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
	static void InitResultsFile();

	static void PoolBasicFunctionality();

	static void ComparativeRandomTests(uint32_t chunks, uint32_t chunkSize, uint32_t tests, uint32_t ticks);
	static void ComparativeSimpleTests(uint32_t chunks, uint32_t chunkSize, uint32_t tests, uint32_t ticks);

private:
	static void PoolRandomAllocation(MemoryPool& pool, uint32_t ticks, uint32_t chunks, uint32_t chunkSize);
	static void MallocRandomAllocation(uint32_t ticks, uint32_t chunks, uint32_t chunkSize);
	static void NewRandomAllocation(uint32_t ticks, uint32_t chunks, uint32_t chunkSize);
};

#endif // !__MEMPOOLTESTS