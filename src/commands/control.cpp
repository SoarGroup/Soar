#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cmath>
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <list>
#include "command.h"
#include "svs.h"
#include "scene.h"
#include "model.h"
#include "common.h"
#include "bullet_support.h"

using namespace std;

// constants for simplex search
const double RCOEF = 1.0;
const double ECOEF = 2.0;
const double CCOEF = 0.5;
const double SCOEF = 0.5;
const int MAXITERS = 50;
const double INF = numeric_limits<double>::infinity();

bool predict_traj(multi_model *mdl, const floatvec &initstate, const std::list<floatvec> &traj, floatvec &finalstate) {
	finalstate = initstate;
	if (traj.size() == 0) {
		return true;
	}
	
	std::list<floatvec>::const_iterator i;
	floatvec x(initstate.size() + traj.front().size());
	
	for (i = traj.begin(); i != traj.end(); ++i) {
		x.graft(0, finalstate);
		x.graft(finalstate.size(), *i);
		if (!mdl->predict(x, finalstate)) {
			return false;
		}
	}
	return true;
}

class objective {
public:
	objective() : negated(false) {}
	
	float evaluate(scene &scn) const {
		if (negated) {
			return -1 * eval(scn);
		}
		return eval(scn);
	}
	
	void set_negated(bool n) {
		negated = n;
	}

	virtual float eval(scene &scn) const = 0;
	
private:
	bool negated;
};

/* Squared Euclidean distance between centroids of two objects */
class euclidean_obj : public objective {
public:
	euclidean_obj(const string &obj1, const string &obj2)
	: obj1(obj1), obj2(obj2) {}
	
	float eval(scene &scn) const {
		sgnode *n1, *n2;
		ptlist p1, p2;
		::vec3 c1, c2;
		
		if (!(n1 = scn.get_node(obj1)) ||
		    !(n2 = scn.get_node(obj2)))
		{
			return INF;
		}
		
		n1->get_world_points(p1);
		n2->get_world_points(p2);
		c1 = calc_centroid(p1);
		c2 = calc_centroid(p2);

		return c1.dist(c2);
	}
	
private:
	string obj1, obj2;
};

/*
 Returns a positive value as long as c is not behind a w.r.t. b, otherwise
 returns 0. Minimized when c is behind a.
*/
class behind_obj : public objective {
public:
	behind_obj(const string &a, const string &b, const string &c)
	: a(a), b(b), c(c) {}
	
	float eval(scene &scn) const {
		sgnode *na, *nb, *nc;
		ptlist pa, pb, pc;
		
		if (!(na = scn.get_node(a)) ||
		    !(nb = scn.get_node(b)) ||
		    !(nc = scn.get_node(c)))
		{
			return INF;
		}
		
		na->get_world_points(pa);
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		vec3 ca = calc_centroid(pa);
		vec3 cb = calc_centroid(pb);
		vec3 u = (cb - ca).unit();
		
		float d = dir_separation(pa, pc, u);
		if (d < 0.) {
			return 0.;
		}
		return d;
	}
	
private:
	string a, b, c;
};

/*
 Returns the shortest distance between the centroid of c and the line
 going through the centroids of a and b. Minimized when the centroids
 of all three objects are collinear.
*/
class collinear_obj : public objective {
public:
	collinear_obj(const string &a, const string &b, const string &c)
	: a(a), b(b), c(c) {}
	
	float eval(scene &scn) const {
		sgnode *na, *nb, *nc;
		ptlist pa, pb, pc;
		
		if (!(na = scn.get_node(a)) ||
		    !(nb = scn.get_node(b)) ||
		    !(nc = scn.get_node(c)))
		{
			return INF;
		}
		
		na->get_world_points(pa);
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		copy(pa.begin(), pa.end(), back_inserter(pc));
		float d = hull_distance(pb, pc);
		if (d < 0) {
			d = 0.;
		}
		/*
		vec3 ca = calc_centroid(pa);
		vec3 cb = calc_centroid(pb);
		vec3 cc = calc_centroid(pc);
		
		float d = cc.line_dist(ca, cb);
		if (d < 0.001) {
			return 0.;
		}
		*/
		return d;
	}
	
private:
	string a, b, c;
};

class align_facing_objective : public objective {
public:
	align_facing_objective(const string &a, const string &b, const string &c)
	: a(a), b(b), c(c) {}
	
	float eval(scene &scn) const {
		sgnode *na, *nb, *nc;
		ptlist pb, pc;
		
		if (!(na = scn.get_node(a)) ||
		    !(nb = scn.get_node(b)) ||
		    !(nc = scn.get_node(c)))
		{
			return INF;
		}
		
		transform3 rot('r', na->get_trans('r'));
		vec3 facing = rot(vec3(1, 0, 0));
		
		nb->get_world_points(pb);
		nc->get_world_points(pc);
		
		vec3 desired = (calc_centroid(pc) - calc_centroid(pb)).unit();
		
		/*
		 Return the negative cosine between the two
		 directions. This will be minimized at -1 if the angle
		 is 0, and maximized at 1 when the angle is 180.
		*/
		float negcos = -(facing.dot(desired));
		//cout << "NEG COS ANGLE " << negcos << endl;
		return negcos;
	}
	
private:
	string a, b, c;
};

class multi_objective {
public:
	multi_objective() {}
	~multi_objective() {
		vector<objective*>::iterator i;
		for (i = objs.begin(); i != objs.end(); ++i) {
			delete *i;
		}
	}
	
	void add(objective *o) {
		objs.push_back(o);
	}
	
	void evaluate(scene &scn, floatvec &val) const {
		val.resize(objs.size());
		for (int i = 0; i < objs.size(); ++i) {
			val[i] = objs[i]->evaluate(scn);
		}
	}
private:
	vector<objective*> objs;
};


/*
 Parse a WME structure and return the appropriate objective function.
 Assumes this format:

 (<o1> ^name <name1>
       ^<param1> <val1>
       ^<param2> <val2>
       ...
       ^next <o2>)
 (<o2> ^name <name2>
       ...
*/
multi_objective *parse_obj_struct(soar_interface *si, Symbol *root) {
	multi_objective *m = new multi_objective();
	wme_list param_wmes;
	
	while (root && si->is_identifier(root) && si->get_child_wmes(root, param_wmes)) {
		wme_list::iterator i;
		string name, attr, val;
		map<string, string> params;
		objective *obj;
		
		for (i = param_wmes.begin(); i != param_wmes.end(); ++i) {
			if (si->get_val(si->get_wme_attr(*i), attr) &&
			    si->get_val(si->get_wme_val(*i), val))
			{
				params[attr] = val;
			}
		}
		if (!map_get(params, string("name"), name)) {
			break;
		}
		
		string negated;
		if (!map_get(params, string("negated"), negated)) {
			negated = "f";
		}
		
		if (name == "euclidean") {
			string a, b;
			if (!map_get(params, string("a"), a) ||
			    !map_get(params, string("b"), b))
			{
				break;
			}
			obj = new euclidean_obj(a, b);
		} else if (name == "behind") {
			string a, b, c;
			if (!map_get(params, string("a"), a) ||
			    !map_get(params, string("b"), b) ||
			    !map_get(params, string("c"), c))
			{
				break;
			}
			obj = new behind_obj(a, b, c);
		} else if (name == "collinear") {
			string a, b, c;
			if (!map_get(params, string("a"), a) ||
			    !map_get(params, string("b"), b) ||
			    !map_get(params, string("c"), c))
			{
				break;
			}
			obj = new collinear_obj(a, b, c);
		} else if (name == "align_facing") {
			string a, b, c;
			if (!map_get(params, string("a"), a) ||
			    !map_get(params, string("b"), b) ||
			    !map_get(params, string("c"), c))
			{
				break;
			}
			obj = new align_facing_objective(a, b, c);
		} else {
			cerr << "skipping unknown objective " << name << endl;
		}
		
		if (negated == "t" || negated == "1") {
			obj->set_negated(true);
		}
		m->add(obj);
		
		wme *next_wme;
		if (!si->find_child_wme(root, "next", next_wme)) {
			break;
		}
		root = si->get_wme_val(next_wme);
	}
	return m;
}

/*
 This class binds the model and objective function together and is
 responsible for simulating a given trajectory and evaluating the
 objective function on the resulting state.
*/
class traj_eval {
public:
	traj_eval(int stepsize, multi_model *m, multi_objective *obj, const scene &init)
	: mdl(m), stepsize(stepsize), obj(obj), numcalls(0), totaltime(0.)
	{
		scn = init.copy();
		scn->get_properties(initvals);
	}
	
	traj_eval(int stepsize, multi_model *m, multi_objective *obj, const scene &tmp, const floatvec &initvals)
	: mdl(m), stepsize(stepsize), obj(obj), numcalls(0), totaltime(0.), initvals(initvals)
	{
		scn = tmp.copy();
	}

	~traj_eval() {
		delete scn;
	}
	
	void set_init(const floatvec &v) {
		initvals = v;
	}
	
	bool evaluate(const floatvec &traj, floatvec &value, floatvec &finalstate) {
		timer tm;
		tm.start();
		
		if (traj.size() > 0) {
			floatvec x(initvals.size() + stepsize), y = initvals;
			for (int i = 0; i < traj.size(); i += stepsize) {
				x.graft(0, y);
				x.graft(y.size(), traj.slice(i, i + stepsize));
				if (!mdl->predict(x, y)) {
					return false;
				}
			}
			finalstate = y;
		} else {
			finalstate = initvals;
		}
		
		scn->set_properties(finalstate);
		obj->evaluate(*scn, value);
		
		totaltime += tm.stop();
		numcalls++;
		return true;
	}
	
	void print_stats() const {
		cout << "ncall: " << numcalls << endl;
		cout << "ttime: " << totaltime << endl;
		cout << "atime: " << totaltime / numcalls << endl;
	}
	
	
private:
	multi_model      *mdl;
	multi_objective  *obj;
	int               stepsize;  // dimensionality of output
	scene            *scn;       // copy of initial scene to be modified after prediction
	floatvec          initvals;  // flattened values of initial scene
	int               numcalls;
	double            totaltime;
};

void constrain(floatvec &v, const floatvec &min, const floatvec &max) {
	for (int i = 0; i < v.size(); ++i) {
		if (v[i] < min[i]) {
			v[i] = min[i];
		} else if (v[i] > max[i]) {
			v[i] = max[i];
		}
	}
}

void argmin(const vector<floatvec> &v, int &worst, int &nextworst, int &best) {
	worst = 0; nextworst = 0; best = 0;
	for (int i = 1; i < v.size(); ++i) {
		if (v[i] > v[worst]) {
			nextworst = worst;
			worst = i;
		} else if (v[i] > v[nextworst]) {
			nextworst = i;
		} else if (v[i] < v[best]) {
			best = i;
		}
	}
}

bool nelder_mead_constrained(const floatvec &min, const floatvec &max,  traj_eval &ev, floatvec &best, floatvec &bestval, floatvec &beststate) {
	int ndim = min.size(), i, wi, ni, bi;
	vector<floatvec> eval(ndim+1), final(ndim+1);
	floatvec reval, eeval, ceval, rstate, estate, cstate;
	
	floatvec range = max - min;
	vector<floatvec> simplex;
	floatvec centroid(ndim), dir(ndim), reflect(ndim), expand(ndim), 
	         contract(ndim), worst(ndim);
	
	/* random initialization */
	floatvec rtmp(ndim);
	for (i = 0; i < ndim + 1; ++i) {
		rtmp.randomize(min, max);
		if (!ev.evaluate(rtmp, eval[i], final[i])) {
			return false;
		}
		simplex.push_back(rtmp);
	}
	
	for(int iters = 0; iters < MAXITERS; ++iters) {
		argmin(eval, wi, ni, bi);
		worst = simplex[wi];
		best = simplex[bi];
		bestval = eval[bi];
		beststate = final[bi];
		floatvec sum(ndim);
		
		/*
		 This used to be
		 
		 centroid = (sum(simplex, 1) - worst) / (simplex.n_cols - 1);
		 
		 which I'm pretty sure was wrong.
		*/
		centroid.zero();
		for (i = 0; i < simplex.size(); ++i) {
			if (i != wi) {
				centroid += simplex[i];
			}
		}
		centroid /= (simplex.size() - 1);
		
		dir = centroid - worst;
		reflect = centroid + dir * RCOEF;
		constrain(reflect, min, max);
		
		if (!ev.evaluate(reflect, reval, rstate)) {
			return false;
		}
		if (eval[bi] <= reval && reval < eval[ni]) {
			// reflection
			simplex[wi] = reflect;
			eval[wi] = reval;
			final[wi] = rstate;
			continue;
		}
		
		if (reval < eval[bi]) {
			// expansion
			expand = centroid + dir * ECOEF;
			constrain(expand, min, max);
			if (!ev.evaluate(expand, eeval, estate)) return false;
			if (eeval < reval) {
				simplex[wi] = expand;
				eval[wi] = eeval;
				final[wi] = estate;
			} else {
				simplex[wi] = reflect;
				eval[wi] = reval;
				final[wi] = rstate;
			}
			continue;
		}
		
		assert(reval >= eval[ni]);
		
		contract = worst + dir * CCOEF;
		if (!ev.evaluate(contract, ceval, cstate)) {
			return false;
		}
		if (ceval < eval[wi]) {
			// contraction
			simplex[wi] = contract;
			eval[wi] = ceval;
			final[wi] = cstate;
			continue;
		}
		
		// shrink
		for (i = 0; i < simplex.size(); ++i) {
			if (i == bi) {
				continue;
			}
			simplex[i] = best + (simplex[i] - best) * SCOEF;
			if (!ev.evaluate(simplex[i], eval[i], final[i])) {
				return false;
			}
		}
	}
	return true;
}

class tree_search {
public:
	tree_search(scene *scn, multi_model *mdl, multi_objective *obj, output_spec *outspec, float thresh)
	: outspec(outspec), thresh(thresh)
	{
		ci.scn = scn->copy();
		ci.obj = obj;
		ci.mdl = mdl;
		ci.outspec = outspec;
		ci.min.resize(outspec->size());
		ci.max.resize(outspec->size());
		ci.range.resize(outspec->size());
		for (int i = 0; i < outspec->size(); ++i) {
			ci.min[i] = (*outspec)[i].min;
			ci.max[i] = (*outspec)[i].max;
			ci.range[i] = ci.max[i] - ci.min[i];
		}
		
		floatvec initstate;
		scn->get_properties(initstate);
		bestnode = new node(initstate, &ci);
		leafs.push_back(bestnode);
		num_nodes = 1;
		total_depth = 1.0;
		avg_depth = 1.0;
		avg_bf = 1.0;
 	}
	
	~tree_search() {
		std::list<node*>::iterator i;
		for (i = leafs.begin(); i != leafs.end(); ++i) {
			delete *i;
		}
		for (i = nonleafs.begin(); i != nonleafs.end(); ++i) {
			delete *i;
		}
	}
	
	bool expand() {
		bool isleaf;
		node *newnode;
		std::list<node*>::iterator selected;
		
		if (nonleafs.size() > 0 && avg_depth / avg_bf > thresh) {
			int r = rand() % nonleafs.size();
			selected = nonleafs.begin();
			advance(selected, r);
			isleaf = false;
		} else {
			int r = rand() % leafs.size();
			selected = leafs.begin();
			advance(selected, r);
			isleaf = true;
		}
		
		newnode = (**selected).extend();
		
		if (newnode == NULL) {
			return false;
		}
		
		if (isleaf) {
			nonleafs.push_back(*selected);
			leafs.erase(selected);
		}
		
		leafs.push_back(newnode);
		num_nodes++;
		total_depth += newnode->traj.size();
		avg_depth = total_depth / num_nodes;
		avg_bf = ((float) num_nodes) / nonleafs.size();
		
		if (newnode->value < bestnode->value) {
			bestnode = newnode;
			return true;
		}
		return false;
	}
	
	void search(int iterations, std::list<floatvec> &besttraj, floatvec &bestval, floatvec &beststate) {
		for (int i = 0; i < iterations; ++i) {
			if (expand()) {
				break;
			}
		}
		
		besttraj = bestnode->traj;
		beststate = bestnode->state;
		bestval = bestnode->value;
		cout << "BEST TRAJ LENGTH " << bestnode->traj.size() << endl;
		cout << "AVG DEPTH " << avg_depth << endl;
		cout << "AVG BF " << avg_bf << endl;
		
		/*
		floatvec lengths(leafs.size());
		std::list<node*>::iterator i;
		for (i = leafs.begin(), j = 0; i != leafs.end(); ++i, ++j) {
			lengths[j] = (**i).depth;
		}
		histogram(lengths, 10);
		*/
	}
	
private:
	struct common_info {
		multi_objective *obj;
		multi_model *mdl;
		scene *scn;
		output_spec *outspec;
		floatvec min, max, range;
	};
	
	class node {
	public:
		std::list<floatvec> traj;
		floatvec value;
		floatvec state;
		common_info *ci;
		bool triedseek;
		
		node(const floatvec &state, common_info *ci)
		: state(state), ci(ci), triedseek(false)
		{
			ci->scn->set_properties(state);
			ci->obj->evaluate(*ci->scn, value);
		}
		
		node(const node &n) 
		: traj(n.traj), value(n.value), state(n.state), ci(n.ci), triedseek(false)
		{}
		
		/*
		 Follow a trajectory to a local minimum of the objective
		 function or the maximum number of steps, whichever
		 comes first.
		*/
		int seek(int maxsteps) {
			cout << "SEEK" << endl;
			floatvec step(ci->outspec->size()), newval, newstate;
			int steps;
			traj_eval eval(ci->outspec->size(), ci->mdl, ci->obj, *ci->scn);
			for (steps = 0; steps < maxsteps; ++steps) {
				eval.set_init(state);
				if (!nelder_mead_constrained(ci->min, ci->max, eval, step, newval, newstate)) {
					return -1;
				}
				if (newval >= value) {
					break;
				}
				traj.push_back(step);
				value = newval;
				state = newstate;
			}
			return steps;
		}
		
		int random_step(int maxsteps) {
			cout << "RANDOM" << endl;
			floatvec step(ci->outspec->size()), newval;
			for(int i = 0; i < step.size(); ++i) {
				step[i] = ci->min[i] + (ci->range[i] * rand()) / RAND_MAX;
			}
			int numsteps = rand() % maxsteps + 1;
			
			floatvec x(state.size() + step.size());
			for (int i = 0; i < numsteps; ++i) {
				traj.push_back(step);
				x.graft(0, state);
				x.graft(state.size(), step);
				if (!ci->mdl->predict(x, state)) {
					return false;
				}
			}
			ci->scn->set_properties(state);
			ci->obj->evaluate(*ci->scn, value);
			/*
			stringstream ss;
			ss << this;
			ci->scn->draw_all(ss.str(), 0.0, 0.0, 1.0);
			*/
			return true;
		}
		
		node *extend() {
			node *n = new node(*this);;
			if (!triedseek) {
				triedseek = true;
				if (n->seek(50) <= 0) {
					delete n;
					return NULL;
				}
			} else {
				if (!n->random_step(50)) {
					delete n;
					return NULL;
				}
			}
			return n;
		}
	};
	
	struct node_compare {
		bool operator()(const node *lhs, const node *rhs) const {
			/*
			 Since the priority queue keeps the largest items,
			 we have to reverse the comparison.
			*/
			return lhs->value > rhs->value;
		}
	};
	
	//priority_queue<node*, vector<node*>, node_compare> nodes;
	std::list<node*> leafs;
	std::list<node*> nonleafs;
	common_info ci;
	output_spec *outspec;
	int num_nodes;
	float total_depth, avg_depth, avg_bf, thresh;
	node *bestnode;
};

class controller {
public:
	controller(multi_model *mmdl, multi_objective *obj, output_spec *outspec, int depth, string type)
	: mmdl(mmdl), obj(obj), outspec(outspec), depth(depth), type(type), incr(depth, outspec)
	{
		int i, j;
		
		stepsize = outspec->size();
		min.resize(depth * outspec->size());
		max.resize(depth * outspec->size());
		for (i = 0; i < depth; ++i) {
			for (j = 0; j < stepsize; ++j) {
				min[i * stepsize + j] = (*outspec)[j].min;
				max[i * stepsize + j] = (*outspec)[j].max;
			}
		}
	}

	int seek(scene *scn, floatvec &bestout) {
		floatvec currval;
		obj->evaluate(*scn, currval);
		cout << "CURR VAL " << currval << endl;
		
		if (cached_traj.size() > 0) {
			// verify that cached trajectory is still valid, given current model
			floatvec currstate, finalstate;
			scn->get_properties(currstate);
			if (currval < cached_value ||
			    !predict_traj(mmdl, currstate, cached_traj, finalstate) ||
			    cached_state.distsq(finalstate) > 0.001)
			{
				cached_traj.clear();
			}
		}
		
		if (cached_traj.size() == 0) {
			// generate a new trajectory
			floatvec bestval, beststate;
			bool result;
			if (type == "tree") {
				tree_search t(scn, mmdl, obj, outspec, 0.5);
				t.search(depth, cached_traj, bestval, beststate);
			} else {
				floatvec besttraj;
				traj_eval evaluator(stepsize, mmdl, obj, *scn);
				if (type == "simplex") {
					result = nelder_mead_constrained(min, max, evaluator, besttraj, bestval, beststate);
				} else {
					result = naive_seek(evaluator, besttraj, bestval, beststate);
				}
				evaluator.print_stats();
				if (!result) {
					return 0;
				}
				for (int i = 0; i < besttraj.size(); i += stepsize) {
					cached_traj.push_back(besttraj.slice(i, i + stepsize));
				}
			}
			
			cout << "BEST VAL " << bestval << endl;
			if (currval < bestval) {
				cached_traj.clear();
			} else {
				cached_state = beststate;
				cached_value = bestval;
				scene *copy = scn->copy();
				copy->set_properties(cached_state);
				copy->draw_all("predict_", 1., 0., 0.);
			}
		}
		
		if (cached_traj.size() == 0) {
			return 1;
		}
		bestout = cached_traj.front();
		cout << "BEST OUT " << bestout << endl;
		cached_traj.pop_front();
		return 2;
	}
	
	bool naive_seek(traj_eval &evaluator, floatvec &besttraj, floatvec &bestval, floatvec &beststate) {
		floatvec val, finalstate;
		bool found = false;
		
		incr.reset();
		while (true) {
			if (!evaluator.evaluate(incr.traj, val, finalstate)) {
				return false;
			}
			if (!found || val < bestval) {
				found = true;
				besttraj = incr.traj;
				bestval = val;
				beststate = finalstate;
			}
			if (!incr.next()) {
				break;
			}
		}
		return found;
	}
	
private:
	/*
	 Incrementer for a single step within a trajectory
	*/
	class step_incr {
	public:
		step_incr(output_spec *outspec, floatvec *traj, int divisions, int start) 
		: outspec(outspec), traj(traj), start(start), divisions(divisions), inc(outspec->size())
		{
			for (int i = 0; i < outspec->size(); ++i) {
				inc[i] = ((*outspec)[i].max - (*outspec)[i].min) / divisions;
			}
			reset();
		}
	
		void reset() {
			for (int i = 0; i < outspec->size(); ++i) {
				(*traj)[start + i] = (*outspec)[i].min;
			}
		}
		
		bool next() {
			for (int i = 0; i < outspec->size(); ++i) {
				(*traj)[start + i] += inc[i];
				if ((*traj)[start + i] <= (*outspec)[i].max) {
					return true;
				} else {
					(*traj)[start + i] = (*outspec)[i].min;  // roll over and move on to the next value
				}
			}
			return false;
		}
		
	private:
		output_spec *outspec;
		int start, divisions;
		floatvec *traj;
		floatvec inc;
	};
	
	/*
	 Incrementer for a trajectory, used with naive search
	*/
	class traj_incr {
	public:
		traj_incr() : len(0) {}
		
		traj_incr(int len, output_spec *outspec)
		: len(len)
		{
			int stepsize = outspec->size();
			traj.resize(len * stepsize);
			for (int i = 0; i < len; i++) {
				steps.push_back(step_incr(outspec, &traj, 3, i * stepsize));
			}
			reset();
		}
		
		void reset() {
			std::vector<step_incr>::iterator i;
			for (i = steps.begin(); i != steps.end(); ++i) {
				i->reset();
			}
		}
		
		bool next() {
			std::vector<step_incr>::iterator i;
			for (i = steps.begin(); i != steps.end(); ++i) {
				if (i->next()) {
					return true;
				}
				i->reset();
			}
			return false;
		}
		
		floatvec traj;
	
	private:
		vector<step_incr> steps;
		int len;
	};
	
	multi_model     *mmdl;
	multi_objective *obj;
	output_spec     *outspec;
	floatvec         min, max;   // for Nelder-Mead
	int              depth;
	int              stepsize;
	string           type;
	traj_incr        incr;
	std::list<floatvec> cached_traj;
	floatvec            cached_state;
	floatvec            cached_value;
};

class seek_command : public command {
public:
	seek_command(svs_state *state, Symbol *root)
	: command(state, root), state(state), step(0),
	  stepwme(NULL), broken(false), ctrl(NULL), obj(NULL)
	{
		si = state->get_svs()->get_soar_interface();
		//update_step();
	}
	
	~seek_command() {
		cleanup();
	}
	
	string description() {
		return string("control");
	}
	
	bool update() {
		floatvec out;
		
		if (changed()) {
			broken = !parse_cmd();
		}
		if (broken) {
			return false;
		}
		
		timer t1;
		t1.start();
		int result = ctrl->seek(state->get_scene(), out);
		switch (result) {
			case 0:
				set_status("no valid output found");
				break;
			case 1:
				set_status("local minimum");
				break;
			case 2:
				state->set_output(out);
				set_status("success");
				step++;
				//update_step();
				break;
		}
		cout << "SEEK " << t1.stop() << endl;
		return true;
	}
	
	bool early() { return true; }
	
private:
	/* Assumes this format:
	   C1 ^outputs ( ... )
	      ^objective ( ... )
	*/
	bool parse_cmd() {
		wme *objective_wme, *model_wme, *depth_wme, *type_wme;
		long depth;
		string type;
		
		cleanup();
		if (!si->find_child_wme(get_root(), "type", type_wme) ||
			!si->get_val(si->get_wme_val(type_wme), type))
		{
			set_status("missing or invalid type");
			return false;
		}
		if (!si->find_child_wme(get_root(), "objective", objective_wme) ||
			!si->is_identifier(si->get_wme_val(objective_wme)) ||
			(obj = parse_obj_struct(si, si->get_wme_val(objective_wme))) == NULL)
		{
			set_status("missing or invalid objective");
			return false;
		}
		
		if (!si->find_child_wme(get_root(), "depth", depth_wme) ||
			!si->get_val(si->get_wme_val(depth_wme), depth))
		{
			set_status("missing or invalid depth");
			return false;
		}
		ctrl = new controller(state->get_model(), obj, state->get_output_spec(), depth, type);
		return true;
	}

	void cleanup() {
		delete obj; obj = NULL;
		delete ctrl; ctrl = NULL;
	}
	
	void update_step() {
		if (stepwme)
			si->remove_wme(stepwme);
		stepwme = si->make_wme(get_root(), "step", step);
	}

	soar_interface  *si;
	svs_state       *state;
	controller      *ctrl;
	multi_objective *obj;
	wme             *stepwme;
	int              step;
	bool             broken;
};

command *_make_seek_command_(svs_state *state, Symbol *root) {
	return new seek_command(state, root);
}

class random_control_command : public command {
public:
	random_control_command(svs_state *state, Symbol *root)
	: command(state, root), state(state)
	{ }
	
	string description() {
		return string("random control");
	}
	
	bool update() {
		if (changed()) {
			wme *outputs_wme;
			output_spec *outspec = state->get_output_spec();
			out.resize(outspec->size());
			min.resize(outspec->size());
			max.resize(outspec->size());
			for (int i = 0; i < outspec->size(); ++i) {
				min[i] = (*outspec)[i].min;
				max[i] = (*outspec)[i].max;
			}
		}
		
		out.randomize(min, max);
		state->set_output(out);
		set_status("success");
		return true;
	}
	
	bool early() { return true; }
	
private:
	svs_state *state;
	floatvec   out, min, max;
};

command *_make_random_control_command_(svs_state *state, Symbol *root) {
	return new random_control_command(state, root);
}

const char *PIPE_PATH = "ctrl";

class manual_control_command : public command {
public:
	
	manual_control_command(svs_state *state, Symbol *root) 
	: command(state, root), state(state), outspec(state->get_output_spec()), root(root)
	{
		si = state->get_svs()->get_soar_interface();
		output.resize(outspec->size());
	}
	
	~manual_control_command() {
		if (pipe.is_open()) {
			pipe.close();
			unlink(PIPE_PATH);
		}
	}
	
	string description() {
		return string("manual control");
	}
	
	bool update() {
		if (changed()) {
			parse_constants();
		}
		
		if (!using_constants) {
			if (!pipe.is_open()) {
				unlink(PIPE_PATH);
				mkfifo(PIPE_PATH, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
				pipe.open(PIPE_PATH);
			}
			for (int i = 0; i < outspec->size(); ++i) {
				if (!(pipe >> output[i])) {
					cerr << "manual control pipe read error" << endl;
					exit(1);
				}
			}
		}
		state->set_output(output);
		set_status("success");
		return true;
	}
	
	bool early() { return true; }
	
private:
	void parse_constants() {
		wme_list children;
		wme_list::iterator i;
		si->get_child_wmes(root, children);
		
		using_constants = false;
		bool first = true;
		for (i = children.begin(); i != children.end(); ++i) {
			string name;
			float val;
			if (!si->get_val(si->get_wme_attr(*i), name)) {
				continue;
			}
			if (!si->get_val(si->get_wme_val(*i), val)) {
				continue;
			}
			for (int j = 0; j < outspec->size(); ++j) {
				if ((*outspec)[j].name == name) {
					using_constants = true;
					if (first) {
						output.zero();
						first = false;
					}
					output[j] = val;
					break;
				}
			}
		}
	}
	
	bool using_constants;
	floatvec output;
	output_spec *outspec;
	svs_state *state;
	ifstream pipe;
	Symbol *root;
	soar_interface *si;
};

command *_make_manual_control_command_(svs_state *state, Symbol *root) {
	return new manual_control_command(state, root);
}
