#include "Timer.h"

Timer::Timer()
{
	gettimeofday(&start_time,NULL);
}
double Timer::TimeElapsed() // return in usec
{
	gettimeofday(&end_time,NULL);
	time_diff = ((double) end_time.tv_sec*1e6   + (double)end_time.tv_usec) -  
	            ((double) start_time.tv_sec*1e6 + (double)start_time.tv_usec);
	return  time_diff;
}
Timer::~Timer()
{
}
void Timer::Reset()
{
	gettimeofday(&start_time,NULL);
}
void Timer::Synch(double period)
{
	double time_elapsed = this->TimeElapsed();
	if( time_elapsed < period*1e3)
		usleep((int)(period*1e3 -time_elapsed)); 
}
