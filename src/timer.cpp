#include <iostream>
#include <iomanip>
#include "timer.h"

using namespace std;

const int w1 = 10;
const int w2 = w1 - 1;
const int prec = 2;

void timer_set::report(ostream &os) const {
	os << "Times reported in milliseconds" << endl << endl;
	int longest_name = 5;   // "total"
	for (int i = 0; i < timers.size(); ++i) {
		if (longest_name < timers[i]->name.size()) {
			longest_name = timers[i]->name.size();
		}
	}
	
	// header
	os << left << setw(longest_name) << "label" << right;
	os << setw(w1) << "count";
	os << setw(w1) << "total";
	os << setw(w1) << "mean";
	os << setw(w1) << "stdev";
	os << setw(w1) << "min";
	os << setw(w1) << "max";
	os << setw(w1) << "last" << endl;
	
	int ttl_count = 0;
	double ttl_total = 0.0;
	streamsize old_prec = os.precision();
	os.precision(prec);
	for (int i = 0; i < timers.size(); ++i) {
		timer *t = timers[i];
		
		os << setw(longest_name) << left << t->name << right;
		os << " " << setw(w2) << t->count;
		os << " " << setw(w2) << scientific << t->total;
		
		if (t->count == 0) {
			os << " " << setw(w2) << '-';
			if (!t->basic) {
				for (int i = 0; i < 4; ++i) {
					os << " " << setw(w2) << '-';
				}
			}
		} else {
			double mean = t->total / t->count;
			os << " " << setw(w2) << scientific << mean;
			
			if (!t->basic) {
				double stdev = sqrt(t->m2 / t->count);
				os << " " << setw(w2) << scientific << stdev;
				os << " " << setw(w2) << scientific << t->min;
				os << " " << setw(w2) << scientific << t->max;
				os << " " << setw(w2) << scientific << t->last;
			}
		}
		os << endl;
		
		ttl_count += t->count;
		ttl_total += t->total;
	}
	
	os << endl;
	os << setw(longest_name) << left << "total" << right;
	os << " " << setw(w2) << ttl_count;
	os << " " << setw(w2) << scientific << ttl_total;
	os << " " << setw(w2) << scientific << ttl_total / ttl_count << endl;
	os.precision(old_prec);
}
