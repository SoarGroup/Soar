#ifndef LINEAR_H
#define LINEAR_H

#include <vector>
#include <string>
#include "common.h"

void lsqr(const mat &X, const mat &Y, const cvec &w, const rvec &x, rvec &yout);
void ridge(const mat &X, const mat &Y, const cvec &w, const rvec &x, rvec &yout);
void pcr(const mat &X, const mat &Y, const rvec &x, rvec &y);

void remove_static(const mat &X, mat &Xout, std::vector<int> &nonstatic);

class LRModel {
public:
	LRModel(const mat &xdata, const mat &ydata);
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
	
	const rvec& get_center() const {
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
	bool predict(const rvec &x, rvec &y);
	bool predict(const mat &X, mat &Y);
	bool fit();
	void fill_data(mat &X, mat &Y) const;
	void save(std::ostream &os) const;
	// this has to be called right after the object is constructed
	void load(std::istream &is);
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;
	
	virtual LRModel* copy() const = 0;
	virtual void fit_drv() = 0;
	virtual bool predict_drv(const rvec &x, rvec &y) = 0;
	virtual bool predict_drv(const mat &X, mat &Y) = 0;

protected:
	virtual bool cli_inspect_drv(std::ostream &os) const = 0;
	
	const mat &xdata;
	const mat &ydata;
	
private:
	void update_error();
	
	std::vector<int> members;
	double error;
	bool isconst, refit;
	rvec xtotals, center, constvals;
	
};

class PCRModel : public LRModel {
public:
	PCRModel(const mat &xdata, const mat &ydata);
	PCRModel(const PCRModel &m);
	~PCRModel() {}
	
	void fit_drv();
	bool predict_drv(const rvec &x, rvec &y);
	bool predict_drv(const mat &X, mat &Y);
	
	LRModel* copy() const {
		return new PCRModel(*this);
	}
	
	bool cli_inspect_drv(std::ostream &os) const;
	
private:
	mat beta;
	rvec means;
	rvec intercept;
	int nfits;
	double fit_time;
};

#endif
