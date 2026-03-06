#include "ProfileTimer.h"
#include <thread>

ProfileTimer::ProfileTimer(const std::string & name)
{
	result.name = name;
	Start();
}
ProfileTimer::~ProfileTimer()
{
	Stop();
}
void ProfileTimer::Start()
{
	static long long lastStartTime = 0;
	startTime = std::chrono::high_resolution_clock::now();
	result.start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
	// if it's the same as the previous start time, add 1 to it
	result.start += (result.start == lastStartTime) ? 1 : 0;

	lastStartTime = result.start;
	stopped = false;
}

void ProfileTimer::Stop()
{
	using namespace std::chrono;
	if(stopped)
	{
		return;
	}
	stopped = true;

	auto etp = high_resolution_clock::now();
	result.start = time_point_cast<microseconds>(startTime).time_since_epoch().count();
	result.end = time_point_cast<microseconds>(etp).time_since_epoch().count();
	result.threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
	Profiler::Instance().WriteProfile(result);
}
