#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
	static long long time_now();
	static void sleep(int milliseconds);
};

#endif

