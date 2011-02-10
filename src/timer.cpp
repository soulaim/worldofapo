#include "timer.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <ctime>

long long Timer::time_now()
{
#ifdef _WIN32
	return clock();
#else
	timeval t;
	gettimeofday(&t,NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
#endif
}

void Timer::sleep(int milliseconds)
{
#ifdef _WIN32
	Sleep(milliseconds);
#else
	timespec duration = { 0, 1000000 * milliseconds };
	nanosleep(&duration, 0);
#endif
}

