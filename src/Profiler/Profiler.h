#pragma once
#include "../../third_party/SDL3.xcframework/macos-arm64_x86_64/SDL3.framework/Headers/SDL_log.h"
#include <fstream>
#include <iosfwd>

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
		SDL_Log("Name: %s Duration: %lld us", r.name.c_str(), r.end - r.start);
		std::lock_guard<std::mutex> lockGuard(lock);
		if(profileCount++ > 0)
		{
			outputStream << ",";
		}
		std::string name = r.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		// outputStream << "\n{";
		// outputStream << "\"cat\":\"function\",";
	}
};