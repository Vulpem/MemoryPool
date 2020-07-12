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

void PoolTests::RunAllTests()
{
	InitResultsFile();
	PoolBasicFunctionality();
	ComparativeTests();
}

void PoolTests::InitResultsFile()
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

void PoolTests::PoolBasicFunctionality()
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

void PoolTests::ComparativeTests()
{
	std::vector<int> seeds;
	srand((unsigned int)time(nullptr));
	for (int n = 0; n < m_randomTestCount; n++)
		seeds.push_back(rand());

	std::vector<long long> poolTimes;
	poolTimes.resize(m_randomTestCount);
	long long poolAverage = 0;
	for (int n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		MemoryPool pool(m_chunkSize, m_chunkCount);
		poolTimes[n] = Measure([this, &pool]() { PoolRandomAllocation(pool); });
		poolAverage += poolTimes[n];
	}
	poolAverage /= m_randomTestCount;

	std::vector<long long> mallocTimes;
	mallocTimes.resize(m_randomTestCount);
	long long mallocAverage = 0;
	for (int n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		mallocTimes[n] = Measure([this]() { MallocRandomAllocation(); });
		mallocAverage += mallocTimes[n];
	}
	mallocAverage /= m_randomTestCount;

	std::vector<long long> newTimes;
	newTimes.resize(m_randomTestCount);
	long long newAverage = 0;
	for (int n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		newTimes[n] = Measure([this]() { NewRandomAllocation(); });
		newAverage += newTimes[n];
	}
	newAverage /= m_randomTestCount;

	ReadWriteFile file(m_outputFile);
	file.Load();
	file.PushBackLine(std::string("-------------- PERFORMANCE TEST --------------"));
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

void PoolTests::PoolFixedAllocation(MemoryPool& pool)
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

void PoolTests::PoolRandomAllocation(MemoryPool& pool)
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

void PoolTests::MallocFixedAllocation()
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

void PoolTests::MallocRandomAllocation()
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

void PoolTests::NewFixedAllocation()
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

void PoolTests::NewRandomAllocation()
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
