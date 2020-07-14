#ifndef __MEMPOOLTESTS
#define __MEMPOOLTESTS

#include <string>

#define DEFAULT_CHUNK_SIZE 32
#define DEFAULT_CHUNK_COUNT 512
#define DEFAULT_TEST_COUNT 1000
#define DEFAULT_TEST_TICKS 1000

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
	PoolTests(uint32_t chunkSize, uint32_t chunkCount, uint32_t testCount = 1000u, uint32_t testTicks = 1000u);

	void RunAllTests() const;

	void InitResultsFile() const;

	void PoolBasicFunctionality() const;
	void PoolMultithreading() const;
	void ThreadTest(MemoryPool& pool) const;

	void ComparativeRandomTests() const;
	void ComparativeSimpleTests() const;

	void PoolRandomAllocation(MemoryPool& pool) const;
	void MallocRandomAllocation() const;
	void NewRandomAllocation() const;

	uint32_t m_testCount;
	uint32_t m_testTicks;
	std::string m_outputFile;

private:
	uint32_t m_chunkSize;
	uint32_t m_chunkCount;
};

#endif // !__MEMPOOLTESTS