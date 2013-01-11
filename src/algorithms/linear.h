#ifndef LINEAR_H
#define LINEAR_H

#include <vector>
#include <string>
#include "common.h"
#include "timer.h"
#include "serializable.h"
#include "mat.h"
#include "scene_sig.h"

enum regression_type { OLS, RIDGE, PCR, LASSO, FORWARD };

void clean_lr_data(mat &X, std::vector<int> &used_cols);
void augment_ones(mat &X);
bool linreg(regression_type t, const_mat_view X, const_mat_view Y, const cvec &w, mat &coefs, rvec &inter);
bool linreg_clean(regression_type t, const_mat_view X, const_mat_view Y, mat &coefs);
bool linreg_d(regression_type t, mat &X, mat &Y, const cvec &w, mat &coefs, rvec &inter);

bool nfoldcv(const_mat_view X, const_mat_view Y, int n, regression_type t, rvec &avg_error);

class LinearModel : public serializable {
public:
	LinearModel();
	LinearModel(regression_type r);
	LinearModel(const LinearModel &m);
	
	int size() {
		return xdata.rows();
	}
	
	double get_train_error() {
		return error;
	}
	
	bool needs_refit() const {
		return refit;
	}
	
	bool is_const() const {
		return isconst;
	}
	
	LinearModel* copy() const { return new LinearModel(*this); }

	void init_fit(const_mat_view X, const_mat_view Y, int target, const scene_sig &sig, std::vector<int> &nonzero);
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
	rvec intercept;
	dyn_mat xdata;
	dyn_mat ydata;
	scene_sig orig_sig;
	
	timer_set timers;
};

#endif
