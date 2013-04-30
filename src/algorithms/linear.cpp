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

// Residual Sum of Squares
double RSS(const_mat_view X, const_mat_view y, const cvec &coefs, double intercept) {
	return ((y - X * coefs).array() - intercept).matrix().squaredNorm();
}

/*
 Mallows' Cp statistic

 This statistic estimates how well a linear model will perform on
 out-of-training examples.
*/
double MallowCp(const_mat_view X, const_mat_view y, const cvec &coefs, double intercept, double variance) {
	int p = X.cols() + 1, N = X.rows();
	double RSSp = RSS(X, y, coefs, intercept);
	return RSSp / variance - N + 2 * p;
}

/* Assume that X is already centered */
void pca(const_mat_view X, mat &comps) {
	JacobiSVD<mat> svd = X.jacobiSvd(Eigen::ComputeFullV);
	comps = svd.matrixV();
}

bool solve(const_mat_view X, const_mat_view Y, mat &C) {
	C = X.colPivHouseholderQr().solve(Y);
	//C = X.jacobiSvd(ComputeThinU | ComputeThinV).solve(Y);
	return normal(C);
}

bool solve(const_mat_view X, const_mat_view y, cvec &coefs) {
	assert(y.cols() == 1);
	mat C;
	if (!solve(X, y, C)) {
		return false;
	}
	coefs = C.col(0);
	return true;
}

bool ridge(const_mat_view X, const_mat_view Y, mat &coefs) {
	mat A = X.transpose() * X;
	
	/*
	 Due to floating point precision limits, sometimes the preset
	 RIDGE_LAMBDA will not change the value of a coefficient when
	 added to it. Therefore, I find the smallest value of lambda
	 that will result in a representable increase for all
	 coefficients.
	*/
	double lambda = RIDGE_LAMBDA;
	if (lambda > 0) {
		for (int i = 0; i < A.cols(); ++i) {
			double inc = nextafter(A(i, i), INFINITY) - A(i, i);
			if (inc > lambda) {
				lambda = inc;
			}
		}
	}
	A.diagonal().array() += lambda;
	mat B = X.transpose() * Y;
	coefs = A.ldlt().solve(B);
	return true;
}

/*
 Implements the so-called "shooting" algorithm for lasso
 regularization. Based on the tutorial and MATLAB code taken from
 
 http://www.gautampendse.com/software/lasso/webpage/lasso_shooting.html
*/
bool lasso_core(const_mat_view X, const rvec &xi_norm2, const_mat_view y, double lambda, cvec &beta) {
	int p = X.cols();
	
	for (int k = 0; k < LASSO_MAX_ITERS; ++k) {
		bool improved = false;
		for (int i = 0; i < p; ++i) {
			double b = beta(i);
			cvec yi = y - X * beta + beta(i) * X.col(i);
			double deltai = yi.dot(X.col(i));
			if (deltai < -lambda) {
				beta(i) = (deltai + lambda) / xi_norm2(i);
			} else if (deltai > LASSO_LAMBDA) {
				beta(i) = (deltai - lambda) / xi_norm2(i);
			} else {
				beta(i) = 0;
			}
			if (abs(b - beta(i)) > LASSO_TOL) {
				improved = true;
			}
		}
		if (!improved) {
			break;
		}
	}
	return true;
}

bool lasso(const_mat_view X, const_mat_view Y, mat &coefs) {
	if (!ridge(X, Y, coefs)) {
		return false;
	}
	rvec xi_norm2 = X.colwise().squaredNorm();
	for (int i = 0; i < coefs.cols(); ++i) {
		cvec beta = coefs.col(i);
		lasso_core(X, xi_norm2, Y.col(i), LASSO_LAMBDA, beta);
		coefs.col(i) = beta;
	}
	return true;
}

/*
 Fits a linear model to the data using as few nonzero coefficients as possible.
 Basically, start with just the intercept, then keep adding additional
 predictors (nonzero coefficients) to the model that lowers Mallows' Cp
 statistic, one at a time. Stop when Cp can't be lowered further with more
 predictors.

 This is a naive version of R's step function.
*/
bool fstep(const_mat_view X, const_mat_view Y, double var, mat &coefs) {
	assert(Y.cols() == 1);
	int ncols = X.cols(), p = 0;
	vector<int> predictors;
	vector<bool> used(ncols, false);
	mat Xcurr(X.rows(), ncols);
	cvec curr_c;
	double curr_Cp = MallowCp(Xcurr.leftCols(p), Y, cvec(), 0, var);

	while (p < ncols) {
		double best_Cp = 0;
		int best_pred = -1;
		cvec best_c;
		for (int i = 0; i < ncols; ++i) {
			if (used[i]) { continue; }
			Xcurr.col(p) = X.col(i);

			cvec c;
			if (!solve(Xcurr.leftCols(p + 1), Y, c)) {
				continue;
			}
			double Cp = MallowCp(Xcurr.leftCols(p + 1), Y, c, 0, var);
			if (best_pred < 0 || Cp < best_Cp || 
			    (Cp == best_Cp && c.squaredNorm() < best_c.squaredNorm()))
			{
				best_Cp = Cp;
				best_c = c;
				best_pred = i;
			}
		}
		if (best_Cp < curr_Cp ||
		    (best_Cp == curr_Cp && best_c.squaredNorm() < curr_c.squaredNorm()))
		{
			predictors.push_back(best_pred);
			used[best_pred] = true;
			Xcurr.col(p) = X.col(best_pred);
			curr_Cp = best_Cp;
			curr_c = best_c;
			++p;
		} else {
			break;
		}
	}

	coefs.resize(ncols, 1);
	coefs.setConstant(0.0);
	for (int i = 0; i < predictors.size(); ++i) {
		coefs(predictors[i], 0) = curr_c(i);
	}
	return true;
}

/*
 Use Leave-one-out cross validation to determine number of components
 to use. This seems to choose numbers that are too low.
*/
void cross_validate(const_mat_view X, const_mat_view Y, const cvec &w, double var, mat &beta, rvec &intercept) {
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
			linreg(OLS, projected.leftCols(j), Y1, w, var, coefs, inter);
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
	linreg(OLS, projected.leftCols(best), Y, cvec(), var, coefs, inter);
	beta = components.leftCols(best) * coefs;
	intercept = inter;
}

/*
 Choose the number of components to minimize prediction error for
 the training instances. Also prevent the beta vector from blowing up
 too much.
*/
bool min_train_error(const_mat_view X, const_mat_view Y, mat &beta) {
	vector<mat> betas;
	vector<rvec> intercepts;
	double minerror;
	int bestn = -1;
	mat components, projected, coefs, b;
	
	pca(X, components);
	projected = X * components;
	for (int i = 0; i < projected.cols(); ++i) {
		if (!solve(projected.leftCols(i), Y, coefs)) {
			continue;
		}
		b = components.leftCols(i) * coefs;
		
		if (b.squaredNorm() > MAX_BETA_NORM) {
			break;
		}
		double error = (X * b - Y).squaredNorm();
		
		if (error < MODEL_ERROR_THRESH) {
			beta = b;
			return true;
		}
		
		if (bestn < 0 || error < minerror) {
			beta = b;
			minerror = error;
		}
	}
	return bestn != -1;
}

bool wpcr(const_mat_view X, const_mat_view Y, mat &coefs) {
	if (!min_train_error(X, Y, coefs)) {
		return false;
	}
	return true;
}

/*
 Cleaning consists of:
 
 1. collapsing all columns whose elements are identical into a single constant column.
 2. Setting elements whose absolute values are smaller than SAME_THRESH to 0.
 3. Centering X and Y data so that intercepts don't need to be considered.
*/
void clean_lr_data(mat &X, vector<int> &used_cols) {
	int xdims = X.cols(), ndata = X.rows(), newdims = 0;
	
	for (int i = 0; i < xdims; ++i) {
		if (!uniform(X.col(i))) {
			if (newdims < i) {
				X.col(newdims) = X.col(i);
			}
			used_cols.push_back(i);
			++newdims;
		}
	}
	
	X.conservativeResize(ndata, newdims);
	for (int i = 0; i < ndata; ++i) {
		for (int j = 0; j < newdims; ++j) {
			if (fabs(X(i, j)) < SAME_THRESH) {
				X(i, j) = 0.0;
			}
		}
	}
}

void center_data(mat &X, rvec &Xm) {
	Xm = X.colwise().mean();
	X.rowwise() -= Xm;
}

void augment_ones(mat &X) {
	X.conservativeResize(X.rows(), X.cols() + 1);
	X.col(X.cols() - 1).setConstant(1.0);
}

void fix_for_wols(mat &X, mat &Y, const cvec &w) {
	assert(w.size() == X.rows());
	X.conservativeResize(X.rows(), X.cols() + 1);
	X.col(X.cols() - 1).setConstant(1.0);
	
	for (int i = 0, iend = X.cols(); i < iend; ++i) {
		X.col(i).array() *= w.array();
	}
	for (int i = 0, iend = Y.cols(); i < iend; ++i) {
		Y.col(i).array() *= w.array();
	}
}

/*
 Perform linear regression. Assumes that input X and Y are already
 cleaned and centered.
*/
bool linreg_clean(regression_type t, const_mat_view X, const_mat_view Y, double var, mat &coefs) {
	switch (t) {
		case OLS:
			return solve(X, Y, coefs);
		case RIDGE:
			return ridge(X, Y, coefs);
		case LASSO:
			return lasso(X, Y, coefs);
		case PCR:
			return wpcr(X, Y, coefs);
			break;
		case FORWARD:
			return fstep(X, Y, var, coefs);
		default:
			assert(false);
	}
	return false;
}


/*
 Clean up input data to avoid instability, then perform some form of
 regression.
 
 Note that this function modifies inputs X and Y to avoid redundant
 copies.
*/
bool linreg_d(regression_type t, mat &X, mat &Y, const cvec &w, double var, mat &coefs, rvec &inter) {
	int ndata = X.rows();
	int xdim = X.cols();
	int ydim = Y.cols();
	mat coefs1;
	rvec Xm, Ym, inter1;
	vector<int> used;
	
	clean_lr_data(X, used);
	/*
	 The two ways to deal with intercepts:
	 
	 1. Center data before regression, then calculate intercept afterwards using
	    means.
	 2. Append a column of 1's to X. The coefficient for this column will be the
	    intercept.
	 
	 Unfortunately from what I can gather, method 1 doesn't work with weighted
	 regression, and method 2 doesn't work with ridge regression or PCR.
	*/
	
	bool augment = (w.size() > 0);
	if (augment) {
		assert(t != RIDGE && t != PCR);
		fix_for_wols(X, Y, w);
	} else {
		center_data(X, Xm);
		center_data(Y, Ym);
	}
	
	if (!linreg_clean(t, X, Y, var, coefs1)) {
		return false;
	}
	
	if (augment) {
		assert(coefs1.rows() == used.size() + 1);
		inter = coefs1.row(coefs1.rows() - 1);
	} else {
		inter = Ym - (Xm * coefs1);
	}
	
	coefs.resize(xdim, ydim);
	coefs.setConstant(0.0);
	for (int i = 0; i < used.size(); ++i) {
		coefs.row(used[i]) = coefs1.row(i);
	}
	return true;
}

bool linreg (
	regression_type t,
	const_mat_view X,
	const_mat_view Y,
	const cvec &w,
	double var,
	mat &coefs,
	rvec &intercept ) 
{
	mat Xc(X), Yc(Y);
	return linreg_d(t, Xc, Yc, w, var, coefs, intercept);
}

bool nfoldcv(const_mat_view X, const_mat_view Y, double var, int n, regression_type t, rvec &avg_error) {
	assert(X.rows() >= n);
	
	int chunk, extra, total_rows, train_rows, test_rows, i, start, end;
	int xcols, ycols;
	mat Xrand, Yrand, Xtrain, Xtest, Ytrain, Ytest, pred, error;
	mat coefs;
	rvec intercept;
	vector<int> r(X.rows());
	
	xcols = X.cols();
	ycols = Y.cols();
	total_rows = X.rows();
	chunk = total_rows / n;
	extra = X.rows() - n * chunk;
	avg_error.resize(ycols);
	avg_error.setConstant(0.0);
	
	// shuffle X and Y
	for (int i = 0, iend = r.size(); i < iend; ++i) {
		r[i] = i;
	}
	random_shuffle(r.begin(), r.end());
	Xrand.resize(total_rows, xcols);
	Yrand.resize(total_rows, ycols);
	for (int i = 0, iend = r.size(); i < iend; ++i) {
		Xrand.row(i) = X.row(r[i]);
		Yrand.row(i) = Y.row(r[i]);
	}
	
	for (i = 0, start = 0; i < n; ++i) {
		if (i < extra) {
			test_rows = chunk + 1;
		} else {
			test_rows = chunk;
		}
		train_rows = total_rows - test_rows;
		end = start + test_rows;
		
		Xtest = Xrand.block(start, 0, test_rows, xcols);
		Ytest = Yrand.block(start, 0, test_rows, ycols);
		
		Xtrain.resize(train_rows, xcols);
		Ytrain.resize(train_rows, ycols);
		
		Xtrain.block(0, 0, start, xcols) = Xrand.block(0, 0, start, xcols);
		Xtrain.block(start, 0, train_rows - start, xcols) = Xrand.block(end, 0, train_rows - start, xcols);
		Ytrain.block(0, 0, start, ycols) = Yrand.block(0, 0, start, ycols);
		Ytrain.block(start, 0, train_rows - start, ycols) = Yrand.block(end, 0, train_rows - start, ycols);
		
		if (!linreg_d(t, Xtrain, Ytrain, cvec(), var, coefs, intercept))
			return false;
		
		pred = Xtest * coefs;
		for (int j = 0, jend = pred.rows(); j < jend; ++j) {
			pred.row(j) += intercept;
		}
		error = (Ytest - pred).array().abs().matrix();
		avg_error += error.colwise().sum();
		start += test_rows;
	}
	avg_error.array() /= total_rows;
	return true;
}
