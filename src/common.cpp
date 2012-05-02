#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <vector>
#include <limits>
#include "common.h"

using namespace std;

void split(const string &s, const string &delim, vector<string> &fields) {
	int start, end = 0;
	while (end < s.size()) {
		start = s.find_first_not_of(delim, end);
		if (start == string::npos) {
			return;
		}
		end = s.find_first_of(delim, start);
		if (end == string::npos) {
			end = s.size();
		}
		fields.push_back(s.substr(start, end - start));
	}
}

ofstream& get_datavis() {
	static bool first = true;
	static ofstream f;
	if (first) {
		string path = get_option("datavis");
		if (path.empty() || access(path.c_str(), W_OK) < 0) {
			cout << "Datavis output to /dev/null" << endl;
			f.open("/dev/null");
		} else {
			cout << "Datavis output to " << path << endl;
			f.open(path.c_str());
			f << "CLEAR" << endl;
		}
		first = false;
	}
	return f;
}

//ostream &operator<<(ostream &os, const floatvec &v) {
//	copy(v.mem, v.mem + v.sz, ostream_iterator<float>(os, " "));
//	return os;
//}

ostream &operator<<(ostream &os, const namedvec &v) {
	string name;
	for (int i = 0; i < v.size(); ++i) {
		if (!v.get_name(i, name)) {
			assert(false);
		}
		os << name << " " << v.vals[i] << endl;
	}
	return os;
}

vec3 calc_centroid(const ptlist &pts) {
	ptlist::const_iterator i;
	int d;
	vec3 c = vec3::Zero();
	
	for (i = pts.begin(); i != pts.end(); ++i) {
		c += *i;
	}

	return c / pts.size();
}

vec3 project(const vec3 &v, const vec3 &u) {
	float m = u.squaredNorm();
	if (m == 0.) {
		return vec3::Zero();
	}
	return u * (v.dot(u) / m);
}

float dir_separation(const ptlist &a, const ptlist &b, const vec3 &u) {
	int counter = 0;
	ptlist::const_iterator i;
	vec3 p;
	float x, min = numeric_limits<float>::max(), max = -numeric_limits<float>::max();
	for (i = a.begin(); i != a.end(); ++i) {
		p = project(*i, u);
		x = p[0] / u[0];
		if (x < min) {
			min = x;
		}
	}
	for (i = b.begin(); i != b.end(); ++i) {
		p = project(*i, u);
		x = p[0] / u[0];
		if (x > max) {
			max = x;
		}
	}
	
	return max - min;
}

void histogram(const rvec &vals, int nbins) {
	assert(nbins > 0);
	float min = vals[0], max = vals[0], binsize, hashes_per;
	int i, b, maxcount = 0;
	vector<int> counts(nbins, 0);
	for (i = 1; i < vals.size(); ++i) {
		if (vals[i] < min) {
			min = vals[i];
		}
		if (vals[i] > max) {
			max = vals[i];
		}
	}
	binsize = (max - min) / (nbins - 1);
	if (binsize == 0) {
		cout << "All values identical (" << min << "), not drawing histogram" << endl;
		return;
	}
	for (i = 0; i < vals.size(); ++i) {
		b = (int) ((vals[i] - min) / binsize);
		assert(b < counts.size());
		counts[b]++;
		if (counts[b] > maxcount) {
			maxcount = counts[b];
		}
	}
	hashes_per = 72.0 / maxcount;
	streamsize p = cout.precision();
	cout.precision(4);
	for (i = 0; i < nbins; ++i) {
		cout << setfill(' ') << setw(5) << min + binsize * i << " - " << setw(5) << min + binsize * (i + 1) << "|";
		cout << setfill('#') << setw((int) (hashes_per * counts[i])) << '/' << counts[i] << endl;
	}
	cout.precision(p);
}

ostream& operator<<(ostream &os, const bbox &b) {
	os << b.min[0] << " " << b.min[1] << " " << b.min[2] << " " << b.max[0] << " " << b.max[1] << " " << b.max[2];
	return os;
}

string get_option(const string &key) {
	static map<string, string> options;
	static bool first = true;
	
	if (first) {
		char *s;
		if ((s = getenv("SVS_OPTS")) != NULL) {
			string optstr(s);
			vector<string> fields;
			split(optstr, ",", fields);
			for (int i = 0; i < fields.size(); ++i) {
				size_t p = fields[i].find_first_of(':');
				if (p == string::npos) {
					options[fields[i]] = "-";
				} else {
					options[fields[i].substr(0, p)] = fields[i].substr(p+1);
				}
			}
		}
		first = false;
	}
	return options[key];
}

#ifndef NDEBUG
void timer_set::start(const string &name) {
	times[name].start = clock();
	if (name.size() > longest_name) {
		longest_name = name.size();
	}
}

double timer_set::stop(const string &name) {
	clock_t t = clock();
	time_table::iterator i = times.find(name);
	assert(i != times.end());
	timer_data &d = i->second;
	double elapsed = (t - d.start) / (double) CLOCKS_PER_SEC;
	
	if (elapsed < d.min) {
		d.min = elapsed;
	}
	if (elapsed > d.max) {
		d.max = elapsed;
	}
	
  		// see http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#On-line_algorithm
	d.cycles++;
	double delta = elapsed - d.mean;
	d.mean += delta / d.cycles;
	d.m2 += delta * (elapsed - d.mean);
	
	return elapsed;
}

void timer_set::report(ostream &os) const {
	time_table::const_iterator i;
	// header
	os << left << setw(longest_name + 2) << "label" << right;
	os << setw(13) << "cycles";
	os << setw(13) << "total";
	os << setw(13) << "mean";
	os << setw(13) << "stdev";
	os << setw(13) << "min";
	os << setw(13) << "max" << endl;
	
	for (i = times.begin(); i != times.end(); ++i) {
		const timer_data &d = i->second;
		double total = d.mean * d.cycles;
		double stdev = sqrt(d.m2 / d.cycles);
		
		os << setw(longest_name + 2) << left << i->first << right;
		os << " " << setw(12) << d.cycles;
		os << " " << setw(12) << total;
		os << " " << setw(12) << d.mean;
		os << " " << setw(12) << stdev;
		os << " " << setw(12) << d.min;
		os << " " << setw(12) << d.max << endl;
	}
}
#endif
