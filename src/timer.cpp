#include <iostream>
#include <iomanip>
#include "timer.h"

using namespace std;

void timer_set::report(ostream &os) const {
	vector<timer>::const_iterator i;
	int longest_name = 5;   // "total"
	for (i = timers.begin(); i != timers.end(); ++i) {
		if (longest_name < i->name.size()) {
			longest_name = i->name.size();
		}
	}
	
	// header
	os << left << setw(longest_name + 2) << "label" << right;
	os << setw(13) << "count";
	os << setw(13) << "total";
	os << setw(13) << "mean";
	os << setw(13) << "stdev";
	os << setw(13) << "min";
	os << setw(13) << "max";
	os << setw(13) << "last" << endl;
	
	int ttl_count = 0;
	double ttl_total = 0.0;
	for (i = timers.begin(); i != timers.end(); ++i) {
		double mean = i->total / i->count;
		os << setw(longest_name + 2) << left << i->name << right;
		os << " " << setw(12) << i->count;
		os << " " << setw(12) << i->total;
		os << " " << setw(12) << mean;
		
		if (!i->basic) {
			double stdev = sqrt(i->m2 / i->count);
			os << " " << setw(12) << stdev;
			os << " " << setw(12) << i->min;
			os << " " << setw(12) << i->max;
			os << " " << setw(12) << i->last;
		}
		os << endl;
		
		ttl_count += i->count;
		ttl_total += i->total;
	}
	
	os << endl;
	os << setw(longest_name + 2) << left << "total" << right;
	os << " " << setw(12) << ttl_count;
	os << " " << setw(12) << ttl_total;
	os << " " << setw(12) << ttl_total / ttl_count << endl;
}
