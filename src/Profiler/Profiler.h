#pragma once
#include <fstream>
#include <iosfwd>

#define PROFILING 0
#ifdef PROFILING
#	define PROFILE_SCOPE(name) ProfileTimer timer##__LINE__(name)
#	define PROFILE_FUNCTION() PROFILE_SCOPE(__func__)
#else
#	define PROFILE_SCOPE(name)
#endif

struct ProfileResult
{
	std::string name = "Default";
	long long start = 0;
	long long end = 0;
	size_t threadId = 0;
};

class Profiler
{
	std::string outputFile = "results.json";
	size_t profileCount = 0;
	std::ofstream outputStream;
	std::mutex lock;

	Profiler()
	{
		outputStream = std::ofstream(outputFile);
		WriteHeader();
	}

	~Profiler()
	{
		WriteFooter();
	}

	void WriteHeader()
	{
		outputStream << "{\"otherData\": {}, \"traceEvents\":[";
	}
	void WriteFooter()
	{
		outputStream << "]}";
	}

public:
	static Profiler & Instance()
	{
		static Profiler instance;
		return instance;
	}

	void WriteProfile(const ProfileResult & r)
	{
		std::lock_guard<std::mutex> lockGuard(lock);
		if(profileCount++ > 0)
		{
			outputStream << ",";
		}
		std::string name = r.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		outputStream << std::format(
			R"({{"cat":"function","name":"{}","ph":"X","ts":{},"dur":{},"pid":0,"tid":{}}})", r.name, r.start, r.end - r.start, r.threadId
		);
	}
};