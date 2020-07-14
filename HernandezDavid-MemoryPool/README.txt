Memory pool implementation
by Vulpem - David Hernandez

Link to repository:
github.com/Vulpem/MemoryPool

All memory pool related files are inside the "Memory Pool" folder.
Other files are for the purpose of testing/profiling only and not meant to be reused.



// --- Using this code
Launching the .exe with no arguments will use default values
It accepts either 0, 2, 3 or 4 args
First arg:	Chunk size in bytes (default 32)
Second arg:	Chunk count (default 512)
Third arg:	Test count, how many tests to run to average out (default 1000)
Fourth arg:	Tick ount, how many iterations or loops take place in a single test (default 1000)

Ex: HernandezDavid-MemoryPool.exe 32 512 1000 1000

Results will be printed into the console,
and also stored into "MemoryPoolTestOutput.txt" in the same folder as the .exe



// --- The two methods: "Cursor" / "Free Markers"
My first approach to was what i call the "FreeMarkers" method. This ensures
the pool has a pointer to any chunk preceeded by  used(allocated) chunks,
hence having a pointer to the "start" of every free group of chunks, to
quickly find slots big enough to allocate new memory.

The first implementation however was slower than malloc on both the
"simple test" (allocating and freeing the same amount of memory repeatedly)
and the "chaotic test" (randomly choose whether to allocate or free memory,
and how much).

That's why I changed my approach and  tried the simpler "Cursor" method,
thinking that a simpler, easier implementation would be faster.
This method simply has a "cursor" that points to a specific point of the pool
and moves forward until there is a space big enough to allocate the desired memory,
going back to the start when reaching the end.

The simplicity of the method allowed me to optimize big part of the Pool, erasing
recursions and unnecesary loops, until it managed to surpass malloc on the
"simple test", but not on the chaotic one.

All of this optimizations were later translated onto the "Free Marker" approach which
managed to do much better than the "cursor" approach on the chaotic test from the start
and, with some more love, surpassed even in the simple test, which had been the bane
of the "Free marker" method up until that moment, taking around x8 longer than the 
cursor method.

See below, how fast both methods were by the end compared to malloc:
_____________________________________________________
|Pool times	|	Release			|	Debug			|
| / Malloc	|Simple	|Chaotic	|Simple	|Chaotic	|
|-----------|-------|-----------|-------|-----------|
|FreeMarkers|0.06	|0.8		|23.4	|2.8		|
|Cursor		|0.3	|6.2		|4.5	|3.1		|
-----------------------------------------------------
Values are an average of Pool Times / Malloc times.
The smaller the value, the quicker it is compared to malloc.
Tests ran on a pool of 512 chunks of 32 bytes each one.



// --- "Free markers" changes
Most of the optimisations for the "Free markers" method have come by avoiding iterating over
the chunks and changing their contents; and by having all the chunks contiguous in memory,
removing the need for "previous" or "next" pointers and simply moving through memory.

As an example, at first all allocated used chunks were marked as "used", having to
iterate all of them:

for(int n = 0; n < chunksToAllocate; ++n)
	chunk->used = true;
	chunk = chunk->nextChunk;
	
By the end, however, thanks to a mixture of cleaner marker generation and other changes,
only the first and last ones need to be marked, and no iteration is required:

chunk->used = true;
(chunk + chunksToAllocate)->used = true;



// --- Next steps / TODO list
With more time, this is the features i'd like to implement/research:
- Detect illegal memory access
	Figure out how to detect illegal memory access. Right now, since all the pool memory 
	is associated with the process, allocating 1 byte of memory and using the pointer it
	yields to access the next 100 bytes won't cause an immediate crash, but by modifying
	that memory unexpected behavior may arise. A rudimentary (and kind of ugly) solution
	has been implemented to work exclusively in DEBUG, but there has to be a better way.
- Make the pool dynamic
	Right now, the speed of the pool lies in its contiguity in memory of all chunks
	and data. Figure out a way to increase / decrease the size of the pool dynamically
	without compromising too much its speed.
- Make working with the pool seamless
	Removing or making "PoolPtr" invisible,	by overwritting "new" operators or making
	PoolPtr behave more like a regular pointer. Although this is probably more
	"context dependant", depending on the requisites of the project/code.
- Make the pool multi-threading safe
	This has been implemented in a diferent branch, but it makes the pool around 2.7 times
	slower (even when not multithreading) to a point where it is slower than malloc and
	makes the pool irrelevant. Would need to find a quicker way of implementing it.

