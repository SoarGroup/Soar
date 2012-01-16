#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <armadillo>
#include "nn.h"
#include "common.h"
#include "linear.h"
#include "soar_interface.h"
#include "scene.h"
#include "model.h"

using namespace std;
using namespace arma;

const double INF = numeric_limits<double>::infinity();

class lwr : public model {
public:
	lwr(int nnbrs, const string &logpath, const string &name)
	: model(name, "lwr"), nnbrs(nnbrs), xsize(-1), ysize(-1), normalized(false)
	{
		nn = new brute_nn(&xnorm);
		if (!logpath.empty()) {
			log = new ofstream(logpath.c_str());
		} else {
			log = NULL;
		}
		init();
	}
	
	~lwr() {
		finish();
		if (log) {
			log->close();
			delete log;
		}
	}
	
	void learn(const floatvec &x, const floatvec &y, float dt) {
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
	
	
	bool predict2(const floatvec &x, floatvec &y) {
		mat X, Y, Xd, Yd;
		rowvec xd, yd;
		vec d;
		vector<int> xdi, ydi;
		di_queue neighbors;
		timer tall, tnn;
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
		
		/*
		 Any neighbor whose weight is infinity is close enough
		 to provide an exact solution.	If any exist, take their
		 average as the solution.  If we don't do this the solve()
		 will fail due to infinite values in Z and V.
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
	
	bool predict(const floatvec &x, floatvec &y) {
		mat X, Y;
		vec d;
		di_queue neighbors;
		timer tall, tnn;
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
		
		/*
		 Any neighbor whose weight is infinity is close enough
		 to provide an exact solution.	If any exist, take their
		 average as the solution.  If we don't do this the solve()
		 will fail due to infinite values in Z and V.
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

		rowvec xv(x.size());
		for (i = 0; i < xv.n_cols; ++i) {
			xv(i) = x[i];
		}
		y[0] = pcr(X, Y, xv);
		return true;
	}
	
	int size() const {
		return examples.size();
	}
	
	void load(istream &is) {
		int nexamples;
		if (!(is >> xsize >> ysize >> nexamples)) {
			assert(false);
		}
		
		floatvec x(xsize), y(ysize);
		
		for (int i = 0; i < nexamples; ++i) {
			for(int j = 0; j < xsize; ++j) {
				if (!(is >> x[j])) {
					assert(false);
				}
			}
			for(int j = 0; j < ysize; ++j) {
				if (!(is >> y[j])) {
					assert(false);
				}
			}
			
			learn(x, y, 1);
		}
	}
	
	void save(ostream &os) const {
		vector<pair<floatvec, floatvec> >::const_iterator i;
		os << xsize << " " << ysize << " " << examples.size() << endl;
		for (i = examples.begin(); i != examples.end(); ++i) {
			for (int j = 0; j < xsize; ++j) {
				os << i->first[j] << " ";
			}
			for (int j = 0; j < ysize; ++j) {
				os << i->second[j] << " ";
			}
			os << endl;
		}
	}
	
	int get_input_size() const {
		return xsize;
	}
	
	int get_output_size() const {
		return ysize;
	}
	
private:
	void normalize() {
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
	
	int xsize, ysize, nnbrs;
	std::vector<std::pair<floatvec, floatvec> > examples;
	std::vector<floatvec> xnorm;
	floatvec xmin, xmax, xrange;
	bool normalized;
	nearest_neighbor *nn;
	std::ofstream *log;
};

model *_make_lwr_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	Symbol *input_root = NULL, *output_root = NULL, *attr;
	wme_list children;
	wme_list::iterator i;
	long nnbrs = 50;
	string attrstr, logpath;
	wme *logwme;
	
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
	
	return new lwr(nnbrs, logpath, name);
}
