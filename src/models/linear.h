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
bool linreg(regression_type t, const_mat_view X, const_mat_view Y, const cvec &w, double var, mat &coefs, rvec &inter);
bool linreg_clean(regression_type t, const_mat_view X, const_mat_view Y, double var, mat &coefs);
bool linreg_d(regression_type t, mat &X, mat &Y, const cvec &w, double var, mat &coefs, rvec &inter);

bool nfoldcv(const_mat_view X, const_mat_view Y, double var, int n, regression_type t, rvec &avg_error);

#endif
