#ifndef __MEMPOOLTESTS
#define __MEMPOOLTESTS

#include <string>

#define DEFAULT_CHUNK_SIZE 32
#define DEFAULT_CHUNK_COUNT 512

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
	PoolTests(uint32_t chunkSize, uint32_t chunkCount, uint32_t randomTestCount = 100);

	void RunAllTests();

	void InitResultsFile();

	void PoolBasicFunctionality();

	void ComparativeTests();

	void PoolFixedAllocation(MemoryPool& pool);
	void PoolRandomAllocation(MemoryPool& pool);

	void MallocFixedAllocation();
	void MallocRandomAllocation();

	void NewFixedAllocation();
	void NewRandomAllocation();

	uint32_t m_randomTestCount;
	std::string m_outputFile;

private:
	uint32_t m_chunkSize;
	uint32_t m_chunkCount;

	inline uint32_t GetTestSteps() const;
};

#endif // !__MEMPOOLTESTS