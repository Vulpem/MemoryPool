#ifndef __MEMPOOLTESTS
#define __MEMPOOLTESTS

#include <string>

class MemoryPool;

namespace PoolTests
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

	void RunAllTests();

	void ClearPreviousResultsFile();

	void PoolBasicFunctionality();

	void ComparativeTests();

	void PoolFixedAllocation(MemoryPool& pool);
	void PoolRandomAllocation(MemoryPool& pool);

	void MallocFixedAllocation();
	void MallocRandomAllocation();

	void NewFixedAllocation();
	void NewRandomAllocation();
};

#endif // !__MEMPOOLTESTS