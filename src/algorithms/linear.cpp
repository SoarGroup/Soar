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

/*
 Output a matrix composed only of those columns in the input matrix with
 significantly different values, meaning the maximum absolute value of
 the column is greater than SAME_THRESH times the minimum absolute value.
*/
void remove_static(const_mat_view X, mat &Xout, vector<int> &nonstatic_cols) {
	for (int i = 0; i < X.cols(); ++i) {
		cvec c = X.col(i).array().abs();
		if (c.maxCoeff() > c.minCoeff() * SAME_THRESH) {
			nonstatic_cols.push_back(i);
		}
	}
	assert(Xout.rows() == X.rows() && Xout.cols() >= nonstatic_cols.size());
	for (int i = 0; i < nonstatic_cols.size(); ++i) {
		Xout.col(i) = X.col(nonstatic_cols[i]);
	}
}

bool solve(const_mat_view X, const_mat_view Y, mat &C) {
	C = X.jacobiSvd(ComputeThinU | ComputeThinV).solve(Y);
	return true;
}

/*
 Performs standard linear regression with solve, but cleans up input
 data first to avoid instability. This consists of:
 
 1. Setting elements whose absolute values are smaller than ZERO_THRESH to 0
 2. collapsing all columns whose elements are identical into a single constant column.
*/
bool solve2(const_mat_view X, const_mat_view Y, mat &coefs, rvec &intercept) {
	mat X1 = X, X2(X.rows(), X.cols() + 1), C;
	vector<int> nonstatic;

	for (int i = 0; i < X.rows(); ++i) {
		for (int j = 0; j < X.cols(); ++j) {
			if (abs(X1(i, j)) < ZERO_THRESH) {
				X1(i, j) = 0.0;
			}
		}
	}

	remove_static(X1, X2, nonstatic);
	X2.resize(X2.rows(), nonstatic.size() + 1);
	X2.rightCols(1).setConstant(1.0);
	if (!solve(X2, Y, C)) {
		return false;
	}

	coefs.resize(X.cols(), C.cols());
	coefs.setConstant(0);
	for (int i = 0; i < nonstatic.size(); ++i) {
		coefs.row(nonstatic[i]) = C.row(i);
	}
	intercept = C.bottomRows(1);
	return true;
}

void lsqr(const_mat_view X, const_mat_view Y, const cvec &w, const rvec &x, rvec &yout) {
	mat W = mat::Zero(w.size(), w.size());
	W.diagonal() = w;
	mat Z = W * X, V = W * Y, C;
	rvec intercepts;
	solve2(Z, V, C, intercepts);
	yout = x * C + intercepts;
}

void ridge(const_mat_view X, const_mat_view Y, const cvec &w, const rvec &x, rvec &yout) {
	int i;
	mat W = mat::Zero(w.size(), w.size());
	W.diagonal() = w;
	mat Z = W * X;
	mat V = W * Y;
	mat Zcenter = Z.rowwise() - Z.colwise().mean();
	mat Vcenter = V.rowwise() - V.colwise().mean();
	mat A = Zcenter.transpose() * Zcenter;
	double lambda = RLAMBDA;
	for (i = 0; i < A.cols(); ++i) {
		double inc = nextafter(A(i, i), INFINITY) - A(i, i);
		lambda = max(lambda, inc);
	}
	A.diagonal().array() += lambda;
	mat B = Z.transpose() * Vcenter, C;
	solve(A, B, C);
	yout = (x - X.colwise().mean()) * C + Y.colwise().mean();
}

/*
 Use Leave-one-out cross validation to determine number of components
 to use. This seems to choose numbers that are too low.
*/
void cross_validate(const_mat_view X, const_mat_view Y, mat &beta, rvec &intercept) {
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
			solve2(projected.leftCols(j), Y1, coefs, inter);
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
	solve2(projected.leftCols(best), Y, coefs, inter);
	beta = components.leftCols(best) * coefs;
	intercept = inter;
}

/*
 Choose the number of components to minimize prediction error for
 the training instances. Also prevent the beta vector from blowing up
 too much.
*/
void min_train_error(const_mat_view X, const_mat_view Y, mat &beta, rvec &intercept) {
	vector<mat> betas;
	vector<rvec> intercepts;
	double minerror;
	int bestn = -1;
	mat components, projected, coefs, b;
	rvec inter;
	
	pca(X, components);
	projected = X * components;
	for (int i = 0; i < projected.cols(); ++i) {
		solve2(projected.leftCols(i), Y, coefs, inter);
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

void pcr(const_mat_view X, const_mat_view Y, const rvec &x, rvec &y) {
	mat beta, X1;
	rvec intercept;
	rvec m = X.colwise().mean();
	X1 = X.rowwise() - m;
	min_train_error(X1, Y, beta, intercept);
	y = (x - m) * beta + intercept;
}

LRModel::LRModel(const dyn_mat &xdata, const dyn_mat &ydata) 
: xdata(xdata), ydata(ydata), constvals(rvec::Zero(ydata.cols())), isconst(true), error(INFINITY), refit(true)
{
	timers.add("predict");
	timers.add("fit");
}

LRModel::LRModel(const LRModel &m)
: xdata(m.xdata), ydata(m.ydata), constvals(m.constvals), members(m.members), isconst(m.isconst),
  xtotals(m.xtotals), center(m.center), error(INFINITY), refit(true)
{
	timers.add("predict");
	timers.add("fit");
}

LRModel::~LRModel() { }

void LRModel::add_example(int i, bool update_refit) {
	if (find(members.begin(), members.end(), i) != members.end()) {
		return;
	}
	
	members.push_back(i);
	if (members.size() == 1) {
		xtotals = xdata.row(i);
		center = xtotals;
		isconst = true;
		constvals = ydata.row(i);
		error = 0.0;
		if (update_refit) {
			refit = false;
		}
		return;
	}
	
	xtotals += xdata.row(i);
	center = xtotals / members.size();
	if (isconst && ydata.row(i) != constvals) {
		isconst = false;
		if (update_refit) {
			refit = true;
		}
	}
	
	DATAVIS("isconst " << isconst << endl)
	if (isconst) {
		DATAVIS("constvals " << constvals << endl)
	} else if (update_refit) {
		rvec py;
		if (!predict_sub(xdata.row(i), py)) {
			refit = true;
			error = INFINITY;
		} else {
			double e = pow(py(0) - ydata(i, 0), 2);
			if (!refit) {
				/*
				 Only refit the model if the average error increases
				 significantly after adding the data point.
				*/
				double olderror = error / (members.size() - 1);
				double newerror = (error + e) / members.size();
				if (newerror > MODEL_ERROR_THRESH || newerror > REFIT_MUL_THRESH * olderror) {
					refit = true;
				}
			}
			error += e;
		}
		DATAVIS("'avg error' " << error / members.size() << endl)
	}
}

void LRModel::add_examples(const vector<int> &inds) {
	vector<int>::const_iterator i;
	for (i = inds.begin(); i != inds.end(); ++i) {
		add_example(*i, false);
	}
}

void LRModel::del_example(int i) {
	members.erase(remove(members.begin(), members.end(), i), members.end());
	
	if (members.size() == 0) {
		// handling of this case is questionable, make it better later
		isconst = true;
		constvals.fill(0.0);
		center.fill(0.0);
		return;
	}
	
	xtotals -= xdata.row(i);
	center = xtotals / members.size();
	
	if (!isconst) {
		/* check if remaining data all have same y */
		isconst = true;
		constvals = ydata.row(members[0]);
		for (int j = 0; j < members.size(); ++j) {
			if (ydata.row(members[j]) != constvals) {
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
		mat X(members.size(), xdata.cols()), Y(members.size(), ydata.cols()), P(members.size(), ydata.cols());
		for (int i = 0; i < members.size(); ++i) {
			X.row(i) = xdata.row(members[i]);
			Y.row(i) = ydata.row(members[i]);
		}
		
		if (!predict_sub(X, P)) {
			error = INFINITY;
		} else {
			error = (Y - P).squaredNorm();
		}
	}
}

void LRModel::save(ostream &os) const {
	os << isconst << endl;
	save_rvec(os, constvals);
	save_vector(members, os);
}

void LRModel::load(istream &is) {
	int n, x;
	is >> isconst;
	load_rvec(is, constvals);
	is >> n;
	members.reserve(n);
	for (int i = 0; i < n; ++i) {
		is >> x;
		add_example(x, false);
	}
	fit();
}

void LRModel::fill_data(mat &X, mat &Y) const {
	if (members.empty()) {
		return;
	}
	X.resize(members.size(), xdata.cols());
	Y.resize(members.size(), ydata.cols());
	
	for (int i = 0; i < members.size(); ++i) {
		X.row(i) = xdata.row(members[i]);
		Y.row(i) = ydata.row(members[i]);
	}
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
		fit_sub();
	}
	update_error();
	refit = false;
	return true;
}

bool LRModel::cli_inspect(int first_arg, const vector<string> &args, ostream &os) const {
	if (first_arg >= args.size()) {
		os << "members:  " << members.size() << endl;
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
		for (int i = 0; i < members.size(); ++i) {
			os << setw(4) << members[i] << " | ";
			output_rvec(os, xdata.row(members[i])) << " ";
			output_rvec(os, ydata.row(members[i])) << endl;
		}
		return true;
	}
	os << "unrecognized argument" << endl;
	return false;
}

PCRModel::PCRModel(const dyn_mat &xdata, const dyn_mat &ydata) 
: LRModel(xdata, ydata), intercept(rvec::Zero(ydata.cols()))
{}

PCRModel::PCRModel(const PCRModel &m)
: LRModel(m), beta(m.beta), intercept(m.intercept), means(m.means)
{}

void PCRModel::fit_sub() {
	mat X, Y;
	
	fill_data(X, Y);
	means = X.colwise().mean();
	X.rowwise() -= means;

	min_train_error(X, Y, beta, intercept);
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
	output_mat(os, beta);
	return true;
}
