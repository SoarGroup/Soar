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

template <typename T>
void remove_from_vector(const vector<int> &inds, vector <T> &v) {
	int i = 0, j = 0;
	for (int k = 0; k < v.size(); ++k) {
		if (i < inds.size() && k == inds[i]) {
			++i;
		} else {
			if (k > j) {
				v[j] = v[k];
			}
			j++;
		}
	}
	assert(v.size() - inds.size() == j);
	v.resize(j);
}

EM::EM(const std::vector<train_inst> &data, const std::vector<propvec_sig> &sigs)
: data(data), sigs(sigs), ndata(data.size()), nmodes(0)
{
	timers.add("e_step");
	timers.add("m_step");
	timers.add("new");
	
	assert(ndata == 0);
}

EM::~EM() {
	// need to delete mode models here?
}

void EM::update_eligibility() {
	if (!TEST_ELIGIBILITY) {
		return;
	}
	for (int i = 0; i < ndata; ++i) {
		const rvec &x = data[i].x;
		for (int j = 0; j < nmodes; ++j) {
			const rvec &c2 = modes[j]->model->get_center();
			rvec d = x - c2;
			for (int k = 0; k < nmodes; ++k) {
				const rvec &c1 = modes[k]->model->get_center();
				if (j != k && d.dot(c1 - c2) < 0) {
					em_info[i].mode_prob[k] = -1.0;
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
		//w = 1.0 / eligible.col(j).head(nmodes - 1).sum();
	} else {
		w = 1.0 / nmodes;
	}
	
	/*
	 Each model has a signature that specifies the types and orders of
	 objects it expects for inputs. This is recorded in modes[m]->sig.
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
	propvec_sig &msig = modes[m]->sig;
	if (msig.empty()) {
		// should be constant prediction
		assert(modes[m]->model->is_const());
		if (!modes[m]->model->predict(rvec(), py)) {
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
		
		if (modes[m]->model->predict(xc, py)) {
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
void EM::update_mode_prob(int i, set<int> &check, obj_map_table &obj_maps) {
	set<int>::iterator j;
	for (j = modes[i]->stale_points.begin(); j != modes[i]->stale_points.end(); ++j) {
		em_data_info &dinfo = em_info[*j];
		double prev = dinfo.mode_prob[i], now;
		int m = dinfo.map_mode;
		if (TEST_ELIGIBILITY && prev < 0.0) {
			now = -1.0;
		} else {
			vector<int> &obj_map = dinfo.obj_map;
			double error;
			now = calc_prob(i, sigs[data[*j].sig_index], data[*j].x, data[*j].y(0),
			                obj_maps[make_pair(i, *j)], error);
		}
		if ((m == i && now < prev) ||
		    (m != i && ((m == -1 && now > PNOISE) ||
		                (m != -1 && now > dinfo.mode_prob[m]))))
		{
			check.insert(*j);
		}
		dinfo.mode_prob[i] = now;
	}
	modes[i]->stale_points.clear();
}

/*
 Add a training example into the model. The information about how the
 example fits into the model should already be calculated in
 EM::calc_prob (hence the assertion minfo.data_map.find(i) !=
 minfo.data_map.end()).
*/
void EM::model_add_example(int m, int i, bool update) {
	mode_info &minfo = *modes[m];
	em_data_info &dinfo = em_info[i];
	
	rvec xc(data[i].x.size());
	int xsize = 0;
	const propvec_sig &sig = sigs[data[i].sig_index];
	for (int j = 0; j < dinfo.obj_map.size(); ++j) {
		int n = sig[dinfo.obj_map[j]].length;
		int s = sig[dinfo.obj_map[j]].start;
		xc.segment(xsize, n) = data[i].x.segment(s, n);
		xsize += n;
	}
	xc.conservativeResize(xsize);
	dinfo.model_row = minfo.model->add_example(xc, data[i].y, update);
	minfo.members.insert(i);
	minfo.stale = true;
}

void EM::model_del_example(int m, int i) {
	mode_info &minfo = *modes[m];
	int r = em_info[i].model_row;
	minfo.model->del_example(r);
	minfo.members.erase(i);
	
	set<int>::iterator j;
	for (j = minfo.members.begin(); j != minfo.members.end(); ++j) {
		if (em_info[*j].model_row > r) {
			em_info[*j].model_row--;
		}
	}
	minfo.stale = true;
}

/* Recalculate MAP model for each index in points */
void EM::update_MAP(const set<int> &points, const obj_map_table &obj_maps) {
	set<int>::iterator j;
	for (j = points.begin(); j != points.end(); ++j) {
		em_data_info &dinfo = em_info[*j];
		int prev = dinfo.map_mode, now;
		if (nmodes == 0) {
			now = -1;
		} else {
			now = argmax(dinfo.mode_prob);
			if (dinfo.mode_prob[now] < PNOISE) {
				now = -1;
			}
		}
		if (now != prev) {
			dinfo.map_mode = now;
			if (prev == -1) {
				noise[data[*j].sig_index].erase(*j);
			} else {
				model_del_example(prev, *j);
			}
			if (now == -1) {
				noise[data[*j].sig_index].insert(*j);
			} else {
				obj_map_table::const_iterator i = obj_maps.find(make_pair(now, *j));
				assert(i != obj_maps.end());
				dinfo.obj_map = i->second;
				model_add_example(now, *j, true);
			}
		}
	}
}

void EM::new_data() {
	em_info.push_back(em_data_info());
	em_data_info &dinfo = em_info.back();
	dinfo.map_mode = -1;
	dinfo.mode_prob.resize(nmodes);
	
	noise[data.back().sig_index].insert(ndata);
	for (int i = 0; i < nmodes; ++i) {
		modes[i]->stale_points.insert(ndata);
	}
	++ndata;
}

void EM::estep() {
	function_timer t(timers.get(E_STEP_T));
	
	set<int> check;
	update_eligibility();
	
	obj_map_table obj_maps;
	for (int i = 0; i < nmodes; ++i) {
		update_mode_prob(i, check, obj_maps);
	}
	
	update_MAP(check, obj_maps);
}

bool EM::mstep() {
	function_timer t(timers.get(M_STEP_T));
	
	bool changed = false;
	for (int i = 0; i < modes.size(); ++i) {
		mode_info &minfo = *modes[i];
		if (minfo.stale) {
			LinearModel *m = minfo.model;
			if (m->needs_refit() && m->fit()) {
				changed = true;
				union_sets_inplace(minfo.stale_points, minfo.members);
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
		for (int i = 0; i < nmodes; ++i) {
			LinearModel *unified(modes[i]->model->copy());
			unified->add_examples(m->get_members());
			unified->fit();
			
			double curr_error = modes[i]->model->get_train_error();
			double uni_error = unified->get_train_error();
			
			if (uni_error < MODEL_ERROR_THRESH ||
				(curr_error > 0.0 && uni_error < UNIFY_MUL_THRESH * curr_error))
			{
				delete modes[i]->model;
				delete m;
				modes[i]->model = unified;
				mark_mode_stale(i);
				LOG(EMDBG) << "UNIFIED " << i << endl;
				return true;
			}
			delete unified;
		}
		*/
		
		LinearModel *m = new LinearModel(REGRESSION_ALG);
		vector<int> obj_map;
		m->init_fit(X, Y, sigs[i->first], obj_map);
		
		mode_info *minfo = new mode_info();
		minfo->model = m;
		copy(seed_inds.begin(), seed_inds.end(), inserter(minfo->members, minfo->members.end()));
		for (int j = 0; j < obj_map.size(); ++j) {
			minfo->sig.push_back(sigs[i->first][obj_map[j]]);
			minfo->sig.back().start = -1;  // this field has no purpose and should never be used
		}
		for (int j = 0; j < seed_inds.size(); ++j) {
			em_data_info &dinfo = em_info[seed_inds[j]];
			dinfo.map_mode = modes.size();
			dinfo.model_row = j;
			dinfo.obj_map = obj_map;
			noise_inds.erase(seed_inds[j]);
		}
		modes.push_back(minfo);
		
		mark_mode_stale(nmodes);
		++nmodes;
		for (int j = 0; j < ndata; ++j) {
			em_info[j].mode_prob.push_back(0.0);
		}
		
		return true;
	}
	return false;
}

void EM::mark_mode_stale(int i) {
	modes[i]->stale = true;
	for (int j = 0; j < ndata; ++j) {
		modes[i]->stale_points.insert(j);
	}
}

bool EM::predict(int mode, const rvec &x, double &y) {
	//timer t("EM PREDICT TIME");
	assert(0 <= mode && mode < nmodes);
	if (ndata == 0) {
		return false;
	}
	
	rvec py;
	if (!modes[mode]->model->predict(x, py)) {
		assert(false);
	}
	y = py(0);
	return true;
}

/*
 Remove all modes that cover fewer than 2 data points.
*/
bool EM::remove_modes() {
	if (nmodes == 0) {
		return false;
	}
	
	/*
	 i is the first free model index. If model j should be kept, all
	 information pertaining to model j will be copied to row/element i
	 in the respective matrix/vector, and i will be incremented. Most
	 efficient way I can think of to remove elements from the middle
	 of vectors. index_map associates old j's to new i's.
	*/
	vector<int> index_map(nmodes), removed;
	int i = 0;
	for (int j = 0; j < nmodes; ++j) {
		if (modes[j]->members.size() > 2) {
			index_map[j] = i;
			if (j > i) {
				modes[i] = modes[j];
			}
			i++;
		} else {
			index_map[j] = -1;
			delete modes[j];
			removed.push_back(j);
		}
	}
	if (removed.empty()) {
		return false;
	}
	for (int j = 0; j < ndata; ++j) {
		em_data_info &dinfo = em_info[j];
		if (dinfo.map_mode >= 0) {
			dinfo.map_mode = index_map[dinfo.map_mode];
		}
		remove_from_vector(removed, dinfo.mode_prob);
	}
	assert(i == nmodes - removed.size());
	nmodes = i;
	modes.resize(nmodes);
	return true;
}

bool EM::step() {
	estep();
	return mstep();
}

bool EM::run(int maxiters) {
	bool changed = false;
	for (int i = 0; i < maxiters; ++i) {
		if (!step()) {
			if (!remove_modes() && !unify_or_add_model()) {
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
	save_vector_rec(em_info, os);
	save_vector_recp(modes, os);
}

void EM::load(istream &is) {
	load_vector_rec(em_info, is);
	load_vector_recp(modes, is);
}

int EM::best_mode(const propvec_sig &sig, const rvec &x, double y, double &besterror) const {
	int best = -1;
	double best_prob = 0.0;
	rvec py;
	vector<int> assign;
	double error;
	for (int i = 0; i < nmodes; ++i) {
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
		os << "modes: " << nmodes << endl;
		os << endl << "subqueries: mode ptable timing noise" << endl;
		return true;
	} else if (args[first_arg] == "ptable") {
		for (int i = 0; i < ndata; ++i) {
			join(os, em_info[i].mode_prob, "\t") << endl;
		}
		return true;
	} else if (args[first_arg] == "mode") {
		if (first_arg + 1 >= args.size()) {
			os << "Specify a mode number (0 - " << nmodes - 1 << ")" << endl;
			return false;
		}
		int n;
		if (!parse_int(args[first_arg+1], n) || n < 0 || n >= nmodes) {
			os << "invalid model number" << endl;
			return false;
		}
		return modes[n]->model->cli_inspect(first_arg + 2, args, os);
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

void EM::get_map_modes(std::vector<int> &modes) const {
	modes.reserve(ndata);
	for (int i = 0; i < ndata; ++i) {
		modes.push_back(em_info[i].map_mode);
	}
}

void EM::em_data_info::save(ostream &os) const {
	save_vector(mode_prob, os);
	os << map_mode << endl;
	save_vector(obj_map, os);
	os << model_row << endl;
}

void EM::em_data_info::load(istream &is) {
	load_vector(mode_prob, is);
	is >> map_mode;
	load_vector(obj_map, is);
	is >> model_row;
}
