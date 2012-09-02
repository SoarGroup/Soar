#ifndef LINEAR_H
#define LINEAR_H

#include <vector>
#include <string>
#include "common.h"
#include "timer.h"

void wpcr  (const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept);
void ridge (const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs);
void ridge2(const_mat_view X, const_mat_view Y, mat &coefs);
bool solve2(const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept);

class LRModel {
public:
	LRModel();
	LRModel(const LRModel &m);
	virtual ~LRModel();
	
	int size() {
		return xdata.rows();
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
	
	void init_fit(const_mat_view X, const_mat_view Y, const propvec_sig &sig, std::vector<int> &nonzero);
	void add_example(const rvec &x, const rvec &y, bool update_refit);
	void del_example(int i);
	bool predict(const rvec &x, rvec &y);
	bool predict(const_mat_view X, mat &Y);
	bool fit();
	void save(std::ostream &os) const;
	// this has to be called right after the object is constructed
	void load(std::istream &is);
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;

	virtual LRModel* copy() const = 0;

private:
	virtual bool cli_inspect_sub(std::ostream &os) const = 0;
	virtual void fit_sub(const_mat_view X, const_mat_view Y) = 0;
	virtual void init_fit_sub(const_mat_view X, const_mat_view Y, const propvec_sig &sig, std::vector<int> &nonzero) = 0;
	virtual bool predict_sub(const rvec &x, rvec &y) = 0;
	virtual bool predict_sub(const_mat_view X, mat &Y) = 0;

	void update_error();
	
	dyn_mat xdata;
	dyn_mat ydata;
	double error;
	bool isconst, refit;
	rvec xtotals, center, constvals;
	
	enum Timers {PREDICT_T, FIT_T};
	timer_set timers;
};

class PCRModel : public LRModel {
public:
	PCRModel();
	PCRModel(const PCRModel &m);
	~PCRModel() {}
	
	LRModel* copy() const {
		return new PCRModel(*this);
	}
	
	bool cli_inspect_sub(std::ostream &os) const;
	
private:
	void init_fit_sub(const_mat_view X, const_mat_view Y, const propvec_sig &sig, std::vector<int> &nonzero);
	void fit_sub(const_mat_view X, const_mat_view Y);
	bool predict_sub(const rvec &x, rvec &y);
	bool predict_sub(const_mat_view X, mat &Y);
	
	mat beta;
	rvec means;
	rvec intercept;
};

class RRModel : public LRModel {
public:
	RRModel();
	RRModel(const RRModel &m);
	
	
	LRModel *copy() const {
		return new RRModel(*this);
	}
	
	bool cli_inspect_sub(std::ostream &os) const;
	
private:
	void init_fit_sub(const_mat_view X, const_mat_view Y, const propvec_sig &sig, std::vector<int> &nonzero);
	void fit_sub(const_mat_view X, const_mat_view Y);
	bool predict_sub(const rvec &x, rvec &y);
	bool predict_sub(const_mat_view X, mat &Y);
	
	mat C;
	rvec xmean, ymean;
};

#endif
