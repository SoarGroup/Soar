#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <limits>
#include <iomanip>
#include <memory>
#include "linear.h"
#include "em.h"
#include "common.h"
#include "filter_table.h"
#include "params.h"
#include "mat.h"

using namespace std;
using namespace Eigen;

const int INIT_NMODELS = 1;
const bool TEST_ELIGIBILITY = false;
const int REGRESSION_ALG = 1; // 1 = ridge regression

template <typename T>
class multi_combination_generator {
public:
	multi_combination_generator(const std::vector<std::vector<T> > &elems, bool allow_repeat)
	: elems(elems), indices(elems.size(), 0), allow_repeat(allow_repeat), finished(false)
	{
		empty = false;
		if (elems.empty()) {
			empty = true;
		} else {
			for (int i = 0; i < elems.size(); ++i) {
				if (elems[i].empty()) {
					empty = true;
					break;
				}
			}
		}
	}

	void reset() {
		finished = false;
		fill(indices.begin(), indices.end(), 0);
	}

	bool next(std::vector<T> &comb) {
		if (empty) {
			return false;
		}
		
		comb.resize(elems.size());
		std::set<int> s;
		while (!finished) {
			bool has_repeat = false;
			s.clear();
			for (int i = 0; i < elems.size(); ++i) {
				comb[i] = elems[i][indices[i]];
				if (!allow_repeat) {
					std::pair<std::set<int>::iterator, bool> p = s.insert(comb[i]);
					if (!p.second) {
						has_repeat = true;
						break;
					}
				}
			}
			increment(0);
			if (allow_repeat || !has_repeat) {
				return true;
			}
		}
		return false;
	}

private:
	void increment(int i) {
		if (i >= elems.size()) {
			finished = true;
		} else if (++indices[i] >= elems[i].size()) {
			indices[i] = 0;
			increment(i + 1);
		}
	}

	const std::vector<std::vector<T> > &elems;
	std::vector<int> indices;
	bool allow_repeat, finished, empty;
};

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

void kernel1(const cvec &d, cvec &w) {
	w.resize(d.size());
	for (int i = 0; i < d.size(); ++i) {
		w(i) = exp(-d(i));
	}
}

void kernel2(const cvec &d, cvec &w, double p) {
	w.resize(d.size());
	for (int i = 0; i < d.size(); ++i) {
		if (d(i) == 0.0) {
			w(i) = 10000.0;
		} else {
			w(i) = min(10000.0, 1.0 / (pow(d(i), p)));
		}
	}
}

void predict(const mat &C, const rvec &intercepts, const rvec &x, rvec &y) {
	y = (x * C) + intercepts;
}

void predict_all(const mat &C, const rvec &intercepts, const mat &X, mat &Y) {
	Y = (X * C).rowwise() + intercepts;
}

/*
 Try to find a set of at least n points that fits a single linear
 function well. The algorithm is:
 
 1. If input X has m non-static columns, assume it has rank = m + 1.
 2. Randomly choose 'rank' data points as the seed members for the
    linear function. Fit the function to the seed members.
 3. Compute the residuals of the function for all data points. Compute
    a weight vector based on the residuals and a kernel.
 4. Refit the linear function biased based on the weight vector.
    Repeat until convergence or the function fits at least n data points.
 5. If the process converged without fitting n data points, repeat
    from 2.
*/
bool mini_em(const_mat_view X, const_mat_view Y, int n, double fit_thresh, int maxiters, vector<int> &points) {
	int ndata = X.rows();
	cvec residuals(ndata), old_res(ndata);
	cvec w(ndata);
	vector<int> nonstatic, init_members;
	get_nonstatic_cols(X, X.cols(), nonstatic);
	int rank = nonstatic.size() + 1;
	mat Xd;
	pick_cols(X, nonstatic, Xd);
	
	for (int iter1 = 0; iter1 < maxiters; ++iter1) {
		w.setConstant(0.0);
		
		sample(rank, 0, ndata, false, init_members);
		for (int i = 0; i < init_members.size(); ++i) {
			w(init_members[i]) = 1.0;
		}
		
		mat C, PY;
		rvec intercepts;
		for (int iter2 = 0; iter2 < maxiters; ++iter2) {
			LOG(EMDBG) << "MINI_EM " << iter1 << " " << iter2 << endl;
			LOG(EMDBG) << "w = ";
			for (int i = 0; i < w.size(); ++i) {
				LOG(EMDBG) << w(i) << " ";
			}
			LOG(EMDBG) << endl;
			
			if (!OLS(Xd, Y, w, C, intercepts)) {
				break;
			}
			
			LOG(EMDBG) << "c = ";
			for (int i = 0; i < C.rows(); ++i) {
				LOG(EMDBG) << C(i, 0) << " ";
			}
			LOG(EMDBG) << endl;
			
			old_res = residuals;
			predict_all(C, intercepts, Xd, PY);
			residuals = (Y - PY).rowwise().squaredNorm();
			points.clear();
			for (int i = 0; i < residuals.size(); ++i) {
				if (residuals(i) < fit_thresh) {
					points.push_back(i);
				}
			}
			if (points.size() >= n) {
				LOG(EMDBG) << "residuals" << endl;
				for (int i = 0; i < residuals.size(); ++i) {
					LOG(EMDBG) << residuals(i) << " ";
				}
				LOG(EMDBG) << endl;
				return true;
			}
			
			if (residuals == old_res) {
				break;
			}
			kernel2(residuals, w, 3.0);
		}
	}
	return false;
}

bool block_seed(const_mat_view X, const_mat_view Y, int n, double fit_thresh, int maxiters, vector<int> &points) {
	int xcols = X.cols(), ycols = Y.cols();
	mat C, PY;
	rvec intercepts, py, w(MODEL_INIT_N);
	w.setConstant(1.0);
	for (int iter = 0; iter < maxiters; ++iter) {
		int start = rand() % (X.rows() - MODEL_INIT_N);
		dyn_mat Xb(X.block(start, 0, MODEL_INIT_N, xcols));
		dyn_mat Yb(Y.block(start, 0, MODEL_INIT_N, ycols));
		OLS(Xb.get(), Yb.get(), w, C, intercepts);

		predict_all(C, intercepts, Xb.get(), PY);
		double e = (Yb.get() - PY).rowwise().squaredNorm().sum();
		if (e > MODEL_ERROR_THRESH) {
			continue;
		}
		
		points.clear();
		for (int i = start; i < start + MODEL_INIT_N; ++i) {
			points.push_back(i);
		}
		
		for (int i = 0; i < X.rows(); ++i) {
			if (i < start || i >= start + MODEL_INIT_N) {
				predict(C, intercepts, X.row(i), py);
				if (pow(py(0) - Y(i, 0), 2) < MODEL_ADD_THRESH) {
					Xb.append_row(X.row(i));
					Yb.append_row(Y.row(i));
					points.push_back(i);
				}
			}
		}
		
		if (points.size() >= n) {
			return true;
		}
	}
	return false;
}

EM::EM(const std::vector<train_inst> &data, const std::vector<propvec_sig> &sigs)
: data(data), sigs(sigs), ndata(data.size()), nmodels(0),
  Py_z(0, 0, INIT_NMODELS, INIT_NDATA), 
  eligible(0, 0, INIT_NMODELS, INIT_NDATA)
{
	timers.add("e_step");
	timers.add("m_step");
	timers.add("new");
}

EM::~EM() {
	// need to delete mode models here?
}

void EM::update_eligibility() {
	if (!TEST_ELIGIBILITY) {
		return;
	}
	
	eligible.get().fill(1);
	for (int i = 0; i < ndata; ++i) {
		rvec x = data[i].x;
		for (int j = 0; j < nmodels; ++j) {
			const rvec &c2 = models[j]->model->get_center();
			rvec d = x - c2;
			for (int k = 0; k < nmodels; ++k) {
				const rvec &c1 = models[k]->model->get_center();
				if (j != k && d.dot(c1 - c2) < 0) {
					eligible(k, i) = 0;
					break;
				}
			}
		}
	}
}

/*
 Calculate probability of data point d belonging to mode m
*/
double EM::calc_prob(int m, const propvec_sig &sig, const rvec &x, double y, vector<int> &best_assign, double &best_error) const {
	rvec py;
	double w;
	if (TEST_ELIGIBILITY) {
		assert(false); // have to fix this later, or drop eligibility altogether
		//w = 1.0 / eligible.col(j).head(nmodels - 1).sum();
	} else {
		w = 1.0 / nmodels;
	}
	
	/*
	 Each model has a signature that specifies the types and orders of
	 objects it expects for inputs. This is recorded in models[m]->sig.
	 Call this the model signature.
	 
	 Each data point has a signature that specifies which types and
	 orders of object properties encoded by the property vector. This
	 is recorded in data[i].sig. Call this the data signature.
	 
	 P(d, m) = MAX[assignment][P(d, m, assignment)] where 'assignment'
	 is a mapping of objects in the data signature to the objects in
	 the model signature.
	*/
	
	/*
	 Create the input table for the combination generator to generate
	 all possible assignments. possibles[i] should be a list of all
	 object indices that can be assigned to position i in the model
	 signature.
	*/
	propvec_sig &msig = models[m]->sig;
	if (msig.empty()) {
		// should be constant prediction
		assert(models[m]->model->is_const());
		if (!models[m]->model->predict(rvec(), py)) {
			assert(false);
		}
		best_error = (y - py(0));
		best_assign.clear();
		double d = gausspdf(y, py(0), MODEL_STD);
		double p = (1.0 - EPSILON) * w * d;
		return p;
	}
	
	// otherwise, check all possible assignments
	vector<vector<int> > possibles(msig.size());
	int xlen = 0;
	for (int i = 0; i < msig.size(); ++i) {
		xlen += msig[i].length;
		for (int j = 0; j < sig.size(); ++j) {
			if (sig[j].type == msig[i].type) {
				possibles[i].push_back(j);
			}
		}
	}
	multi_combination_generator<int> gen(possibles, false);
	
	/*
	 Iterate through all assignments and find the one that gives
	 highest probability.
	*/
	vector<int> assign;
	rvec xc(xlen);
	double best_prob = -1.0;
	static int count = 0;
	cout << ++count << endl;
	while (gen.next(assign)) {
		int s = 0;
		for (int i = 0; i < assign.size(); ++i) {
			int l = sig[assign[i]].length;
			assert(msig[i].length == l);
			xc.segment(s, l) = x.segment(sig[assign[i]].start, l);
			s += l;
		}
		assert(s == xlen);
		
		if (models[m]->model->predict(xc, py)) {
			double d = gausspdf(y, py(0), MODEL_STD);
			double p = (1.0 - EPSILON) * w * d;
			if (p > best_prob) {
				best_prob = p;
				best_assign = assign;
				best_error = y - py(0);
			}
		}
	}
	assert(best_prob >= 0.0);
	return best_prob;
}

/*
 Update probability estimates for stale points for the i'th model. If
 for point j probability increases and i was not the MAP model or if
 probability decreases and i was the MAP model, then we mark j as a point
 we have to recalculate the MAP model for using the set "check". I
 don't recalculate MAP immediately because the probabilities for other
 models may change in the same round.
*/
void EM::update_Py_z(int i, set<int> &check) {
	set<int>::iterator j;
	for (j = models[i]->stale_points.begin(); j != models[i]->stale_points.end(); ++j) {
		double prev = Py_z(i, *j), now;
		int m = map_mode[*j];
		if (TEST_ELIGIBILITY && eligible(i, *j) == 0) {
			now = 0.;
		} else {
			vector<int> &obj_map = models[i]->data_map[*j].obj_map;
			double error;
			now = calc_prob(i, sigs[data[*j].sig_index], data[*j].x, data[*j].y(0), obj_map, error);
		}
		if ((m == i && now < prev) ||
		    (m != i && ((m == -1 && now > PNOISE) ||
		                (m != -1 && now > Py_z(m, *j)))))
		{
			check.insert(*j);
		}
		Py_z(i, *j) = now;
	}
	models[i]->stale_points.clear();
}

/*
 Add a training example into the model. The information about how the
 example fits into the model should already be calculated in
 EM::calc_prob (hence the assertion minfo.data_map.find(i) !=
 minfo.data_map.end()).
*/
void EM::model_add_example(int m, int i, bool update) {
	model_info &minfo = *models[m];
	assert(minfo.data_map.find(i) != minfo.data_map.end());
	model_data_info &dinfo = minfo.data_map[i];
	
	rvec xc(data[i].x.size());
	int xsize = 0;
	for (int j = 0; j < dinfo.obj_map.size(); ++j) {
		const propvec_sig &sig = sigs[data[i].sig_index];
		int n = sig[dinfo.obj_map[j]].length;
		int s = sig[dinfo.obj_map[j]].start;
		xc.segment(xsize, n) = data[i].x.segment(s, n);
		xsize += n;
	}
	xc.conservativeResize(xsize);
	dinfo.row = minfo.model->size();
	minfo.model->add_example(xc, data[i].y, update);
	minfo.stale = true;
}

void EM::model_del_example(int m, int i) {
	model_info &minfo = *models[m];
	assert(minfo.data_map.find(i) != minfo.data_map.end());
	int r = minfo.data_map[i].row;
	minfo.model->del_example(r);
	minfo.data_map.erase(i);
	
	map<int, model_data_info>::iterator j;
	for (j = minfo.data_map.begin(); j != minfo.data_map.end(); ++j) {
		if (j->second.row > r) {
			--(j->second.row);
		}
	}
	
	minfo.stale = true;
}

/* Recalculate MAP model for each index in points */
void EM::update_MAP(const set<int> &points) {
	set<int>::iterator j;
	for (j = points.begin(); j != points.end(); ++j) {
		int prev = map_mode[*j], now;
		if (nmodels == 0) {
			now = -1;
		} else {
			Py_z.get().topLeftCorner(nmodels, ndata).col(*j).maxCoeff(&now);
			if (Py_z(now, *j) < PNOISE) {
				now = -1;
			}
		}
		if (now != prev) {
			map_mode[*j] = now;
			if (prev == -1) {
				noise[data[*j].sig_index].erase(*j);
			} else {
				model_del_example(prev, *j);
			}
			if (now == -1) {
				noise[data[*j].sig_index].insert(*j);
			} else {
				model_add_example(now, *j, true);
			}
		}
	}
}

void EM::new_data() {
	++ndata;
	Py_z.append_col();
	if (TEST_ELIGIBILITY) {
		eligible.append_col();
	}
	map_mode.push_back(-1);
	noise[data.back().sig_index].insert(ndata - 1);
	for (int i = 0; i < nmodels; ++i) {
		models[i]->stale_points.insert(ndata - 1);
	}
}

void EM::estep() {
	function_timer t(timers.get(E_STEP_T));
	
	set<int> check;
	update_eligibility();
	for (int i = 0; i < nmodels; ++i) {
		update_Py_z(i, check);
	}
	
	update_MAP(check);
}

bool EM::mstep() {
	function_timer t(timers.get(M_STEP_T));
	
	bool changed = false;
	for (int i = 0; i < models.size(); ++i) {
		model_info &minfo = *models[i];
		if (minfo.stale) {
			LinearModel *m = minfo.model;
			if (m->needs_refit() && m->fit()) {
				changed = true;
				map<int, model_data_info>::const_iterator j;
				for (j = minfo.data_map.begin(); j != minfo.data_map.end(); ++j) {
					minfo.stale_points.insert(j->first);
				}
			}
			minfo.stale = false;
		}
	}
	return changed;
}

bool EM::find_new_mode_inds(const set<int> &noise_inds, int sig_ind, vector<int> &mode_inds) const {
	// matrices containing only the unique rows of xdata and ydata
	dyn_mat unique_x(0, data[0].x.cols()), unique_y(0, 1);
	
	// Y value -> vector of indexes into total data
	map<double, vector<int> > const_noise_inds;
	
	// index into unique data -> vector of indexes into total data
	vector<vector<int> > unique_map;

	set<int>::const_iterator ni;
	for (ni = noise_inds.begin(); ni != noise_inds.end(); ++ni) {
		int i = *ni;
		vector<int> &const_inds = const_noise_inds[data[i].y(0)];
		const_inds.push_back(i);
		if (const_inds.size() >= K) {
			LOG(EMDBG) << "found constant model in noise_inds" << endl;
			mode_inds = const_inds;
			return true;
		}
		
		bool new_unique = true;
		for (int j = 0; j < unique_map.size(); ++j) {
			if (data[i].x == unique_x.row(j) &&
			    data[i].y == unique_y.row(j))
			{
				unique_map[j].push_back(i);
				new_unique = false;
				break;
			}
		}
		if (new_unique) {
			unique_x.append_row(data[i].x);
			unique_y.append_row(data[i].y);
			unique_map.push_back(vector<int>());
			unique_map.back().push_back(i);
		}
	}
	
	if (unique_x.rows() < K) {
		return false;
	}
	
	vector<int> seed;
	if (!mini_em(unique_x.get(), unique_y.get(), K, MODEL_ERROR_THRESH, 10, seed) &&
		!block_seed(unique_x.get(), unique_y.get(), K, MODEL_ERROR_THRESH, 10, seed))
	{
		return false;
	}
	for (int i = 0; i < seed.size(); ++i) {
		const vector<int> &real_inds = unique_map[seed[i]];
		copy(real_inds.begin(), real_inds.end(), back_inserter(mode_inds));
	}
	return true;
}

bool EM::unify_or_add_model() {
	function_timer t(timers.get(NEW_T));
	
	map<int, set<int> >::iterator i;
	for (i = noise.begin(); i != noise.end(); ++i) {
		std::set<int> &noise_inds = i->second;
		if (noise_inds.size() < K) {
			continue;
		}
	
		vector<int> seed_inds;
		if (!find_new_mode_inds(noise_inds, i->first, seed_inds)) {
			return false;
		}
		
		mat X(seed_inds.size(), data[0].x.size());
		mat Y(seed_inds.size(), data[0].y.size());
		for (int j = 0; j < seed_inds.size(); ++j) {
			X.row(j) = data[seed_inds[j]].x;
			Y.row(j) = data[seed_inds[j]].y;
		}
	
		/*
		 Try to add noise data to each current model and refit. If the
		 resulting model is just as accurate as the original, then just
		 add the noise to that model instead of creating a new one.
		*/
		
		/*
		 I'm going to put off fixing this part up for the new style
		 models. For now just don't do this at all.
		*/
		
		/*
		for (int i = 0; i < nmodels; ++i) {
			LinearModel *unified(modes[i]->model->copy());
			unified->add_examples(m->get_members());
			unified->fit();
			
			double curr_error = models[i]->get_train_error();
			double uni_error = unified->get_train_error();
			
			if (uni_error < MODEL_ERROR_THRESH ||
				(curr_error > 0.0 && uni_error < UNIFY_MUL_THRESH * curr_error))
			{
				delete models[i];
				delete m;
				models[i] = unified;
				mark_model_stale(i);
				LOG(EMDBG) << "UNIFIED " << i << endl;
				return true;
			}
			delete unified;
		}
		*/
		
		LinearModel *m = new LinearModel(REGRESSION_ALG);
		vector<int> nonzero;
		m->init_fit(X, Y, sigs[i->first], nonzero);
		
		model_info *minfo = new model_info();
		minfo->model = m;
		for (int j = 0; j < nonzero.size(); ++j) {
			minfo->sig.push_back(sigs[i->first][nonzero[j]]);
			minfo->sig.back().start = -1;  // this field has no purpose and should never be used
		}
		for (int j = 0; j < seed_inds.size(); ++j) {
			model_data_info &data_info = minfo->data_map[seed_inds[j]];
			data_info.row = j;
			data_info.obj_map = nonzero;
		}
		models.push_back(minfo);
		
		mark_model_stale(nmodels);
		++nmodels;
		Py_z.append_row();
		if (TEST_ELIGIBILITY) {
			eligible.append_row();
		}
		
		return true;
	}
	return false;
}

void EM::mark_model_stale(int i) {
	models[i]->stale = true;
	for (int j = 0; j < ndata; ++j) {
		models[i]->stale_points.insert(j);
	}
}

bool EM::predict(int mode, const rvec &x, double &y) {
	//timer t("EM PREDICT TIME");
	assert(0 <= mode && mode < nmodels);
	if (ndata == 0) {
		return false;
	}
	
	rvec py;
	if (!models[mode]->model->predict(x, py)) {
		assert(false);
	}
	y = py(0);
	return true;
}

/*
 Remove all models that cover fewer than 2 data points.
*/
bool EM::remove_models() {
	if (nmodels == 0) {
		return false;
	}
	
	/*
	 i is the first free model index. If model j should be kept, all
	 information pertaining to model j will be copied to row/element i
	 in the respective matrix/vector, and i will be incremented. Most
	 efficient way I can think of to remove elements from the middle
	 of vectors. index_map associates old j's to new i's.
	*/
	bool removed = false;
	vector<int> index_map(nmodels);
	int i = 0;
	for (int j = 0; j < nmodels; ++j) {
		if (models[j]->data_map.size() > 2) {
			index_map[j] = i;
			if (j > i) {
				models[i] = models[j];
				Py_z.row(i) = Py_z.row(j);
				if (TEST_ELIGIBILITY) {
					eligible.row(i) = eligible.row(j);
				}
			}
			i++;
		} else {
			index_map[j] = -1;
			delete models[j];
			removed = true;
		}
	}
	for (int j = 0; j < ndata; ++j) {
		if (map_mode[j] >= 0) {
			map_mode[j] = index_map[map_mode[j]];
		}
	}
	
	nmodels = i;
	Py_z.resize(nmodels, ndata);
	if (TEST_ELIGIBILITY) {
		eligible.resize(nmodels, ndata);
	}
	models.resize(nmodels);
	return removed;
}

bool EM::step() {
	estep();
	return mstep();
}

bool EM::run(int maxiters) {
	bool changed = false;
	for (int i = 0; i < maxiters; ++i) {
		if (!step()) {
			if (!remove_models() && !unify_or_add_model()) {
				// reached quiescence
				return changed;
			}
		}
		changed = true;
	}
	LOG(EMDBG) << "Reached max iterations without quiescence" << endl;
	return changed;
}

void EM::save(ostream &os) const {
	Py_z.save(os);
	if (TEST_ELIGIBILITY) {
		eligible.save(os);
	}
	save_vector(map_mode, os);
	
	os << models.size() << endl;
	for (int i = 0; i < nmodels; ++i) {
		models[i]->save(os);
	}
}

void EM::load(istream &is) {
	Py_z.load(is);
	if (TEST_ELIGIBILITY) {
		eligible.load(is);
	}
	load_vector(map_mode, is);
	
	is >> nmodels;
	for (int i = 0; i < nmodels; ++i) {
		model_info *minfo = new model_info;
		minfo->load(is);
		models.push_back(minfo);
	}
}

int EM::best_mode(const propvec_sig &sig, const rvec &x, double y, double &besterror) const {
	int best = -1;
	double best_prob = 0.0;
	rvec py;
	vector<int> assign;
	double error;
	for (int i = 0; i < nmodels; ++i) {
		double p = calc_prob(i, sig, x, y, assign, error);
		if (best == -1 || p > best_prob) {
			best = i;
			best_prob = p;
			besterror = error;
		}
	}
	return best;
}

bool EM::cli_inspect(int first_arg, const vector<string> &args, ostream &os) const {
	stringstream ss;

	if (first_arg >= args.size()) {
		os << "modes: " << nmodels << endl;
		os << endl << "subqueries: mode ptable timing noise" << endl;
		return true;
	} else if (args[first_arg] == "ptable") {
		for (int i = 0; i < ndata; ++i) {
			for (int j = 0; j < nmodels; ++j) {
				os << Py_z(j, i) << "\t";
			}
			os << endl;
		}
		return true;
	} else if (args[first_arg] == "mode") {
		if (first_arg + 1 >= args.size()) {
			os << "Specify a mode number (0 - " << nmodels - 1 << ")" << endl;
			return false;
		}
		int n;
		if (!parse_int(args[first_arg+1], n) || n < 0 || n >= nmodels) {
			os << "invalid model number" << endl;
			return false;
		}
		return models[n]->model->cli_inspect(first_arg + 2, args, os);
	} else if (args[first_arg] == "timing") {
		timers.report(os);
		return true;
	} else if (args[first_arg] == "noise") {
		map<int, set<int> >::const_iterator i;
		for (i = noise.begin(); i != noise.end(); ++i) {
			join(os, i->second, " ") << endl;
		}
		return true;
	}
	
	return false;
}

