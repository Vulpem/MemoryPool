#include "MemoryPool.h"
#include "ReadWriteFile.h"
#include "MemoryPoolTests.h"
#include "Measure.h"

#include <iostream>
#include <queue>
#include <assert.h>

#define TEST_ITERATIONS 10000u
#define RESULTS_FILE "MemoryPoolTests.txt"

void PoolTests::RunAllTests()
{
	ClearPreviousResultsFile();
	ComparativeTests();
	PoolBasicFunctionality();
}

void PoolTests::ClearPreviousResultsFile()
{
	ReadWriteFile file(RESULTS_FILE);
	file.Clear();
	file.Save();
}

void PoolTests::PoolBasicFunctionality()
{
	ReadWriteFile file(RESULTS_FILE);
	file.Load();
	file.PushBackLine("-------------- FUNCTIONALITY TEST -------------- ");
	file.Save();

	MemoryPool pool(sizeof(testStructSmall) * 6, sizeof(testStructSmall) - 2);

	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "1- Initial state");

	PoolAllocation small1 = pool.Alloc<testStructSmall>();
	testStructSmall* smallPtr = (testStructSmall*)small1.GetData();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "2- Small allocation(1)");

	PoolAllocation singleLetter = pool.Alloc<char>();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "3- Char allocation");

	PoolAllocation small2 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "4- Small allocation(2)");

	pool.Free(singleLetter);
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "5- Char release");

	PoolAllocation small3 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "6- Small allocation(3)");

	PoolAllocation big1 = pool.Alloc<testStructLarge>();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "7- Big allocation(1)");

	pool.Free(small2);
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "8- Small release(2)");

	PoolAllocation small4 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "9- Small allocation(4)");

	pool.Free(small3);
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE, "10- Small release(3)");
	pool.Free(small4);
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "10- Small release(4)");

	PoolAllocation big2 = pool.Alloc<testStructLarge>();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "11- Big allocation(2)");

	PoolAllocation small5 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE, "12- Small allocation(5)");

	PoolAllocation small6 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE, "13- Overflowing Small allocation(5)");

	pool.Clear();
	pool.DumpDetailedDebugChunksToFile(RESULTS_FILE,  "14- Clear");
}

void PoolTests::ComparativeTests()
{
	const unsigned int randomTestsAmount = 15;

	std::vector<long long> poolTimes;
	poolTimes.resize(randomTestsAmount);
	long long poolAverage = 0;
	for (int n = 0; n < randomTestsAmount; n++)
	{
		MemoryPool pool(TEST_ITERATIONS * 60u, 16u);
		poolTimes[n] = Measure(PoolRandomAllocation, pool);
		poolAverage += poolTimes[n];
	}
	poolAverage /= randomTestsAmount;

	std::vector<long long> mallocTimes;
	mallocTimes.resize(randomTestsAmount);
	long long mallocAverage = 0;
	for (int n = 0; n < randomTestsAmount; n++)
	{
		mallocTimes[n] = Measure(MallocRandomAllocation);
		mallocAverage += mallocTimes[n];
	}
	mallocAverage /= randomTestsAmount;

	std::vector<long long> newTimes;
	newTimes.resize(randomTestsAmount);
	long long newAverage = 0;
	for (int n = 0; n < randomTestsAmount; n++)
	{
		newTimes[n] = Measure(MallocRandomAllocation);
		newAverage += newTimes[n];
	}
	newAverage /= randomTestsAmount;

	ReadWriteFile file(RESULTS_FILE);
	file.Load();
	file.PushBackLine(std::string("-------------- PERFORMANCE TEST --------------"));
	file.PushBackLine("Time in microseconds");
	file.PushBackLine("");
	file.PushBackLine("Pool time average:   " + std::to_string(poolAverage) + "\tAll times: ");
	for (unsigned int n = 0; n < randomTestsAmount; n++)
	{
		file.AppendToLine(file.GetNumLines() - 1, "\t" + std::to_string(poolTimes[n]));
	}

	file.PushBackLine("Malloc time average: " + std::to_string(mallocAverage) + " \tAll times: ");
	for (unsigned int n = 0; n < randomTestsAmount; n++)
	{
		file.AppendToLine(file.GetNumLines() - 1, "\t" + std::to_string(mallocTimes[n]));
	}

	file.PushBackLine("New time average:    " + std::to_string(newAverage) + " \tAll times: ");
	for (unsigned int n = 0; n < randomTestsAmount; n++)
	{
		file.AppendToLine(file.GetNumLines() - 1, "\t" + std::to_string(newTimes[n]));
	}

	file.PushBackLine("");
	const float poolMallocDiff = (float)poolAverage / (float)mallocAverage;
	const float poolNewDiff = (float)poolAverage / (float)newAverage;

	file.PushBackLine("Pool takes " + std::to_string(poolMallocDiff)
		+ " as much time as to Malloc in average having done "
		+ std::to_string(randomTestsAmount) + " tests");
	file.PushBackLine("Pool takes " + std::to_string(poolNewDiff)
		+ " as much time as to New in average having done "
		+ std::to_string(randomTestsAmount) + " tests");
	file.Save();
}

void PoolTests::PoolFixedAllocation(MemoryPool& pool)
{
	uint32_t smallStructSize = sizeof(testStructSmall);
	uint32_t largeStructSize = sizeof(testStructLarge);

	std::vector<PoolAllocation> storedAllocations;
	storedAllocations.reserve(TEST_ITERATIONS);

	for (uint32_t n = 0; n < TEST_ITERATIONS; ++n)
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

	//MemoryPool pool(TEST_ITERATIONS * 100, 16);

	for (uint32_t n = 0u; n < TEST_ITERATIONS; ++n)
	{
		uint32_t randomNumber = std::rand() % 60;
		//If the number is even, we'll allocate new memory
		if (randomNumber % 2 == 0 || n < TEST_ITERATIONS / 50u || allocatedChunks.empty())
		{
			PoolAllocation newChunk = pool.Alloc(randomNumber + 10);
			if (newChunk.IsValid())
				allocatedChunks.push(newChunk);
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
	storedInstances.reserve(TEST_ITERATIONS);
	for (uint32_t n = 0; n < TEST_ITERATIONS; ++n)
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

	for (uint32_t n = 0u; n < TEST_ITERATIONS; ++n)
	{
		uint32_t randomNumber = std::rand() % 60;
		//If the number is even, we'll allocate new memory
		if (randomNumber % 2 == 0 || n < TEST_ITERATIONS / 50u || allocatedMemory.empty())
		{
			allocatedMemory.push(malloc(randomNumber));
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
	storedInstances.reserve(TEST_ITERATIONS);
	for (uint32_t n = 0; n < TEST_ITERATIONS; ++n)
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

	for (uint32_t n = 0u; n < TEST_ITERATIONS; ++n)
	{
		uint32_t randomNumber = std::rand() % 60;
		//If the number is even, we'll allocate new memory
		if (randomNumber % 2 == 0 || n < TEST_ITERATIONS / 50u || allocatedMemory.empty())
		{
			allocatedMemory.push(new byte[randomNumber]);
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