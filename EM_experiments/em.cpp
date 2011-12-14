#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <armadillo>

using namespace std;
using namespace arma;

const double sqrt2pi = 2.5066282746310002;
const bool use_locality = false;

double gausspdf(double x, double mean, double std) {
	return (1. / std * sqrt2pi) * exp(-((x - mean) * (x - mean) / (2 * std * std)));
}

/* Box-Muller method */
double randgauss(double mean, double std) {
	double x1, x2, w;
	do {
		x1 = 2.0 * ((double) rand()) / RAND_MAX - 1.0;
		x2 = 2.0 * ((double) rand()) / RAND_MAX - 1.0;
		w = x1 * x1 + x2 * x2;
	} while (w >= 1.0);
	
	w = sqrt((-2.0 * log(w)) / w);
	return mean + std * (x1 * w);
}

void gen_data(int n, const rowvec &xmin, const rowvec &xmax, const rowvec &a, double b, mat &X, vec &y) {
	assert(X.n_rows == y.n_rows);
	int n1 = X.n_rows;
	X.reshape(n1 + n, xmin.n_cols);
	y.reshape(n1 + n, 1);
	rowvec xrange = xmax - xmin;
	for (int i = 0; i < n; ++i) {
		X.row(n1 + i) = xmin + randu<rowvec>(xmin.n_elem) % xrange;
		y(n1 + i) = dot(a, X.row(n1 + i)) + b; // + randgauss(0., 0.001);
	}
}

void load_data(mat &X, vec &y) {
	ifstream file("data");
	string line;
	
	while (getline(file, line)) {
		int i = line.find_first_not_of(" \t");
		if (i == string::npos || line[i] == '#') {
			continue;
		}
		i = line.find(';');
		if (i == string::npos) {
			cerr << "data file error" << endl;
			exit(1);
		}
		stringstream xpart(line.substr(0, i));
		stringstream ypart(line.substr(i + 1));
		
		vector<double> vals;
		double v;
		while (xpart >> v) {
			vals.push_back(v);
		}
		
		if (X.n_cols == 0) {
			X.reshape(1, vals.size());
		} else if (vals.size() != X.n_cols) {
			cerr << "X dimension mismatch" << endl;
			exit(1);
		} else {
			X.insert_rows(X.n_rows, 1);
		}
		
		for (int j = 0; j < vals.size(); ++j) {
			X(X.n_rows - 1, j) = vals[j];
		}
		
		vals.clear();
		while (ypart >> v) {
			vals.push_back(v);
		}
		y.insert_rows(y.n_elem, 1);
		y(y.n_elem - 1) = vals[7];
	}
}

/*
Output a matrix composed only of those columns in the input matrix with
different values.
*/
void remove_static(const mat &X, mat &Xout, vector<int> &dynamic) {
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

class LinearModel {
public:
	LinearModel() {}
	
	void fit(const mat &X, const vec &y, const rowvec &w) {
		mat X1;
		vector<int> dynamic;
		remove_static(X, X1, dynamic);
		X1.insert_cols(X1.n_cols, ones<vec>(X1.n_rows));
		mat W = diagmat(w);
		mat Z = W * X1;
		vec v = y % trans(w);   // % is element-wise multiplication
		mat C = solve(Z, v);
		if (C.n_elem == 0) {
			// solve failed
			w.print("w:");
			Z.print("Z:");
			v.print("v:");
			assert(false);
		}
		a.reshape(1, X.n_cols);
		a.fill(0.);
		for (int i = 0; i < dynamic.size(); ++i) {
			a(dynamic[i]) = C(i);
		}
		b = C(C.n_rows - 1, 0);
		cout << "a = " << a << "b = " << b << endl;
	}
	
	double predict(const rowvec &x) {
		return dot(a, x) + b;
	}
	
	double density(const rowvec &x, double y) {
		double p = gausspdf(y, predict(x), 0.001);
		cout << "P(m | x = " << x(0) << ", y = " << y << ") = " << p << endl;
		return p;
	}
	
	rowvec a;
	double b;
	rowvec center;
};

class EM {
public:
	vector<LinearModel> models;
	double epsilon, Pnoise;
	mat X;
	vec y;
	mat Pm_x, Pm_yx, normPm_yx;
	ivec map_model;
	int ndata, nmodels, K;

	EM(const mat &X, const vec &y, double epsilon)
	: X(X), y(y), K(5), epsilon(epsilon)
	{
		Pnoise = epsilon / (y.max() - y.min());
		nmodels  = 0;
		ndata = X.n_rows;
		map_model.reshape(ndata, 1);
		map_model.fill(-1);
		Pm_x.reshape(0, ndata);
		Pm_yx.reshape(0, ndata);
		normPm_yx.reshape(0, ndata);
	}
	
	void estep() {
		map_model.fill(-1);
		Pm_yx.fill(0.);
		for (int i = 0; i < nmodels; ++i) {
			cout << "Model " << i << endl;
			LinearModel &m = models[i];
			for (int j = 0; j < ndata; ++j) {
				Pm_yx(i, j) = (1.0 - epsilon) * Pm_x(i, j) * m.density(X.row(j), y(j));
				if ((map_model(j) == -1 && (epsilon == 0. || Pm_yx(i, j) > Pnoise)) ||
				    (map_model(j) != -1 && Pm_yx(i, j) > Pm_yx(map_model(j), j)))
				{
					map_model(j) = i;
				}
			}
		}
		cout << "map_model " << map_model << endl;
	}
	
	void mstep() {
		rowvec Px = sum(Pm_yx, 0) + Pnoise * ones<rowvec>(ndata);
		mat normPm_yx(nmodels, ndata);
		for (int i = 0; i < nmodels; ++i) {
			normPm_yx.row(i) = Pm_yx.row(i) / Px;
		}
		vec Pc = sum(normPm_yx, 1);
		double Pc_total = sum(Px);
		
		mat W(nmodels, ndata);
		for (int i = 0; i < nmodels; ++i) {
			for (int j = 0; j < ndata; ++j) {
				W(i, j) = normPm_yx(i, j) / Pc(i);
			}
		}
		
		for (int i = 0; i < nmodels; ++i) {
			rowvec w = W.row(i);
			uvec nonzero = find(w);
			rowvec wp = w.elem(nonzero);
			mat Xp(nonzero.n_elem, X.n_cols);
			for (int j = 0; j < nonzero.n_elem; j++) {
				Xp.row(j) = X.row(nonzero(j));
			}
			vec yp = y.elem(nonzero);
			models[i].fit(Xp, yp, wp);
		}
	}
	
	void step() {
		if (nmodels > 0) {
			calc_Pm_x();
			estep();
			mstep();
			
			vector<LinearModel> new_models;
			// remove useless models
			for (int i = 0; i < nmodels; ++i) {
				int count = 0;
				for (int j = 0; j < ndata; ++j) {
					if (map_model(j) == i) {
						count++;
					}
				}
				if (count >= 2) {
					new_models.push_back(models[i]);
				}
			}
			
			models = new_models;
			nmodels = models.size();
		}
		
		add_model();
	}
	
	/* Uniform locality prior for P(c|x) */
	void calc_Pm_x() {
		int ndata = X.n_rows;
		int i, j, k;
		umat eligible(ndata, nmodels);
		eligible.fill(1);
		
		if (use_locality) {
			for (i = 0; i < ndata; ++i) {
				const rowvec &x = X.row(i);
				for (j = 0; j < nmodels; ++j) {
					rowvec &cj = models[j].center;
					for (k = 0; k < nmodels; ++k) {
						rowvec &ck = models[k].center;
						if (k != j && dot(x - ck, cj - ck) < 0.) {
							eligible(i, j) = 0;
							break;
						}
					}
				}
			}
		}
		
		Pm_x.reshape(nmodels, ndata);
		Pm_x.fill(0.);
		for (j = 0; j < ndata; ++j) {
			double p = 1.0 / ((double) sum(eligible.row(j)));
			for (i = 0; i < nmodels; ++i) {
				if (eligible(j, i)) {
					Pm_x(i, j) = p;
				}
			}
		}
		
		// calculate new centers
		for (i = 0; i < nmodels; ++i) {
			rowvec &c = models[i].center;
			c.fill(0.);
			for (j = 0; j < ndata; ++j) {
				if (eligible(j, i)) {
					c += X.row(j);
				}
			}
			c /= sum(eligible.col(i));
		}
	}

	void add_model() {
		vector<int> noise_data;
		for (int i = 0; i < ndata; ++i) {
			if (map_model(i) == -1) {
				noise_data.push_back(i);
			}
		}

		int nnoise = noise_data.size();
		if (nnoise < K) {
			return;
		}
		
		const rowvec &seed = X.row(noise_data[rand() % nnoise]);
		vec dists(nnoise);
		for (int i = 0; i < nnoise; ++i) {
			dists(i) = accu(pow(X.row(noise_data[i]) - seed, 2));
		}
		uvec close = sort_index(dists);
		uvec close1(K);
		for (int i = 0; i < K; ++i) {
			close1(i) = noise_data[close(i)];
		}
		cout << "Fitting to points " << close1 << endl;
		mat closeX(K, X.n_cols);
		for (int i = 0; i < K; ++i) {
			closeX.row(i) = X.row(close1(i));
		}
		vec closeY = y.elem(close1);
		rowvec w = ones<rowvec>(K);
		cout << "Adding model" << endl;
		LinearModel m;
		m.fit(closeX, closeY, w);
		m.center = sum(closeX, 0) / K;
		models.push_back(m);
		
		Pm_x.insert_rows(nmodels, 1);
		Pm_yx.insert_rows(nmodels, 1);
		normPm_yx.insert_rows(nmodels, 1);
		
		nmodels++;
	}
	
	double error() {
		if (nmodels == 0) {
			return 0.;
		}
		double error = 0.;
		for (int i = 0; i < ndata; ++i) {
			error += ::pow(y(i) - models[map_model(i)].predict(X.row(i)), 2);
		}
		return error;
	}
};

int main(int argc, char *argv[]) {
	mat X;
	vec y;
	
	/*
	rowvec xmin1(1), xmax1(1), xmin2(1), xmax2(1), a1(1), a2(1);
	
	xmin1(0) = 0.;
	xmax1(0) = 1.;
	xmin2(0) = 1.;
	xmax2(0) = 2.;
	
	a1(0) = 1.;
	a2(0) = 2.;
	
	gen_data(5, xmin1, xmax1, a1, 0, X, y);
	gen_data(5, xmin2, xmax2, a2, 3, X, y);
	*/
	
	load_data(X, y);
	
	EM em(X, y, 0.0001);
	for (int i = 0; i < 20; ++i) {
		em.step();
	}
	
	return 0;
}
