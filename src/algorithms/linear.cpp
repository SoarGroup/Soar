#include <iomanip>
#include <cmath>
#include <cassert>
#include <vector>
#include <sstream>
#include "linear.h"
#include "common.h"
#include "params.h"

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
bool solve2(const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept) {
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
	
	/*
	if (!solve(X1, Y1, C)) {
		return false;
	}
	*/
	ridge2(X1, Y1, C);

	coefs.resize(X.cols(), Y.cols());
	coefs.setConstant(0);
	for (int i = 0; i < nonstatic.size(); ++i) {
		coefs.row(nonstatic[i]) = C.row(i);
	}
	intercept = C.bottomRows(1);
	return true;
}

void ridge(const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs) {
	mat W = mat::Zero(w.size(), w.size());
	W.diagonal() = w;
	mat Z = W * X;
	mat V = W * Y;
	mat Zcenter = Z.rowwise() - Z.colwise().mean();
	mat Vcenter = V.rowwise() - V.colwise().mean();
	mat A = Zcenter.transpose() * Zcenter;
	double lambda = RLAMBDA;
	for (int i = 0; i < A.cols(); ++i) {
		double inc = nextafter(A(i, i), INFINITY) - A(i, i);
		lambda = max(lambda, inc);
	}
	A.diagonal().array() += lambda;
	mat B = Z.transpose() * Vcenter;
	solve(A, B, coefs);
}

void ridge2(const_mat_view X, const_mat_view Y, mat &coefs) {
	mat Xcenter = X.rowwise() - X.colwise().mean();
	mat Ycenter = Y.rowwise() - Y.colwise().mean();
	mat A = Xcenter.transpose() * Xcenter;
	double lambda = RLAMBDA;
	for (int i = 0; i < A.cols(); ++i) {
		double inc = nextafter(A(i, i), INFINITY) - A(i, i);
		lambda = max(lambda, inc);
	}
	A.diagonal().array() += lambda;
	mat B = X.transpose() * Ycenter;
	solve(A, B, coefs);
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
			solve2(projected.leftCols(j), Y1, w, coefs, inter);
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
	solve2(projected.leftCols(best), Y, cvec(), coefs, inter);
	beta = components.leftCols(best) * coefs;
	intercept = inter;
}

/*
 Choose the number of components to minimize prediction error for
 the training instances. Also prevent the beta vector from blowing up
 too much.
*/
void min_train_error(const_mat_view X, const_mat_view Y, const cvec &w, mat &beta, rvec &intercept) {
	vector<mat> betas;
	vector<rvec> intercepts;
	double minerror;
	int bestn = -1;
	mat components, projected, coefs, b;
	rvec inter;
	
	pca(X, components);
	projected = X * components;
	for (int i = 0; i < projected.cols(); ++i) {
		solve2(projected.leftCols(i), Y, w, coefs, inter);
		b = components.leftCols(i) * coefs;
		
		if (b.squaredNorm() > MAX_BETA_NORM) {
			break;
		}
		double error = ((X * b).rowwise() + inter - Y).squaredNorm();
		
		if (error < MODEL_ERROR_THRESH) {
			beta = b;
			intercept = inter;
			return;
		}
		
		if (bestn < 0 || error < minerror) {
			beta = b;
			intercept = inter;
			minerror = error;
		}
	}
}

void wpcr(const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept) {
	mat X1;
	rvec m = X.colwise().mean();
	X1 = X.rowwise() - m;
	min_train_error(X1, Y, w, coefs, intercept);
}

LRModel::LRModel() 
: isconst(true), error(INFINITY), refit(true)
{
	timers.add("predict");
	timers.add("fit");
}

LRModel::LRModel(const LRModel &m)
: constvals(m.constvals), isconst(m.isconst),
  xtotals(m.xtotals), center(m.center), error(INFINITY), refit(true)
{
	timers.add("predict");
	timers.add("fit");
}

LRModel::~LRModel() { }

void LRModel::init_fit(const_mat_view X, const_mat_view Y, const propvec_sig &sig, std::vector<int> &nonzero)
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
	
	init_fit_sub(X, Y, sig, nonzero);
	int ncols = 0;
	for (int i = 0; i < nonzero.size(); ++i) {
		ncols += sig[nonzero[i]].length;
	}
	xdata.resize(0, ncols);
	ydata.resize(0, Y.cols());
	
	/*
	for (int i = 0; i < X.rows(); ++i) {
		int c1 = 0;
		for (int j = 0; j < nonzero.size(); ++j) {
			int c2 = sig[nonzero[j]].start;
			int n = sig[nonzero[j]].length;
			xdata.get().block(i, c1, 1, n) = X.block(i, c2, 1, n);
		}
	}
	ydata = Y;
	*/
}

void LRModel::add_example(const rvec &x, const rvec &y, bool update_refit) {
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
		return;
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
		rvec py;
		if (!predict_sub(x, py)) {
			refit = true;
			error = INFINITY;
		} else {
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
}

void LRModel::del_example(int r) {
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

void LRModel::update_error() {
	if (xdata.rows() == 0) {
		error = INFINITY;
	} else if (isconst) {
		error = 0.0;
	} else {
		mat P(ydata.rows(), ydata.cols());
		if (!predict_sub(xdata.get(), P)) {
			error = INFINITY;
		} else {
			error = (ydata.get() - P).squaredNorm();
		}
	}
}

void LRModel::save(ostream &os) const {
	os << isconst << endl;
	save_rvec(os, constvals);
	xdata.save(os);
	ydata.save(os);
}

void LRModel::load(istream &is) {
	int n, x;
	is >> isconst;
	load_rvec(is, constvals);
	xdata.load(is);
	ydata.load(is);
	fit();
}

bool LRModel::predict(const rvec &x, rvec &y) {
	if (isconst) {
		y = constvals;
		return true;
	}
	if (refit) {
		fit();
	}
	return predict_sub(x, y);
}

bool LRModel::predict(const_mat_view X, mat &Y) {
	function_timer t(timers.get(PREDICT_T));
	
	if (isconst) {
		Y.resize(X.rows(), constvals.size());
		Y.rowwise() = constvals;
		return true;
	}
	if (refit) {
		fit();
	}
	return predict_sub(X, Y);
}

bool LRModel::fit() {
	function_timer t(timers.get(FIT_T));
	
	if (!isconst) {
		fit_sub(xdata.get(), ydata.get());
	}
	update_error();
	refit = false;
	return true;
}

bool LRModel::cli_inspect(int first_arg, const vector<string> &args, ostream &os) const {
	if (first_arg >= args.size()) {
		os << "error:    " << error << endl;
		bool success;
		if (isconst) {
			os << "constant: ";
			output_rvec(os, constvals) << endl;
		} else if (!cli_inspect_sub(os)) {
			return false;
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

PCRModel::PCRModel() {}

PCRModel::PCRModel(const PCRModel &m)
: LRModel(m), beta(m.beta), intercept(m.intercept), means(m.means)
{}

void PCRModel::fit_sub(const_mat_view X, const_mat_view Y) {
	means = X.colwise().mean();
	mat Xc = X.rowwise() - means;

	min_train_error(Xc, Y, cvec(), beta, intercept);
}

void PCRModel::init_fit_sub(const_mat_view X, const_mat_view Y, const propvec_sig &sig, vector<int> &nonzero) {
	fit_sub(X, Y);
	int newrows = 0;
	for(int i = 0; i < sig.size(); ++i) {
		for (int j = 0; j < sig[i].length; ++j) {
			if (beta.row(sig[i].start + j).sum() > 0.0) {
				nonzero.push_back(i);
				newrows += sig[i].length;
				break;
			}
		}
	}
	
	// compact beta
	mat beta2(newrows, beta.cols());
	rvec means2(newrows);
	int i = 0;
	for (int j = 0; j < nonzero.size(); ++j) {
		int s = sig[nonzero[j]].start;
		int l = sig[nonzero[j]].length;
		for (int k = 0; k < l; ++k) {
			beta2.col(i) = beta.col(s + k);
			means2(i++) = means(s + k);
		}
	}
	beta = beta2;
	means = means2;
}

bool PCRModel::predict_sub(const rvec &x, rvec &y) {
	if (beta.size() == 0) {
		return false;
	}
	y = (x - means) * beta + intercept;
	return true;
}

bool PCRModel::predict_sub(const_mat_view X, mat &Y) {
	if (beta.size() == 0) {
		return false;
	}
	Y = (X.rowwise() - means) * beta;
	Y.rowwise() += intercept;
	return true;
}

bool PCRModel::cli_inspect_sub(ostream &os) const {
	os << "intercept: " << intercept << endl;
	os << "beta:" << endl;
	for (int i = 0; i < beta.rows(); ++i) {
		os << setw(4) << i << " ";
		output_rvec(os, beta.row(i)) << endl;
	}
	return true;
}

RRModel::RRModel() {}
RRModel::RRModel(const RRModel &m) : LRModel(m), C(m.C), xmean(m.xmean), ymean(m.ymean) {}

void RRModel::fit_sub(const_mat_view X, const_mat_view Y) {
	/*
	 I'm not weighting instances right now, but if I did, this
	 would be the code.
	*/
	//W.diagonal() = w;
	//mat Z = W * get_member_X();
	//mat V = W * get_member_Y();

	const_mat_view Z(X);
	const_mat_view V(Y);
	
	/*
	 If you're weighting and Z != X and V != Y, then xmean and
	 ymean need to be calculated for X and Y.
	*/
	xmean = Z.colwise().mean();
	ymean = V.colwise().mean();
	
	mat Zcenter = Z.rowwise() - xmean;
	mat Vcenter = V.rowwise() - ymean;
	mat A = Zcenter.transpose() * Zcenter;
	double lambda = RLAMBDA;
	for (int i = 0; i < A.cols(); ++i) {
		double inc = nextafter(A(i, i), INFINITY) - A(i, i);
		lambda = max(lambda, inc);
	}
	A.diagonal().array() += lambda;
	mat B = Z.transpose() * Vcenter;
	if (!solve(A, B, C)) {
		C.resize(0, 0);
	}
}

bool RRModel::predict_sub(const rvec &x, rvec &y) {
	if (C.size() == 0) {
		return false;
	}
	assert(C.rows() == x.size());
	y = (x - xmean) * C + ymean;
	return true;
}

bool RRModel::predict_sub(const_mat_view X, mat &Y) {
	if (C.size() == 0) {
		return false;
	}
	assert(C.rows() == X.cols());
	Y = ((X.rowwise() - xmean) * C).rowwise() + ymean;
	return true;
}

bool RRModel::cli_inspect_sub(ostream &os) const {
	os << "C:" << endl << C << endl;
	os << "xmean:" << endl << xmean << endl;
	os << "ymean:" << endl << ymean << endl;
	return true;
}

void RRModel::init_fit_sub(const_mat_view X, const_mat_view Y, const propvec_sig &sig, vector<int> &nonzero) {
	fit_sub(X, Y);
	int newrows = 0;
	for(int i = 0; i < sig.size(); ++i) {
		for (int j = 0; j < sig[i].length; ++j) {
			if (C.row(sig[i].start + j).sum() > 0.0) {
				nonzero.push_back(i);
				newrows += sig[i].length;
				break;
			}
		}
	}
	
	// compact
	mat C2(newrows, C.cols());
	rvec xmean2(newrows);
	int i = 0;
	for (int j = 0; j < nonzero.size(); ++j) {
		int s = sig[nonzero[j]].start;
		int l = sig[nonzero[j]].length;
		for (int k = 0; k < l; ++k) {
			C2.row(i) = C.row(s + k);
			xmean2(i++) = xmean(s + k);
		}
	}
	C = C2;
	xmean = xmean2;
}
