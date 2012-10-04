#ifndef LINEAR_H
#define LINEAR_H

#include <vector>
#include <string>
#include "common.h"
#include "timer.h"
#include "serializable.h"
#include "mat.h"
#include "scene.h"

bool wpcr  (const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept);
bool ridge (const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept);
bool ols   (const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept);
bool lasso (const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &intercept);
bool fstep (const_mat_view X, const_mat_view y, double variance, cvec &coefs, double &intercept);

class LinearModel : public serializable {
public:
	enum regression_type { OLS, RIDGE, PCR, LASSO, FORWARD };
	
	LinearModel();
	LinearModel(regression_type r);
	LinearModel(const LinearModel &m);
	
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
	
	LinearModel* copy() const { return new LinearModel(*this); }

	void init_fit(const_mat_view X, const_mat_view Y, const scene_sig &sig, std::vector<int> &nonzero);
	int  add_example(const rvec &x, const rvec &y, bool update_refit);
	void del_example(int i);
	bool predict(const rvec &x, rvec &y);
	bool predict(const_mat_view X, mat &Y);
	bool fit();
	void serialize(std::ostream &os) const;
	// this has to be called right after the object is constructed
	void unserialize(std::istream &is);
	bool cli_inspect(int first_arg, const std::vector<std::string> &args, std::ostream &os) const;

private:
	bool fit_sub(const_mat_view X, const_mat_view Y);
	void update_error();
	
	regression_type alg;
	double error;
	bool isconst, refit;
	mat coefs;
	rvec xtotals, center, constvals, intercept;
	dyn_mat xdata;
	dyn_mat ydata;
	scene_sig orig_sig;
	
	enum Timers {PREDICT_T, FIT_T};
	timer_set timers;
};

#endif
