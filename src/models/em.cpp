#include <iostream>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <limits>
#include <armadillo>
#include "linear.h"
#include "em.h"
#include "common.h"
#include "dtree.h"
#include "scene.h"
#include "filter_table.h"

#define DBG cerr

using namespace std;
using namespace arma;

const double INF = numeric_limits<double>::infinity();
const double SQRT2PI = 2.5066282746310002;
const double PNOISE = 0.0001;
const double STD = 0.001;
const double UNIFY_ABS_THRESH = 1e-5;
const double UNIFY_MUL_THRESH = 1.00001;
const int SEL_NOISE_MAX_TRIES = 10;

const int INIT_NDATA = 1000;
const int INIT_NMODELS = 10;
const bool TEST_ELIGIBILITY = false;

int dbgcount = 0;

double gausspdf(double x, double mean) {
	return (1. / STD * SQRT2PI) * exp(-((x - mean) * (x - mean) / (2 * STD * STD)));
}

void argmax_cols(const mat &m, ivec &max, int nrows, int ncols) {
	max.subvec(0, ncols - 1).fill(-1);
	for (int j = 0; j < ncols; ++j) {
		for (int i = 0; i < nrows; ++i) {
			if (max(j) < 0 || m(max(j), j) < m(i, j)) {
				max(j) = i;
			}
		}
	}
}

/* Return the row with the max value in column "col" of matrix "m" */
int argmax_col(const mat &m, int nrows, int col) {
	int mi = 0;
	for (int i = 1; i < nrows; ++i) {
		if (m(mi, col) < m(i, col)) {
			mi = i;
		}
	}
	return mi;
}

double distsq(const rowvec &a, const rowvec &b) {
	return accu(pow(a - b, 2));
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


EM::EM(double epsilon, scene *scn)
: K(5), epsilon(epsilon), Pnoise(PNOISE), xdim(0), scn(scn), scncopy(scn->copy()), dtree(NULL), ndata(0), nmodels(0),
  Py_z(INIT_NMODELS, INIT_NDATA), eligible(INIT_NMODELS, INIT_NDATA), ydata(INIT_NDATA)
{
}

EM::~EM() {
	delete dtree;
	delete scncopy;
}

/* double storage size if we run out */
void EM::resize() {
	int nd = ndata > xdata.n_rows ? ndata * 2 : xdata.n_rows;
	int nm = nmodels > Py_z.n_rows ? nmodels * 2 : Py_z.n_rows;
	
	if (nd > xdata.n_rows) {
		xdata.resize(nd, xdim);
		ydata.resize(nd);
	}
	if (nm > Py_z.n_rows || nd > Py_z.n_cols) {
		Py_z.resize(nm, nd);
		if (TEST_ELIGIBILITY) {
			eligible.resize(nm, nd);
		}
	}
}

void EM::update_eligibility() {
	if (!TEST_ELIGIBILITY) {
		return;
	}
	
	eligible.fill(1);
	for (int i = 0; i < ndata; ++i) {
		rowvec x = xdata.row(i);
		for (int j = 0; j < nmodels; ++j) {
			const rowvec &c2 = models[j]->get_center();
			rowvec d = x - c2;
			for (int k = 0; k < nmodels; ++k) {
				const rowvec &c1 = models[k]->get_center();
				if (j != k && dot(d, c1 - c2) < 0) {
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
	
	DATAVIS() << "BEGIN Py_z" << endl;
	for (j = stale_points[i].begin(); j != stale_points[i].end(); ++j) {
		double prev = Py_z(i, *j), now;
		category c = dtree_insts[*j].cat;
		if (TEST_ELIGIBILITY && eligible(i, *j) == 0) {
			now = 0.;
		} else {
			double w;
			if (TEST_ELIGIBILITY) {
				w = 1.0 / accu(eligible.submat(0, *j, nmodels - 1, *j));
			} else {
				w = 1.0 / nmodels;
			}
			double p = models[i]->predict(xdata.row(*j));
			assert(!isnan(p));
			double d = gausspdf(ydata(*j), p);
			now = (1.0 - epsilon) * w * d;
		}
		if ((c == i && now < prev) ||
		    (c != i && ((c == -1 && now > Pnoise) ||
		                (c != -1 && now > Py_z(c, *j)))))
		{
			check.insert(*j);
		}
		Py_z(i, *j) = now;
		DATAVIS() << "'" << i << ", " << *j << "' " << Py_z(i, *j) << endl;
	}
	DATAVIS() << "END" << endl;
	stale_points[i].clear();
}

/* Recalculate MAP model for each index in points */
void EM::update_MAP(const set<int> &points) {
	set<int>::iterator j;
	for (j = points.begin(); j != points.end(); ++j) {
		category prev = dtree_insts[*j].cat, now;
		if (nmodels == 0) {
			now = -1;
		} else {
			now = argmax_col(Py_z, nmodels, *j);
			if (Py_z(now, *j) < Pnoise) {
				now = -1;
			}
		}
		if (now != prev) {
			dtree_insts[*j].cat = now;
			dtree->update_category(*j, prev);
			if (prev != -1) {
				stale_models.insert(prev);
				models[prev]->del_example(*j);
			}
			if (now != -1) {
				stale_models.insert(now);
				DATAVIS() << "BEGIN 'model " << now << "'" << endl;
				models[now]->add_example(*j);
				DATAVIS() << "END" << endl;
			}
		}
	}
	
	DATAVIS() << "BEGIN MAP" << endl;
	for (int j = 0; j < ndata; ++j) {
		DATAVIS() << j << " " << dtree_insts[j].cat << endl;
	}
	DATAVIS() << "END" << endl;
	/*
	 Do the update after all categories have been changed to save
	 on thrashing.
	*/
	dtree->update_tree(-1);
}

void EM::add_data(const floatvec &x, double y) {
	if (xdim == 0) {
		xdim = x.size();
		xdata = zeros<mat>(INIT_NDATA, xdim);
	} else {
		assert(xdata.n_cols == x.size());
	}
	++ndata;
	DATAVIS() << "ndata " << ndata << endl;
	resize();
	
	for (int i = 0; i < xdim; ++i) {
		xdata(ndata - 1, i) = x[i];
	}
	ydata(ndata - 1) = y;
	
	dtree_insts.push_back(instance());
	instance &inst = dtree_insts.back();
	inst.cat = -1;
	inst.attrs = scn->get_atom_vals();
	if (!dtree) {
		dtree = new ID5Tree(dtree_insts);
	}
	dtree->update_tree(dtree_insts.size() - 1);
	
	for (int i = 0; i < nmodels; ++i) {
		stale_points[i].insert(ndata - 1);
	}
}

void EM::estep() {
	update_eligibility();
	/*
	if (stale_models.empty()) {
		return;
	}
	set<int>::iterator i = stale_models.begin();
	advance(i, rand() % stale_models.size());
	DBG << "estep: updating model " << *i << endl;
	update_Py_z(*i);
	stale_models.erase(i);
	*/

	set<int> check;
	
	timer t;
	t.start();
	int nstale = 0;
	for (int i = 0; i < nmodels; ++i) {
		nstale += stale_points[i].size();
		update_Py_z(i, check);
	}
	DATAVIS() << "'num stale' " << nstale << endl;
	DATAVIS() << "'Py_z update' %+" << t.stop() << endl;
	
	t.start();
	update_MAP(check);
	DATAVIS() << "'MAP update' %+" << t.stop() << endl;
}

bool EM::mstep() {
	bool changed = false;
	set<int>::iterator i;
	for (i = stale_models.begin(); i != stale_models.end(); ++i) {
		RPLSModel *m = models[*i];
		DATAVIS() << "BEGIN 'model " << *i << "'" << endl;
		if (m->fit()) {
			changed = true;
			stale_points[*i].insert(m->get_members().begin(), m->get_members().end());
		}
		DATAVIS() << "END" << endl;
	}
	stale_models.clear();
	return changed;
}

bool EM::unify_or_add_model() {
	vector<int> noise_data;
	for (int i = 0; i < ndata; ++i) {
		if (dtree_insts[i].cat == -1) {
			noise_data.push_back(i);
		}
	}

	DATAVIS() << "noise '";
	copy(noise_data.begin(), noise_data.end(), ostream_iterator<int>(DATAVIS(), " "));
	DATAVIS() << "'" << endl;
	
	int nnoise = noise_data.size();
	if (nnoise < K) {
		return false;
	}
	
	for (int n = 0; n < SEL_NOISE_MAX_TRIES; ++n) {
		const rowvec &seed = xdata.row(noise_data[rand() % nnoise]);
		vec dists(nnoise);
		for (int i = 0; i < nnoise; ++i) {
			dists(i) = distsq(xdata.row(noise_data[i]), seed);
		}
		uvec close = sort_index(dists);
		
		/*
		 Try to add noise data to each current model and refit. If the
		 resulting model is just as accurate as the original, then just
		 add the noise to that model instead of creating a new one.
		*/
		for (int i = 0; i < nmodels; ++i) {
			DATAVIS() << "BEGIN 'extended model " << i << "'" << endl;
			RPLSModel *nmodel = new RPLSModel(*models[i]);
			for (int j = 0; j < K; ++j) {
				nmodel->add_example(noise_data[close(j)]);
			}
			nmodel->fit();
			DATAVIS() << "END" << endl;
			
			double curr_error = models[i]->get_error();
			double uni_error = nmodel->get_error();
			
			if (uni_error < UNIFY_ABS_THRESH ||
			    curr_error > 0.0 && uni_error < UNIFY_MUL_THRESH * curr_error)
			{
				delete models[i];
				models[i] = nmodel;
				mark_model_stale(i);
				cerr << "UNIFIED " << i << endl;
				return true;
			}
		}
		
		RPLSModel *m = new RPLSModel(xdata, ydata);
		
		DATAVIS() << "BEGIN 'potential model'" << endl;
		for (int i = 0; i < K; ++i) {
			int j = noise_data[close(i)];
			m->add_example(j);
		}
		m->fit();
		bool good_model = true;
		for (int i = 0; i < K; ++i) {
			int j = noise_data[close(i)];
			if (gausspdf(ydata(j), m->predict(xdata.row(j))) < PNOISE) {
				good_model = false;
				break;
			}
		}
		DATAVIS() << "END" << endl;
		
		if (good_model) {
			DBG << "Adding new linear model" << endl;

			models.push_back(m);
			mark_model_stale(nmodels);
			++nmodels;
			resize();
			return true;
		} else {
			delete m;
		}
	}

	return false;
}

void EM::mark_model_stale(int i) {
	stale_models.insert(i);
	set<int> &pts = stale_points[i];
	for (int j = 0; j < ndata; ++j) {
		pts.insert(j);
	}
}

bool EM::predict(const floatvec &x, float &y) {
	rowvec v(x.size());
	for (int i = 0; i < x.size(); ++i) {
		v(i) = x[i];
	}
	scncopy->set_properties(x);
	attr_vec attrs = scncopy->get_atom_vals();

	int mdl = dtree->classify(attrs);
	if (mdl == -1) {
		return false;
	}
	DATAVIS() << "BEGIN 'model " << mdl << "'" << endl;
	y = models[mdl]->predict(v);
	DATAVIS() << "END" << endl;
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
	ivec index_map(nmodels);
	int i = 0;
	for (int j = 0; j < nmodels; ++j) {
		if (models[j]->size() > 2) {
			index_map(j) = i;
			if (j > i) {
				models[i] = models[j];
				Py_z.row(i) = Py_z.row(j);
				if (TEST_ELIGIBILITY) {
					eligible.row(i) = eligible.row(j);
				}
			}
			i++;
		} else {
			index_map(j) = -1;
			delete models[j];
			removed = true;
		}
	}
	for (int j = 0; j < ndata; ++j) {
		if (dtree_insts[j].cat >= 0) {
			category old = dtree_insts[j].cat;
			dtree_insts[j].cat = index_map(old);
			if (dtree_insts[j].cat != old) {
				DATAVIS() << "'num removed' %+1" << endl;
				dtree->update_category(j, old);
			}
		}
	}
	
	nmodels = i;
	resize();
	models.erase(models.begin() + nmodels, models.end());
	return removed;
}

bool EM::step() {
	timer t;
	t.start();
	estep();
	DATAVIS() << "'E-step time' %+" << t.stop() << endl;
	
	t.start();
	bool changed = mstep();
	DATAVIS() << "'M-step time' %+" << t.stop() << endl;
	
	return changed;
}

void EM::run(int maxiters) {
	for (int i = 0; i < maxiters; ++i) {
		if (!step()) {
			bool changed = false;
			changed |= remove_models();
			changed |= unify_or_add_model();
			if (!changed) {
				return;
			}
		}
	}
	cerr << "Reached max iterations without quiescence" << endl;
	cerr << "Noise Data X" << endl;
	for (int i = 0; i < ndata; ++i) {
		if (dtree_insts[i].cat == -1) {
			cerr << xdata.row(i);
		}
	}
	cerr << "Noise Data Y" << endl;
	for (int i = 0; i < ndata; ++i) {
		if (dtree_insts[i].cat == -1) {
			cerr << ydata(i) << endl;
		}
	}
}

double EM::error() {
	if (nmodels == 0) {
		return 0.;
	}
	double error = 0.;
	for (int i = 0; i < ndata; ++i) {
		floatvec x(xdim);
		float y;
		for (int j = 0; j < xdim; ++j) {
			x[j] = xdata(i, j);
		}
		if (!predict(x, y)) {
			return -1.0;
		}
		error += ::pow(ydata(i) - y, 2);
	}
	return error;
}
