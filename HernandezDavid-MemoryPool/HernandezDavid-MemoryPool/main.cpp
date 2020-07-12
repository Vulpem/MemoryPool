#include "MemoryPoolTests.h"

int main(int argc, char** argv)
{
	//unsigned int blockBytes = (argc >= 2 ?  std::stoi(argv[1]) : DEFAULT_BLOCKSIZE);
	//unsigned int numBlocks = (argc >= 3 ? std::stoi(argv[2]) : DEFAULT_NUMBLOCKS);

	PoolTests::RunAllTests();
	
	return 0;
}