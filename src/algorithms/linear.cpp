#include <iomanip>
#include <cmath>
#include <cassert>
#include <vector>
#include <sstream>
#include "linear.h"
#include "common.h"
#include "params.h"
#include "serialize.h"
#include "mat.h"

using namespace std;
using namespace Eigen;

/* Assume that X is already centered */
void pca(const_mat_view X, mat &comps) {
	JacobiSVD<mat> svd = X.jacobiSvd(Eigen::ComputeFullV);
	comps = svd.matrixV();
}

bool solve(const_mat_view X, const_mat_view Y, mat &C) {
	C = X.jacobiSvd(ComputeThinU | ComputeThinV).solve(Y);
	return is_normal(C);
}

/*
 Clean up input data to avoid instability, then perform weighted least
 squares regression. Cleaning consists of:
 
 1. Setting elements whose absolute values are smaller than ZERO_THRESH to 0
 2. collapsing all columns whose elements are identical into a single constant column.
*/
bool OLS(const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept) {
	mat X1(X.rows(), X.cols() + 1), Y1, C;
	vector<int> nonstatic;

	for (int i = 0; i < X.rows(); ++i) {
		for (int j = 0; j < X.cols(); ++j) {
			if (fabs(X(i, j)) < ZERO_THRESH) {
				X1(i, j) = 0.0;
			} else {
				X1(i, j) = X(i, j);
			}
		}
	}

	del_static_cols(X1, X.cols(), nonstatic);
	X1.conservativeResize(X.rows(), nonstatic.size() + 1);
	X1.rightCols(1).setConstant(1.0);
	
	if (w.size() == X.rows()) {
		cvec w2 = w.array().sqrt();
		X1.array().colwise() *= w2.array();
		Y1 = Y.array().colwise() * w2.array();
	} else {
		Y1 = Y;
	}
	
	if (!solve(X1, Y1, C)) {
		return false;
	}

	coefs.resize(X.cols(), Y.cols());
	coefs.setConstant(0);
	for (int i = 0; i < nonstatic.size(); ++i) {
		coefs.row(nonstatic[i]) = C.row(i);
	}
	intercept = C.bottomRows(1);
	return true;
}

bool ridge(const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept) {
	mat Z, V;
	if (w.size() == 0) {
		Z = X;
		V = Y;
	} else {
		assert(w.size() == X.rows());
		mat W = mat::Zero(w.size(), w.size());
		W.diagonal() = w;
		Z = W * X;
		V = W * Y;
	}
	
	rvec Zmean = Z.colwise().mean();
	rvec Vmean = V.colwise().mean();
	Z.rowwise() -= Zmean;
	V.rowwise() -= Vmean;
	
	mat A = Z.transpose() * Z;
	double lambda = RLAMBDA;
	for (int i = 0; i < A.cols(); ++i) {
		double inc = nextafter(A(i, i), INFINITY) - A(i, i);
		lambda = max(lambda, inc);
	}
	A.diagonal().array() += lambda;
	mat B = Z.transpose() * V;
	if (!solve(A, B, coefs)) {
		return false;
	}
	intercept = Vmean - (Zmean * coefs);
	return true;
}

/*
 Use Leave-one-out cross validation to determine number of components
 to use. This seems to choose numbers that are too low.
*/
void cross_validate(const_mat_view X, const_mat_view Y, const cvec &w, mat &beta, rvec &intercept) {
	int ndata = X.rows(), maxcomps = X.cols();
	
	mat X1(ndata - 1, X.cols()), Y1(ndata - 1, Y.cols());
	rvec errors = rvec::Zero(maxcomps);
	vector<mat> betas;
	vector<rvec> intercepts;
	vector<int> leave_out;
	int ntest = min(LOO_NTEST, ndata);
	
	mat components, projected, coefs, b;
	rvec inter;
	pca(X, components);
	
	leave_out.reserve(ndata);
	for (int i = 0; i < ndata; ++i) {
		leave_out.push_back(i);
	}
	random_shuffle(leave_out.begin(), leave_out.end());
	
	for (int i = 0; i < ntest; ++i) {
		int n = leave_out[i];
		if (n > 0) {
			X1.topRows(n) = X.topRows(n);
			Y1.topRows(n) = Y.topRows(n);
		}
		if (n < ndata - 1) {
			X1.bottomRows(ndata - n - 1) = X.bottomRows(ndata - n - 1);
			Y1.bottomRows(ndata - n - 1) = Y.bottomRows(ndata - n - 1);
		}
		projected = X1 * components;
		for (int j = 0; j < maxcomps; ++j) {
			OLS(projected.leftCols(j), Y1, w, coefs, inter);
			b = components.leftCols(i) * coefs;
			errors(j) += (X.row(n) * b + inter - Y.row(n)).array().abs().sum();
		}
	}
	int best = -1;
	for (int i = 0; i < maxcomps; ++i) {
		if (best == -1 || errors(best) > errors(i)) {
			best = i;
		}
	}
	projected = X * components;
	OLS(projected.leftCols(best), Y, cvec(), coefs, inter);
	beta = components.leftCols(best) * coefs;
	intercept = inter;
}

/*
 Choose the number of components to minimize prediction error for
 the training instances. Also prevent the beta vector from blowing up
 too much.
*/
bool min_train_error(const_mat_view X, const_mat_view Y, const cvec &w, mat &beta, rvec &intercept) {
	vector<mat> betas;
	vector<rvec> intercepts;
	double minerror;
	int bestn = -1;
	mat components, projected, coefs, b;
	rvec inter;
	
	pca(X, components);
	projected = X * components;
	for (int i = 0; i < projected.cols(); ++i) {
		if (!OLS(projected.leftCols(i), Y, w, coefs, inter)) {
			continue;
		}
		b = components.leftCols(i) * coefs;
		
		if (b.squaredNorm() > MAX_BETA_NORM) {
			break;
		}
		double error = ((X * b).rowwise() + inter - Y).squaredNorm();
		
		if (error < MODEL_ERROR_THRESH) {
			beta = b;
			intercept = inter;
			return true;
		}
		
		if (bestn < 0 || error < minerror) {
			beta = b;
			intercept = inter;
			minerror = error;
		}
	}
	return bestn != -1;
}

bool wpcr(const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept) {
	mat X1;
	rvec xmean = X.colwise().mean(), inter;
	X1 = X.rowwise() - xmean;
	if (!min_train_error(X1, Y, w, coefs, inter)) {
		return false;
	}
	intercept = inter - (xmean * coefs);
	return true;
}

LinearModel::LinearModel()
: isconst(true), error(INFINITY), refit(true), alg(0)
{
	timers.add("predict");
	timers.add("fit");
}

LinearModel::LinearModel(int alg) 
: isconst(true), error(INFINITY), refit(true), alg(alg)
{
	timers.add("predict");
	timers.add("fit");
}

LinearModel::LinearModel(const LinearModel &m)
: constvals(m.constvals), isconst(m.isconst),
  xtotals(m.xtotals), center(m.center), error(INFINITY), refit(true),
  coefs(m.coefs), intercept(m.intercept), alg(m.alg)
{
	timers.add("predict");
	timers.add("fit");
}

void LinearModel::init_fit(const_mat_view X, const_mat_view Y, const state_sig &sig, std::vector<int> &obj_map)
{
	isconst = true;
	for (int i = 1; i < Y.rows(); ++i) {
		if (Y.row(i) != Y.row(0)) {
			isconst = false;
			break;
		}
	}
	if (isconst) {
		constvals = Y.row(0);
		xdata.resize(0, 0);
		ydata.resize(0, Y.cols());
		return;
	}
	
	fit_sub(X, Y);
	
	// find relevant objects (with nonzero coefficients)
	int ndims = 0, ndata = X.rows(), nout = Y.cols();
	for(int i = 0; i < sig.size(); ++i) {
		for (int j = 0; j < sig[i].length; ++j) {
			if (coefs.row(sig[i].start + j).sum() > 0.0) {
				obj_map.push_back(i);
				ndims += sig[i].length;
				break;
			}
		}
	}
	
	// discard information about irrelevant objects
	mat coefs2(ndims, nout);
	xdata.resize(ndata, ndims);

	int i = 0;
	for (int j = 0; j < obj_map.size(); ++j) {
		int s = sig[obj_map[j]].start;
		int l = sig[obj_map[j]].length;
		coefs2.block(i, 0, l, nout) = coefs.block(s, 0, l, nout);
		xdata.get().block(0, i, ndata, l) = X.block(0, s, ndata, l);
		i += l;
	}
	coefs = coefs2;
	xtotals = xdata.get().colwise().sum();	
	ydata = Y;
}

int LinearModel::add_example(const rvec &x, const rvec &y, bool update_refit) {
	assert(xdata.cols() == x.size() && ydata.cols() == y.size());
	
	xdata.append_row(x);
	ydata.append_row(y);
	if (xdata.rows() == 1) {
		xtotals = x;
		center = xtotals;
		isconst = true;
		constvals = y;
		error = 0.0;
		if (update_refit) {
			refit = false;
		}
		return 0;
	}
	
	xtotals += x;
	center = xtotals / xdata.rows();
	if (isconst && y != constvals) {
		isconst = false;
		if (update_refit) {
			refit = true;
		}
	}
	
	if (!isconst && update_refit) {
		if (coefs.size() == 0) {
			refit = true;
			error = INFINITY;
		} else {
			rvec py = x * coefs + intercept;
			double e = pow(py(0) - y(0), 2);
			if (!refit) {
				/*
				 Only refit the model if the average error increases
				 significantly after adding the data point.
				*/
				double olderror = error / (xdata.rows() - 1);
				double newerror = (error + e) / xdata.rows();
				//if (newerror > MODEL_ERROR_THRESH || newerror > REFIT_MUL_THRESH * olderror)
				if (newerror > MODEL_ERROR_THRESH)
				{
					refit = true;
					cout << "Needs refit, old = " << olderror << " new = " << newerror << endl;
				}
			}
			error += e;
		}
	}
	return xdata.rows() - 1;
}

void LinearModel::del_example(int r) {
	rvec x = xdata.row(r);
	xdata.remove_row(r);
	ydata.remove_row(r);
	
	if (xdata.rows() == 0) {
		// handling of this case is questionable, make it better later
		isconst = true;
		constvals.fill(0.0);
		center.fill(0.0);
		return;
	}
	
	xtotals -= x;
	center = xtotals / xdata.rows();
	
	if (!isconst) {
		/* check if remaining data all have same y */
		isconst = true;
		constvals = ydata.row(0);
		for (int j = 0; j < ydata.rows(); ++j) {
			if (ydata.row(j) != constvals) {
				isconst = false;
				break;
			}
		}
		if (isconst) {
			error = 0.0;
		} else {
			update_error();
		}
	}
}

void LinearModel::update_error() {
	if (xdata.rows() == 0) {
		error = INFINITY;
	} else if (isconst) {
		error = 0.0;
	} else {
		if (coefs.size() == 0) {
			error = INFINITY;
		} else {
			mat PY = (xdata.get() * coefs).rowwise() + intercept;
			error = (ydata.get() - PY).squaredNorm();
		}
	}
}

void LinearModel::serialize(ostream &os) const {
	serializer(os) << alg << error << isconst << xtotals << center << constvals << intercept << xdata << ydata;
}

void LinearModel::unserialize(istream &is) {
	unserializer(is) >> alg >> error >> isconst >> xtotals >> center >> constvals >> intercept >> xdata >> ydata;
	fit();
}

bool LinearModel::predict(const rvec &x, rvec &y) {
	if (isconst) {
		y = constvals;
		return true;
	}
	if (refit) {
		fit();
	}
	if (coefs.size() == 0) {
		return false;
	}
	assert(coefs.rows() == x.size());
	y = x * coefs + intercept;
	return true;
}

bool LinearModel::predict(const_mat_view X, mat &Y) {
	function_timer t(timers.get(PREDICT_T));
	
	if (isconst) {
		Y.resize(X.rows(), constvals.size());
		Y.rowwise() = constvals;
		return true;
	}
	if (refit) {
		fit();
	}
	if (coefs.size() == 0) {
		return false;
	}
	assert(coefs.rows() == X.cols());
	Y = (X * coefs).rowwise() + intercept;
	return true;
}

bool LinearModel::fit() {
	function_timer t(timers.get(FIT_T));
	
	if (!isconst) {
		if (!fit_sub(xdata.get(), ydata.get())) {
			return false;
		}
	}
	update_error();
	refit = false;
	return true;
}

bool LinearModel::fit_sub(const_mat_view X, const_mat_view Y) {
	switch (alg) {
		case 0:
			return OLS(X, Y, cvec(), coefs, intercept);
		case 1:
			return ridge(X, Y, cvec(), coefs, intercept);
		case 2:
			return wpcr(X, Y, cvec(), coefs, intercept);
	}
	assert(false);
}

bool LinearModel::cli_inspect(int first_arg, const vector<string> &args, ostream &os) const {
	if (first_arg >= args.size()) {
		os << "error:    " << error << endl;
		bool success;
		if (isconst) {
			os << "constant: ";
			output_rvec(os, constvals) << endl;
		} else {
			os << "intercept: " << intercept << endl;
			os << "coefs:" << endl;
			for (int i = 0; i < coefs.rows(); ++i) {
				os << setw(4) << i << " ";
				output_rvec(os, coefs.row(i)) << endl;
			}
			return true;
		}
		os << "subqueries: timing train" << endl;
		return true;
	} else if (args[first_arg] == "timing") {
		timers.report(os);
		return true;
	} else if (args[first_arg] == "train") {
		for (int i = 0; i < xdata.rows(); ++i) {
			output_rvec(os, xdata.row(i)) << " : ";
			output_rvec(os, ydata.row(i)) << endl;
		}
		return true;
	}
	os << "unrecognized argument" << endl;
	return false;
}
