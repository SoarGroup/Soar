#include <iostream>
#include <iomanip>
#include "timer.h"
#include "common.h"

using namespace std;

const int prec = 2;

void timer_set::report(ostream &os) const {
	os << "Times reported in milliseconds" << endl << endl;
	
	table_printer p;
	p.set_precision(prec);
	p.set_scientific();
	p.add_row() << "name" << "count" << "total" << "mean" << "stdev" << "min" << "max" << "last";
	
	int ttl_count = 0;
	double ttl_total = 0.0;
	for (int i = 0; i < timers.size(); ++i) {
		timer *t = timers[i];
		p.add_row() << t->name << t->count << t->total;
		if (t->count > 0) {
			double mean = t->total / t->count;
			p << mean;
			if (!t->basic) {
				double stdev = sqrt(t->m2 / t->count);
				p << stdev << t->min << t->max << t->last;
			}
		}
		
		ttl_count += t->count;
		ttl_total += t->total;
	}
	
	p.add_row().add_row();
	p << "total" << ttl_count << ttl_total << ttl_total / ttl_count;
	p.print(os);
}
