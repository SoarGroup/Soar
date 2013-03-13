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
bool fstep(const_mat_view X, const_mat_view Y, mat &coefs) {
	assert(Y.cols() == 1);
	int ncols = X.cols(), p = 0;
	vector<int> predictors;
	vector<bool> used(ncols, false);
	mat Xcurr(X.rows(), ncols);
	cvec curr_c;
	double curr_Cp = MallowCp(Xcurr.leftCols(p), Y, cvec(), 0, MEASURE_VAR);

	while (p < ncols) {
		double best_Cp = 0;
		int best_pred = -1;
		cvec best_c;
		for (int i = 0; i < ncols; ++i) {
			if (used[i]) { continue; }
			Xcurr.col(p) = X.col(i);

			cvec c;
			if (!solve(Xcurr.leftCols(p + 1), Y, c)) {
				return false;
			}
			double Cp = MallowCp(Xcurr.leftCols(p + 1), Y, c, 0, MEASURE_VAR);
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
			linreg(OLS, projected.leftCols(j), Y1, w, coefs, inter);
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
	linreg(OLS, projected.leftCols(best), Y, cvec(), coefs, inter);
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
	int ndata = X.rows(), xdim = X.cols(), ydim = Y.cols();
	X.conservativeResize(ndata, xdim + 1);
	
	for (int i = 0; i < xdim; ++i) {
		X.col(i).array() *= w.array();
	}
	X.col(xdim) = w;
	for (int i = 0; i < ydim; ++i) {
		Y.col(i).array() *= w.array();
	}
}

/*
 Perform linear regression. Assumes that input X and Y are already
 cleaned and centered.
*/
bool linreg_clean(regression_type t, const_mat_view X, const_mat_view Y, mat &coefs) {
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
			return fstep(X, Y, coefs);
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
bool linreg_d(regression_type t, mat &X, mat &Y, const cvec &w, mat &coefs, rvec &inter) {
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
	if (w.size() > 0) {
		assert(t != RIDGE && t != PCR);
		fix_for_wols(X, Y, w);
	} else {
		center_data(X, Xm);
		center_data(Y, Ym);
	}
	
	if (!linreg_clean(t, X, Y, coefs1)) {
		return false;
	}
	
	if (w.size() > 0) {
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
	mat &coefs,
	rvec &intercept ) 
{
	mat Xc(X), Yc(Y);
	return linreg_d(t, Xc, Yc, w, coefs, intercept);
}

LinearModel::LinearModel()
: isconst(true), error(INFINITY), refit(true), alg(FORWARD)
{}

LinearModel::LinearModel(regression_type alg) 
: isconst(true), error(INFINITY), refit(true), alg(alg)
{}

LinearModel::LinearModel(const LinearModel &m)
: isconst(m.isconst), error(INFINITY), refit(true),
  coefs(m.coefs), intercept(m.intercept), alg(m.alg),
  xdata(m.xdata), ydata(m.ydata)
{}

void LinearModel::init_fit(const_mat_view X, const_mat_view Y, int target, const scene_sig &sig, std::vector<int> &obj_map)
{
	isconst = true;
	obj_map.clear();
	for (int i = 1; i < Y.rows(); ++i) {
		if (Y.row(i) != Y.row(0)) {
			isconst = false;
			break;
		}
	}
	if (isconst) {
		intercept = Y.row(0);
		xdata.resize(0, 0);
		ydata.resize(0, Y.cols());
		return;
	}
	
	fit_sub(X, Y);
	
	int ndims = 0, ndata = X.rows(), nout = Y.cols();
	
	// target is always first object
	obj_map.push_back(target);
	ndims += sig[target].props.size();
	
	// find other relevant objects (with nonzero coefficients)
	for (int i = 0; i < sig.size(); ++i) {
		if (i == target) {
			continue;
		}
		int start = sig[i].start;
		int end = start + sig[i].props.size();
		for (int j = start; j < end; ++j) {
			if (!coefs.row(j).isConstant(0.0)) {
				obj_map.push_back(i);
				ndims += sig[i].props.size();
				break;
			}
		}
	}
	
	// discard information about irrelevant objects
	mat coefs2(ndims, nout);
	xdata.resize(ndata, ndims);

	orig_sig.clear();
	int i = 0;
	for (int j = 0; j < obj_map.size(); ++j) {
		const scene_sig::entry &e = sig[obj_map[j]];
		orig_sig.add(e);
		int s = e.start;
		int l = e.props.size();
		coefs2.block(i, 0, l, nout) = coefs.block(s, 0, l, nout);
		xdata.get().block(0, i, ndata, l) = X.block(0, s, ndata, l);
		i += l;
	}
	coefs = coefs2;
	ydata = Y;
	update_error();
}

int LinearModel::add_example(const rvec &x, const rvec &y, bool update_refit) {
	assert(xdata.cols() == x.size() && ydata.cols() == y.size());
	
	xdata.append_row(x);
	ydata.append_row(y);
	if (xdata.rows() == 1) {
		isconst = true;
		intercept = y;
		error = 0.0;
		if (update_refit) {
			refit = false;
		}
		return 0;
	}
	
	if (isconst && y != intercept) {
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
		intercept.fill(0.0);
		return;
	}
		
	if (!isconst) {
		/* check if remaining data all have same y */
		isconst = true;
		for (int j = 1; j < ydata.rows(); ++j) {
			if (ydata.row(j) != ydata.row(0)) {
				isconst = false;
				break;
			}
		}
		if (isconst) {
			intercept = ydata.row(0);
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
	serializer(os) << alg << error << isconst << intercept
	               << xdata << ydata << orig_sig;
}

void LinearModel::unserialize(istream &is) {
	int a;
	unserializer(is) >> a >> error >> isconst >> intercept
	                 >> xdata >> ydata >> orig_sig;

	assert(a == OLS || a == RIDGE || a == PCR || a == LASSO || a == FORWARD);
	alg = static_cast<regression_type>(a);
	fit();
}

bool LinearModel::predict(const rvec &x, rvec &y) {
	if (isconst) {
		y = intercept;
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
	function_timer t(timers.get_or_add("predict"));
	
	if (isconst) {
		Y.resize(X.rows(), intercept.size());
		Y.rowwise() = intercept;
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
	function_timer t(timers.get_or_add("fit"));
	
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
	return linreg(alg, X, Y, cvec(), coefs, intercept);
}

bool LinearModel::cli_inspect(int first_arg, const vector<string> &args, ostream &os) const {
	if (first_arg >= args.size()) {
		os << "error:    " << error << endl;
		bool success;
		if (isconst) {
			os << "constant: ";
			output_rvec(os, intercept) << endl;
		} else {
			os << "intercept: " << intercept << endl;
			os << "coefs:" << endl;
			table_printer t;
			int k = 0;
			for (int i = 0; i < orig_sig.size(); ++i) {
				for (int j = 0; j < orig_sig[i].props.size(); ++j) {
					t.add_row() << k;
					if (j == 0) {
						t << orig_sig[i].name;
					} else {
						t.skip(1);
					}
					t << orig_sig[i].props[j];
					for (int l = 0; l < coefs.cols(); ++l) {
						t << coefs(k, l);
					}
					++k;
				}
			}
			t.print(os);
			return true;
		}
		os << "subqueries: timing train" << endl;
		return true;
	} else if (args[first_arg] == "train") {
		table_printer t;
		t.add_row();
		for (int i = 0; i < orig_sig.size(); ++i) {
			t << orig_sig[i].name;
			t.skip(orig_sig[i].props.size() - 1);
		}
		t.add_row();
		for (int i = 0; i < orig_sig.size(); ++i) {
			for (int j = 0; j < orig_sig[i].props.size(); ++j) {
				t << orig_sig[i].props[j];
			}
		}
		for (int i = 0; i < xdata.rows(); ++i) {
			t.add_row();
			for (int j = 0; j < xdata.cols(); ++j) {
				t << xdata(i, j);
			}
			for (int j = 0; j < ydata.cols(); ++j) {
				t << ydata(i, j);
			}
		}
		t.print(os);
		return true;
	}
	os << "unrecognized argument" << endl;
	return false;
}

bool nfoldcv(const_mat_view X, const_mat_view Y, int n, regression_type t, rvec &avg_error) {
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
		
		if (!linreg_d(t, Xtrain, Ytrain, cvec(), coefs, intercept))
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
