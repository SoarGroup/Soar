#ifndef TIMER_H
#define TIMER_H

#include <vector>
#include <cmath>
#include "portability.h"
#include "misc.h"

class timer_set;

class timer {
public:
	timer(const std::string &name, bool basic) 
	: name(name), basic(basic), count(0), total(0), last(0), mn(0), min(INFINITY), max(0), m2(0)
	{}
	
#ifdef NO_SVS_TIMING
	inline void start() {}
	inline double stop() { return 0.0; }
#else
	inline void start() {
		t.start();
	}
	
	inline double stop() {
		t.stop();
		
		double elapsed = t.get_usec() / 1000.0; 
		last = elapsed;
		total += elapsed;
		count++;
		
		if (!basic) {
			min = std::min(min, elapsed);
			max = std::max(max, elapsed);
			
		  	// see http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#On-line_algorithm
			double delta = elapsed - mn;
			mn += delta / count;
			m2 += delta * (elapsed - mn);
		}
		
		return elapsed;
	}
#endif
	
private:
	soar_wallclock_timer t;
	
	std::string name;
	bool basic;
	
	int count;
	double total;
	double last;
	double mn;
	double min;
	double max;
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

class timer_set {
public:
	timer_set() {}
	
	~timer_set() {
		for (int i = 0; i < timers.size(); ++i) {
			delete timers[i];
		}
	}
	
	void add(const std::string &name, bool basic = false) {
		timers.push_back(new timer(name, basic));
	}
	
	timer &get(int i) {
		return *timers[i];
	}
	
	void start(int i) {
		timers[i]->start();
	}
	
	double stop(int i) {
		return timers[i]->stop();
	}
	
	void report(std::ostream &os) const;
	
private:
	std::vector<timer*> timers;
};

#endif
