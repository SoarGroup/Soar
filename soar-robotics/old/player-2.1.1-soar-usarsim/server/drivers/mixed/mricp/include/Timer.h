#ifndef TIMER_H_
#define TIMER_H_
#include <sys/time.h>
#include <iostream>
class Timer
{
	private:
		struct timeval start_time,end_time;
		double time_diff;
	public:
		Timer();
		double TimeElapsed(); // time elapsed in usec since last call
		void Reset(); 				// resets timer
		virtual ~Timer();
		/* Synchronize the loop within a period
		 * To use this u will have to initialize the timer
		 * reset the timer at the beginning of the loop
		 * and call the Synch function at the end of the loop
		 */
		void Synch(double period); // period should be in msec
};

#endif /*TIMER_H_*/
