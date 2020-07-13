#ifndef __MEASURE
#define __MEASURE

#include <chrono>

namespace Time
{
	std::chrono::steady_clock::time_point GetTime()
	{
		return std::chrono::steady_clock::now();
	}

	template<typename timeFrame = std::chrono::microseconds>
	long long GetTimeDiference(const std::chrono::steady_clock::time_point& start, const std::chrono::steady_clock::time_point& end)
	{
		return (std::chrono::duration_cast<timeFrame>(end - start)).count();
	}

	template<typename timeFrame = std::chrono::microseconds>
	long long GetTimeDiference(const std::chrono::steady_clock::time_point& start)
	{
		return GetTimeDiference<timeFrame>(start, GetTime());
	}

	template<typename F, typename ...Args>
	typename std::chrono::microseconds::rep Measure(F&& func, Args&&... args)
	{
		auto start = std::chrono::steady_clock::now();
		std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>
			(std::chrono::steady_clock::now() - start);
		return duration.count();
	}
}
#endif // !__MEASURE