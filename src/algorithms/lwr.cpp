#include <iostream>
#include <cassert>
#include "lwr.h"
#include "nn.h"
#include "linear.h"

using namespace std;

rvec norm_vec(const rvec &v, const rvec &min, const rvec &range) {
	return ((v.array() - min.array()) / range.array()).matrix();
}

LWR::LWR(int nnbrs)
: nnbrs(nnbrs), xsz(-1), ysz(-1), normalized(false)
{}

void LWR::learn(const rvec &x, const rvec &y) {
	if (xsz < 0) {
		xsz = x.size();
		ysz = y.size();
		xmin.resize(xsz);
		xmax.resize(xsz);
		xrange.resize(xsz);
	}
	
	examples.push_back(make_pair(x, y));
	if (examples.size() == 1) {
		xmin = x;
		xmax = x;
		xrange.setZero();
	} else {
		for (int i = 0; i < xsz; ++i) {
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
		xnorm.push_back(norm_vec(x, xmin, xrange));
	}
}

bool LWR::predict(const rvec &x, rvec &y) {
	mat X, Y;

	int k = examples.size() > nnbrs ? nnbrs : examples.size();
	if (k == 0) {
		return false;
	}
	
	if (!normalized) {
		normalize();
		normalized = true;
	}
	
	rvec xn = norm_vec(x, xmin, xrange);
	
	vector<int> inds;
	rvec d(k);
	brute_nearest_neighbor(xnorm, xn, k, inds, d);
	
	X.resize(k, xsz);
	Y.resize(k, ysz);
	for(int i = 0; i < k; ++i) {
		X.row(i) = examples[inds[i]].first;
		Y.row(i) = examples[inds[i]].second;
	}
	
	rvec w = d.array().pow(-3).sqrt();
	
	/*
	 Any neighbor whose weight is infinity is close enough
	 to provide an exact solution.	If any exist, take their
	 average as the solution.  If we don't do this the solve()
	 will fail due to infinite values in Z and V.
	*/
	rvec closeavg = rvec::Zero(ysz);
	int nclose = 0;
	for (int i = 0; i < w.size(); ++i) {
		if (w(i) == INFINITY) {
			closeavg += Y.row(i);
			++nclose;
		}
	}
	if (nclose > 0) {
		for(int i = 0; i < closeavg.size(); ++i) {
			y[i] = closeavg(i) / nclose;
		}
		return true;
	}

	wpcr(X, Y, w, x, y);
	return true;
}

void LWR::load(istream &is) {
	int nexamples;
	if (!(is >> xsz >> ysz >> nexamples)) {
		assert(false);
	}
	
	rvec x(xsz), y(ysz);
	
	for (int i = 0; i < nexamples; ++i) {
		for(int j = 0; j < xsz; ++j) {
			if (!(is >> x[j])) {
				assert(false);
			}
		}
		for(int j = 0; j < ysz; ++j) {
			if (!(is >> y[j])) {
				assert(false);
			}
		}
		
		learn(x, y);
	}
}

void LWR::save(ostream &os) const {
	vector<pair<rvec, rvec> >::const_iterator i;
	os << xsz << " " << ysz << " " << examples.size() << endl;
	for (i = examples.begin(); i != examples.end(); ++i) {
		for (int j = 0; j < xsz; ++j) {
			os << i->first[j] << " ";
		}
		for (int j = 0; j < ysz; ++j) {
			os << i->second[j] << " ";
		}
		os << endl;
	}
}

void LWR::normalize() {
	vector<pair<rvec, rvec> >::iterator i;
	
	xrange = xmax - xmin;
	// can't have division by 0
	for (int i = 0; i < xrange.size(); ++i) {
		if (xrange[i] == 0.0) {
			xrange[i] = 1.0;
		}
	}
	xnorm.clear();
	xnorm.reserve(examples.size());
	for (i = examples.begin(); i != examples.end(); ++i) {
		xnorm.push_back(norm_vec(i->first, xmin, xrange));
	}
}
