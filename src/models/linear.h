#ifndef LINEAR_H
#define LINEAR_H

#include <vector>
#include <string>
#include <armadillo>
#include <Rcpp.h>

void lsqr(const arma::mat &X, const arma::mat &Y, const arma::vec &w, const arma::rowvec &x, arma::rowvec &yout);
void ridge(const arma::mat &X, const arma::mat &Y, const arma::vec &w, const arma::rowvec &x, arma::rowvec &yout);
void remove_static(arma::mat &X, arma::mat &Xout, std::vector<int> &dynamic);

class RPLSModel {
public:
	RPLSModel(const arma::mat &xdata, const arma::vec &ydata);
	RPLSModel(const RPLSModel &m);
	~RPLSModel();
	
	void add_example(int i);
	void del_example(int i);
	bool fit();
	double predict(const arma::rowvec &x);
	bool predict(const arma::mat &x, arma::vec &result);
	
	int size() const {
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
	
private:
	void print_loadings();
	bool predict(const Rcpp::NumericMatrix &x, arma::vec &result);
	void refresh_error();
	
	std::vector<int> members;
	RPLSModel *pls;
	double constval, error;
	const arma::mat &xdata;
	const arma::vec &ydata;
	bool isconst, refit;
	arma::rowvec xtotals, center;

	static int count;
	std::string plsobj;
};

#endif
