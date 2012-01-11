#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <armadillo>
#include "lwr.h"
#include "nn.h"
#include "common.h"
#include "linear.h"
#include "soar_interface.h"

using namespace std;
using namespace arma;

const double INF = numeric_limits<double>::infinity();

void print_mat(const mat &X) {
	X.print("");
}

void print_vec(const vec &v) {
	v.print("");
}

void print_rowvec(const rowvec &v) {
	v.print("");
}

lwr::lwr(int nnbrs, const string &logpath, bool test)
: nnbrs(nnbrs), xsize(-1), ysize(-1), normalized(false), test(test)
{
	nn = new brute_nn(&xnorm);
	if (!logpath.empty()) {
		log = new ofstream(logpath.c_str());
	} else {
		log = NULL;
	}
}

lwr::~lwr() {
	log->close();
	delete log;
}

void lwr::learn(scene *scn, const floatvec &x, const floatvec &y, float dt) {
	if (xsize < 0) {
		xsize = x.size();
		ysize = y.size();
		xmin.resize(xsize);
		xmax.resize(xsize);
		xrange.resize(xsize);
		
		if (log) {
			(*log) << "@relation model" << endl;
			for (int i = 0; i < xsize; ++i) {
				(*log) << "@attribute x" << i << " real" << endl;
			}
			for (int i = 0; i < ysize; ++i) {
				(*log) << "@attribute y" << i << " real" << endl;
			}
			(*log) << "@data" << endl;
		}
	}
	
	if (log) {
		(*log) << x << y << endl;
	}
	
	if (test) {
		floatvec p(y.size());
		if (!predict(x, p)) {
			cerr << "NO PREDICTION" << endl;
		}
		cerr << "PREDICTION ERROR: " << p.dist(y) << endl;
	}
	
	examples.push_back(make_pair(x, y));
	if (examples.size() == 1) {
		xmin = x;
		xmax = x;
		xrange.zero();
	} else {
		for (int i = 0; i < xsize; ++i) {
			if (x[i] < xmin[i]) {
				xmin[i] = x[i];
				normalized = false;
			} else if (x[i] > xmax[i]) {
				xmax[i] = x[i];
				normalized = false;
			}
		}
	}
	
	if (normalized) {
		// otherwise just wait for renormalization
		xnorm.push_back((x - xmin) / xrange);
	}
}

void lwr::normalize() {
	vector<pair<floatvec, floatvec> >::iterator i;
	
	xrange = xmax;
	xrange -= xmin;
	xrange.replace(0.0, 1.0);  // can't have division by 0
	xnorm.clear();
	xnorm.reserve(examples.size());
	for (i = examples.begin(); i != examples.end(); ++i) {
		xnorm.push_back(i->first);
		xnorm.back() -= xmin;
		xnorm.back() /= xrange;
	}
}

bool lwr::predict(const floatvec &x, floatvec &y) {
	mat X, Y, Xd, Yd;
	rowvec xd, yd;
	vec d;
	vector<int> xdi, ydi;
	di_queue neighbors;
	timer tall, tnn, tslv;
	int i, j, k;

	tall.start();
	k = examples.size() > nnbrs ? nnbrs : examples.size();
	if (k == 0) {
		return false;
	}
	
	if (!normalized) {
		normalize();
		normalized = true;
	}
	
	floatvec xn((x - xmin) / xrange);
	
	tnn.start();
	nn->query(xn, k, neighbors);
	//cout << "NN:  " << tnn.stop() << endl;
	
	X.reshape(k, xsize);
	Y.reshape(k, ysize);
	d.reshape(k, 1);
	for(i = 0; i < k; ++i) {
		d(i) = neighbors.top().first;
		int ind = neighbors.top().second;
		neighbors.pop();
		floatvec &tx = examples[ind].first;
		floatvec &ty = examples[ind].second;
		for(j = 0; j < xsize; ++j) {
			X(i, j) = tx[j];
		}
		for(j = 0; j < ysize; ++j) {
			Y(i, j) = ty[j];
		}
	}
	
	vec w = sqrt(pow(d, -3));
	
	/* Any neighbor whose weight is infinity is close
	   enough to provide an exact solution.  If any
	   exist, take their average as the solution.  If we
	   don't do this the solve() will fail due to infinite
	   values in Z and V.
	*/
	rowvec closeavg = zeros<rowvec>(ysize);
	int nclose = 0;
	for (i = 0; i < w.n_elem; ++i) {
		if (w(i) == INF) {
			closeavg += Y.row(i);
			++nclose;
		}
	}
	if (nclose > 0) {
		for(i = 0; i < closeavg.n_elem; ++i) {
			y[i] = closeavg(i) / nclose;
		}
		return true;
	}
	
	remove_static(X, Xd, xdi);
	remove_static(Y, Yd, ydi);
	
	if (ydi.size() == 0) {
		// all neighbors are identical, use first as prediction
		for (i = 0; i < ysize; ++i) {
			y[i] = Y(0, i);
		}
		return true;
	}
	
	if (X.n_rows < xdi.size()) {
		// would result in underconstrained system
		return false;
	}
	
	xd = ones<rowvec>(1, xdi.size());
	for (i = 0; i < xdi.size(); ++i) {
		xd(i) = x[xdi[i]];
	}
	
	ridge(Xd, Yd, w, xd, yd);
	
	/* final answer is first (actually any will do) row of
	   Y with dynamic columns changed
	*/
	for (i = 0; i < ysize; ++i) {
		y[i] = Y(0, i);
	}
	for (i = 0; i < ydi.size(); ++i) {
		y[ydi[i]] = yd(i);
	}
	
	//cout << "ALL: " << tall.stop() << endl;
	return true;
}

int lwr::size() const {
	return examples.size();
}

bool lwr::load_file(const char *file) {
	ifstream f(file);
	string line;
	floatvec x(xsize), y(ysize);
	float dt;
	int linenum = 0;
	size_t p;
	while (getline(f, line)) {
		++linenum;
		if ((p = line.find('#')) != string::npos) {
			line.erase(p);
		}
		if (line.find_first_not_of(" \t\n") == string::npos) {
			continue;
		}
		stringstream ss(line);
		for(int i = 0; i < xsize; ++i) {
			if (!(ss >> x[i])) {
				cerr << "Error in file \"" << file << "\" on line " << linenum << endl;
				return false;
			}
		}
		for(int i = 0; i < ysize; ++i) {
			if (!(ss >> y[i])) {
				cerr << "Error in file \"" << file << "\" on line " << linenum << endl;
				return false;
			}
		}
		if (!(ss >> dt)) {
			cerr << "Error in file \"" << file << "\" on line " << linenum << endl;
			return false;
		}
		
		learn(NULL, x, y, dt);
	}
	return true;
}

model *_make_lwr_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	Symbol *input_root = NULL, *output_root = NULL, *attr;
	wme_list children;
	wme_list::iterator i;
	long nnbrs = 50;
	string attrstr, logpath, testval;
	wme *logwme, *testwme;
	bool test = false;
	
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		attr = si->get_wme_attr(*i);
		if (!si->get_val(attr, attrstr)) {
			continue;
		}
		if (attrstr == "num-neighbors") {
			if (!si->get_val(si->get_wme_val(*i), nnbrs)) {
				cerr << "WARNING: attribute num-neighbors does not have integer value, using default 50 neighbors" << endl;
			}
		}
	}
	
	if (si->find_child_wme(root, "log", logwme)) {
		si->get_val(si->get_wme_val(logwme), logpath);
	}
	if (si->find_child_wme(root, "test", testwme)) {
		si->get_val(si->get_wme_val(testwme), testval);
		if (testval == "t") {
			test = true;
		}
	}
	
	return new lwr(nnbrs, logpath, test);
}
