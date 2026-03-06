#pragma once
#include "Profiler.h"
#include <chrono>
class ProfileTimer
{
	ProfileResult result;
	bool stopped = false;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

public:
	ProfileTimer(const std::string & name);
	~ProfileTimer();
	void Start();
	void Stop();
	long long Elapsed();
};