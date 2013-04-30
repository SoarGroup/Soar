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
#include "params.h"
#include "mat.h"
#include "serialize.h"
#include "lda.h"
#include "drawer.h"

//#define DBGCOUNT(n) { static int count = 0; fprintf(stderr, "%s %d\n", n, count++); }
#define DBGCOUNT(n)

using namespace std;
using namespace Eigen;

bool approx_equal(double a, double b) {
	return fabs(a - b) / min(fabs(a), fabs(b)) < .001;
}


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
	const double maxw = 1.0e9;
	w.resize(d.size());
	for (int i = 0; i < d.size(); ++i) {
		if (d(i) == 0.0) {
			w(i) = maxw;
		} else {
			w(i) = min(maxw, pow(d(i), p));
		}
	}
}

void predict(const mat &C, const rvec &intercepts, const rvec &x, rvec &y) {
	y = (x * C) + intercepts;
}

/*
 Upon return, X and Y will contain the training data, Xtest and Ytest the
 testing data.
*/
void split_data(
	const_mat_view X,
	const_mat_view Y,
	const vector<int> &use,
	int ntest,
	mat &Xtrain, mat &Xtest, 
	mat &Ytrain, mat &Ytest)
{
	int ntrain = use.size() - ntest;
	vector<int> test;
	sample(ntest, 0, use.size(), test);
	sort(test.begin(), test.end());
	
	int train_end = 0, test_end = 0, i = 0;
	for (int j = 0; j < use.size(); ++j) {
		if (i < test.size() && j == test[i]) {
			Xtest.row(test_end) = X.row(use[j]);
			Ytest.row(test_end) = Y.row(use[j]);
			++test_end;
			++i;
		} else {
			Xtrain.row(train_end) = X.row(use[j]);
			Ytrain.row(train_end) = Y.row(use[j]);
			++train_end;
		}
	}
	assert(test_end == ntest && train_end == ntrain);
}

template <typename T>
struct indirect_cmp {
	indirect_cmp(const vector<T> &v) : v(v) {}
	
	bool operator()(int a, int b) const {
		return v[a] < v[b];
	}
	
	const vector<T> &v;
};

template <typename T>
void get_ordering(const vector<T> &v, vector<int> &order) {
	order.resize(v.size());
	for (int i = 0, iend = order.size(); i < iend; ++i) {
		order[i] = i;
	}
	sort(order.begin(), order.end(), indirect_cmp<T>(v));
}

/*
 Assume that data from a single mode comes in blocks. Try to discover
 a mode by randomly fitting a line to a block of data and then finding
 all data that fit the line.
*/
void EM::find_linear_subset_block(const_mat_view X, const_mat_view Y, vector<int> &subset) const {
	function_timer t(timers.get_or_add("block_subset"));
	
	int xcols = X.cols();
	int rank = xcols + 1;
	int ndata = X.rows();
	mat Xb(rank, xcols), Yb(xcols+1, 1), coefs;
	
	int start = rand() % (ndata - rank);
	for (int i = 0; i < rank; ++i) {
		Xb.row(i) = X.row(start + i);
		Yb.row(i) = Y.row(start + i);
	}
	linreg_clean(FORWARD, Xb, Yb, noise_var, coefs);

	cvec errors = (Y - (X * coefs)).col(0).array().abs();
	subset.clear();
	for (int i = 0; i < ndata; ++i) {
		if (errors(i) < MODEL_ERROR_THRESH) {
			subset.push_back(i);
		}
	}
}


/*
 Use a simple version of EM to discover a mode in noise data. This
 method works better than block_seed when data from a single mode
 doesn't come in contiguous blocks.
 
 The algorithm is:
 
 1. If input X has m non-static columns, assume it has rank = m + 1.
 2. Randomly choose 'rank' training points as the seed members for the linear
    function. Fit the function to the seed members.
 3. Compute the residuals of the function for training data. Compute a weight
    vector based on the residuals and a kernel.
 4. Refit the linear function biased based on the weight vector. Repeat until
    convergence or the function fits at least n data points.
*/
void EM::find_linear_subset_em(const_mat_view X, const_mat_view Y, vector<int> &subset) const {
	function_timer t(timers.get_or_add("em_block"));
	
	int ndata = X.rows(), xcols = X.cols();
	vector<int> init, nonuniform_cols;
	cvec w(ndata), error(ndata), old_error(ndata);
	mat Xc(ndata, xcols), Yc(ndata, 1), coefs(xcols, 1), coefs2;
	
	error.setConstant(INFINITY);
	sample(min(xcols + 1, ndata), 0, ndata, init);
	w.setConstant(0.0);
	for (int i = 0; i < init.size(); ++i) {
		w(init[i]) = 1.0;
	}

	for (int iter = 0; iter < MINI_EM_MAX_ITERS; ++iter) {
		DBGCOUNT("EMLSB")
		int i = 0;
		nonuniform_cols.clear();
		for (int j = 0; j < xcols; ++j) {
			Xc.col(i) = X.col(j).array() * w.array();
			if (!uniform(Xc.col(i))) {
				nonuniform_cols.push_back(j);
				++i;
			}
		}
		Yc.col(0) = Y.col(0).array() * w.array();
		if (i == 0 || uniform(Yc.col(0)))
			return;
			
		if (!linreg_clean(LASSO, Xc.leftCols(i), Yc, noise_var, coefs2)) {
			assert(false);
		}
		coefs.setConstant(0);
		for (int j = 0; j < nonuniform_cols.size(); ++j) {
			coefs.row(nonuniform_cols[j]) = coefs2.row(j);
		}
		
		old_error = error;
		error = (Y - (X * coefs)).col(0).array().abs();
		if (error.maxCoeff() <= MODEL_ERROR_THRESH)
			break;
		
		double diff = (error - old_error).array().abs().maxCoeff();
		if (iter > 0 && diff < SAME_THRESH) {
			break;
		}
		kernel2(error, w, -3.0);
	}
	for (int i = 0; i < ndata; ++i) {
		if (error(i) < MODEL_ERROR_THRESH) {
			subset.push_back(i);
		}
	}
}

void erase_inds(vector<int> &v, const vector<int> &inds) {
	int i = 0, j = 0;
	for (int k = 0; k < v.size(); ++k) {
		if (i < inds.size() && k == inds[i]) {
			++i;
		} else {
			if (j < k) {
				v[j] = v[k];
			}
			++j;
		}
	}
	assert(i == inds.size() && j == v.size() - inds.size());
	v.resize(j);
}

int ransac_iters(int ninliers, int mss_size, int ndata, double alarm_rate, int max_iters) {
	double q = pow(ninliers / static_cast<double>(ndata), mss_size);
	double lq = log(1 - q);
	if (lq == 0) {
		return max_iters;
	}
	int i1 = static_cast<int>(floor(log(alarm_rate) / lq) + 1);
	return min(i1, max_iters);
}

void ransac(const mat &X, const mat &Y, double noise_var, double alarm_rate, int max_iters, int size_thresh, const vector<int> &blocks,
            vector<int> &subset, mat &coefs, rvec &intercept)
{
	vector<int> mss, fit_set;
	mat Xmss, Ymss, Yp, C;
	cvec dummy, error;
	rvec inter;
	
	double max_error = noise_var * 5;
	int ndata = X.rows();
	int mss_size = 5;
	int iters = ransac_iters(mss_size, mss_size, ndata, alarm_rate, max_iters);
	
	double mss_per_block = mss_size / static_cast<double>(blocks.size());
	mss.reserve(mss_size);
	fit_set.reserve(ndata);
	subset.clear();
	
	for (int i = 0; i < iters; ++i) {
		mss.clear();
		int low = 0;
		for (int j = 0, jend = blocks.size(); j < jend; ++j) {
			// this is convoluted, but allows for even distribution when mss_size
			// doesn't divide into the number of blocks evenly
			int s = (mss_per_block * (j + 1)) - mss.size();
			sample(s, low, low + blocks[j], mss);
			low += blocks[j];
		}
		assert(mss.size() == mss_size);
		
		pick_rows(X, mss, Xmss);
		pick_rows(Y, mss, Ymss);
		static int dbgcount = 0;
		cout << "RANSAC " << dbgcount++ << endl;
		if (!linreg_d(FORWARD, Xmss, Ymss, dummy, noise_var, C, inter)) {
			assert(false);
		}
		Yp = (X * C).rowwise() + inter;
		error = (Y - Yp).cwiseAbs().rowwise().sum();
		
		fit_set.clear();
		for (int j = 0; j < ndata; ++j) {
			if (error(j) <= max_error) {
				fit_set.push_back(j);
			}
		}
		if (fit_set.size() > subset.size()) {
			subset = fit_set;
			coefs = C;
			intercept = inter;
			if (subset.size() >= size_thresh) {
				return;
			}
			iters = ransac_iters(subset.size(), mss_size, ndata, alarm_rate, max_iters);
		}
	}
}

void ransac1(const mat &X, const mat &Y, vector<int> &subset, mat &coefs, rvec &intercept)
{
	vector<int> blocks;
	blocks.push_back(X.rows());
	ransac(X, Y, 1e-8, 0.05, 2000, NEW_MODE_THRESH, blocks, subset, coefs, intercept);
}

int EM::find_linear_subset(mat &X, mat &Y, vector<int> &subset, mat &coefs, rvec &inter) const {
	DBGCOUNT("FLSUB")
	
	function_timer t(timers.get_or_add("find_seed"));

	const double TEST_RATIO = 0.5;
	
	int orig_xcols = X.cols();
	int largest = 0, nleft = X.rows();
	
	// preprocess the data as much as possible
	vector<int> used_cols;
	clean_lr_data(X, used_cols);
	augment_ones(X);

	mat Xsub(X.rows(), X.cols()), Ysub(Y.rows(), 1);
	rvec avg_error;

	vector<int> ungrouped(nleft);
	for (int i = 0; i < nleft; ++i) {
		ungrouped[i] = i;
	}
	
	/*
	 Outer loop ranges over sets of random initial points
	*/
	for (int iter = 0; iter < LINEAR_SUBSET_MAX_ITERS; ++iter) {
		DBGCOUNT("LSUBI")
		vector<int> subset2;
		find_linear_subset_em(X.topRows(nleft), Y.topRows(nleft), subset2);
		if (subset2.size() < 10)  // arbitrary, fix later
			continue;
		
		pick_rows(X, subset2, Xsub);
		pick_rows(Y, subset2, Ysub);
		nfoldcv(Xsub.topRows(subset2.size()), Ysub.topRows(subset2.size()), noise_var, 5, FORWARD, avg_error);
		if (avg_error(0) > MODEL_ERROR_THRESH) {
			/*
			 There isn't a clear linear relationship between the points, so I can't
			 consider them a single block.
			*/
			continue;
		}
		
		if (subset2.size() > largest) {
			subset.clear();
			for (int i = 0; i < subset2.size(); ++i) {
				subset.push_back(ungrouped[subset2[i]]);
			}
			largest = subset.size();
			if (largest >= NEW_MODE_THRESH) {
				mat subcoefs;
				linreg(FORWARD, Xsub.topRows(subset2.size()), Ysub.topRows(subset2.size()), cvec(), noise_var, subcoefs, inter);
				coefs.resize(orig_xcols, Y.cols());
				coefs.setConstant(0.0);
				for (int i = 0; i < used_cols.size(); ++i) {
					coefs.row(used_cols[i]) = subcoefs.row(i);
				}
				return largest;
			}
		}
		
		/*
		 Assume this group of points won't fit linearly in any other group, so they
		 can be excluded from consideration in the next iteration.
		*/
		pick_rows(X, subset2);
		erase_inds(ungrouped, subset2);
		nleft = ungrouped.size();
		if (nleft < NEW_MODE_THRESH) {
			break;
		}
	}
	return largest;
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

EM::EM(const model_train_data &data)
: data(data), use_em(true), use_unify(true), learn_new_modes(true),
  check_after(NEW_MODE_THRESH), clsfr(data)
{
	add_mode(false); // noise mode
	noise_var = 1e-8;
}

EM::EM(const model_train_data &data, bool use_em, bool use_unify, bool learn_new_modes)
: data(data), use_em(use_em), use_unify(use_unify), learn_new_modes(learn_new_modes),
  check_after(NEW_MODE_THRESH), clsfr(data)
{
	add_mode(false); // noise mode
}

EM::~EM() {
	clear_and_dealloc(insts);
	clear_and_dealloc(modes);
	clear_and_dealloc(sigs);
}


void EM::add_data(int t) {
	function_timer tm(timers.get_or_add("learn"));
	
	const model_train_inst &d = data.get_inst(t);
	inst_info *inst = new inst_info;
	sig_info *s = NULL;
	if (has(sigs, d.sig)) {
		s = sigs[d.sig];
	} else {
		s = new sig_info;
		sigs[d.sig] = s;
	}
	s->members.push_back(t);
	s->noise.insert(t);
	
	inst->mode = 0;
	inst->minfo.resize(modes.size());
	inst->minfo[0].prob = PNOISE;
	inst->minfo[0].prob_stale = false;
	insts.push_back(inst);
	
	modes[0]->add_example(t, vector<int>());
	clsfr.update_inst(t, 0);
}

void EM::estep() {
	DBGCOUNT("ESTEP")
	
	/*
	 For data i and mode j, if:
	 
	  * P(i, j) increases and j was not the MAP mode, or
	  * P(i, j) decreases and j was the MAP mode
	 
	 then we mark i as a point we have to recalculate the MAP mode for.
	*/
	for (int i = 0, iend = insts.size(); i < iend; ++i) {
		const model_train_inst &d = data.get_inst(i);
		inst_info &inst = *insts[i];
		bool stale = false;
		for (int j = 1, jend = modes.size(); j < jend; ++j) {
			inst_info::mode_info &m = inst.minfo[j];
			if (!m.prob_stale && !modes[j]->is_new_fit()) {
				continue;
			}
			double prev = inst.minfo[inst.mode].prob, now, error;
			now = modes[j]->calc_prob(d.target, *d.sig, d.x, d.y(0), m.sig_map, error);
			assert(m.sig_map.size() == modes[j]->get_sig().size());
			if ((inst.mode == j && now < prev) || (inst.mode != j && now > m.prob)) {
				stale = true;
			}
			m.prob = now;
			m.prob_stale = false;
		}
		if (stale) {
			int prev = inst.mode, best = 0;
			for (int j = 1, jend = modes.size(); j < jend; ++j) {
				/*
				 These conditions look awkward, but have justification. If I tested the >
				 condition before the approx_equal condition, the test would succeed even
				 if the probability of j was only slightly better than the probability of
				 best.
				*/
				if (approx_equal(inst.minfo[j].prob, inst.minfo[best].prob)) {
					if (modes[j]->get_num_nonzero_coefs() < modes[best]->get_num_nonzero_coefs()) {
						best = j;
					}
				} else if (inst.minfo[j].prob > inst.minfo[best].prob) {
					best = j;
				}
			}
			if (best != prev) {
				inst.mode = best;
				modes[prev]->del_example(i);
				if (prev == 0) {
					sigs[d.sig]->noise.erase(i);
				}
				assert(modes[best]->get_sig().size() == inst.minfo[best].sig_map.size());
				modes[best]->add_example(i, inst.minfo[best].sig_map);
				if (best == 0) {
					sigs[d.sig]->noise.insert(i);
				}
				clsfr.update_inst(i, best);
			}
		}
	}
	
	for (int i = 1; i < modes.size(); ++i) {
		modes[i]->reset_new_fit();
	}
}

bool EM::mstep() {
	DBGCOUNT("MSTEP")
	function_timer t(timers.get_or_add("m-step"));
	
	bool changed = false;
	for (int i = 1, iend = modes.size(); i < iend; ++i) {
		changed = changed || modes[i]->update_fits(noise_var);
	}
	return changed;
}

void EM::fill_xy(const interval_set &rows, mat &X, mat &Y) const {
	if (rows.empty()) {
		X.resize(0, 0);
		Y.resize(0, 0);
		return;
	}

	X.resize(rows.size(), data.get_inst(rows.ith(0)).x.size());
	Y.resize(rows.size(), 1);

	interval_set::const_iterator i, iend;
	int j;
	for (i = rows.begin(), iend = rows.end(), j = 0; i != iend; ++i, ++j) {
		X.row(j) = data.get_inst(*i).x;
		Y.row(j) = data.get_inst(*i).y;
	}
}

em_mode *EM::add_mode(bool manual) {
	em_mode *new_mode = new em_mode(modes.size() == 0, manual, data);
	modes.push_back(new_mode);
	for (int i = 0, iend = insts.size(); i < iend; ++i) {
		grow_vec(insts[i]->minfo);
	}
	clsfr.add_class();
	return new_mode;
}

bool EM::unify_or_add_mode() {
	DBGCOUNT("UNIFY")
	function_timer t(timers.get_or_add("new"));

	assert(check_after >= NEW_MODE_THRESH);
	if (!learn_new_modes || modes[0]->size() < check_after) {
		return false;
	}
	
	vector<int> largest;
	mat coefs;
	rvec inter;
	modes[0]->largest_const_subset(largest);
	int potential = largest.size();
	inter = data.get_inst(largest[0]).y; // constant model
	if (largest.size() < NEW_MODE_THRESH) {
		mat X, Y;
		sig_table::const_iterator i, iend;
		for (i = sigs.begin(), iend = sigs.end(); i != iend; ++i) {
			const interval_set &ns = i->second->noise;
			if (ns.size() < check_after) {
				if (ns.size() > potential) {
					potential = ns.size();
				}
				continue;
			}
			interval_set inds(ns);
			vector<int> subset;
			fill_xy(inds, X, Y);
			ransac1(X, Y, subset, coefs, inter);
			if (subset.size() > potential) {
				potential = subset.size();
			}
			if (subset.size() > largest.size()) {
				largest.clear();
				for (int i = 0; i < subset.size(); ++i) {
					largest.push_back(inds.ith(subset[i]));
				}
				if (largest.size() >= NEW_MODE_THRESH) {
					break;
				}
			}
		}
	}
	
	if (largest.size() < NEW_MODE_THRESH) {
		check_after += (NEW_MODE_THRESH - potential);
		return false;
	}
	
	/*
	 From here I know the noise data is going to either become a new mode or unify
	 with an existing mode, so reset check_after assuming the current noise is
	 gone.
	*/
	check_after = NEW_MODE_THRESH;
	
	int seed_sig = data.get_inst(largest[0]).sig_index;
	int seed_target = data.get_inst(largest[0]).target;

	if (use_unify) {
		/*
		 Try to add noise data to each current model and refit. If the resulting
		 model is just as accurate as the original, then just add the noise to that
		 model instead of creating a new one.
		*/
		mat X, Y, ucoefs;
		rvec uinter;
		for (int j = 1, jend = modes.size(); j < jend; ++j) {
			em_mode &m = *modes[j];
	
			if (m.is_manual() || !m.uniform_sig(seed_sig, seed_target)) {
				continue;
			}
	
			interval_set combined;
			vector<int> subset;
			m.get_members(combined);
			for (int k = 0, kend = largest.size(); k < kend; ++k) {
				combined.insert(largest[k]);
			}
			fill_xy(combined, X, Y);
			LOG(EMDBG) << "Trying to unify with mode " << j << endl;
			ransac1(X, Y, subset, ucoefs, uinter);
			int unified_size = subset.size();
			
			if (unified_size >= m.size() + .9 * largest.size()) {
				LOG(EMDBG) << "Successfully unified with mode " << j << endl;
				const model_train_inst &d0 = data.get_inst(combined.ith(subset[0]));
				m.set_params(*d0.sig, d0.target, ucoefs, uinter);
				return true;
			}
			LOG(EMDBG) << "Failed to unify with mode " << j << endl;
		}
	}
	
	em_mode *new_mode = add_mode(false);
	const model_train_inst &d0 = data.get_inst(largest[0]);
	new_mode->set_params(*d0.sig, d0.target, coefs, inter);
	return true;
}


bool EM::predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, int &mode, double &y) {
	if (insts.empty()) {
		mode = 0;
		return false;
	}
	
	vector<int> obj_map;
	if (use_em) {
		mode = classify(target, sig, rels, x, obj_map);
		if (mode > 0) {
			modes[mode]->predict(sig, x, obj_map, y);
			return true;
		}
	}
	
	y = NAN;
	return false;
}

/*
 Remove all modes that cover fewer than NEW_MODE_THRESH data points.
*/
bool EM::remove_modes() {
	if (modes.size() == 1) {
		return false;
	}
	
	/*
	 i is the first free model index. If model j should be kept, all
	 information pertaining to model j will be copied to row/element i
	 in the respective matrix/vector, and i will be incremented. Most
	 efficient way I can think of to remove elements from the middle
	 of vectors. index_map associates old j's to new i's.
	*/
	vector<int> index_map(modes.size()), removed;
	int i = 1;  // start with 1, noise mode (0) should never be removed
	for (int j = 1, jend = modes.size(); j < jend; ++j) {
		if (modes[j]->size() >= NEW_MODE_THRESH || modes[j]->is_manual()) {
			index_map[j] = i;
			if (j > i) {
				modes[i] = modes[j];
			}
			i++;
		} else {
			index_map[j] = 0;
			delete modes[j];
			removed.push_back(j);
		}
	}
	if (removed.empty()) {
		return false;
	}
	assert(i == modes.size() - removed.size());
	modes.resize(i);
	for (int j = 0, jend = insts.size(); j < jend; ++j) {
		if (insts[j]->mode >= 0) {
			insts[j]->mode = index_map[insts[j]->mode];
			if (insts[j]->mode == 0) {
				clsfr.update_inst(j, 0);
			}
		}
		remove_from_vector(removed, insts[j]->minfo);
	}
	clsfr.del_classes(removed);
	return true;
}

bool EM::run(int maxiters) {
	if (use_em) {
		for (int i = 0; i < maxiters; ++i) {
			estep();
			bool changed = mstep();
			if (!changed && !remove_modes() && !unify_or_add_mode()) {
				// reached quiescence
				return true;
			}
		}
		LOG(EMDBG) << "Reached max iterations without quiescence" << endl;
	}
	return false;
}

int EM::best_mode(int target, const scene_sig &sig, const rvec &x, double y, double &besterror) const {
	int best = -1;
	double best_prob = 0.0;
	rvec py;
	vector<int> assign;
	double error;
	for (int i = 0, iend = modes.size(); i < iend; ++i) {
		double p = modes[i]->calc_prob(target, sig, x, y, assign, error);
		if (best == -1 || p > best_prob) {
			best = i;
			best_prob = p;
			besterror = error;
		}
	}
	return best;
}

void EM::proxy_get_children(map<string, cliproxy*> &c) {
	proxy_group *mode_group = new proxy_group;
	
	for (int i = 0, iend = modes.size(); i < iend; ++i) {
		mode_group->add(tostring(i), modes[i]);
	}
	
	c["mode"] =        mode_group;
	c["classifier"] =  &clsfr;
	c["timers"] =      &timers;
	c["use_em"] =      new bool_proxy(&use_em);
	c["unify_modes"] = new bool_proxy(&use_unify);
	c["learn_modes"] = new bool_proxy(&learn_new_modes);
	c["noise_var"] =   new float_proxy(&noise_var);
	
	c["ptable"] =      new memfunc_proxy<EM>(this, &EM::cli_ptable);
	c["add_mode"] =    new memfunc_proxy<EM>(this, &EM::cli_add_mode);
}

void EM::cli_ptable(ostream &os) const {
	table_printer t;
	for (int i = 0, iend = insts.size(); i < iend; ++i) {
		t.add_row() << i << insts[i]->mode;
		for (int j = 0, jend = modes.size(); j < jend; ++j) {
			t << insts[i]->minfo[j].prob;
		}
	}
	t.print(os);
}

/*
 The format will be [coef] [dim] [coef] [dim] ... [intercept]
*/
void EM::cli_add_mode(const vector<string> &args, ostream &os) {
	if (insts.empty()) {
		os << "need at least one training example to get the signature from" << endl;
		return;
	}
	
	const model_train_inst &inst = data.get_last_inst();
	mat coefs(inst.sig->dim(), 1);
	rvec intercept(1);
	coefs.setConstant(0.0);
	intercept.setConstant(0.0);
	
	for (int i = 0, iend = args.size(); i < iend; i += 2) {
		double c;
		if (!parse_double(args[i], c)) {
			os << "expecting a number, got " << args[i] << endl;
			return;
		}
		
		if (i + 1 >= args.size()) {
			intercept(0) = c;
			break;
		}
		
		vector<string> parts;
		split(args[i+1], ":", parts);
		if (parts.size() != 2) {
			os << "expecting object:property, got " << args[i+1] << endl;
			return;
		}
		
		int obj_ind, prop_ind;
		if (!inst.sig->get_dim(parts[0], parts[1], obj_ind, prop_ind)) {
			os << args[i+1] << " not found" << endl;
			return;
		}
		assert(prop_ind >= 0 && prop_ind < coefs.rows());
		coefs(prop_ind, 0) = c;
	}
	
	em_mode *new_mode = add_mode(true);
	new_mode->set_params(*inst.sig, inst.target, coefs, intercept);
}


void EM::serialize(ostream &os) const {
	serializer sr(os);
	sr << insts << clsfr << nc_type << modes.size() << '\n';
	vector<const scene_sig*> s = data.get_sigs();
	for (int i = 0, iend = s.size(); i < iend; ++i) {
		sr << *map_get(sigs, s[i]) << '\n';
	}
	for (int i = 0, iend = modes.size(); i < iend; ++i) {
		sr << *modes[i] << '\n';
	}
}

void EM::unserialize(istream &is) {
	unserializer unsr(is);
	int nmodes;
	
	clear_and_dealloc(insts);
	unsr >> insts >> clsfr >> nc_type >> nmodes;
	assert(insts.size() == data.size());
	
	clear_and_dealloc(sigs);
	vector<const scene_sig*> s = data.get_sigs();
	for (int i = 0, iend = s.size(); i < iend; ++i) {
		sig_info *si = new sig_info;
		unsr >> *si;
		sigs[s[i]] = si;
	}
	
	clear_and_dealloc(modes);
	for (int i = 0, iend = nmodes; i < iend; ++i) {
		em_mode *m = new em_mode(i == 0, false, data);
		m->unserialize(is);
		modes.push_back(m);
	}
}

void EM::print_ptable() const {
	table_printer t;
	for (int i = 0, iend = insts.size(); i < iend; ++i) {
		inst_info *inst = insts[i];
		t.add_row() << i << inst->mode;
		for (int j = 0, jend = inst->minfo.size(); j < jend; ++j) {
			t << inst->minfo[j].prob;
		}
	}
	t.print(cout);
}

void EM::print_modes() const {
	for (int i = 1, iend = modes.size(); i < iend; ++i) {
		scene_sig sig;
		rvec coefs;
		double intercept;
		table_printer t;
		
		modes[i]->get_params(sig, coefs, intercept);
		for (int j = 0, jend = sig.size(), xi = 0; j < jend; ++j) {
			t.add_row() << sig[j].name;
			for (int k = 0, kend = sig[j].props.size(); k < kend; ++k, ++xi) {
				if (coefs(xi) != 0.0) {
					t.add_row() << sig[j].props[k] << coefs(xi);
				}
			}
		}
		t.add_row() << "intercept " << intercept;
		
		cout << "MODE " << i << endl;
		t.print(cout);
	}
}

void inst_info::serialize(ostream &os) const {
	serializer(os) << mode << minfo;
}

void inst_info::unserialize(istream &is) {
	unserializer(is) >> mode >> minfo;
}

void inst_info::mode_info::serialize(ostream &os) const {
	serializer(os) << prob << prob_stale << sig_map;
}

void inst_info::mode_info::unserialize(istream &is) {
	unserializer(is) >> prob >> prob_stale >> sig_map;
}

int EM::classify(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, vector<int> &obj_map) {
	LOG(EMDBG) << "classification" << endl;

	vector<int> votes, order;
	clsfr.classify(target, sig, rels, x, votes);
	
	LOG(EMDBG) << "votes:" << endl;
	for (int i = 0, iend = votes.size(); i < iend; ++i) {
		LOG(EMDBG) << i << " = " << votes[i] << endl;
	}
	
	get_ordering(votes, order);
		
	/*
	 The scene has to contain the objects used by the linear model of
	 a mode for it to possibly qualify for that mode.
	*/
	for (int i = order.size() - 1; i >= 0; --i) {
		if (order[i] == 0) {
			// don't need mapping for noise mode
			return 0;
		}
		em_mode &m = *modes[order[i]];
		if (m.get_sig().size() > sig.size()) {
			continue;
		}
		obj_map.clear();
		if (!m.map_objs(target, sig, rels, obj_map)) {
			LOG(EMDBG) << "mapping failed for " << i << endl;
			continue;
		}
		
		// mapping worked, classify as this mode;
		LOG(EMDBG) << "best mode = " << order[i] << endl;
		return order[i];
	}
	
	// should never reach here
	assert(false);
	return -1;
}

sig_info::sig_info() {}

void sig_info::serialize(ostream &os) const {
	serializer(os) << members << noise;
}

void sig_info::unserialize(istream &is) {
	unserializer(is) >> members >> noise;
}

