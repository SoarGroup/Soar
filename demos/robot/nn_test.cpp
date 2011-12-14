#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <sys/time.h>

using namespace std;

int nn(const vector<vector<float>> &data, const vector<float> &q) {
	float bestdist;
	int best;
	for (int i = 0; i < data.size(); ++i) {
		float dist = 0.f;
		const vector<float> &v = data[i];
		for (int j = 0; j < v.size(); ++j) {
			dist += pow(v[j] - q[j], 2);
		}
		if (best < 0 || dist < bestdist) {
			best = i;
			bestdist = dist;
		}
	}
	return best;
}

class timer {
public:
	timer() {}
	
	void start() {
		gettimeofday(&t1, NULL);
	}
	
	double stop() {
		timeval t2, t3;
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
		return t3.tv_sec + t3.tv_usec / 1000000.0;
	}
	
	timeval t1;
};

int main() {
	ifstream file("10box.txt");
	string line;
	vector<vector<float>> data;
	
	while (getline(file, line)) {
		stringstream ss(line);
		float v;
		vector<float> instance;
		while (ss >> v) {
			instance.push_back(v);
		}
		data.emplace_back(move(instance));
	}
	
	timer t;
	t.start();
	for (int i = 0; i < 1000; ++i) {
		int q = rand() % data.size();
		int r = nn(data, data[q]);
	}
	cout << t.stop() << endl;
	return 0;
}