#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include "common.h"

using namespace std;

logger LOG(std::cout);

const char *log_type_names[NUM_LOG_TYPES] = {
	"WARN",
	"ERROR",
	"CTRLDBG",
	"EMDBG",
	"SGEL",
	"FOILDBG"
};

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

void strip(string &s, const string &whitespace) {
	size_t begin = s.find_first_not_of(whitespace);
	if (begin == string::npos) {
		s.clear();
		return;
	}
	size_t end = s.find_last_not_of(whitespace) + 1;
	s = s.substr(begin, end - begin);
}

void sample(int k, int low, int high, std::vector<int> &output) {
	int range = high - low;
	assert(k <= range);
	output.resize(k);
	for (int i = 0; i < range; ++i) {
		if (i < k) {
			output[i] = low + i;
		} else {
			int r = rand() % (i + 1);
			if (r < k) {
				output[r] = low + i;
			}
		}
	}
}

ostream &histogram(const vector<double> &vals, int nbins, ostream &os) {
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
		LOG(WARN) << "All values identical (" << min << "), not drawing histogram" << endl;
		return os;
	}
	for (i = 0; i < vals.size(); ++i) {
		b = (int) ((vals[i] - min) / binsize);
		assert(b < counts.size());
		counts[b]++;
		if (counts[b] > maxcount) {
			maxcount = counts[b];
		}
	}
	hashes_per = 60.0 / maxcount;
	streamsize p = os.precision();
	os.precision(3);
	ios::fmtflags f = os.flags();
	os << scientific;
	for (i = 0; i < nbins; ++i) {
		os << min + binsize * i << "|";
		os << setfill('#') << setw((int) (hashes_per * counts[i])) << '/' << counts[i] << endl;
	}
	os.precision(p);
	os.flags(f);
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

bool parse_double(const string &s, double &v) {
	if (s.empty()) {
		return false;
	}
	
	char *end;
	v = strtod(s.c_str(), &end);
	if (*end != '\0') {
		return false;
	}
	return true;
}

bool parse_int(const string &s, int &v) {
	if (s.empty()) {
		return false;
	}
	
	char *end;
	v = strtol(s.c_str(), &end, 10);
	if (*end != '\0') {
		return false;
	}
	return true;
}
