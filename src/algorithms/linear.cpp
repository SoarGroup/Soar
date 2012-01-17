#include <assert.h>
#include <vector>
#include <armadillo>
#include "linear.h"
#include "common.h"

using namespace std;
using namespace arma;

const double INF = numeric_limits<double>::infinity();
const double RLAMBDA = 0.0000001;

/*
 A local model does not need to be refit to data if its prediction error
 is lower than this value.
*/
const double REFIT_ABS_THRESH = 1e-5;

/*
 A local model does not need to be refit to data if its prediction error
 increases by less than this factor with a new data point.
*/
const double REFIT_MUL_THRESH = 1.0001;

/*
 When solving linear systems, columns whose min and max values are within
 this factor of each other are removed.
*/
const double SAME_THRESH = 1 + 1e-10;

/*
 When solving linear systems, elements whose absolute value is smaller
 than this are zeroed.
*/
const double ZERO_THRESH = 1e-15;

/*
 Determines cutoff for the number of components to use in PCR.
*/
const double ABS_ERROR_THRESH = 1e-10;

/*
 Maximum number of times Leave One Out cross-validation will run for a
 single PCR fitting
*/
const int LOO_NTEST = 30;

/*
 In PCR, don't use a beta vector with a norm larger than this.
*/
const double MAX_BETA_NORM = 1.0e3;

/*
 Output a matrix composed only of those columns in the input matrix with
 significantly different values, meaning the maximum absolute value of
 the column is greater than SAME_THRESH times the minimum absolute value.
*/
void remove_static(const mat &X, mat &Xout, vector<int> &nonstatic_cols) {
	for (int i = 0; i < X.n_cols; ++i) {
		vec c = abs(X.col(i));
		if (max(c) > min(c) * SAME_THRESH) {
			nonstatic_cols.push_back(i);
		}
	}
	Xout.reshape(X.n_rows, nonstatic_cols.size());
	for (int i = 0; i < nonstatic_cols.size(); ++i) {
		Xout.col(i) = X.col(nonstatic_cols[i]);
	}
}

/*
 Performs standard linear regression with arma::solve, but cleans up input
 data first to avoid instability. This consists of:
 
 1. Setting elements whose absolute values are smaller than ZERO_THRESH to 0
 2. collapsing all columns whose elements are identical into a single constant column.
*/
bool solve2(const mat &X, const vec &y, vec &coefs, double &intercept) {
	mat X1 = X, X2, C;
	vector<int> nonstatic;
	
	for (int i = 0; i < X.n_rows; ++i) {
		for (int j = 0; j < X.n_cols; ++j) {
			if (abs(X1(i, j)) < ZERO_THRESH) {
				X1(i, j) = 0.0;
			}
		}
	}
	
	remove_static(X1, X2, nonstatic);
	X2.insert_cols(X2.n_cols, ones<vec>(X2.n_rows, 1));
	if (!solve(C, X2, y)) {
		return false;
	}
	
	coefs.set_size(X.n_cols, 1);
	coefs.fill(0.0);
	for (int i = 0; i < nonstatic.size(); ++i) {
		coefs(nonstatic[i]) = C(i, 0);
	}
	intercept = C(C.n_rows - 1, 0);
	return true;
}

void lsqr(const mat &X, const mat &Y, const vec &w, const rowvec &x, rowvec &yout) {
	mat X1 = X;
	X1.insert_cols(X.n_cols, ones<vec>(X.n_rows));
	mat W = diagmat(w);
	mat Z = W * X1;
	mat V = W * Y;
	mat C = solve(Z, V);
	if (C.n_elem == 0) {
		// solve failed
		w.print("w:");
		Z.print("Z:");
		V.print("V:");
		assert(false);
	}
	rowvec x1 = ones<rowvec>(1, x.n_cols + 1);
	x1.subvec(0, x.n_cols - 1) = x;
	yout = x1 * C;
}

void ridge(const mat &X, const mat &Y, const vec &w, const rowvec &x, rowvec &yout) {
	int i;
	mat W = diagmat(w);
	mat Z = W * X;
	mat V = W * Y;
	rowvec Zmean = mean(Z, 0);
	mat Zcenter = Z;
	for (i = 0; i < Z.n_rows; ++i) {
		Zcenter.row(i) -= Zmean;
	}
	rowvec Vmean = mean(V, 0);
	mat Vcenter = V;
	for (i = 0; i < Z.n_rows; ++i) {
		Vcenter.row(i) -= Vmean;
	}
	mat Zt = trans(Zcenter);
	mat A = Zt * Zcenter;
	double lambda = RLAMBDA;
	for (i = 0; i < A.n_cols; ++i) {
		double inc = nextafter(A(i, i), INF) - A(i, i);
		lambda = max(lambda, inc);
	}
	for (i = 0; i < A.n_cols; ++i) {
		A(i, i) += lambda;
	}
	mat B = Zt * Vcenter;
	mat C = solve(A, B);
	assert(C.n_elem != 0);
	yout = (x - mean(X, 0)) * C + mean(Y, 0);
}

void pcr_fit(const mat &X, const vec &y, int ncomp, vector<vec> &betas, vector<double> &intercepts) {
	mat loadings, scores;
	vec variances, b, coefs;
	double inter;
	
	betas.clear();
	intercepts.clear();
	if (!princomp(loadings, scores, variances, X)) {
		assert(false);
	}
	if (ncomp > 0) {
		solve2(scores.cols(0, ncomp - 1), y, coefs, inter);
		b = loadings.cols(0, ncomp - 1) * coefs;
		betas.push_back(b);
		intercepts.push_back(inter);
		return;
	}
	for (int n = 0; n < scores.n_cols; ++n) {
		solve2(scores.cols(0, n), y, coefs, inter);
		b = loadings.cols(0, n) * coefs;
		betas.push_back(b);
		intercepts.push_back(inter);
	}
}

/*
 Use Leave-one-out cross validation to determine number of components
 to use. This seems to choose numbers that are too low.
*/
void cross_validate(const mat &X, const vec &y, vec &beta, double &intercept) {
	int ndata = X.n_rows, maxcomps = X.n_cols;
	
	mat X1(ndata - 1, X.n_cols);
	vec y1(ndata - 1), errors = zeros<vec>(maxcomps, 1);
	vector<vec> betas;
	vector<double> intercepts;
	vector<int> leave_out;
	int ntest = min(LOO_NTEST, ndata);
	
	leave_out.reserve(ndata);
	for (int i = 0; i < ndata; ++i) {
		leave_out.push_back(i);
	}
	random_shuffle(leave_out.begin(), leave_out.end());
	
	for (int i = 0; i < ntest; ++i) {
		int n = leave_out[i];
		if (n > 0) {
			X1.rows(0, n - 1) = X.rows(0, n - 1);
			y1.rows(0, n - 1) = y.rows(0, n - 1);
		}
		if (n < ndata - 1) {
			X1.rows(n, ndata - 2) = X.rows(n + 1, ndata - 1);
			y1.rows(n, ndata - 2) = y.rows(n + 1, ndata - 1);
		}
		pcr_fit(X1, y1, -1, betas, intercepts);
		for (int j = 0; j < maxcomps; ++j) {
			errors(j) += abs(dot(X.row(n), betas[j]) + intercepts[j] - y(n));
		}
	}
	int best = -1;
	for (int i = 0; i < maxcomps; ++i) {
		if (best == -1 || errors(best) > errors(i)) {
			best = i;
		}
	}
	pcr_fit(X, y, best, betas, intercepts);
	beta = betas[0];
	intercept = intercepts[0];
}

/*
 Choose the number of components to minimize prediction error for
 the training instances. Also prevent the beta vector from blowing up
 too much.
*/
void min_train_error(const mat &X, const vec &y, vec &beta, double &intercept) {
	double error = INF;
	vector<vec> betas;
	vector<double> intercepts;
	
	for (int ncomp = 1; ncomp < X.n_cols; ++ncomp) {
		pcr_fit(X, y, ncomp, betas, intercepts);
		if (norm(betas[0], 1) > MAX_BETA_NORM) {
			return;
		}
		beta = betas[0];
		intercept = intercepts[0];
		double newerror = sqrt(accu(pow((X * beta + intercept) - y, 2)) / X.n_rows);
		
		if (newerror < ABS_ERROR_THRESH) {
			break;
		}
		
		error = newerror;
	}
}

LRModel::LRModel(const mat &xdata, const vec &ydata) 
: xdata(xdata), ydata(ydata), constval(0.0), isconst(true), error(INF), refit(true)
{}

LRModel::LRModel(const LRModel &m)
: xdata(m.xdata), ydata(m.ydata), constval(m.constval), members(m.members), isconst(m.isconst),
  xtotals(m.xtotals), center(m.center), error(INF), refit(true)
{}

LRModel::~LRModel() { }

void LRModel::add_example(int i, bool update_refit) {
	if (find(members.begin(), members.end(), i) != members.end()) {
		return;
	}
	
	members.push_back(i);
	DATAVIS("'training set' '")
	for (int j = 0; j < members.size(); ++j) {
		DATAVIS(members[j] << " ")
	}
	DATAVIS("'" << endl)
	
	if (members.size() == 1) {
		xtotals = xdata.row(i);
		center = xtotals;
		isconst = true;
		constval = ydata(i);
		error = 0.0;
		if (update_refit) {
			refit = false;
		}
		return;
	}
	
	xtotals += xdata.row(i);
	center = xtotals / members.size();
	if (isconst && ydata(i) != constval) {
		isconst = false;
		if (update_refit) {
			refit = true;
		}
	}
	
	DATAVIS("isconst " << isconst << endl)
	if (isconst) {
		DATAVIS("constval " << constval << endl)
	} else if (update_refit) {
		double e = pow(predict(xdata.row(i)) - ydata(i), 2);
		if (!refit) {
			/*
			 Only refit the model if the average error increases
			 significantly after adding the data point.
			*/
			double olderror = error / (members.size() - 1);
			double newerror = (error + e) / members.size();
			if (newerror > REFIT_ABS_THRESH && newerror > REFIT_MUL_THRESH * olderror) {
				refit = true;
			}
		}
		error += e;
		DATAVIS("'avg error' " << error / members.size() << endl)
	}
}

void LRModel::del_example(int i) {
	members.erase(remove(members.begin(), members.end(), i), members.end());
	DATAVIS("'training set' '")
	for (int j = 0; j < members.size(); ++j) {
		DATAVIS(members[j] << " ")
	}
	DATAVIS("'" << endl)
	
	
	if (members.size() == 0) {
		// handling of this case is questionable, make it better later
		isconst = true;
		constval = 0.0;
		center.fill(0.0);
		return;
	}
	
	xtotals -= xdata.row(i);
	center = xtotals / members.size();
	
	if (!isconst) {
		/* check if remaining data all have same y */
		isconst = true;
		constval = ydata(members[0]);
		for (int j = 0; j < members.size(); ++j) {
			if (ydata(members[j]) != constval) {
				isconst = false;
				break;
			}
		}
		if (isconst) {
			error = 0.0;
		} else {
			refresh_error();
		}
	}
	DATAVIS("isconst " << isconst << endl)
}

void LRModel::refresh_error() {
	if (xdata.n_rows == 0) {
		error = INF;
	}
	
	mat X(members.size(), xdata.n_cols);
	for (int i = 0; i < members.size(); ++i) {
		X.row(i) = xdata.row(members[i]);
	}
	
	vec predictions(members.size());
	if (!predict(X, predictions)) {
		error = INF;
	} else {
		error = 0.;
		for (int i = 0; i < members.size(); ++i) {
			error += pow(ydata(members[i]) - predictions(i), 2);
		}
	}
	DATAVIS("'avg error' " << error / members.size() << endl)
}

void LRModel::save(ostream &os) const {
	os << isconst << " " << constval << endl;
	save_vector(members, os);
}

void LRModel::load(istream &is) {
	int n, x;
	is >> isconst >> constval;
	is >> n;
	members.reserve(n);
	for (int i = 0; i < n; ++i) {
		is >> x;
		add_example(x, false);
	}
	fit();
}

void LRModel::fill_data(mat &X, vec &y) const {
	if (members.empty()) {
		return;
	}
	X.set_size(members.size(), xdata.n_cols);
	y.set_size(members.size(), 1);
	
	for (int i = 0; i < members.size(); ++i) {
		X.row(i) = xdata.row(members[i]);
		y(i) = ydata(members[i]);
	}
}

double LRModel::predict(const rowvec &x) {
	if (isconst) {
		return constval;
	}
	if (refit) {
		fit();
	}
	return predict_me(x);
}

bool LRModel::predict(const arma::mat &X, arma::vec &result) {
	result.set_size(X.n_rows);
	if (isconst) {
		result.fill(constval);
		return true;
	}
	if (refit) {
		fit();
	}
	return predict_me(X, result);
}

bool LRModel::fit() {
	if (isconst) {
		return false;
	}
	fit_me();
	refit = false;
	refresh_error();
	return true;
}

PCRModel::PCRModel(const mat &xdata, const vec &ydata) 
: LRModel(xdata, ydata), intercept(0.0)
{}

PCRModel::PCRModel(const PCRModel &m)
: LRModel(m), beta(m.beta), intercept(m.intercept), means(m.means), stdevs(m.stdevs)
{}

void PCRModel::fit_me() {
	DATAVIS("BEGIN PCR" << endl)
	DATAVIS("'num fits' %+1" << endl)
	mat X, loadings, scores;
	vec y, variances;
	
	fill_data(X, y);
	
	// center X
	means = mean(X);
	stdevs = stddev(X);
	
	/*
	 If any columns are uniform, then standard deviation will be
	 zero and cause division by zero. These columns will be zeroed
	 out after subtracting their mean anyway, so just set their
	 divisors to 0.
	*/
	for (int i = 0; i < X.n_cols; ++i) {
		if (stdevs(i) == 0.0) {
			stdevs(i) = 1.0;
		}
	}
	
	for (int i = 0; i < X.n_rows; ++i) {
		X.row(i) -= means;
		X.row(i) /= stdevs;
	}
	
	min_train_error(X, y, beta, intercept);
	DATAVIS("END" << endl)
}


double PCRModel::predict_me(const rowvec &x) {
	return dot((x - means) / stdevs, beta) + intercept;
}

bool PCRModel::predict_me(const mat &X, vec &result) {
	mat Xc(X.n_rows, X.n_cols);
	
	for (int i = 0; i < X.n_rows; ++i) {
		Xc.row(i) = (X.row(i) - means) / stdevs;
	}
	
	result = Xc * beta + intercept;
	return true;
}

double pcr(const mat &X, const mat &Y, const rowvec &x) {
	vec y = Y.col(0);
	PCRModel m(X, y);
	for (int i = 0; i < X.n_rows; ++i) {
		m.add_example(i, false);
	}
	m.fit();
	return m.predict(x);
}

