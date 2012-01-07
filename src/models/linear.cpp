#include <assert.h>
#include <vector>
#include <armadillo>
#include <Rcpp.h>
#include <RInside.h>
#include "linear.h"
#include "common.h"

using namespace std;
using namespace arma;

const double INF = numeric_limits<double>::infinity();
const double RLAMBDA = 0.0000001;
const double REFIT_ABS_THRESH = 1e-5;
const double REFIT_MUL_THRESH = 1.0001;

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

/* Output a matrix composed only of those columns in the input
   matrix with different values.
*/
void remove_static(mat &X, mat &Xout, vector<int> &dynamic) {
	for (int c = 0; c < X.n_cols; ++c) {
		for (int r = 1; r < X.n_rows; ++r) {
			if (X(r, c) != X(0, c)) {
				dynamic.push_back(c);
				break;
			}
		}
	}
	Xout.reshape(X.n_rows, dynamic.size());
	for (int i = 0; i < dynamic.size(); ++i) {
		Xout.col(i) = X.col(dynamic[i]);
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

void LRModel::add_example(int i) {
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
		return;
	}
	
	xtotals += xdata.row(i);
	center = xtotals / members.size();
	if (isconst && ydata(i) != constval) {
		isconst = false;
	}
	
	DATAVIS("isconst " << isconst << endl)
	if (!isconst) {
		double e = pow(predict(xdata.row(i)) - ydata(i), 2);
		/*
		 Only refit the model if the average error increases
		 significantly after adding the data point.
		*/
		double olderror = error / (members.size() - 1);
		double newerror = (error + e) / members.size();
		if (newerror > REFIT_ABS_THRESH && newerror > REFIT_MUL_THRESH * olderror) {
			refit = true;
		}
		error += e;
		DATAVIS("'avg error' " << newerror << endl)
	} else {
		DATAVIS("constval " << constval << endl)
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
	save_vector(members, os);
}

void LRModel::load(istream &is) {
	int n, x;
	is >> n;
	members.reserve(n);
	for (int i = 0; i < n; ++i) {
		is >> x;
		add_example(x);
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

PCRModel::PCRModel(const mat &xdata, const vec &ydata) 
: LRModel(xdata, ydata)
{
	ncomp = 2;  // temporary
}

PCRModel::PCRModel(const PCRModel &m)
: LRModel(m), V(m.V), C(m.C), ncomp(m.ncomp), means(m.means), stdevs(m.stdevs)
{}

void PCRModel::fit_me() {
	mat X, U, Z, V1;
	vec y, s;
	
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
	
	if (!svd(U, s, V1, X)) {
		assert(false);
	}
	V = V1;
	
	//cout << "U" << endl << U << endl;
	//cout << "V" << endl << V << endl;
	//cout << "s" << endl << s << endl;
	
	Z = ones<mat>(X.n_rows, ncomp + 1);
	Z.cols(0, ncomp - 1) = X * V.cols(0, ncomp - 1);
	
	//cout << "Z" << endl << Z << endl;
	
	mat temp = solve(Z, y);
	C = temp.col(0);
	//cout << "C" << endl << C << endl;
}

double PCRModel::predict_me(const rowvec &x) {
	rowvec xc = (x - means) / stdevs;
	rowvec z = ones<rowvec>(1, ncomp+1);
	
	for (int i = 0; i < ncomp; ++i) {
		z(i) = dot(xc, V.col(i));
	}
	//cout << "z" << endl << z << endl;
	return dot(z, C);
}

bool PCRModel::predict_me(const mat &X, vec &result) {
	mat Xc(X.n_rows, X.n_cols);
	
	for (int i = 0; i < X.n_rows; ++i) {
		Xc.row(i) = (X.row(i) - means) / stdevs;
	}
	
	mat Z = ones<mat>(X.n_rows, ncomp + 1);
	Z.cols(0, ncomp - 1) = Xc * V.cols(0, ncomp - 1);
	result = Z * C;
	return true;
}

static RInside *R = NULL;
int PLSModel::count = 0;

Rcpp::NumericMatrix to_rmat(const mat &m) {
	Rcpp::NumericMatrix rm(m.n_rows, m.n_cols);
	for (int i = 0; i < m.n_rows; ++i) {
		for (int j = 0; j < m.n_cols; ++j) {
			rm(i, j) = m(i, j);
		}
	}
	return rm;
}

Rcpp::NumericMatrix to_rmat(const rowvec &v) {
	Rcpp::NumericMatrix rm(1, v.n_elem);
	for (int i = 0; i < v.n_elem; ++i) {
		rm(0, i) = v(i);
	}
	return rm;
}

PLSModel::PLSModel(const mat &xdata, const vec &ydata) 
: LRModel(xdata, ydata)
{
	if (R == NULL) {
		R = new RInside(0, NULL);
		srand(1); // because RInside constructor calls srand with current time
		R->parseEvalQ("set.seed(0)");
		R->parseEvalQ("suppressMessages(library('pls'))");
	}
	
	stringstream ss;
	ss << "plsobj" << count++;
	plsobj = ss.str();
}

PLSModel::PLSModel(const PLSModel &m)
: LRModel(m)
{
	stringstream ss;
	ss << "plsobj" << count++;
	plsobj = ss.str();
}

PLSModel::~PLSModel() {
	stringstream ss;
	ss << "rm(" << plsobj << ")";
	R->parseEvalQ(ss.str());
}

void PLSModel::fit_me() {
	const vector<int> &mems = get_members();
	Rcpp::NumericMatrix x(mems.size(), xdata.n_cols);
	Rcpp::NumericVector y(mems.size());
	for (int i = 0; i < mems.size(); ++i) {
		for (int j = 0; j < xdata.n_cols; ++j) {
			x(i, j) = xdata(mems[i], j);
		}
		y(i) = ydata(mems[i]);
	}
	(*R)["X"] = x;
	(*R)["y"] = y;
	R->parseEvalQ("traindata <- data.frame(X = I(X), y = y)");
	stringstream ss;
	ss << plsobj << "<-plsr(y ~ X, data=traindata)";
	R->parseEvalQ(ss.str());
}

bool PLSModel::predict_meat(const Rcpp::NumericMatrix &x, vec &result) {
	(*R)["X"] = x;
	R->parseEvalQ("testdata <- data.frame(X = I(X))");
	
	stringstream ss;
	ss << "p<-predict(" << plsobj << ",newdata=testdata)";
	R->parseEvalQ(ss.str());

	bool valid = true;
	int ncomp = R->parseEval("dim(p)[3]");
	for (; ncomp >= 1; ncomp--) {
		stringstream ss1;
		ss1 << "p[,1," << ncomp << "]";
		Rcpp::NumericVector ans = R->parseEval(ss1.str());
		valid = true;
		for (int i = 0; i < ans.size(); ++i) {
			if (isnan(ans(i))) {
				valid = false;
				break;
			}
			result(i) = ans(i);
		}
		if (valid) {
			DATAVIS("ncomp " << ncomp << endl)
			break;
		}
	}
	if (!valid) {
		DATAVIS("ncomp -1" << endl)
	}
	return valid;
}

bool PLSModel::predict_me(const mat &x, vec &result) {
	return predict_meat(to_rmat(x), result);
}

double PLSModel::predict_me(const rowvec &x) {
	vec res(1);
	if (!predict_meat(to_rmat(x), res)) {
		return numeric_limits<double>::signaling_NaN();
	}
	return res(0);
}

void PLSModel::print_loadings() {
	stringstream ss;
	ss << "loadings(" << plsobj << ")";
	Rcpp::NumericMatrix loadings = R->parseEval(ss.str());
	for (int i = 0; i < loadings.rows(); ++i) {
		for (int j = 0; j < loadings.cols(); ++j) {
			cout << loadings(i, j) << " ";
		}
		cout << endl;
	}
}


