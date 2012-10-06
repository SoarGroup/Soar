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
#include "serialize.h"
#include "scene.h"

using namespace std;
using namespace Eigen;

const bool TEST_ELIGIBILITY = false;
const regression_type REGRESSION_ALG = FORWARD;

/*
 Generate all possible combinations of sets of items
*/
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

void read_til_semi(istream &is, vector<double> &buf) {
	string s;
	while (true) {
		double x;
		is >> s;
		if (s == ";") {
			return;
		}
		if (!parse_double(s, x)) {
			assert(false);
		}
		buf.push_back(x);
	}
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
	vector<int> nonuniform, init_members;
	get_nonuniform_cols(X, X.cols(), nonuniform);
	int rank = nonuniform.size() + 1;
	mat Xd;
	pick_cols(X, nonuniform, Xd);
	
	for (int iter1 = 0; iter1 < maxiters; ++iter1) {
		w.setConstant(0.0);
		
		sample(rank, 0, ndata, init_members);
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
			
			if (!linear_regression(OLS, Xd, Y, w, C, intercepts)) {
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
		linear_regression(OLS, Xb.get(), Yb.get(), w, C, intercepts);

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

EM::EM()
: ndata(0), nmodes(0)
{
	timers.add("e_step");
	timers.add("m_step");
	timers.add("new");
}

EM::~EM() {
	for (int i = 0; i < data.size(); ++i) {
		delete data[i];
	}
	for (int i = 0; i < modes.size(); ++i) {
		delete modes[i];
	}
}

void EM::update_eligibility() {
	if (!TEST_ELIGIBILITY) {
		return;
	}
	for (int i = 0; i < ndata; ++i) {
		const rvec &x = data[i]->x;
		for (int j = 0; j < nmodes; ++j) {
			const rvec &c2 = modes[j]->model->get_center();
			rvec d = x - c2;
			for (int k = 0; k < nmodes; ++k) {
				const rvec &c1 = modes[k]->model->get_center();
				if (j != k && d.dot(c1 - c2) < 0) {
					data[i]->mode_prob[k] = -1.0;
					break;
				}
			}
		}
	}
}

/*
 Calculate probability of data point d belonging to mode m
*/
double EM::calc_prob(int m, const scene_sig &sig, const rvec &x, double y, int target, vector<int> &best_assign, double &best_error) const {
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
	 is recorded in data[i]->sig_index. Call this the data signature.
	 
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
	scene_sig &msig = modes[m]->sig;
	if (msig.empty()) {
		// should be constant prediction
		assert(modes[m]->model->is_const());
		if (!modes[m]->model->predict(rvec(), py)) {
			assert(false);
		}
		best_error = (y - py(0));
		best_assign.clear();
		double d = gausspdf(y, py(0), MEASURE_VAR);
		double p = (1.0 - EPSILON) * w * d;
		return p;
	}
	
	// otherwise, check all possible assignments
	vector<vector<int> > possibles(msig.size());
	for (int i = 0; i < msig.size(); ++i) {
		if (i == modes[m]->target) {
			possibles[i].push_back(target);
		} else {
			for (int j = 0; j < sig.size(); ++j) {
				if (sig[j].type == msig[i].type && j != target) {
					possibles[i].push_back(j);
				}
			}
		}
	}
	multi_combination_generator<int> gen(possibles, false);
	
	/*
	 Iterate through all assignments and find the one that gives
	 highest probability.
	*/
	vector<int> assign;
	int xlen = msig.dim();
	rvec xc(xlen);
	double best_prob = -1.0;
	while (gen.next(assign)) {
		int s = 0;
		for (int i = 0; i < assign.size(); ++i) {
			int l = sig[assign[i]].props.size();
			assert(msig[i].props.size() == l);
			xc.segment(s, l) = x.segment(sig[assign[i]].start, l);
			s += l;
		}
		assert(s == xlen);
		
		if (modes[m]->model->predict(xc, py)) {
			double d = gausspdf(y, py(0), MEASURE_VAR);
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
void EM::update_mode_prob(int i, set<int> &check) {
	set<int>::iterator j;
	mode_info &minfo = *modes[i];
	for (j = minfo.stale_points.begin(); j != minfo.stale_points.end(); ++j) {
		em_data &dinfo = *data[*j];
		double prev = dinfo.mode_prob[i], now;
		int m = dinfo.map_mode;
		if (TEST_ELIGIBILITY && prev < 0.0) {
			now = -1.0;
		} else {
			vector<int> &obj_map = dinfo.obj_map;
			double error;
			now = calc_prob(i, sigs[dinfo.sig_index], dinfo.x, dinfo.y(0), dinfo.target,
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
	minfo.stale_points.clear();
}

/*
 Add a training example into the model. The information about how the
 example fits into the model should already be calculated in
 EM::calc_prob (hence the assertion minfo.data_map.find(i) !=
 minfo.data_map.end()).
*/
void EM::mode_add_example(int m, int i, bool update) {
	mode_info &minfo = *modes[m];
	em_data &dinfo = *data[i];
	
	rvec xc;
	if (!minfo.model->is_const()) {
		xc.resize(dinfo.x.size());
		int xsize = 0;
		const scene_sig &sig = sigs[dinfo.sig_index];
		for (int j = 0; j < dinfo.obj_map.size(); ++j) {
			int n = sig[dinfo.obj_map[j]].props.size();
			int s = sig[dinfo.obj_map[j]].start;
			xc.segment(xsize, n) = dinfo.x.segment(s, n);
			xsize += n;
		}
		xc.conservativeResize(xsize);
	}
	dinfo.model_row = minfo.model->add_example(xc, dinfo.y, update);
	
	minfo.members.insert(i);
	minfo.stale = true;
	minfo.pos.add(i, dinfo.target);
	minfo.neg.del(i, dinfo.target);
	minfo.clauses_dirty = true;
}

void EM::mode_del_example(int m, int i) {
	mode_info &minfo = *modes[m];
	em_data &dinfo = *data[i];
	int r = dinfo.model_row;
	minfo.model->del_example(r);
	minfo.members.erase(i);
	
	set<int>::iterator j;
	for (j = minfo.members.begin(); j != minfo.members.end(); ++j) {
		if (data[*j]->model_row > r) {
			data[*j]->model_row--;
		}
	}
	minfo.stale = true;
	minfo.pos.del(i, dinfo.target);
	minfo.neg.add(i, dinfo.target);
	minfo.clauses_dirty = true;
}

/* Recalculate MAP model for each index in points */
void EM::update_MAP(const set<int> &points) {
	set<int>::iterator j;
	for (j = points.begin(); j != points.end(); ++j) {
		em_data &dinfo = *data[*j];
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
				noise[dinfo.sig_index].erase(*j);
			} else {
				mode_del_example(prev, *j);
			}
			if (now == -1) {
				noise[dinfo.sig_index].insert(*j);
			} else {
				obj_map_table::const_iterator i = obj_maps.find(make_pair(now, *j));
				assert(i != obj_maps.end());
				dinfo.obj_map = i->second;
				mode_add_example(now, *j, true);
			}
		}
	}
}

void EM::learn(const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y) {
	int sig_index = -1, target = -1;
	for (int i = 0; i < sigs.size(); ++i) {
		if (sigs[i] == sig) {
			sig_index = i;
			break;
		}
	}
	
	if (sig_index < 0) {
		sigs.push_back(sig);
		sig_index = sigs.size() - 1;
	}

	for (int i = 0; i < sig.size(); ++i) {
		if (sig[i].target == 0) {
			target = sig[i].id;
		} else {
			assert(sig[i].target == -1);
		}
	}
	
	em_data *dinfo = new em_data;
	dinfo->x = x;
	dinfo->y = y;
	dinfo->target = target;
	dinfo->sig_index = sig_index;
	
	dinfo->map_mode = -1;
	dinfo->mode_prob.resize(nmodes);
	data.push_back(dinfo);
	
	noise[sig_index].insert(ndata);
	for (int i = 0; i < nmodes; ++i) {
		modes[i]->stale_points.insert(ndata);
		modes[i]->neg.add(ndata, dinfo->target);
	}
	extend_relations(rels, ndata);
	++ndata;
}

void EM::estep() {
	function_timer t(timers.get(E_STEP_T));
	
	set<int> check;
	update_eligibility();
	
	obj_map_table obj_maps;
	for (int i = 0; i < nmodes; ++i) {
		update_mode_prob(i, check);
	}
	
	update_MAP(check);
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

void EM::fill_xy(const vector<int> &rows, mat &X, mat &Y) const {
	if (rows.empty()) {
		X.resize(0, 0);
		Y.resize(0, 0);
		return;
	}

	X.resize(rows.size(), data[rows[0]]->x.size());
	Y.resize(rows.size(), 1);

	for (int i = 0; i < rows.size(); ++i) {
		X.row(i) = data[rows[i]]->x;
		Y.row(i) = data[rows[i]]->y;
	}
}

bool EM::find_new_mode_inds(const set<int> &noise_inds, int sig_ind, vector<int> &mode_inds) const {
	// matrices containing only the unique rows of xdata and ydata
	dyn_mat unique_x(0, data[0]->x.cols()), unique_y(0, 1);
	
	// Y value -> vector of indexes into total data
	map<double, vector<int> > const_noise_inds;
	
	// index into unique data -> vector of indexes into total data
	vector<vector<int> > unique_map;

	set<int>::const_iterator ni;
	for (ni = noise_inds.begin(); ni != noise_inds.end(); ++ni) {
		int i = *ni;
		em_data &dinfo = *data[i];
		vector<int> &const_inds = const_noise_inds[dinfo.y(0)];
		const_inds.push_back(i);
		if (const_inds.size() >= NOISE_SIZE_THRESH) {
			LOG(EMDBG) << "found constant model in noise_inds" << endl;
			mode_inds = const_inds;
			return true;
		}
		
		bool new_unique = true;
		for (int j = 0; j < unique_map.size(); ++j) {
			if (dinfo.x == unique_x.row(j) &&
			    dinfo.y == unique_y.row(j))
			{
				unique_map[j].push_back(i);
				new_unique = false;
				break;
			}
		}
		if (new_unique) {
			unique_x.append_row(dinfo.x);
			unique_y.append_row(dinfo.y);
			unique_map.push_back(vector<int>());
			unique_map.back().push_back(i);
		}
	}
	
	if (unique_x.rows() < NOISE_SIZE_THRESH) {
		return false;
	}
	
	vector<int> seed;
	if (!mini_em(unique_x.get(), unique_y.get(), NOISE_SIZE_THRESH, MODEL_ERROR_THRESH, 10, seed) &&
		!block_seed(unique_x.get(), unique_y.get(), NOISE_SIZE_THRESH, MODEL_ERROR_THRESH, 10, seed))
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
		if (noise_inds.size() < NOISE_SIZE_THRESH) {
			continue;
		}
	
		vector<int> seed_inds;
		if (!find_new_mode_inds(noise_inds, i->first, seed_inds)) {
			return false;
		}
		
		int xsize = data[seed_inds[0]]->x.size();
		int ysize = data[seed_inds[0]]->y.size();
		LinearModel *m = new LinearModel(REGRESSION_ALG);
		int seed_sig = data[seed_inds[0]]->sig_index;
		mat X, Y;
	
		/*
		 Try to add noise data to each current model and refit. If the
		 resulting model is just as accurate as the original, then just
		 add the noise to that model instead of creating a new one.
		*/
		
		vector<int> obj_map;
		bool unified = false;
		for (int j = 0; j < nmodes; ++j) {
			mode_info &minfo = *modes[j];
			set<int> &members = minfo.members;

			bool same_sig = true;
			set<int>::const_iterator k;
			for (k = members.begin(); k != members.end(); ++k) {
				if (data[*k]->sig_index != seed_sig) {
					same_sig = false;
					break;
				}
			}
			if (!same_sig) {
				continue;
			}
			vector<int> combined;
			extend(combined, members);
			extend(combined, seed_inds);
			
			fill_xy(combined, X, Y);
			m->init_fit(X, Y, sigs[seed_sig], obj_map);
			double curr_error = minfo.model->get_train_error();
			double combined_error = m->get_train_error();
			
			if (combined_error < MODEL_ERROR_THRESH ||
				(curr_error > 0.0 && combined_error < UNIFY_MUL_THRESH * curr_error))
			{
				init_mode(j, seed_sig, m, combined, obj_map);
				LOG(EMDBG) << "UNIFIED " << j << endl;
				unified = true;
				break;
			}
		}
		
		if (!unified) {
			fill_xy(seed_inds, X, Y);
			m->init_fit(X, Y, sigs[i->first], obj_map);
			modes.push_back(new mode_info);
			for (int j = 0; j < ndata; ++j) {
				data[j]->mode_prob.push_back(0.0);
			}
			++nmodes;
			init_mode(nmodes - 1, i->first, m, seed_inds, obj_map);
		}

		for (int j = 0; j < seed_inds.size(); ++j) {
			noise_inds.erase(seed_inds[j]);
		}
		return true;
	}
	return false;
}

void EM::init_mode(int mode, int sig, LinearModel *m, const vector<int> &members, const vector<int> &obj_map) {
	assert(!members.empty());

	mode_info &minfo = *modes[mode];
	int target = data[members.front()]->target;
	if (minfo.model) {
		delete minfo.model;
	}
	minfo.model = m;
	minfo.target = -1;
	minfo.members.clear();
	minfo.sig.clear();
	extend(minfo.members, members);
	if (m->is_const()) {
		minfo.target = 0;
		minfo.sig.add(sigs[sig][target]);
	} else {
		for (int i = 0; i < obj_map.size(); ++i) {
			if (obj_map[i] == target) {
				minfo.target = i;
			}
			minfo.sig.add(sigs[sig][obj_map[i]]);
		}
	}
	minfo.obj_clauses.resize(minfo.sig.size());
	minfo.pos.clear();
	minfo.neg.clear();
	// there's probably a more efficient way to initialize neg
	for (int i = 0; i < ndata; ++i) {
		minfo.neg.add(i, target);
	}
	for (int i = 0; i < members.size(); ++i) {
		em_data &dinfo = *data[members[i]];
		dinfo.map_mode = mode;
		dinfo.model_row = i;
		dinfo.obj_map = obj_map;
		minfo.pos.add(members[i], dinfo.target);
		minfo.neg.del(members[i], dinfo.target);
	}
	mark_mode_stale(mode);
}

void EM::mark_mode_stale(int i) {
	modes[i]->stale = true;
	for (int j = 0; j < ndata; ++j) {
		modes[i]->stale_points.insert(j);
	}
}

bool EM::map_objs(int mode, int target, const scene_sig &sig, const relation_table &rels, vector<int> &mapping) const {
	const mode_info &minfo = *modes[mode];
	vector<bool> used(sig.size(), false);
	used[target] = true;
	
	for (int i = 0; i < minfo.sig.size(); ++i) {
		int obj = -1;
		if (i == minfo.target) {
			// target always maps to target
			mapping[i] = target;
		} else {
			set<int> candidates;
			for (int j = 0; j < used.size(); ++j) {
				if (!used[j] && sig[j].type == minfo.sig[i].type) {
					candidates.insert(j);
				}
			}
			if (candidates.empty()) {
				return false;
			} else if (candidates.size() == 1 || minfo.obj_clauses[i].empty()) {
				mapping[i] = *candidates.begin();
			} else {
				map<int, int> assign;
				assign[0] = 0;
				assign[1] = target;
				if (test_clause_vec(minfo.obj_clauses[i], rels, candidates, assign) < 0) {
					return false;
				}
				assert(assign.find(2) != assign.end());
				mapping[i] = assign[2];
			}
		}
		used[mapping[i]] = true;
	}
	return true;
}

bool EM::predict(const scene_sig &sig, const relation_table &rels, const rvec &x, int &mode, rvec &y) {
	if (ndata == 0 || nmodes < 0) {
		mode = -1;
		return false;
	}
	
	int target = -1;
	set<int> all_objs;
	for (int i = 0; i < sig.size(); ++i) {
		all_objs.insert(i);
		if (sig[i].target == 0) {
			target = i;
		}
	}
	assert(target != -1);
		
	for (int i = 0; i < modes.size(); ++i) {
		mode_info &minfo = *modes[i];
		if (minfo.sig.size() > sig.size()) {
			continue;
		}
		update_clauses(i);
		vector<int> mapping(minfo.sig.size(), -1);
		if (!map_objs(i, target, sig, rels, mapping)) {
			continue;
		}
		
		map<int, int> assign;
		assign[0] = 0;
		int c = test_clause_vec(minfo.mode_clauses, rels, all_objs, assign);
		if (c >= 0) {
			LOG(EMDBG) << "mode " << i << " clause " << c << " satisfied" << endl;
			LOG(EMDBG) << minfo.mode_clauses[c] << endl;
			map<int, int>::const_iterator j;
			for (j = assign.begin(); j != assign.end(); ++j) {
				LOG(EMDBG) << j->first << " = " << j->second << endl;
			}
			rvec xc;
			if (!minfo.model->is_const()) {
				xc.resize(x.size());
				int xsize = 0;
				for (int j = 0; j < mapping.size(); ++j) {
					int n = sig[mapping[j]].props.size();
					xc.segment(xsize, n) = x.segment(sig[mapping[j]].start, n);
					xsize += n;
				}
				xc.conservativeResize(xsize);
			}
			if (minfo.model->predict(xc, y)) {
				mode = i;
				return true;
			}
		}
	}
	LOG(EMDBG) << "no suitable modes" << endl;
	mode = -1;
	return false;
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
		em_data &dinfo = *data[j];
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

int EM::best_mode(const scene_sig &sig, const rvec &x, double y, double &besterror) const {
	int target = -1;
	for (int i = 0; i < sig.size(); ++i) {
		if (sig[i].target == 0) {
			target = i;
			break;
		}
	}
	assert(target != -1);

	int best = -1;
	double best_prob = 0.0;
	rvec py;
	vector<int> assign;
	double error;
	for (int i = 0; i < nmodes; ++i) {
		double p = calc_prob(i, sig, x, y, target, assign, error);
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

	for (int i = 0; i < modes.size(); ++i) {
		const_cast<EM*>(this)->update_clauses(i);
	}

	if (first_arg >= args.size()) {
		os << "modes: " << nmodes << endl;
		os << endl << "subqueries: mode ptable timing noise train foil" << endl;
		return true;
	} else if (args[first_arg] == "ptable") {
		table_printer t;
		for (int i = 0; i < ndata; ++i) {
			t.add_row() << i;
			t.add(data[i]->mode_prob);
		}
		t.print(os);
		return true;
	} else if (args[first_arg] == "train") {
		int start = 0, end = ndata - 1;
		if (first_arg + 1 < args.size()) {
			if (!parse_int(args[first_arg + 1], start) || start < 0 || start >= ndata - 1) {
				os << "invalid data range" << endl;
				return false;
			}
		}
		if (first_arg + 2 < args.size()) {
			if (!parse_int(args[first_arg + 2], end) || end < start || end >= ndata - 1) {
				os << "invalid data range" << endl;
				return false;
			}
		}
		table_printer t;
		t.add_row() << "N" << "CLS" << "|" << "DATA";
		for (int i = start; i <= end; ++i) {
			if (i == start || (i > start && data[i]->sig_index != data[i-1]->sig_index)) {
				const scene_sig &s = sigs[data[i]->sig_index];
				t.add_row().skip(2) << "|";
				for (int j = 0; j < s.size(); ++j) {
					t << s[j].name;
					t.skip(s[j].props.size() - 1);
				}
				t.add_row().skip(2) << "|";
				for (int j = 0; j < s.size(); ++j) {
					const vector<string> &props = s[j].props;
					for (int k = 0; k < props.size(); ++k) {
						t << props[k];
					}
				}
			}
			t.add_row();
			t << i << data[i]->map_mode << "|";
			for (int j = 0; j < data[i]->x.size(); ++j) {
				t << data[i]->x(j);
			}
			for (int j = 0; j < data[i]->y.size(); ++j) {
				t << data[i]->y(j);
			}
		}
		t.print(os);
		return true;
	} else if (args[first_arg] == "mode") {
		if (first_arg + 1 >= args.size()) {
			os << "Specify a mode number (0 - " << nmodes - 1 << ")" << endl;
			return false;
		}
		int n;
		if (!parse_int(args[first_arg+1], n) || n < 0 || n >= nmodes) {
			os << "invalid mode number" << endl;
			return false;
		}
		return modes[n]->cli_inspect(first_arg + 2, args, os);
	} else if (args[first_arg] == "timing") {
		timers.report(os);
		return true;
	} else if (args[first_arg] == "noise") {
		map<int, set<int> >::const_iterator i;
		for (i = noise.begin(); i != noise.end(); ++i) {
			join(os, i->second, " ") << endl;
		}
		return true;
	} else if (args[first_arg] == "foil") {
		int mode;
		if (first_arg + 1 >= args.size() || !parse_int(args[first_arg + 1], mode)) {
			os << "specify a mode" << endl;
			return false;
		}
		print_foil6_data(os, mode);
		return true;
	} else if (args[first_arg] == "relations") {
		return cli_inspect_relations(first_arg + 1, args, os);
	}

	return false;
}

void EM::get_map_modes(std::vector<int> &modes) const {
	modes.reserve(ndata);
	for (int i = 0; i < ndata; ++i) {
		modes.push_back(data[i]->map_mode);
	}
}

/*
 pos_obj and neg_obj can probably be cached and updated as data points
 are assigned to modes.
*/
void EM::learn_obj_clause(int m, int i) {
	relation pos_obj(3), neg_obj(3);
	tuple objs(2);
	int type = modes[m]->sig[i].type;
	
	set<int>::const_iterator j;
	for (j = modes[m]->members.begin(); j != modes[m]->members.end(); ++j) {
		const scene_sig &sig = sigs[data[*j]->sig_index];
		int o = data[*j]->obj_map[i];
		objs[0] = data[*j]->target;
		objs[1] = o;
		pos_obj.add(*j, objs);
		for (int k = 0; k < sig.size(); ++k) {
			if (sig[k].type == type && k != objs[0] && k != o) {
				objs[1] = k;
				neg_obj.add(*j, objs);
			}
		}
	}
	
	relation uncovered;
	FOIL foil(pos_obj, neg_obj, rel_tbl);
	if (!foil.learn(modes[m]->obj_clauses[i], uncovered)) {
		// respond to this situation appropriately
	}
}

void EM::update_clauses(int m) {
	mode_info &minfo = *modes[m];
	if (!minfo.clauses_dirty) {
		return;
	}
	
	for (int i = 0; i < minfo.sig.size(); ++i) {
		if (i != minfo.target) {
			learn_obj_clause(m, i);
		}
	}
		
	if (minfo.pos.empty()) {
		minfo.mode_clauses.clear();
	} else {
		relation uncovered;
		FOIL foil(minfo.pos, minfo.neg, rel_tbl);
		if (!foil.learn(minfo.mode_clauses, uncovered)) {
			// add numeric classifier
		}
	}
	minfo.clauses_dirty = false;
}

void EM::print_foil6_data(ostream &os, int mode) const {
	set<int> all_times, objs;
	
	relation_table::const_iterator i;
	for (i = rel_tbl.begin(); i != rel_tbl.end(); ++i) {
		set<int> s;
		i->second.at_pos(0, all_times);
		for (int j = 1; j < i->second.arity(); ++j) {
			i->second.at_pos(j, objs);
		}
	}
	
	os << "O: ";
	join(os, objs, ",") << "." << endl;
	os << "T: ";
	join(os, all_times, ",") << "." << endl << endl;
	
	for (i = rel_tbl.begin(); i != rel_tbl.end(); ++i) {
		os << "*" << i->first << "(T";
		for (int j = 1; j < i->second.arity(); ++j) {
			os << ",O";
		}
		os << ") #";
		for (int j = 1; j < i->second.arity(); ++j) {
			os << "-";
		}
		os << endl << i->second << "." << endl;
	}
	
	os << "positive(T) #" << endl;
	for (int j = 0; j < data.size(); ++j) {
		if (data[j]->map_mode == mode) {
			os << j << endl;
		}
	}
	os << "." << endl << endl;
}

bool EM::mode_info::cli_inspect(int first, const vector<string> &args, ostream &os) {
	if (first >= args.size()) {
		// some kind of default action
	} else if (args[first] == "clauses") {
		os << "classifier" << endl;
		clause_vec::const_iterator i;
		for (i = mode_clauses.begin(); i != mode_clauses.end(); ++i) {
			os << *i << endl;
		}
		os << endl << "object clauses" << endl;
		table_printer t;
		for (int j = 0; j < obj_clauses.size(); ++j) {
			t.add_row() << j;
			if (target == j) {
				t << "target";
			} else if (obj_clauses[j].empty()) {
				t << "empty";
			} else {
				for (int k = 0; k < obj_clauses[j].size(); ++k) {
					if (k > 0) {
						t.add_row().skip(1);
					}
					t << obj_clauses[j][k];
				}
			}
		}
		t.print(os);
		return true;
	} else if (args[first] == "signature") {
		for (int i = 0; i < sig.size(); ++i) {
			os << sig[i].type;
			if (i == target) {
				os << "t";
			}
			os << " ";
		}
		os << endl;
		return true;
	} else if (args[first] == "members") {
		join(os, members, ' ') << endl;
		return true;
	} else if (args[first] == "model") {
		return model->cli_inspect(first + 1, args, os);
	}
	return false;
}

/*
 Add tuples from a single time point into the relation table
*/
void EM::extend_relations(const relation_table &add, int time) {
	set<tuple> t;
	relation_table::const_iterator i;
	for (i = add.begin(); i != add.end(); ++i) {
		const string &name = i->first;
		const relation &r = i->second;
		relation_table::iterator j = rel_tbl.find(name);
		if (j == rel_tbl.end()) {
			rel_tbl[name] = r;
		} else {
			relation &r2 = j->second;
			t.clear();
			/*
			 The assumption here is that all the tuples
			 have the same value in the first position,
			 since they're all from the same time.
			*/
			r.drop_first(t);
			set<tuple>::const_iterator k;
			for (k = t.begin(); k != t.end(); ++k) {
				r2.add(time, *k);
			}
		}
	}
}

void EM::serialize(ostream &os) const {
	serializer(os) << ndata << nmodes << data << sigs << modes << noise << rel_tbl;
}

void EM::unserialize(istream &is) {
	unserializer(is) >> ndata >> nmodes >> data >> sigs >> modes >> noise >> rel_tbl;
	assert(data.size() == ndata && modes.size() == nmodes);
}

void EM::em_data::serialize(ostream &os) const {
	serializer(os) << target << sig_index << map_mode << model_row
	               << x << y << mode_prob << obj_map;
}

void EM::em_data::unserialize(istream &is) {
	unserializer(is) >> target >> sig_index >> map_mode >> model_row
	                 >> x >> y >> mode_prob >> obj_map;
}

void EM::mode_info::serialize(ostream &os) const {
	serializer(os) << stale << target << stale_points << members << sig
	               << mode_clauses << obj_clauses << clauses_dirty << pos << neg;

	assert(model);
	model->serialize(os);
}

void EM::mode_info::unserialize(istream &is) {
	unserializer(is) >> stale >> target >> stale_points >> members >> sig
	                 >> mode_clauses >> obj_clauses >> clauses_dirty >> pos >> neg;
	
	if (model) {
		delete model;
	}
	model = new LinearModel;
	model->unserialize(is);
}

bool EM::cli_inspect_relations(int i, const vector<string> &args, ostream &os) const {
	if (i >= args.size()) {
		os << rel_tbl << endl;
		return true;
	}
	const relation *r = map_getp(rel_tbl, args[i]);
	if (!r) {
		os << "no such relation" << endl;
		return false;
	}
	if (i + 1 >= args.size()) {
		os << *r << endl;
		return true;
	}

	// process pattern
	vector<int> pattern;
	for (int j = i + 1; j < args.size(); ++j) {
		if (args[j] == "*") {
			pattern.push_back(-1);
		} else {
			int obj;
			if (!parse_int(args[j], obj)) {
				os << "invalid pattern" << endl;
				return false;
			}
			pattern.push_back(obj);
		}
	}

	if (pattern.size() != r->arity()) {
		os << "pattern arity doesn't match relation arity" << endl;
		return false;
	}
	relation matches;
	r->match(pattern, matches);
	os << matches << endl;
	return true;
}

