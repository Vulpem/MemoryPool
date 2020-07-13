#include "MemoryPool.h"
#include "ReadWriteFile.h"
#include "MemoryPoolTests.h"
#include "Measure.h"

#include <iostream>
#include <queue>
#include <assert.h>

PoolTests::PoolTests(uint32_t chunkSize, uint32_t chunkCount, uint32_t randomTestCount)
	: m_randomTestCount(randomTestCount)
	, m_outputFile("MemoryPoolTestOutput.txt")
	, m_chunkSize(chunkSize)
	, m_chunkCount(chunkCount)
{}

void PoolTests::RunAllTests() const
{
	InitResultsFile();
	PoolBasicFunctionality();
	PoolFunctionsTests();
	ComparativeSimpleTests();
	ComparativeRandomTests();
}

void PoolTests::InitResultsFile() const
{
	ReadWriteFile file(m_outputFile);
	file.Clear();

#ifdef _DEBUG
	file.PushBackLine("Tests done in DEBUG");
#elif defined NDEBUG 
	file.PushBackLine("Tests done in RELEASE");
#else
	file.PushBackLine("Tests done in UNKOWN CONFIG");
#endif

#ifdef _WIN64
	file.AppendToLine(0, " in x64 target platform");
#elif defined WIN32
	file.AppendToLine(0, " in x32 target platform");
#else
	file.AppendToLine(0, " in UNKOWN target platform");
#endif

	file.PushBackLine("");
	file.Save();
}

void PoolTests::PoolBasicFunctionality() const
{
	ReadWriteFile file(m_outputFile);
	file.Load();
	file.PushBackLine("-------------- FUNCTIONALITY TEST -------------- ");
	file.Save();



	MemoryPool pool(3, 10);

	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "1- Initial state");

	PoolAllocation small1 = pool.Alloc<testStructSmall>();
	testStructSmall* smallPtr = (testStructSmall*)small1.GetData();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "2- Small allocation(1)");

	PoolAllocation singleLetter = pool.Alloc<char>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "3- Char allocation");

	PoolAllocation small2 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "4- Small allocation(2)");

	pool.Free(singleLetter);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "5- Char release");

	PoolAllocation small3 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "6- Small allocation(3)");

	PoolAllocation big1 = pool.Alloc<testStructLarge>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "7- Big allocation(1)");

	pool.Free(small2);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "8- Small release(2)");

	PoolAllocation small4 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "9- Small allocation(4)");

	pool.Free(small3);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "10- Small release(3)");
	pool.Free(small4);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "10- Small release(4)");

	PoolAllocation big2 = pool.Alloc<testStructLarge>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "11- Big allocation(2)");

	PoolAllocation small5 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "12- Small allocation(5)");

	PoolAllocation small6 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "13- Overflowing Small allocation(5)");

	pool.Free(big2);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "14-Big release(2)");

	pool.Free(big1);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "15-Big release(1)");

	pool.Free(small5);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "16-Small release(5)");

	pool.Free(small1);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "17-Small release(1)");

}

void PoolTests::PoolFunctionsTests() const
{
	MemoryPool pool(m_chunkSize, m_chunkCount);
	std::vector<PoolAllocation> allocations;

	long long cleanAlloc = Time::Measure([&pool]() { pool.Alloc(1); });
	for (uint32_t n = 1; n < m_chunkCount - 2; ++n)
	{
		allocations.push_back(pool.Alloc(1u));
	}
	long long almostFilledAlloc = Time::Measure([&pool]() { pool.Alloc(1); });
	long long lastAlloc = Time::Measure([&pool]() { pool.Alloc(1); });
	long long overflow = Time::Measure([&pool]() { pool.Alloc(1); });

	PoolAllocation allocationToFree = allocations[5];
	long long singleFree = Time::Measure([&pool, &allocationToFree](){pool.Free(allocationToFree); });

	allocationToFree = allocations[4];
	long long previousFree = Time::Measure([&pool, &allocationToFree]() {pool.Free(allocationToFree); });

	allocationToFree = allocations[6];
	long long nextFree = Time::Measure([&pool, &allocationToFree]() {pool.Free(allocationToFree); });

	allocationToFree = allocations[8];
	long long singleFree2 = Time::Measure([&pool, &allocationToFree]() {pool.Free(allocationToFree); });

	allocationToFree = allocations[7];
	long long centralFree = Time::Measure([&pool, &allocationToFree]() {pool.Free(allocationToFree); });

	//5 free
	long long allocatingOneOfReleased = Time::Measure([&pool]() { pool.Alloc(1); });
	//4 free
	allocations.push_back(pool.Alloc(1));
	//3 free
	allocations.push_back(pool.Alloc(1));
	//2 free
	pool.Free(allocations[allocations.size() - 2]);
	//3 free split
	long long enoughFreeSpaceButFragmented = Time::Measure([&pool]() { pool.Alloc(pool.GetChunkSize() * 3); });
}

void PoolTests::ComparativeRandomTests() const
{
	std::vector<int> seeds;
	std::chrono::steady_clock::time_point start;
	srand((unsigned int)time(nullptr));
	for (uint32_t n = 0; n < m_randomTestCount; n++)
		seeds.push_back(rand());

	std::vector<long long> poolTimes;
	poolTimes.resize(m_randomTestCount);
	long long poolAverage = 0;
	for (uint32_t n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		MemoryPool pool(m_chunkSize, m_chunkCount);
		start = Time::GetTime();
		PoolRandomAllocation(pool);
		poolTimes[n] = Time::GetTimeDiference(start);
		poolAverage += poolTimes[n];
	}
	poolAverage /= m_randomTestCount;

	std::vector<long long> mallocTimes;
	mallocTimes.resize(m_randomTestCount);
	long long mallocAverage = 0;
	for (uint32_t n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		start = Time::GetTime();
		MallocRandomAllocation();
		mallocTimes[n] = Time::GetTimeDiference(start);
		mallocAverage += mallocTimes[n];
	}
	mallocAverage /= m_randomTestCount;

	std::vector<long long> newTimes;
	newTimes.resize(m_randomTestCount);
	long long newAverage = 0;
	for (uint32_t n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		start = Time::GetTime();
		NewRandomAllocation();
		newTimes[n] = Time::GetTimeDiference(start);
		newAverage += newTimes[n];
	}
	newAverage /= m_randomTestCount;

	ReadWriteFile file(m_outputFile);
	file.Load();
	file.PushBackLine(std::string("-------------- RANDOM PERFORMANCE TEST --------------"));
	file.PushBackLine("Time in microseconds");
	file.PushBackLine("");
	file.PushBackLine("Pool time average:   " + std::to_string(poolAverage));
	file.PushBackLine(" - All times: ");
	for (unsigned int n = 0; n < m_randomTestCount; n++)
	{
		file.AppendToLine(file.GetNumLines() - 1, "\t" + std::to_string(poolTimes[n]));
	}

	file.PushBackLine("Malloc time average: " + std::to_string(mallocAverage));
	file.PushBackLine(" - All times: ");
	for (unsigned int n = 0; n < m_randomTestCount; n++)
	{
		file.AppendToLine(file.GetNumLines() - 1, "\t" + std::to_string(mallocTimes[n]));
	}

	file.PushBackLine("New time average:    " + std::to_string(newAverage));
	file.PushBackLine(" - All times: ");
	for (unsigned int n = 0; n < m_randomTestCount; n++)
	{
		file.AppendToLine(file.GetNumLines() - 1, "\t" + std::to_string(newTimes[n]));
	}

	file.PushBackLine("");
	const float poolMallocDiff = (float)poolAverage / (float)mallocAverage;
	const float poolNewDiff = (float)poolAverage / (float)newAverage;

	file.PushBackLine("Average between " + std::to_string(m_randomTestCount) + " tests, with "
		+ std::to_string(GetTestSteps()) + " randomly selected allocs/free each one. Chunks of "
		+ std::to_string(m_chunkSize) + " bytes");
	file.PushBackLine("Pool takes x" + std::to_string(poolMallocDiff)
		+ " times as Malloc in average");
	file.PushBackLine("Pool takes x" + std::to_string(poolNewDiff)
		+ " times as New in average");
	file.Save();
}

void PoolTests::ComparativeSimpleTests() const
{
	struct small { byte a[32]; };
	struct medium { small a, b; };
	struct big { medium a, b; };

	std::chrono::steady_clock::time_point start;
	long long poolSlowest = 0;
	long long poolQuickest = LLONG_MAX;
	for (uint32_t n = 0; n < 10000u; ++n)
	{
		MemoryPool pool(32, 10);
		start = Time::GetTime();
		PoolAllocation ptr1 = pool.Alloc<small>(2);
		PoolAllocation ptr2 = pool.Alloc<medium>();
		PoolAllocation ptr3 = pool.Alloc<big>();
		pool.Free(ptr1);
		pool.Free(ptr2);
		pool.Free(ptr3);
		long long time = Time::GetTimeDiference<std::chrono::nanoseconds>(start);
		if (poolSlowest < time)
			poolSlowest = time;
		if (poolQuickest > time)
			poolQuickest = time;
	}

	long long mallocSlowest = 0;
	long long mallocQuickest = LLONG_MAX;
	for (uint32_t n = 0; n < 10000u; ++n)
	{
		start = Time::GetTime();
		small* ptr1 = (small*)malloc(sizeof(small) * 2);
		medium* ptr2 = (medium*)malloc(sizeof(medium));
		big* ptr3 = (big*)malloc(sizeof(big));
		free(ptr1);
		free(ptr2);
		free(ptr3);
		long long time = Time::GetTimeDiference<std::chrono::nanoseconds>(start);
		if (mallocSlowest < time)
			mallocSlowest = time;
		if (mallocQuickest > time)
			mallocQuickest = time;
	}

	long long newSlowest = 0;
	long long newQuickest = LLONG_MAX;
	for (uint32_t n = 0; n < 10000u; ++n)
	{
		start = Time::GetTime();
		small* ptr1 = new small[2];
		medium* ptr2 = new medium;
		big* ptr3 = new big;
		delete[](ptr1);
		delete(ptr2);
		delete(ptr3);
		long long time = Time::GetTimeDiference<std::chrono::nanoseconds>(start);
		if (newSlowest < time)
			newSlowest = time;
		if (newQuickest > time)
			newQuickest = time;
	}

	ReadWriteFile file(m_outputFile);
	file.Load();
	file.PushBackLine(std::string("-------------- SIMPLE PERFORMANCE TEST --------------"));
	file.PushBackLine("");
	file.PushBackLine("Pool   Slowest: " + std::to_string(poolSlowest)
		+ " \tQuickest: " + std::to_string(poolQuickest));
	file.PushBackLine("Malloc Slowest: " + std::to_string(mallocSlowest)
		+ " \tQuickest: " + std::to_string(mallocQuickest));
	file.PushBackLine("New    Slowest: " + std::to_string(newSlowest)
		+ " \tQuickest: " + std::to_string(newQuickest));
	file.PushBackLine("");
	file.Save();
}

void PoolTests::PoolFixedAllocation(MemoryPool& pool) const
{
	uint32_t smallStructSize = sizeof(testStructSmall);
	uint32_t largeStructSize = sizeof(testStructLarge);

	std::vector<PoolAllocation> storedAllocations;
	storedAllocations.reserve(GetTestSteps());

	for (uint32_t n = 0; n < GetTestSteps(); ++n)
	{
		PoolAllocation id1 = pool.Alloc<testStructSmall>();
		PoolAllocation id2 = pool.Alloc<testStructSmall>();
		PoolAllocation id3 = pool.Alloc<testStructLarge>();
		pool.Free(id1);
		pool.Free(id2);

		storedAllocations.push_back(pool.Alloc<testStructLarge>());
		pool.Free(id3);
	}
	for (auto storedAllocation : storedAllocations)
	{
		pool.Free(storedAllocation);
	}
}

void PoolTests::PoolRandomAllocation(MemoryPool& pool) const
{
	std::queue<PoolAllocation> allocatedChunks;

	for (uint32_t n = 0u; n < GetTestSteps(); ++n)
	{
		uint32_t randomNumber = std::rand() % 8;
		//If the number is even, we'll allocate new memory
		if (randomNumber < 4  || n < GetTestSteps() / 1000u || allocatedChunks.empty())
		{
			PoolAllocation newChunk = pool.Alloc((randomNumber+1) * pool.GetChunkSize());
			if (newChunk.IsValid())
				allocatedChunks.push(newChunk);
			else
			{
				//Safeguard in case RNG gets real evil and decides to keep on allocating non-stop
				pool.Free(allocatedChunks.front());
				allocatedChunks.pop();
			}
		}
		//If the number is odd, we'll free some memory
		else
		{
			pool.Free(allocatedChunks.front());
			allocatedChunks.pop();
		}
	}
}

void PoolTests::MallocFixedAllocation() const
{
	uint32_t smallStructSize = sizeof(testStructSmall);
	uint32_t largeStructSize = sizeof(testStructLarge);

	std::vector<testStructLarge*> storedInstances;
	storedInstances.reserve(GetTestSteps());
	for (uint32_t n = 0; n < GetTestSteps(); ++n)
	{
		testStructSmall* id1 = (testStructSmall*)malloc(smallStructSize);
		testStructSmall* id2 = (testStructSmall*)malloc(smallStructSize);
		testStructSmall* id3 = (testStructSmall*)malloc(smallStructSize);

		testStructLarge* id4 = (testStructLarge*)malloc(largeStructSize);
		free(id1);
		free(id2);
		free(id3);

		storedInstances.push_back((testStructLarge*)malloc(largeStructSize));
		free(id4);
	}
	for (auto storedInstance : storedInstances)
	{
		free(storedInstance);
	}
}

void PoolTests::MallocRandomAllocation() const
{
	std::queue<void*> allocatedMemory;

	for (uint32_t n = 0u; n < GetTestSteps(); ++n)
	{
		uint32_t randomNumber = std::rand() % 8;
		//If the number is even, we'll allocate new memory
		if (randomNumber < 4 || n < GetTestSteps() / 1000u || allocatedMemory.empty())
		{
			allocatedMemory.push(malloc((randomNumber + 1) * 32));
		}
		//If the number is odd, we'll free some memory
		else
		{
			free(allocatedMemory.front());
			allocatedMemory.pop();
		}
	}

	while (allocatedMemory.empty() == false)
	{
		free(allocatedMemory.front());
		allocatedMemory.pop();
	}
}

void PoolTests::NewFixedAllocation() const
{
	uint32_t smallStructSize = sizeof(testStructSmall);
	uint32_t largeStructSize = sizeof(testStructLarge);

	std::vector<testStructLarge*> storedInstances;
	storedInstances.reserve(GetTestSteps());
	for (uint32_t n = 0; n < GetTestSteps(); ++n)
	{
		testStructSmall* id1 = new testStructSmall;
		testStructSmall* id2 = new testStructSmall;
		testStructSmall* id3 = new testStructSmall;

		testStructLarge* id4 = new testStructLarge;
		delete id1;
		delete id2;
		delete id3;

		storedInstances.push_back(new testStructLarge);
		delete id4;
	}
	for (auto storedInstance : storedInstances)
	{
		delete storedInstance;
	}
}

void PoolTests::NewRandomAllocation() const
{
	std::queue<byte*> allocatedMemory;

	for (uint32_t n = 0u; n < GetTestSteps(); ++n)
	{
		uint32_t randomNumber = std::rand() % 8;
		//If the number is even, we'll allocate new memory
		if (randomNumber < 4 || n < GetTestSteps() / 1000u || allocatedMemory.empty())
		{
			allocatedMemory.push(new byte[(randomNumber + 1) * 32]);
		}
		//If the number is odd, we'll free some memory
		else
		{
			delete [] allocatedMemory.front();
			allocatedMemory.pop();
		}
	}

	while (allocatedMemory.empty() == false)
	{
		delete[] allocatedMemory.front();
		allocatedMemory.pop();
	}
}

inline uint32_t PoolTests::GetTestSteps() const
{
	return m_chunkCount * 2;
}
