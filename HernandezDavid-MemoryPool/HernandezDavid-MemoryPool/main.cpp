#include "MemoryPoolTests.h"

int main(int argc, char** argv)
{
	unsigned int chunkBytes = (argc >= 2 ?  std::stoi(argv[1]) : DEFAULT_CHUNK_SIZE);
	unsigned int chunkCount = (argc >= 3 ? std::stoi(argv[2]) : DEFAULT_CHUNK_COUNT);

	PoolTests tests(chunkBytes, chunkCount);
	tests.RunAllTests();
	
	return 0;
} 