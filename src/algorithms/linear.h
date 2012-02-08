#ifndef LINEAR_H
#define LINEAR_H

#include <vector>
#include <string>
#include <armadillo>

void lsqr(const arma::mat &X, const arma::mat &Y, const arma::vec &w, const arma::rowvec &x, arma::rowvec &yout);
void ridge(const arma::mat &X, const arma::mat &Y, const arma::vec &w, const arma::rowvec &x, arma::rowvec &yout);
double pcr(const arma::mat &X, const arma::mat &Y, const arma::rowvec &x);

void remove_static(const arma::mat &X, arma::mat &Xout, std::vector<int> &nonstatic);

class LRModel {
public:
	LRModel(const arma::mat &xdata, const arma::vec &ydata);
	LRModel(const LRModel &m);
	virtual ~LRModel();
	
	int size() {
		return members.size();
	}
	
	const std::vector<int>& get_members() const {
		return members;
	}
	
	double get_train_error() {
		return error;
	}
	
	const arma::rowvec& get_center() const {
		return center;
	}
	
	bool needs_refit() const {
		return refit;
	}
	
	bool is_const() const {
		return isconst;
	}
	
	void add_example(int i, bool update_refit);
	void add_examples(const std::vector<int> &inds);
	void del_example(int i);
	double predict(const arma::rowvec &x);
	bool predict(const arma::mat &X, arma::vec &result);
	bool fit();
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
	void update_error();
	
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
	arma::vec beta;
	double intercept;
	arma::rowvec means;
};

#endif
