#include "ReadWriteFile.h"
#include "Measure.h"
#include "MemoryPoolTests.h"

#include <iostream>
#include <queue>

PoolTests::PoolTests(uint32_t chunkSize, uint32_t chunkCount, uint32_t testCount, uint32_t testTicks)
	: m_testCount(testCount)
	, m_testTicks(testTicks)
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

	testStructSmall* small1 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "2- Small allocation(1)");

	char* singleLetter = pool.Alloc<char>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "3- Char allocation");

	testStructSmall* small2 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "4- Small allocation(2)");

	pool.Free(singleLetter);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "5- Char release");

	testStructSmall* small3 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "6- Small allocation(3)");

	testStructLarge* big1 = pool.Alloc<testStructLarge>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "7- Big allocation(1)");

	pool.Free(small2);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "8- Small release(2)");

	testStructSmall* small4 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "9- Small allocation(4)");

	pool.Free(small3);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "10- Small release(3)");
	pool.Free(small4);
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "10- Small release(4)");

	testStructLarge* big2 = pool.Alloc<testStructLarge>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile,  "11- Big allocation(2)");

	testStructSmall* small5 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "12- Small allocation(5)");

	testStructSmall* small6 = pool.Alloc<testStructSmall>();
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "13- Overflowing Small allocation(5)");

	pool.Free(big2);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "14-Big release(2)");

	pool.Free(big1);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "15-Big release(1)");

	pool.Free(small5);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "16-Small release(5)");

	pool.Free(small1);
	pool.DumpDetailedDebugChunksToFile(m_outputFile, "17-Small release(1)");

	file.Load(false);
	file.PushBackLine("Basic functionality working as expected.");
	file.Save();
}

void PoolTests::ComparativeRandomTests() const
{
	ReadWriteFile file(m_outputFile);
	file.Load();
	file.PushBackLine(std::string("-------------- CHAOTIC PERFORMANCE TEST --------------"));
	file.PushBackLine("Using a pool with " + std::to_string(m_chunkCount) + "  chunks of " + std::to_string(m_chunkSize) + " bytes each one.");
	file.PushBackLine("This test will randomly free/allocate between " + std::to_string(m_chunkSize)
		+ " and " + std::to_string(4 * m_chunkSize) + " bytes every tick");
	file.PushBackLine("Tests ran for " + std::to_string(m_testTicks) + " ticks.");
	file.PushBackLine("Ran " + std::to_string(m_testCount) + " tests.");
	file.PushBackLine("");

	std::vector<int> seeds;
	srand((unsigned int)time(nullptr));
	for (uint32_t n = 0; n < m_testCount; n++)
		seeds.push_back(rand());

	std::chrono::steady_clock::time_point start;

	MemoryPool pool(m_chunkSize, m_chunkCount);

	long long slowest[(unsigned int)Allocator::END];
	long long quickest[(unsigned int)Allocator::END];
	long long average[(unsigned int)Allocator::END];

	for (Allocator type = (Allocator)0u; type < Allocator::END; type = (Allocator)((unsigned int)type + 1))
	{
		slowest[type] = 0;
		quickest[type] = LLONG_MAX;
		average[type] = 0;
		for (uint32_t testN = 0; testN < m_testCount; testN++)
		{
			srand(seeds[testN]);

			start = Time::GetTime();
			std::queue<MP_byte*> allocations;

			for (uint32_t tickN = 0u; tickN < m_testTicks; ++tickN)
			{
				uint32_t randomNumber = std::rand() % 8;
				//If the number is even, we'll allocate new memory
				if (randomNumber < 4 || tickN < m_chunkCount / 4 || allocations.empty())
				{
					MP_byte* ptr = Alloc<MP_byte>(pool, type, (randomNumber + 1) * m_chunkSize);
					if(ptr != nullptr)
						allocations.push(ptr);
				}
				//If the number is odd, we'll free some memory
				else
				{
					Free(pool, type, allocations.front());
					allocations.pop();
				}
			}
			while (allocations.empty() == false)
			{
				Free(pool, type, allocations.front());
				allocations.pop();
			}
			long long time = Time::GetTimeDiference(start);
			if (time < quickest[type])
				quickest[type] = time;
			if (time > slowest[type])
				slowest[type] = time;
			average[type] += time;
		}
		average[type] /= m_testCount;


		switch (type) {
		case PoolTests::Pool:
			file.PushBackLine("Pool   "); break;
		case PoolTests::Malloc:
			file.PushBackLine("Malloc "); break;
		case PoolTests::New:
			file.PushBackLine("New    "); break;
		}

		file.AppendToLine(file.GetNumLines() - 1, "Slowest: " + std::to_string(slowest[type])
			+ "\tQuickest: " + std::to_string(quickest[type])
			+ "\tAverage: " + std::to_string(average[type]));
	}
	file.PushBackLine("");
	file.Save();
}

void PoolTests::ComparativeSimpleTests() const
{
	ReadWriteFile file(m_outputFile);
	file.Load();
	file.PushBackLine(std::string("-------------- SIMPLE PERFORMANCE TEST --------------"));
	file.PushBackLine("Using a pool with " + std::to_string(m_chunkCount) + "  chunks of " + std::to_string(m_chunkSize) + " bytes each one.");
	file.PushBackLine("This test will allocate 64, 64 and 128 bytes of spaces and free them sequentially on every tick.");
	file.PushBackLine("Test ran for " + std::to_string(m_testTicks) + " ticks.");
	file.PushBackLine("Ran " + std::to_string(m_testCount) + " tests.");
	file.PushBackLine("");

	struct small { MP_byte a[32]; }; //32 bytes structure
	struct medium { small a, b; }; //64 bytes structure
	struct big { medium a, b; }; //128 bytes structure

	long long slowest[(unsigned int)Allocator::END];
	long long quickest[(unsigned int)Allocator::END];
	long long average[(unsigned int)Allocator::END];
	MemoryPool pool(m_chunkSize, m_chunkCount);
	std::chrono::steady_clock::time_point start;

	for (Allocator type = (Allocator)0u; type < Allocator::END; type = (Allocator)((unsigned int)type + 1))
	{
		slowest[type] = 0;
		quickest[type] = LLONG_MAX;
		average[type] = 0;
		for (uint32_t testN = 0; testN < m_testCount; ++testN)
		{
			start = Time::GetTime();
			for (uint32_t tickN = 0; tickN < m_testTicks; ++tickN)
			{
				small* ptr1 = Alloc<small>(pool, type, 2u);
				medium* ptr2 = Alloc<medium>(pool, type);
				big* ptr3 = Alloc<big>(pool, type);
				Free(pool, type, ptr2);
				Free(pool, type, ptr1);
				Free(pool, type, ptr3);
			}
			long long time = Time::GetTimeDiference(start);

			if (slowest[type] < time)
				slowest[type] = time;
			if (quickest[type] > time)
				quickest[type] = time;
			average[type] += time;
		}
		average[type]/= m_testCount;

		switch (type) {
		case PoolTests::Pool:
			file.PushBackLine("Pool   "); break;
		case PoolTests::Malloc:
			file.PushBackLine("Malloc "); break;
		case PoolTests::New:
			file.PushBackLine("New    "); break;
		}
		file.AppendToLine(file.GetNumLines() - 1, "Slowest: " + std::to_string(slowest[type])
			+ "\tQuickest: " + std::to_string(quickest[type])
			+ "\tAverage: " + std::to_string(average[type]));

	}
	file.PushBackLine("");
	file.Save();
}