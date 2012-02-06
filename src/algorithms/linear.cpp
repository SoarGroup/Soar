#include <cmath>
#include <cassert>
#include <vector>
#include "linear.h"
#include "common.h"
#include "params.h"

using namespace std;
using namespace Eigen;

void pca(const mat &X, mat &comps, mat &signals, evec &variances) {
	mat Y = (X.rowwise() - X.colwise().mean());
	JacobiSVD<mat> svd = Y.jacobiSvd(Eigen::ComputeFullV);
	variances = (svd.singularValues() / sqrt(X.rows() - 1)).array().pow(2);
	comps = svd.matrixV();
	signals = Y * comps;
}

/*
 Output a matrix composed only of those columns in the input matrix with
 significantly different values, meaning the maximum absolute value of
 the column is greater than SAME_THRESH times the minimum absolute value.
*/
void remove_static(const mat &X, mat &Xout, vector<int> &nonstatic_cols) {
	for (int i = 0; i < X.cols(); ++i) {
		evec c = X.col(i).array().abs();
		if (c.maxCoeff() > c.minCoeff() * SAME_THRESH) {
			nonstatic_cols.push_back(i);
		}
	}
	assert(Xout.rows() == X.rows() && Xout.cols() >= nonstatic_cols.size());
	for (int i = 0; i < nonstatic_cols.size(); ++i) {
		Xout.col(i) = X.col(nonstatic_cols[i]);
	}
}

bool solve(const mat &X, const mat &Y, mat &C) {
	C = X.jacobiSvd(ComputeThinU | ComputeThinV).solve(Y);
	return true;
}

/*
 Performs standard linear regression with solve, but cleans up input
 data first to avoid instability. This consists of:
 
 1. Setting elements whose absolute values are smaller than ZERO_THRESH to 0
 2. collapsing all columns whose elements are identical into a single constant column.
*/
bool solve2(const mat &X, const mat &Y, mat &coefs, evec &intercept) {
    mat X1 = X, X2 = mat::Constant(X.rows(), X.cols() + 1, 1), C;
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
    if (!solve(X2, Y, C)) {
        return false;
    }

    coefs = mat::Zero(X.cols(), C.cols());
    for (int i = 0; i < nonstatic.size(); ++i) {
        coefs.row(nonstatic[i]) = C.row(i);
    }
    intercept = C.bottomRows(1);
    return true;
}

void lsqr(const mat &X, const mat &Y, const evec &w, const evec &x, evec &yout) {
	mat W = mat::Zero(w.size(), w.size());
	W.diagonal() = w;
	mat Z = W * X, V = W * Y, C;
	evec intercept;
	solve2(Z, V, C, intercept);
	yout = x * C + intercept;
}

void ridge(const mat &X, const mat &Y, const evec &w, const evec &x, evec &yout) {
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

void pcr_fit(const mat &X, const evec &y, int ncomp, vector<evec> &betas, vector<double> &intercepts) {
	mat components, projected, coefs;
	evec variances, inter;
	
	betas.clear();
	intercepts.clear();
	pca(X, components, projected, variances);
	if (ncomp > 0) {
		solve2(projected.leftCols(ncomp), y, coefs, inter);
		betas.push_back(components.leftCols(ncomp) * coefs);
		intercepts.push_back(inter(0));
		return;
	}
	for (int n = 1; n < projected.cols(); ++n) {
		solve2(projected.leftCols(n), y, coefs, inter);
		betas.push_back(components.leftCols(n) * coefs);
		intercepts.push_back(inter(0));
	}
}

/*
 Use Leave-one-out cross validation to determine number of components
 to use. This seems to choose numbers that are too low.
*/
void cross_validate(const mat &X, const evec &y, evec &beta, double &intercept) {
	int ndata = X.rows(), maxcomps = X.cols();
	
	mat X1(ndata - 1, X.cols());
	evec y1(ndata - 1), errors = evec::Zero(maxcomps);
	vector<evec> betas;
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
			X1.topRows(n) = X.topRows(n);
			y1.head(n) = y.head(n);
		}
		if (n < ndata - 1) {
			X1.bottomRows(ndata - n - 1) = X.bottomRows(ndata - n - 1);
			y1.tail(ndata - n - 1) = y.tail(ndata - n - 1);
		}
		pcr_fit(X1, y1, -1, betas, intercepts);
		for (int j = 0; j < maxcomps; ++j) {
			errors(j) += abs(X.row(n).dot(betas[j]) + intercepts[j] - y(n));
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
void min_train_error(const mat &X, const evec &y, evec &beta, double &intercept) {
	double error = INFINITY;
	vector<evec> betas;
	vector<double> intercepts;
	
	pcr_fit(X, y, -1, betas, intercepts);
	for (int i = 0; i < betas.size(); ++i) {
		if (betas[i].norm() > MAX_BETA_NORM) {
			return;
		}
		beta = betas[i];
		intercept = intercepts[i];
		double newerror = sqrt(((X * beta).array() + intercept - y.array()).matrix().squaredNorm() / X.rows());
		
		if (newerror < MODEL_ERROR_THRESH) {
			break;
		}
		
		error = newerror;
	}
}

LRModel::LRModel(const mat &xdata, const evec &ydata) 
: xdata(xdata), ydata(ydata), constval(0.0), isconst(true), error(INFINITY), refit(true)
{}

LRModel::LRModel(const LRModel &m)
: xdata(m.xdata), ydata(m.ydata), constval(m.constval), members(m.members), isconst(m.isconst),
  xtotals(m.xtotals), center(m.center), error(INFINITY), refit(true)
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
		double e = pow(predict_me(xdata.row(i)) - ydata(i), 2);
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
			update_error();
		}
	}
	DATAVIS("isconst " << isconst << endl)
}

void LRModel::update_error() {
	if (xdata.rows() == 0) {
		error = INFINITY;
	} else if (isconst) {
		error = 0.0;
	} else {
		mat X(members.size(), xdata.cols());
		evec y(members.size()), predictions(members.size());
		for (int i = 0; i < members.size(); ++i) {
			X.row(i) = xdata.row(members[i]);
			y(i) = ydata(members[i]);
		}
		
		if (!predict_me(X, predictions)) {
			error = INFINITY;
		} else {
			error = (y - predictions).squaredNorm();
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

void LRModel::fill_data(mat &X, evec &y) const {
	if (members.empty()) {
		return;
	}
	X.resize(members.size(), xdata.cols());
	y.resize(members.size());
	
	for (int i = 0; i < members.size(); ++i) {
		X.row(i) = xdata.row(members[i]);
		y(i) = ydata(members[i]);
	}
}

double LRModel::predict(const evec &x) {
	if (isconst) {
		return constval;
	}
	if (refit) {
		fit();
	}
	return predict_me(x);
}

bool LRModel::predict(const mat &X, evec &result) {
	result.resize(X.rows());
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
	if (!isconst) {
		fit_me();
	}
	update_error();
	refit = false;
	return true;
}

PCRModel::PCRModel(const mat &xdata, const evec &ydata) 
: LRModel(xdata, ydata), intercept(0.0)
{}

PCRModel::PCRModel(const PCRModel &m)
: LRModel(m), beta(m.beta), intercept(m.intercept), means(m.means)
{}

void PCRModel::fit_me() {
	DATAVIS("BEGIN PCR" << endl)
	DATAVIS("'num fits' %+1" << endl)
	mat X;
	evec y, variances;
	
	fill_data(X, y);
	means = X.colwise().mean();
	X.rowwise() -= means;

	min_train_error(X, y, beta, intercept);
	DATAVIS("END" << endl)
}


double PCRModel::predict_me(const evec &x) {
	if (beta.size() == 0) {
		return NAN;
	}
	return (x - means).dot(beta) + intercept;
}

bool PCRModel::predict_me(const mat &X, evec &result) {
	if (beta.size() == 0) {
		return false;
	}
	result = ((X.rowwise() - means) * beta.transpose()).array() + intercept;
	return true;
}

double pcr(const mat &X, const mat &Y, const evec &x) {
	evec y = Y.col(0);
	PCRModel m(X, y);
	for (int i = 0; i < X.rows(); ++i) {
		m.add_example(i, false);
	}
	m.fit();
	return m.predict(x);
}

