#ifndef TIMER_H
#define TIMER_H

#include <vector>
#include <cmath>
#include <ctime>
#include "cliproxy.h"
#include "platform_specific.h"

class timer_set;

class timer {
public:

#ifdef NO_SVS_TIMING

	timer(const std::string &name, bool basic) {}
	inline void start() {}
	inline double stop() { return 0.0; }

#else

	timer(const std::string &name, bool basic) 
	: name(name), basic(basic), count(0), total(0), last(0), mn(0), m2(0)
	{}
	
	const std::string &get_name() const { return name; }
	
	void start() {
		start_time_ns = get_time_nanosecs();
	}
	
	long stop() {
		long end = get_time_nanosecs();
		
		last = end - start_time_ns;
		total += last;
		count++;
		
		if (!basic) {
			min = (count == 1 || last < min) ? last : min;
			max = (count == 1 || last > max) ? last : max;
			
		  	// see http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#On-line_algorithm
			double delta = last - mn;
			mn += delta / count;
			m2 += delta * (last - mn);
		}
		
		return last;
	}

#endif
	
private:
	long start_time_ns;
	
	std::string name;
	bool basic;
	
	int count;
	long total, last, min, max;
	double mn;
	double m2;
	
	friend class timer_set;
};
	
/*
 Create an instance of this class at the beginning of a
 function. The timer will stop regardless of how the function
 returns.
*/
class function_timer {
public:
	function_timer(timer &t) : t(t) { t.start(); }
	~function_timer() { t.stop(); }
	
private:
	timer &t;
};

class timer_set : public cliproxy {
public:
	timer_set() {
		set_help("Reports timing information.");
	}
	
	~timer_set() {
		for (int i = 0; i < timers.size(); ++i) {
			delete timers[i];
		}
	}
	
	timer &get_or_add(const char *name, bool basic = false) {
		for (int i = 0; i < timers.size(); ++i) {
			if (timers[i]->get_name() == name) {
				return *timers[i];
			}
		}
		timer *t = new timer(name, basic);
		timers.push_back(t);
		return *t;
	}
	
	timer &get(int i) {
		return *timers[i];
	}
	
	void start(int i) {
		timers[i]->start();
	}
	
	long stop(int i) {
		return timers[i]->stop();
	}
	
private:
	void proxy_use_sub(const std::vector<std::string> &args, std::ostream &os);
	
	std::vector<timer*> timers;
};

#endif
