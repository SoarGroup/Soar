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

using namespace std;
using namespace Eigen;

const int INIT_NMODELS = 1;
const bool TEST_ELIGIBILITY = false;

typedef PCRModel LinearModel;

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
	for (int iter1 = 0; iter1 < maxiters; ++iter1) {
		w.setConstant(0.0);
		
		/*
		vector<int> nonstatic, init_members;
		find_nonstatic_cols(X, X.cols(), nonstatic);
		int rank = nonstatic.size() + 1;
		sample(rank, 0, ndata, false, init_members);
		for (int i = 0; i < init_members.size(); ++i) {
			w(init_members[i]) = 1.0;
		}
		*/
		for (int i = 0; i < w.size(); i += 2) {
			w(i) = 1.0;
		}
		
		mat C;
		rvec intercepts;
		for (int iter2 = 0; iter2 < maxiters; ++iter2) {
			LOG(EMDBG) << "MINI_EM " << iter1 << " " << iter2 << endl;
			LOG(EMDBG) << "w = ";
			for (int i = 0; i < w.size(); ++i) {
				LOG(EMDBG) << w(i) << " ";
			}
			LOG(EMDBG) << endl;
			
			if (!solve2(X, Y, w, C, intercepts)) {
				break;
			}
			
			LOG(EMDBG) << "c = ";
			for (int i = 0; i < C.rows(); ++i) {
				LOG(EMDBG) << C(i, 0) << " ";
			}
			LOG(EMDBG) << endl;
			
			old_res = residuals;
			residuals = (Y - ((X * C).rowwise() + intercepts)).rowwise().squaredNorm();
			points.clear();
			for (int i = 0; i < residuals.size(); ++i) {
				if (residuals(i) < fit_thresh) {
					points.push_back(i);
				}
			}
			if (points.size() > n) {
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

EM::EM(const dyn_mat &xdata, const dyn_mat &ydata)
: xdata(xdata), ydata(ydata), ndata(0), nmodels(0),
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
		rvec x = xdata.row(i);
		for (int j = 0; j < nmodels; ++j) {
			const rvec &c2 = models[j]->get_center();
			rvec d = x - c2;
			for (int k = 0; k < nmodels; ++k) {
				const rvec &c1 = models[k]->get_center();
				if (j != k && d.dot(c1 - c2) < 0) {
					eligible(k, i) = 0;
					break;
				}
			}
		}
	}
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
	rvec py;
	
	for (j = stale_points[i].begin(); j != stale_points[i].end(); ++j) {
		double prev = Py_z(i, *j), now;
		int m = map_mode[*j];
		if (TEST_ELIGIBILITY && eligible(i, *j) == 0) {
			now = 0.;
		} else {
			double w;
			if (TEST_ELIGIBILITY) {
				w = 1.0 / eligible.col(*j).head(nmodels - 1).sum();
			} else {
				w = 1.0 / nmodels;
			}
			if (!models[i]->predict(xdata.row(*j), py)) {
				assert(false);
			}
			double d = gausspdf(ydata(*j, 0), py(0), MODEL_STD);
			now = (1.0 - EPSILON) * w * d;
		}
		if ((m == i && now < prev) ||
		    (m != i && ((m == -1 && now > PNOISE) ||
		                (m != -1 && now > Py_z(m, *j)))))
		{
			check.insert(*j);
		}
		Py_z(i, *j) = now;
	}
	stale_points[i].clear();
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
				noise_inds.erase(*j);
			} else {
				stale_models.insert(prev);
				models[prev]->del_example(*j);
			}
			if (now == -1) {
				noise_inds.insert(*j);
			} else {
				stale_models.insert(now);
				models[now]->add_example(*j, true);
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
	noise_inds.insert(ndata - 1);
	for (int i = 0; i < nmodels; ++i) {
		stale_points[i].insert(ndata - 1);
	}
}

void EM::estep() {
	function_timer t(timers.get(E_STEP_T));
	
	update_eligibility();
	/*
	if (stale_models.empty()) {
		return;
	}
	set<int>::iterator i = stale_models.begin();
	advance(i, rand() % stale_models.size());
	update_Py_z(*i);
	stale_models.erase(i);
	*/

	set<int> check;
	
	int nstale = 0;
	for (int i = 0; i < nmodels; ++i) {
		nstale += stale_points[i].size();
		update_Py_z(i, check);
	}
	
	update_MAP(check);
}

bool EM::mstep() {
	function_timer t(timers.get(M_STEP_T));
	
	bool changed = false;
	set<int>::iterator i;
	for (i = stale_models.begin(); i != stale_models.end(); ++i) {
		LRModel *m = models[*i];
		if (m->needs_refit() && m->fit()) {
			changed = true;
			stale_points[*i].insert(m->get_members().begin(), m->get_members().end());
		}
	}
	stale_models.clear();
	return changed;
}

bool EM::find_new_mode_inds(vector<int> &mode_inds) const {
	// matrices containing only the unique rows of xdata and ydata
	dyn_mat unique_x(0, xdata.cols()), unique_y(0, 1);
	
	// Y value -> vector of indexes into total data
	map<double, vector<int> > const_noise;
	
	// index into unique data -> vector of indexes into total data
	vector<vector<int> > unique_map;

	set<int>::const_iterator ni;
	for (ni = noise_inds.begin(); ni != noise_inds.end(); ++ni) {
		int i = *ni;
		vector<int> &const_inds = const_noise[ydata(i, 0)];
		const_inds.push_back(i);
		if (const_inds.size() >= K) {
			LOG(EMDBG) << "found constant model in noise" << endl;
			mode_inds = const_inds;
			return true;
		}
		
		bool new_unique = true;
		for (int j = 0; j < unique_x.rows(); ++j) {
			if (xdata.row(i) == unique_x.row(j) &&
			    ydata.row(i) == unique_y.row(j))
			{
				unique_map[j].push_back(i);
				new_unique = false;
				break;
			}
		}
		if (new_unique) {
			unique_x.append_row(xdata.row(i));
			unique_y.append_row(ydata.row(i));
			unique_map.resize(unique_map.size() + 1);
			unique_map.back().push_back(i);
		}
	}
	
	if (unique_x.rows() < K) {
		return false;
	}
	
	vector<int> seed;
	if (mini_em(unique_x.get(), unique_y.get(), K, MODEL_ERROR_THRESH, 10, seed)) {
		for (int i = 0; i < seed.size(); ++i) {
			const vector<int> &real_inds = unique_map[seed[i]];
			copy(real_inds.begin(), real_inds.end(), back_inserter(mode_inds));
		}
		return true;
	}

	for (int n = 0; n < SEL_NOISE_MAX_TRIES; ++n) {
		auto_ptr<LinearModel> m(new LinearModel(unique_x, unique_y));
		int start = rand() % (unique_x.rows() - MODEL_INIT_N);
		for (int i = 0; i < MODEL_INIT_N; ++i) {
			m->add_example(start + i, false);
		}
		m->fit();
		
		if (m->get_train_error() > MODEL_ERROR_THRESH) {
			continue;
		}
		
		for (int i = 0; i < unique_x.rows(); ++i) {
			if (i < start || i >= start + MODEL_INIT_N) {
				rvec py;
				if (m->predict(unique_x.row(i), py) &&
					pow(py[0] - unique_y(i, 0), 2) < MODEL_ADD_THRESH)
				{
					m->add_example(i, true);
					if (m->needs_refit()) {
						m->fit();
					}
				}
			}
		}
		
		if (m->size() >= K) {
			const vector<int> &unique_mems = m->get_members();
			for (int i = 0; i < unique_mems.size(); ++i) {
				const vector<int> &real_inds = unique_map[unique_mems[i]];
				copy(real_inds.begin(), real_inds.end(), back_inserter(mode_inds));
			}
			return true;
		}
	}
	

	return false;
}

bool EM::unify_or_add_model() {
	if (noise_inds.size() < K || noise_inds == old_noise_inds) {
		return false;
	}

	function_timer t(timers.get(NEW_T));
	old_noise_inds = noise_inds;
	
	vector<int> inds;
	if (!find_new_mode_inds(inds)) {
		return false;
	}
	
	LRModel *m = new LinearModel(xdata, ydata);
	m->add_examples(inds);
	m->fit();
	
	/*
	 Try to add noise data to each current model and refit. If the
	 resulting model is just as accurate as the original, then just
	 add the noise to that model instead of creating a new one.
	*/
	for (int i = 0; i < nmodels; ++i) {
		LRModel *unified(models[i]->copy());
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
	
	models.push_back(m);
	mark_model_stale(nmodels);
	++nmodels;
	Py_z.append_row();
	if (TEST_ELIGIBILITY) {
		eligible.append_row();
	}
	return true;
}

void EM::mark_model_stale(int i) {
	stale_models.insert(i);
	set<int> &pts = stale_points[i];
	for (int j = 0; j < ndata; ++j) {
		pts.insert(j);
	}
}

bool EM::predict(int mode, const rvec &x, double &y) {
	//timer t("EM PREDICT TIME");
	assert(0 <= mode && mode < nmodels);
	if (ndata == 0) {
		return false;
	}
	
	rvec py;
	if (!models[mode]->predict(x, py)) {
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
		if (models[j]->size() > 2) {
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
	models.erase(models.begin() + nmodels, models.end());
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
	
	std::vector<LRModel*>::const_iterator j;
	os << models.size() << endl;
	for (j = models.begin(); j != models.end(); ++j) {
		(**j).save(os);
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
		LRModel *m = new LinearModel(xdata, ydata);
		m->load(is);
		models.push_back(m);
	}
}

int EM::best_mode(const rvec &x, double y, double &besterror) const {
	int best = -1;
	rvec py;
	for (int i = 0; i < nmodels; ++i) {
		if (!models[i]->predict(x, py)) {
			continue;
		}
		double error = fabs(py(0) - y);
		if (best == -1 || error < besterror) {
			best = i;
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
		return models[n]->cli_inspect(first_arg + 2, args, os);
	} else if (args[first_arg] == "timing") {
		timers.report(os);
		return true;
	} else if (args[first_arg] == "noise") {
		set<int>::const_iterator i;
		for (i = noise_inds.begin(); i != noise_inds.end(); ++i) {
			os << *i << " ";
		}
		os << endl;
		return true;
	}
	
	return false;
}

