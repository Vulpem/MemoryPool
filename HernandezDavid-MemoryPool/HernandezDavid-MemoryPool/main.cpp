#include "MemoryPoolTests.h"
#include <iostream>

int main(int argc, char** argv)
{
	unsigned int chunkBytes = DEFAULT_CHUNK_SIZE;
	unsigned int chunkCount = DEFAULT_CHUNK_COUNT;
	if (argc >= 3)
	{
		chunkBytes = std::stoi(argv[1]);
		chunkCount = std::stoi(argv[2]);
	}
	unsigned int testCount = (argc > 3 ? std::stoi(argv[3]) : DEFAULT_TEST_COUNT);
	unsigned int testTicks = (argc > 4 ? std::stoi(argv[4]) : DEFAULT_TEST_TICKS);

	if (argc == 2 || argc > 5)
	{
		std::cout << "Executable recieved " << argc - 1 << " arguments."
			<< std::endl << "Expecting either 0, 2, 3 or 4 args."
			<< std::endl << "Arg 1 (default:" << DEFAULT_CHUNK_SIZE << ")\t-Chunk size in bytes"
			<< std::endl << "Arg 2 (default:" << DEFAULT_CHUNK_COUNT << ")\t-Chunk count for the pool"
			<< std::endl << "Arg 3 (default:" << DEFAULT_TEST_COUNT << ")\t-Test count. How many tests to run"
			<< std::endl << "Arg 4 (default:" << DEFAULT_TEST_TICKS << ")\t-How many ticks (iterations) each test will do"
			<< std::endl;
	}
	else
	{
		PoolTests tests(chunkBytes, chunkCount, testCount, testTicks);
		tests.RunAllTests();
	}
	system("pause");
	return 0;
} 