#include <iostream>
#include <iomanip>
#include "timer.h"
#include "common.h"

using namespace std;

const int prec = 2;

double msec(long t) {
	return t / 1.0e6;
}

void timer_set::proxy_use_sub(const vector<string> &args, ostream &os) {
	os << "Times reported in milliseconds" << endl << endl;
	
	table_printer p;
	p.set_precision(prec);
	p.set_scientific(true);
	p.add_row() << "name" << "count" << "total" << "mean" << "stdev" << "min" << "max" << "last";
	
	int ttl_count = 0;
	double ttl_total = 0.0;
	for (int i = 0; i < timers.size(); ++i) {
		timer *t = timers[i];
		p.add_row() << t->name << t->count << msec(t->total);
		if (t->count > 0) {
			double mean = msec(t->total) / t->count;
			p << mean;
			if (!t->basic) {
				double stdev = sqrt(t->m2 / t->count);
				p << stdev << msec(t->min) << msec(t->max) << msec(t->last);
			}
		}
		
		ttl_count += t->count;
		ttl_total += msec(t->total);
	}
	
	p.add_row().add_row();
	p << "total" << ttl_count << ttl_total << ttl_total / ttl_count;
	p.print(os);
}
