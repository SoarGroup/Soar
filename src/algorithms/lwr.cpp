#include <iostream>
#include <cassert>
#include "lwr.h"
#include "nn.h"
#include "linear.h"
#include "serialize.h"

using namespace std;

void norm_vec(const rvec &v, const rvec &min, const rvec &range, rvec &n) {
	n = ((v.array() - min.array()) / range.array()).matrix();
}

void LWR::data::serialize(ostream &os) const {
	serializer(os) << x << y << xnorm;
}

void LWR::data::unserialize(istream &is) {
	unserializer(is) >> x >> y >> xnorm;
}

LWR::LWR(int nnbrs, bool alloc)
: nnbrs(nnbrs), alloc(alloc), xsz(-1), ysz(-1), normalized(false)
{}

LWR::~LWR() {
	for (int i = 0; i < examples.size(); ++i) {
		if (alloc) {
			delete examples[i]->x;
			delete examples[i]->y;
		}
		delete examples[i];
	}
}

void LWR::learn(const rvec &x, const rvec &y) {
	if (xsz < 0) {
		xsz = x.size();
		ysz = y.size();
		xmin.resize(xsz);
		xmax.resize(xsz);
		xrange.resize(xsz);
	}
	
	data *d = new data;
	if (alloc) {
		d->x = new rvec(x);
		d->y = new rvec(y);
	} else {
		d->x = &x;
		d->y = &y;
	}
	examples.push_back(d);
	xnptrs.push_back(&d->xnorm);
	
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
		norm_vec(x, xmin, xrange, d->xnorm);
	}
}

bool LWR::predict(const rvec &x, rvec &y) {
	int k = examples.size() > nnbrs ? nnbrs : examples.size();
	if (k == 0) {
		return false;
	}
	
	if (!normalized) {
		normalize();
		normalized = true;
	}
	
	rvec xn;
	norm_vec(x, xmin, xrange, xn);
	
	vector<int> inds;
	rvec d(k);
	brute_nearest_neighbor(xnptrs, xn, k, inds, d);
	
	mat X(k, xsz);
	mat Y(k, ysz);
	for(int i = 0; i < k; ++i) {
		X.row(i) = *examples[inds[i]]->x;
		Y.row(i) = *examples[inds[i]]->y;
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

	mat coefs;
	rvec intercept;
	
	/*
	 Using non-regularized methods seems to cause problems, so I'm switching over
	 to ridge regression. Unfortunately I haven't figured out how to do weighted
	 ridge regression, so for now I'm just dropping the weights.
	
	linreg_d(FORWARD, X, Y, w, coefs, intercept);
	*/
	linreg_d(LASSO, X, Y, cvec(), coefs, intercept);
	y = x * coefs + intercept;
	return true;
}

void LWR::normalize() {
	xrange = xmax - xmin;
	// can't have division by 0
	for (int i = 0; i < xrange.size(); ++i) {
		if (xrange[i] == 0.0) {
			xrange[i] = 1.0;
		}
	}

	for (int i = 0; i < examples.size(); ++i) {
		norm_vec(*examples[i]->x, xmin, xrange, examples[i]->xnorm);
	}
}

void LWR::serialize(ostream &os) const {
	assert(alloc);  // it doesn't make sense to serialize points we don't own
	serializer(os) << alloc << nnbrs << xsz << ysz << xmin << xmax
	               << normalized << examples;
}

void LWR::unserialize(istream &is) {
	assert(alloc);
	unserializer(is) >> alloc >> nnbrs >> xsz >> ysz >> xmin >> xmax 
	                 >> normalized >> examples;
	xnptrs.clear();
	for (int i = 0; i < examples.size(); ++i) {
		xnptrs.push_back(&examples[i]->xnorm);
	}
}
