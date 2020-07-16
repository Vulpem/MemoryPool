#include "MemoryPoolTests.h"
#include "External/getopt/getopt.h"
#include "Log.h"
#include <iostream>

int main(int argc, char** argv)
{
	int chunksToAllocate = DEFAULT_CHUNK_COUNT;
	int chunkSizeInBytes = DEFAULT_CHUNK_SIZE;
	int basicFunctionalityTest = -1;
	int simplePerfTestIterations = -1;
	int randomPerfTestIterations = -1;
	int ticksPerTest = DEFAULT_TEST_TICKS;
	int pauseAtEnd = 1;

	opterr = 1;

	static struct option longOptions[] =
	{/*
		{"chunks",			required_argument,	0,	'c'},
		{"bytes",			required_argument,	0,	'b'},
		{"functionality",	no_argument,		&basicFunctionalityTest, 1},
		{"simple",			optional_argument,	0,	's'},
		{"random",			optional_argument,	0,	'r'},
		{"ticks",			required_argument,	0,	't'},
	*/
		{0,0,0,0}
	};

	int optionIndex = 0;
	int c;
	
	try {
		while ((c = getopt_long(argc, argv, "ofc:b:t:s::r::p", longOptions, &optionIndex)) != -1)
		{
			switch (c)
			{
			case 0:
				longOptions[optionIndex];
				break;
			case 1: {
				int test = 0;
				break; }
			case 2: {
				int a = 0;
				break;
			}
			case 'c':
				 chunksToAllocate = std::stoi(optarg); 
				break;
			case 'b':
				chunkSizeInBytes = std::stoi(optarg);
				break;
			case 'f':
				basicFunctionalityTest = 1;
				break;
			case 's':
				simplePerfTestIterations = (optarg ? std::stoi(optarg) : DEFAULT_SIMPLE_TEST_COUNT);
				break;
			case 'r':
				randomPerfTestIterations = (optarg ? std::stoi(optarg) : DEFAULT_RANDOM_TEST_COUNT);
				break;
			case 't':
				ticksPerTest = std::stoi(optarg);
				break;
			case 'p':
				pauseAtEnd = 0;
				break;
			case 'o':
				Log::StoreLogsForSaving(true);
				break;
			case '?':
				if (optopt == 'c' || optopt == 'b' || optopt == 't')
					std::cout << "Option " << (char)optopt << " requires an argument." << std::endl;
				else if (isprint(optopt))
					std::cout << "Unknown option `" << (char)optopt << "'." << std::endl;
				else
					std::cout << "Unknown option character `" << (char)optopt << "'." << std::endl;
				return 1;
			default:
				break;
			}
		}
	}
	catch (std::exception& e)
	{
		std::cout << " Exception: " << e.what() << std::endl;
		std::cout << "Unexpected argument for '" << (char)optopt << "'" <<
			std::endl << "'" << optarg << "' is not a valid argument" << std::endl;
		return 1;
	}

	if (basicFunctionalityTest == -1 && simplePerfTestIterations == -1 && randomPerfTestIterations == -1)
	{
		basicFunctionalityTest = 1;
		simplePerfTestIterations = DEFAULT_SIMPLE_TEST_COUNT;
		randomPerfTestIterations = DEFAULT_RANDOM_TEST_COUNT;
	}

	std::cout << "- Chunks: " << chunksToAllocate
		<< std::endl << "- Chunk size in bytes: " << chunkSizeInBytes
		<< std::endl << "- Basic functionalty test " << (basicFunctionalityTest != 1
			? "won't be executed"
			: "will be executed")
		<< std::endl << "- Simple performance test ";
	if (simplePerfTestIterations != -1)
		std::cout << "will be executed " << simplePerfTestIterations << " times";
	else
		std::cout << "won't be executed";
	std::cout << std::endl << "- Random performance test ";
	if (randomPerfTestIterations != -1)
		std::cout << "will be executed " << randomPerfTestIterations << " times";
	else
		std::cout << "won't be executed";
	if (simplePerfTestIterations != -1 || randomPerfTestIterations != -1)
		std::cout << std::endl << "- Each performance test will have " << ticksPerTest << " ticks";
	std::cout << std::endl << "- Logs " << (Log::StoringLogsForSaving() ? "will" : "won't") << "be saved";
	std::cout << std::endl;

	PoolTests::InitResultsFile();
	if (basicFunctionalityTest == 1)
		PoolTests::PoolBasicFunctionality();
	if (simplePerfTestIterations > 0)
		PoolTests::ComparativeSimpleTests(chunksToAllocate, chunkSizeInBytes, simplePerfTestIterations, ticksPerTest);
	if (randomPerfTestIterations > 0)
		PoolTests::ComparativeRandomTests(chunksToAllocate, chunkSizeInBytes, randomPerfTestIterations, ticksPerTest);

	if (pauseAtEnd != 0)
		system("pause");

	return 0;
} 