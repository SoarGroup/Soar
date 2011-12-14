#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <armadillo>
#include "balltree.h"

using namespace std;
using namespace arma;


inline double distsq(const rowvec &a, const rowvec &b) {
	return accu(pow(a - b, 2));
}

void naive_nn(const rowvec &q, int k, const vector<rowvec> &data, di_queue &nn) {
	double d;
	int i;
	
	for (i = 0; i < data.size(); ++i) {
		d = distsq(q, data[i]);
		if (nn.size() < k || (nn.size() > 0 && d < nn.top().first)) {
			nn.push(make_pair(d, i));
			if (nn.size() > k) {
				nn.pop();
			}
		}
	}
}

long usec_time() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000000 + t.tv_usec;
}

double time() {
	return usec_time() / 1.0e6;
}

int main(int argc, char *argv[]) {
	int ndim, npts, k, leafsize;
	long seed;
	double t, balltree_query_time, naive_query_time, nqueries;
	stringstream ss;
	
	if (argc != 6) {
		cerr << "usage: balltree_test <num dims> <num points> <k> <leaf size> <num queries>" << endl << endl
		     << "Checks correctness of ball tree knn queries against brute force queries and " << endl
		     << "prints timing information." << endl;
		return 1;
	}
	
	for (int i = 1; i < 6; ++i) {
		ss << argv[i] << " ";
	}
	
	if (!(ss >> ndim >> npts >> k >> leafsize >> nqueries)) {
		cerr << "error parsing arguments" << endl;
		return 1;
	}
	
	seed = usec_time();
	cout << "seed: " << seed << endl;
	srand(seed);
	
	vector<rowvec> data;
	for (int i = 0; i < npts; ++i) {
		rowvec v = randu<rowvec>(ndim);
		data.push_back(v);
	}
	
	t = time();
	balltree tree(ndim, leafsize, &data, vector<int>());
	cout << "Ball tree construction: " << time() - t << " secs" << endl;
	
	di_queue nn1, nn2;
	balltree_query_time = 0.0; naive_query_time = 0.0;
	for(int i = 0; i < nqueries; ++i) {
		rowvec q = randu<rowvec>(1, ndim);
		t = time();
		int pruned = tree.query(q, k, nn1);
		balltree_query_time += time() - t;
		cout << "pruned " << ((float) pruned) / npts << endl;
		t = time();
		naive_nn(q, k, data, nn2);
		naive_query_time += time() - t;
		
		while (nn1.size() > 0) {
			if (nn1.top().second != nn2.top().second) {
				q.print("mismatched query");
				return 1;
			}
			nn1.pop();
			nn2.pop();
		}
	}
	
	cout << "Average ball tree query: " << balltree_query_time / nqueries << endl;
	cout << "Average naive query: " << naive_query_time / nqueries << endl;
	return 0;
}
