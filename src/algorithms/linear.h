#ifndef LINEAR_H
#define LINEAR_H

#include <vector>
#include <string>
#include "common.h"

void lsqr(const mat &X, const mat &Y, const evec &w, const evec &x, evec &yout);
void ridge(const mat &X, const mat &Y, const evec &w, const evec &x, evec &yout);
double pcr(const mat &X, const mat &Y, const evec &x);

void remove_static(const mat &X, mat &Xout, std::vector<int> &nonstatic);

class LRModel {
public:
	LRModel(const mat &xdata, const evec &ydata);
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
	
	const evec& get_center() const {
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
	double predict(const evec &x);
	bool predict(const mat &X, evec &result);
	bool fit();
	void fill_data(mat &X, evec &y) const;
	void save(std::ostream &os) const;
	// this has to be called right after the object is constructed
	void load(std::istream &is);
	
	virtual LRModel* copy() const = 0;
	virtual void fit_me() = 0;
	virtual double predict_me(const evec &x) = 0;
	virtual bool predict_me(const mat &X, evec &result) = 0;

protected:
	const mat &xdata;
	const evec &ydata;
	
private:
	void update_error();
	
	std::vector<int> members;
	double constval, error;
	bool isconst, refit;
	evec xtotals, center;
	
};

class PCRModel : public LRModel {
public:
	PCRModel(const mat &xdata, const evec &ydata);
	PCRModel(const PCRModel &m);
	~PCRModel() {}
	
	void fit_me();
	double predict_me(const evec &x);
	bool predict_me(const mat &X, evec &result);
	
	LRModel* copy() const {
		return new PCRModel(*this);
	}
	
private:
	evec beta, means;
	double intercept;
};

#endif
