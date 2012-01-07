#ifndef LINEAR_H
#define LINEAR_H

#include <vector>
#include <string>
#include <armadillo>
#include <Rcpp.h>

void lsqr(const arma::mat &X, const arma::mat &Y, const arma::vec &w, const arma::rowvec &x, arma::rowvec &yout);
void ridge(const arma::mat &X, const arma::mat &Y, const arma::vec &w, const arma::rowvec &x, arma::rowvec &yout);
void remove_static(arma::mat &X, arma::mat &Xout, std::vector<int> &dynamic);

class LRModel {
public:
	LRModel(const arma::mat &xdata, const arma::vec &ydata);
	LRModel(const LRModel &m);
	virtual ~LRModel();
	
	void add_example(int i);
	void del_example(int i);
	
	int size() {
		return members.size();
	}
	
	const std::vector<int>& get_members() const {
		return members;
	}
	
	double get_error() {
		return error;
	}
	
	const arma::rowvec& get_center() const {
		return center;
	}
	
	double predict(const arma::rowvec &x) {
		if (refit) {
			fit();
		}
		if (isconst) {
			return constval;
		}
		return predict_me(x);
	}
	
	bool predict(const arma::mat &X, arma::vec &result) {
		if (refit) {
			fit();
		}
		if (isconst) {
			return constval;
		}
		result.resize(X.n_rows);
		return predict_me(X, result);
	}
	
	bool fit() { 
		if (!refit || isconst) {
			return false;
		}
		fit_me();
		refit = false;
		refresh_error();
		return true;
	}
	
	void fill_data(arma::mat &X, arma::vec &y) const;
	void save(std::ostream &os) const;
	// this has to be called right after the object is constructed
	void load(std::istream &is);
	
	virtual LRModel* copy() const = 0;
	virtual void fit_me() = 0;
	virtual double predict_me(const arma::rowvec &x) = 0;
	virtual bool predict_me(const arma::mat &X, arma::vec &result) = 0;

protected:
	const arma::mat &xdata;
	const arma::vec &ydata;
	
private:
	void refresh_error();
	
	std::vector<int> members;
	double constval, error;
	bool isconst, refit;
	arma::rowvec xtotals, center;
	
};

class PCRModel : public LRModel {
public:
	PCRModel(const arma::mat &xdata, const arma::vec &ydata);
	PCRModel(const PCRModel &m);
	~PCRModel() {}
	
	void fit_me();
	double predict_me(const arma::rowvec &x);
	bool predict_me(const arma::mat &X, arma::vec &result);
	
	LRModel* copy() const {
		return new PCRModel(*this);
	}
	
private:
	arma::mat V;
	arma::vec C;
	int ncomp;
	arma::rowvec means, stdevs;
};

class PLSModel : public LRModel {
public:
	PLSModel(const arma::mat &xdata, const arma::vec &ydata);
	PLSModel(const PLSModel &m);
	~PLSModel();
	
	void fit_me();
	double predict_me(const arma::rowvec &x);
	bool predict_me(const arma::mat &x, arma::vec &result);
	
	LRModel* copy() const {
		return new PLSModel(*this);
	}
	
private:
	void print_loadings();
	bool predict_meat(const Rcpp::NumericMatrix &x, arma::vec &result);
	
	static int count;
	std::string plsobj;
};

#endif
