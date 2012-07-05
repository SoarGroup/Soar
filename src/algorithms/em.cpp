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
#include "classify.h"
#include "scene.h"
#include "filter_table.h"
#include "params.h"

using namespace std;
using namespace Eigen;

const int INIT_NDATA = 1;
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

EM::EM(scene *scn)
: xdim(0), clsfr(xdata, ydata, scn), ndata(0), nmodels(0),
  Py_z(INIT_NMODELS, INIT_NDATA), eligible(INIT_NMODELS, INIT_NDATA), ydata(INIT_NDATA, 1)
{
	timers.add("e_step");
	timers.add("m_step");
}

EM::~EM() {
}

/* double storage size if we run out */
void EM::resize() {
	int nd = ndata > xdata.rows() ? ndata * 2 : xdata.rows();
	int nm = nmodels > Py_z.rows() ? nmodels * 2 : Py_z.rows();
	
	if (nd > xdata.rows()) {
		xdata.conservativeResize(nd, xdim);
		ydata.conservativeResize(nd, 1);
	}
	if (nm > Py_z.rows() || nd > Py_z.cols()) {
		Py_z.conservativeResize(nm, nd);
		if (TEST_ELIGIBILITY) {
			eligible.conservativeResize(nm, nd);
		}
	}
}

void EM::update_eligibility() {
	if (!TEST_ELIGIBILITY) {
		return;
	}
	
	eligible.fill(1);
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
	
	DATAVIS("BEGIN Py_z" << endl)
	for (j = stale_points[i].begin(); j != stale_points[i].end(); ++j) {
		double prev = Py_z(i, *j), now;
		category c = map_class[*j];
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
		if ((c == i && now < prev) ||
		    (c != i && ((c == -1 && now > PNOISE) ||
		                (c != -1 && now > Py_z(c, *j)))))
		{
			check.insert(*j);
		}
		Py_z(i, *j) = now;
		DATAVIS("'" << i << ", " << *j << "' " << Py_z(i, *j) << endl)
	}
	DATAVIS("END" << endl)
	stale_points[i].clear();
}

/* Recalculate MAP model for each index in points */
void EM::update_MAP(const set<int> &points) {
	set<int>::iterator j;
	for (j = points.begin(); j != points.end(); ++j) {
		category prev = map_class[*j], now;
		if (nmodels == 0) {
			now = -1;
		} else {
			Py_z.topLeftCorner(nmodels, ndata).col(*j).maxCoeff(&now);
			if (Py_z(now, *j) < PNOISE) {
				now = -1;
			}
		}
		if (now != prev) {
			map_class[*j] = now;
			clsfr.change_cat(*j, now);
			if (prev != -1) {
				stale_models.insert(prev);
				models[prev]->del_example(*j);
			}
			if (now != -1) {
				stale_models.insert(now);
				DATAVIS("BEGIN 'model " << now << "'" << endl)
				models[now]->add_example(*j, true);
				DATAVIS("END" << endl)
			}
		}
	}
	
	/*
	 Do the update after all categories have been changed to save
	 on thrashing.
	*/
	clsfr.update();
}

void EM::add_data(const rvec &x, double y) {
	if (xdim == 0) {
		xdim = x.size();
		xdata = mat::Zero(INIT_NDATA, xdim);
	} else {
		assert(xdata.cols() == x.size());
	}
	++ndata;
	DATAVIS("ndata " << ndata << endl)
	resize();
	
	xdata.row(ndata - 1) = x;
	ydata(ndata - 1, 0) = y;
	map_class.push_back(-1);
	clsfr.add(-1);
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
		DATAVIS("BEGIN 'model " << *i << "'" << endl)
		if (m->needs_refit() && m->fit()) {
			changed = true;
			stale_points[*i].insert(m->get_members().begin(), m->get_members().end());
		}
		DATAVIS("END" << endl)
	}
	stale_models.clear();
	return changed;
}

bool EM::unify_or_add_model() {
	if (ndata < K) {
		return false;
	}
	
	vector<int> noise_data;
	for (int i = 0; i < ndata; ++i) {
		if (map_class[i] == -1) {
			noise_data.push_back(i);
		}
	}
	
	if (noise_data.size() < K) {
		return false;
	}
	
	for (int n = 0; n < SEL_NOISE_MAX_TRIES; ++n) {
		auto_ptr<LRModel> m(new LinearModel(xdata, ydata));
		int start = rand() % (noise_data.size() - MODEL_INIT_N);
		for (int i = 0; i < MODEL_INIT_N; ++i) {
			m->add_example(noise_data[start + i], false);
		}
		m->fit();
		
		if (m->get_train_error() > MODEL_ERROR_THRESH) {
			continue;
		}
		
		for (int i = 0; i < noise_data.size(); ++i) {
			if (i < start || i >= start + MODEL_INIT_N) {
				rvec py;
				if (m->predict(xdata.row(noise_data[i]), py) &&
				    pow(py[0] - ydata(noise_data[i], 0), 2) < MODEL_ADD_THRESH)
				{
					m->add_example(noise_data[i], true);
					if (m->needs_refit()) {
						m->fit();
					}
				}
			}
		}
		
		if (m->size() < K) {
			continue;
		}
		
		/*
		 Try to add noise data to each current model and refit. If the
		 resulting model is just as accurate as the original, then just
		 add the noise to that model instead of creating a new one.
		*/
		for (int i = 0; i < nmodels; ++i) {
			DATAVIS("BEGIN 'extended model " << i << "'" << endl)
			auto_ptr<LRModel> unified(models[i]->copy());
			unified->add_examples(m->get_members());
			unified->fit();
			DATAVIS("END" << endl)
			
			double curr_error = models[i]->get_train_error();
			double uni_error = unified->get_train_error();
			
			if (uni_error < MODEL_ERROR_THRESH ||
			    (curr_error > 0.0 && uni_error < UNIFY_MUL_THRESH * curr_error))
			{
				delete models[i];
				models[i] = unified.release();
				mark_model_stale(i);
				LOG(EMDBG) << "UNIFIED " << i << endl;
				return true;
			}
		}
		
		models.push_back(m.release());
		mark_model_stale(nmodels);
		++nmodels;
		resize();
		return true;
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

bool EM::predict(const rvec &x, double &y) {
	//timer t("EM PREDICT TIME");
	if (ndata == 0) {
		return false;
	}
	
	int mdl = clsfr.classify(x);
	if (mdl == -1) {
		return false;
	}
	DATAVIS("BEGIN 'model " << mdl << "'" << endl)
	rvec py;
	if (!models[mdl]->predict(x, py)) {
		assert(false);
	}
	y = py(0);
	DATAVIS("END" << endl)
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
		if (map_class[j] >= 0) {
			category old = map_class[j];
			map_class[j] = index_map[old];
			if (map_class[j] != old) {
				clsfr.change_cat(j, map_class[j]);
				DATAVIS("'num removed' %+1" << endl)
			}
		}
	}
	
	nmodels = i;
	resize();
	models.erase(models.begin() + nmodels, models.end());
	clsfr.update();
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

double EM::error() {
	if (nmodels == 0) {
		return 0.;
	}
	double error = 0.;
	for (int i = 0; i < ndata; ++i) {
		rvec x(xdim);
		double y;
		for (int j = 0; j < xdim; ++j) {
			x[j] = xdata(i, j);
		}
		if (!predict(x, y)) {
			return -1.0;
		}
		error += std::pow(ydata(i, 0) - y, 2);
	}
	return error;
}

void EM::save(ostream &os) const {
	os << ndata << " " << nmodels << " " << xdim << endl;
	save_mat(os, xdata);
	save_mat(os, ydata);
	save_mat(os, Py_z);
	if (TEST_ELIGIBILITY) {
		save_imat(os, eligible);
	}
	save_vector(map_class, os);
	
	std::vector<LRModel*>::const_iterator j;
	os << models.size() << endl;
	for (j = models.begin(); j != models.end(); ++j) {
		(**j).save(os);
	}
}

void EM::load(istream &is) {
	int ninsts;
	
	is >> ndata >> nmodels >> xdim;
	
	load_mat(is, xdata);
	load_mat(is, ydata);
	load_mat(is, Py_z);
	if (TEST_ELIGIBILITY) {
		load_imat(is, eligible);
	}
	load_vector(map_class, is);
	
	is >> nmodels;
	for (int i = 0; i < nmodels; ++i) {
		DATAVIS("BEGIN 'model " << i << "'" << endl)
		LRModel *m = new LinearModel(xdata, ydata);
		m->load(is);
		models.push_back(m);
		DATAVIS("END" << endl)
	}
	
	clsfr.batch_update(map_class);
}


void EM::test_classify(const rvec &x, double y, int &best, int &predicted, double &besterror) {
	best = -1;
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
	predicted = clsfr.classify(x);
}

bool EM::cli_inspect(int first_arg, const vector<string> &args, ostream &os) const {
	stringstream ss;

	if (first_arg >= args.size()) {
		os << "EM model learner" << endl;
		os << "nmodels: " << nmodels << endl;
		os << "ndata:   " << ndata << endl;
		os << endl << "subelements: function ptable train classifier timing noise" << endl;
		return true;
	} else if (args[first_arg] == "ptable") {
		for (int i = 0; i < ndata; ++i) {
			for (int j = 0; j < nmodels; ++j) {
				os << Py_z(j, i) << "\t";
			}
			os << endl;
		}
		return true;
	} else if (args[first_arg] == "function") {
		if (first_arg + 1 >= args.size()) {
			os << "Specify a function number (0 - " << nmodels - 1 << ")" << endl;
			return false;
		}
		int n;
		if (!parse_int(args[first_arg+1], n) || n < 0 || n >= nmodels) {
			os << "invalid model number" << endl;
			return false;
		}
		return models[n]->cli_inspect(first_arg + 2, args, os);
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
		
		os << "   N  CLS | DATA" << endl;  // header
		for (int i = start; i <= end; ++i) {
			os << setw(4) << i << "  " << setw(3) << map_class[i] << " | ";
			output_rvec(os, xdata.row(i)) << " ";
			output_rvec(os, ydata.row(i)) << endl;
		}
		return true;
	} else if (args[first_arg] == "classifier") {
		return clsfr.cli_inspect(first_arg + 1, args, os);
	} else if (args[first_arg] == "timing") {
		timers.report(os);
		return true;
	} else if (args[first_arg] == "noise") {
		for (int i = 0; i < map_class.size(); ++i) {
			if (map_class[i] == -1) {
				os << i << " ";
			}
		}
		os << endl;
		return true;
	}
	
	os << "no such property" << endl;
	return false;
}

