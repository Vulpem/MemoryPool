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

	PoolPtr<testStructSmall> small1 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "2- Small allocation(1)");

	PoolPtr<char> singleLetter = pool.Alloc<char>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "3- Char allocation");

	PoolPtr<testStructSmall> small2 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "4- Small allocation(2)");

	pool.Free(singleLetter);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "5- Char release");

	PoolPtr<testStructSmall> small3 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "6- Small allocation(3)");

	PoolPtr<testStructLarge> big1 = pool.Alloc<testStructLarge>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "7- Big allocation(1)");

	pool.Free(small2);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "8- Small release(2)");

	PoolPtr<testStructSmall> small4 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "9- Small allocation(4)");

	pool.Free(small3);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "10- Small release(3)");
	pool.Free(small4);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "10- Small release(4)");

	PoolPtr<testStructLarge> big2 = pool.Alloc<testStructLarge>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "11- Big allocation(2)");

	PoolPtr<testStructSmall> small5 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "12- Small allocation(5)");

	PoolPtr<testStructSmall> small6 = pool.Alloc<testStructSmall>();
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

void PoolTests::ComparativeRandomTests() const
{
	std::vector<int> seeds;
	std::chrono::steady_clock::time_point start;
	srand((unsigned int)time(nullptr));
	for (uint32_t n = 0; n < m_randomTestCount; n++)
		seeds.push_back(rand());

	long long poolQuickest = LLONG_MAX;
	long long poolSlowest = 0;
	long long poolAverage = 0;
	for (uint32_t n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		MemoryPool pool(m_chunkSize, m_chunkCount);
		start = Time::GetTime();
		PoolRandomAllocation(pool);
		long long time = Time::GetTimeDiference(start);
		if (time < poolQuickest)
			poolQuickest = time;
		if (time > poolSlowest)
			poolSlowest = time;
		poolAverage += time;
	}
	poolAverage /= m_randomTestCount;

	long long mallocQuickest = LLONG_MAX;
	long long mallocSlowest = 0;
	long long mallocAverage = 0;
	for (uint32_t n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		start = Time::GetTime();
		MallocRandomAllocation();
		long long time = Time::GetTimeDiference(start);
		if (time < mallocQuickest)
			mallocQuickest = time;
		if (time > mallocSlowest)
			mallocSlowest = time;
		mallocAverage += time;
	}
	mallocAverage /= m_randomTestCount;

	long long newQuickest = LLONG_MAX;
	long long newSlowest = 0;
	long long newAverage = 0;
	for (uint32_t n = 0; n < m_randomTestCount; n++)
	{
		srand(seeds[n]);
		start = Time::GetTime();
		NewRandomAllocation();
		long long time = Time::GetTimeDiference(start);
		if (time < newQuickest)
			newQuickest = time;
		if (time > newSlowest)
			newSlowest = time;
		newAverage += time;
	}
	newAverage /= m_randomTestCount;

	ReadWriteFile file(m_outputFile);
	file.Load();
	file.PushBackLine(std::string("-------------- CHAOTIC PERFORMANCE TEST --------------"));
	file.PushBackLine("This test will randomly free/allocate between " + std::to_string(m_chunkSize)
		+ " and " + std::to_string(4 * m_chunkSize) + " bytes every tick");
	file.PushBackLine("Tests ran for " + std::to_string(GetTestSteps()) + " ticks.");
	file.PushBackLine("Ran " + std::to_string(m_randomTestCount) + " tests.");
	file.PushBackLine("");
	file.PushBackLine("Pool   Slowest: " + std::to_string(poolSlowest)
		+ " \tQuickest: " + std::to_string(poolQuickest)
		+ " \tAverage: " + std::to_string(poolAverage));
	file.PushBackLine("Malloc Slowest: " + std::to_string(mallocSlowest)
		+ " \tQuickest: " + std::to_string(mallocQuickest)
		+ " \tAverage: " + std::to_string(mallocAverage));
	file.PushBackLine("New    Slowest: " + std::to_string(newSlowest)
		+ " \tQuickest: " + std::to_string(newQuickest)
		+ " \tAverage: " + std::to_string(newAverage));
	file.PushBackLine("");
	file.Save();
}

void PoolTests::ComparativeSimpleTests() const
{
	uint32_t numLectures = 1000;
	uint32_t numTestsPerLecture = 1000;

	struct small { byte a[32]; };
	struct medium { small a, b; };
	struct big { medium a, b; };

	std::chrono::steady_clock::time_point start;
	long long poolSlowest = 0;
	long long poolQuickest = LLONG_MAX;
	long long poolAverage = 0;
	for (uint32_t n = 0; n < numLectures; ++n)
	{
		MemoryPool pool(32, 10);
		start = Time::GetTime();
		for (uint32_t m = 0; m < numTestsPerLecture; ++m)
		{
			PoolPtr<small> ptr1 = pool.Alloc<small>(2);
			PoolPtr<medium> ptr2 = pool.Alloc<medium>();
			PoolPtr<big> ptr3 = pool.Alloc<big>();
			pool.Free(ptr1);
			pool.Free(ptr2);
			pool.Free(ptr3);
		}
		long long time = Time::GetTimeDiference(start);
		if (poolSlowest < time)
			poolSlowest = time;
		if (poolQuickest > time)
			poolQuickest = time;
		poolAverage += time;
	}
	poolAverage /= numLectures;

	long long mallocSlowest = 0;
	long long mallocQuickest = LLONG_MAX;
	long long mallocAverage = 0;
	for (uint32_t n = 0; n < numLectures; ++n)
	{
		start = Time::GetTime();
		for (uint32_t m = 0; m < numTestsPerLecture; ++m)
		{
			small* ptr1 = (small*)malloc(sizeof(small) * 2);
			medium* ptr2 = (medium*)malloc(sizeof(medium));
			big* ptr3 = (big*)malloc(sizeof(big));
			free(ptr1);
			free(ptr2);
			free(ptr3);
		}
		long long time = Time::GetTimeDiference(start);
		if (mallocSlowest < time)
			mallocSlowest = time;
		if (mallocQuickest > time)
			mallocQuickest = time;
		mallocAverage += time;
	}
	mallocAverage /= numLectures;

	long long newSlowest = 0;
	long long newQuickest = LLONG_MAX;
	long long newAverage = 0;
	for (uint32_t n = 0; n < numLectures; ++n)
	{
		start = Time::GetTime();
		for (uint32_t m = 0; m < numTestsPerLecture; ++m)
		{
			small* ptr1 = new small[2];
			medium* ptr2 = new medium;
			big* ptr3 = new big;
			delete[](ptr1);
			delete(ptr2);
			delete(ptr3);
		}
		long long time = Time::GetTimeDiference(start);
		if (newSlowest < time)
			newSlowest = time;
		if (newQuickest > time)
			newQuickest = time;
		newAverage += time;
	}
	newAverage /= numLectures;

	ReadWriteFile file(m_outputFile);
	file.Load();
	file.PushBackLine(std::string("-------------- SIMPLE PERFORMANCE TEST --------------"));
	file.PushBackLine("This test will allocate 64, 64 and 128 bytes of spaces and free them sequentially on every tick.");
	file.PushBackLine("Test ran for " + std::to_string(numTestsPerLecture) + " ticks.");
	file.PushBackLine("Ran " + std::to_string(numLectures) + " tests.");
	file.PushBackLine("");
	file.PushBackLine("Pool   Slowest: " + std::to_string(poolSlowest)
		+ " \tQuickest: " + std::to_string(poolQuickest)
		+ " \tAverage: " + std::to_string(poolAverage));
	file.PushBackLine("Malloc Slowest: " + std::to_string(mallocSlowest)
		+ " \tQuickest: " + std::to_string(mallocQuickest)
		+ " \tAverage: " + std::to_string(mallocAverage));
	file.PushBackLine("New    Slowest: " + std::to_string(newSlowest)
		+ " \tQuickest: " + std::to_string(newQuickest)
		+ " \tAverage: " + std::to_string(newAverage));
	file.PushBackLine("");
	file.Save();
}

void PoolTests::PoolRandomAllocation(MemoryPool& pool) const
{
	std::queue<PoolPtr<byte>> allocatedChunks;

	for (uint32_t n = 0u; n < GetTestSteps(); ++n)
	{
		uint32_t randomNumber = std::rand() % 8;
		//If the number is even, we'll allocate new memory
		if (randomNumber < 4  || n < GetTestSteps() / 1000u || allocatedChunks.empty())
		{
			PoolPtr<byte> newChunk = pool.Alloc((randomNumber+1) * m_chunkSize);
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

void PoolTests::MallocRandomAllocation() const
{
	std::queue<void*> allocatedMemory;

	for (uint32_t n = 0u; n < GetTestSteps(); ++n)
	{
		uint32_t randomNumber = std::rand() % 8;
		//If the number is even, we'll allocate new memory
		if (randomNumber < 4 || n < GetTestSteps() / 1000u || allocatedMemory.empty())
		{
			allocatedMemory.push(malloc(((size_t)(randomNumber) + 1) * 32));
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

void PoolTests::NewRandomAllocation() const
{
	std::queue<byte*> allocatedMemory;

	for (uint32_t n = 0u; n < GetTestSteps(); ++n)
	{
		uint32_t randomNumber = std::rand() % 8;
		//If the number is even, we'll allocate new memory
		if (randomNumber < 4 || n < GetTestSteps() / 1000u || allocatedMemory.empty())
		{
			allocatedMemory.push(new byte[((size_t)(randomNumber) + 1) * 32]);
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
