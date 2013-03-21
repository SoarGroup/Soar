#include "classifier.h"
#include "serialize.h"
#include "common.h"

using namespace std;

void print_first_arg(const relation &r, ostream &os) {
	interval_set first;
	r.at_pos(0, first);
	os << first;
}

void extract_vec(const tuple &t, const rvec &x, const scene_sig &sig, rvec &out) {
	out.resize(x.size());
	int end = 0, s, n;
	for (int i = 1, iend = t.size(); i < iend; ++i) {
		bool found = false;
		for (int j = 0, jend = sig.size(); j < jend; ++j) {
			if (sig[j].id == t[i]) {
				s = sig[j].start;
				n = sig[j].props.size();
				found = true;
				break;
			}
		}
		assert(found);
		out.segment(end, n) = x.segment(s, n);
		end += n;
	}
	out.conservativeResize(end);
}

/*
 positive = class 0, negative = class 1
*/
num_classifier *learn_numeric_classifier(int type, const relation &pos, const relation &neg, const model_train_data &data) {
	int npos = pos.size(), nneg = neg.size();
	if (npos < 2 || nneg < 2) {
		return NULL;
	}
	
	// figure out matrix columns
	tuple t = *pos.begin();
	rvec xpart;
	extract_vec(t, data.get_inst(t[0]).x, *data.get_inst(t[0]).sig, xpart);
	int ncols = xpart.size();
	
	mat train(npos + nneg, ncols);
	vector<int> classes;
	
	relation::const_iterator i, iend;
	int j = 0;
	for (i = pos.begin(), iend = pos.end(); i != iend; ++i, ++j) {
		t = *i;
		const model_train_inst &inst = data.get_inst(t[0]);
		extract_vec(t, inst.x, *inst.sig, xpart);
		assert(xpart.size() == ncols);
		train.row(j) = xpart;
		classes.push_back(0);
	}
	
	for (i = neg.begin(), iend = neg.end(); i != iend; ++i, ++j) {
		t = *i;
		const model_train_inst &inst = data.get_inst(t[0]);
		extract_vec(t, inst.x, *inst.sig, xpart);
		assert(xpart.size() == ncols);
		train.row(j) = xpart;
		classes.push_back(1);
	}
	
	num_classifier *nc = new num_classifier(type);
	nc->learn(train, classes);
	return nc;
}

binary_classifier::binary_classifier()
: const_vote(0), use_const(true), neg_nc(NULL), use_foil(true), use_pruning(true), nc_type(NC_SIGN)
{}

binary_classifier::binary_classifier(bool use_foil, bool use_pruning, int nc_type)
: const_vote(0), use_const(true), neg_nc(NULL), use_foil(use_foil), use_pruning(use_pruning), nc_type(nc_type)
{}

binary_classifier::~binary_classifier() {
	if (neg_nc) {
		delete neg_nc;
	}
}

void binary_classifier::serialize(ostream &os) const {
	serializer(os) << const_vote << use_const << clauses 
	               << false_negatives << true_negatives
	               << neg_nc;
}

void binary_classifier::unserialize(istream &is) {
	unserializer(is) >> const_vote >> use_const >> clauses 
	                 >> false_negatives >> true_negatives
	                 >> neg_nc;
}

void binary_classifier::inspect(ostream &os) const {
	if (use_const) {
		os << "Constant Vote: " << const_vote << endl;
		return;
	}
	
	table_printer t;
	t.add_row() << "clause" << "Correct" << "Incorrect" << "NumCls?";
	for (int i = 0, iend = clauses.size(); i < iend; ++i) {
		t.add_row() << clauses[i].cl << clauses[i].true_pos.size() << clauses[i].false_pos.size() << (clauses[i].nc != NULL);
	}
	t.add_row() << "NEGATIVE" << true_negatives.size() << false_negatives.size() << (neg_nc != NULL);
	t.print(os);
	os << endl;
}

void binary_classifier::inspect_detailed(ostream &os) const {
	if (use_const) {
		os << "Constant Vote: " << const_vote << endl;
		return;
	}
	
	if (clauses.empty()) {
		os << "No clauses" << endl;
	} else {
		for (int k = 0; k < clauses.size(); ++k) {
			os << "Clause: " << clauses[k].cl << endl;
			
			os << "True positives: ";
			//print_first_arg(clauses[k].true_pos, os);
			os << endl << clauses[k].true_pos;
			os << endl << endl;
			
			os << "False positives: ";
			print_first_arg(clauses[k].false_pos, os);
			os << endl << endl;
			
			if (clauses[k].nc) {
				os << "Numeric classifier:" << endl;
				clauses[k].nc->inspect(os);
				os << endl << endl;
			}
		}
	}
	os << "NEGATIVE:" << endl;
	
	os << "True negatives: ";
	print_first_arg(true_negatives, os);
	os << endl << endl;
	
	os << "False negatives: ";
	print_first_arg(false_negatives, os);
	os << endl << endl;
	
	if (neg_nc) {
		os << "Negative numeric classifier:" << endl;
		neg_nc->inspect(os);
		os << endl;
	}
}

/*
 Return 0 to vote for i, 1 to vote for j
*/
int binary_classifier::vote(int target, const scene_sig &sig, const relation_table &rels, const rvec &x) const {
	function_timer t(timers.get_or_add("vote"));
	
	if (use_const || (!use_foil && nc_type == NC_NONE)) {
		LOG(EMDBG) << "Constant vote for " << const_vote << endl;
		return const_vote;
	}
	
	if (clauses.size() > 0) {
		var_domains domains;
		domains[0].insert(0);       // rels is only for the current timestep, time should always be 0
		domains[1].insert(sig[target].id);
		
		for (int i = 0, iend = clauses.size(); i < iend; ++i) {
			const clause &cl = clauses[i].cl;
			const num_classifier *nc = clauses[i].nc;
			if (test_clause(cl, rels, domains)) {
				LOG(EMDBG) << "matched clause:" << endl << cl << endl;
				var_domains::const_iterator vi, viend;
				for (vi = domains.begin(), viend = domains.end(); vi != viend; ++vi) {
					assert(vi->second.size() == 1);
					LOG(EMDBG) << vi->first << " = " << *vi->second.begin() << endl;
				}
				if (nc_type != NC_NONE && nc) {
					int result = nc->classify(x);
					LOG(EMDBG) << "NC votes for " << result << endl;
					if (result == 0) {
						return result;
					}
				} else {
					LOG(EMDBG) << "No NC, voting for 0" << endl;
					return 0;
				}
			}
		}
	}
	// no matched clause, FOIL thinks this is a negative
	if (nc_type != NC_NONE && neg_nc) {
		int result = 1 - neg_nc->classify(x);
		LOG(EMDBG) << "No matched clauses, NC votes for " << result << endl;
		return result;
	}
	// no false negatives in training, so this must be a negative
	LOG(EMDBG) << "No matched clauses, no NC, vote for 1" << endl;
	return 1;
}

void binary_classifier::update(const relation &mem_i, const relation &mem_j, const relation_table &rels, const model_train_data &data) {
	function_timer t(timers.get_or_add("update"));
	
	clauses.clear();
	if (neg_nc) {
		delete neg_nc;
		neg_nc = NULL;
	}
	
	const_vote = mem_i.size() > mem_j.size() ? 0 : 1;
	if (mem_i.empty() || mem_j.empty()) {
		use_const = true;
		return;
	}
	
	use_const = false;
	if (use_foil) {
		FOIL foil;
		foil.set_problem(mem_i, mem_j, rels);
		foil.learn(use_pruning, true);
		clauses.resize(foil.num_clauses());
		for (int k = 0, kend = foil.num_clauses(); k < kend; ++k) {
			clauses[k].cl = foil.get_clause(k);
			clauses[k].false_pos = foil.get_false_positives(k);
			clauses[k].true_pos = foil.get_true_positives(k);
		}
		false_negatives = foil.get_false_negatives();
		true_negatives = foil.get_true_negatives();
	} else {
		/*
		 Don't learn any clauses. Instead consider every member of i a false negative
		 and every member of j a true negative, and let the numeric classifier take care of it.
		*/
		false_negatives = mem_i;
		true_negatives = mem_j;
	}
	
	/*
	 For each clause cl in clauses, if cl misclassified any of the
	 members of j in the training set as a member of i (false positive
	 for cl), train a numeric classifier to classify it correctly.
	 
	 Also train a numeric classifier to catch misclassified members of
	 i (false negatives for the entire clause vector).
	*/
	if (nc_type != NC_NONE) {
		for (int k = 0, kend = clauses.size(); k < kend; ++k) {
			if (clauses[k].nc) {
				delete clauses[k].nc;
				clauses[k].nc = NULL;
			}
			double fp = clauses[k].false_pos.size(), tp = clauses[k].true_pos.size();
			if (fp / (fp + tp) > .5) {
				clauses[k].nc = learn_numeric_classifier(nc_type, clauses[k].true_pos, clauses[k].false_pos, data);
			}
		}
		
		double fn = false_negatives.size(), tn = true_negatives.size();
		if (fn / (fn + tn) > .5) {
			neg_nc = learn_numeric_classifier(nc_type, true_negatives, false_negatives, data);
		}
	}
}

void clause_info::serialize(ostream &os) const {
	serializer(os) << cl << false_pos << true_pos << nc;
}

void clause_info::unserialize(istream &is) {
	unserializer(is) >> cl >> false_pos >> true_pos >> nc;
}

classifier::classifier(const model_train_data &data) 
: data(data), foil(true), prune(true), context(true), nc_type(NC_LDA)
{
}

classifier::~classifier() {
	clear_and_dealloc(pairs);
	clear_and_dealloc(classes);
}

void classifier::proxy_get_children(map<string, cliproxy*> &c) {
	c["use_foil"]    = new bool_proxy(&foil);
	c["use_pruning"] = new bool_proxy(&prune);
	c["use_context"] = new bool_proxy(&context);
	c["nc_type"]     = new memfunc_proxy<classifier>(this, &classifier::cli_nc_type);
	c["dump_foil6"]  = new memfunc_proxy<classifier>(this, &classifier::cli_dump_foil6);
}

void classifier::cli_nc_type(const vector<string> &args, ostream &os) {
	static const char *names[] = { "none", "dtree", "lda", "sign" };
	
	if (args.empty()) {
		os << names[nc_type] << endl;
	} else {
		for (int i = 0, iend = sizeof(names) / sizeof(names[0]); i < iend; ++i) {
			if (args[0] == names[i]) {
				nc_type = i;
				return;
			}
		}
		os << "invalid numeric classifier type" << endl;
	}
}

void classifier::set_options(bool foil, bool prune, bool context, int nc_type) {
	foil = foil;
	prune = prune;
	context = context;
	nc_type = nc_type;
	for (int i = 0, iend = classes.size(); i < iend; ++i) {
		classes[i]->stale = true;
	}
}

void classifier::add_class() {
	int c = classes.size();
	for (int i = 0, iend = classes.size(); i < iend; ++i) {
		binary_classifier *b = new binary_classifier(foil, prune, nc_type);
		pairs.push_back(new pair_info(i, c, b));
	}
	classes.push_back(new class_info);
}

void classifier::del_classes(const vector<int> &c) {
	vector<int> class_map(classes.size());
	
	int n = 0;
	for (int i = 0, iend = classes.size(); i < iend; ++i) {
		if (has(c, i)) {
			assert(classes[i]->mem_rel.empty());
			delete classes[i];
			class_map[i] = -1;
		} else {
			classes[n] = classes[i];
			class_map[i] = n++;
		}
	}
	classes.resize(n);
	
	std::list<pair_info*>::iterator i, iend;
	for (i = pairs.begin(), iend = pairs.end(); i != iend; ) {
		pair_info &p = **i;
		if (class_map[p.cls_i] == -1 || class_map[p.cls_j] == -1) {
			delete *i;
			i = pairs.erase(i);
		} else {
			p.cls_i = class_map[p.cls_i];
			p.cls_j = class_map[p.cls_j];
			++i;
		}
	}
}

void classifier::update_inst(int i, int c) {
	assert(0 <= i && i <= membership.size() && 0 <= c && c < classes.size());
	// assume new instances can only arrive in order
	if (i == membership.size()) {
		membership.push_back(-1);
	}
	
	if (membership[i] == c) {
		return;
	}
	
	const model_train_inst &inst = data.get_inst(i);
	int target = (*inst.sig)[inst.target].id;
	if (membership[i] >= 0) {
		class_info *oldc = classes[membership[i]];
		oldc->mem_rel.del(i, target);
		oldc->stale = true;
	}
	
	class_info *newc = classes[c];
	newc->mem_rel.add(i, target);
	newc->stale = true;
	
	membership[i] = c;
}

classifier::pair_info *classifier::find(int i, int j) {
	assert(i < j);
	
	list<pair_info*>::iterator pi, pend;
	for (pi = pairs.begin(), pend = pairs.end(); pi != pend; ++pi) {
		if ((**pi).cls_i == i && (**pi).cls_j == j) {
			return *pi;
		}
	}
	return NULL;
}

void classifier::update() {
	list<pair_info*>::iterator i, iend;
	for (i = pairs.begin(), iend = pairs.end(); i != iend; ++i) {
		pair_info *p = *i;
		class_info *ci = classes[p->cls_i], *cj = classes[p->cls_j];
		if (!ci->stale && !cj->stale) {
			continue;
		}
		
		binary_classifier &c = *p->clsfr;
		if (context) {
			c.update(ci->mem_rel, cj->mem_rel, data.get_context_rels(), data);
		} else {
			c.update(ci->mem_rel, cj->mem_rel, data.get_all_rels(), data);
		}
	}
	
	for (int i = 0, iend = classes.size(); i < iend; ++i) {
		classes[i]->stale = false;
	}
}

void classifier::classify(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, vector<int> &votes) {
	update();
	votes.clear();
	votes.resize(classes.size(), 0);
	
	const relation_table *r;
	if (context) {
		relation_table *r1 = new relation_table;
		get_context_rels(sig[target].id, rels, *r1);
		r = r1;
	} else {
		r = &rels;
	}
	
	list<pair_info*>::const_iterator i, iend;
	for (i = pairs.begin(), iend = pairs.end(); i != iend; ++i) {
		const pair_info &p = **i;
		int winner = p.clsfr->vote(target, sig, *r, x);
		if (winner == 0) {
			++votes[p.cls_i];
		} else if (winner == 1) {
			++votes[p.cls_j];
		} else {
			assert(false);
		}
	}
	
	if (context) {
		delete r;
	}
}

void classifier::proxy_use_sub(const vector<string> &args, ostream &os) {
	update();
	
	if (args.empty()) {
		// print summary of all classifiers
		for (int i = 0, iend = classes.size(); i < iend; ++i) {
			for (int j = i + 1, jend = classes.size(); j < jend; ++j) {
				pair_info *p = find(i, j);
				assert(p);
				os << "=== FOR MODES " << i << "/" << j << " ===" << endl;
				if (p->clsfr) {
					p->clsfr->inspect(os);
				} else {
					// why would this happen?
					os << "no classifier" << endl;
				}
			}
		}
		return;
	}
	
	int i, j;
	pair_info *p;
	if (args.size() != 2) {
		os << "Specify two modes" << endl;
		return;
	}
	
	if (!parse_int(args[0], i) || !parse_int(args[1], j) || !(p = find(i, j))) {
		os << "invalid modes, make sure i < j" << endl;
		return;
	}
	
	if (p->clsfr) {
		p->clsfr->inspect_detailed(os);
	} else {
		os << "null" << endl;
	}
}

void classifier::cli_dump_foil6(const vector<string> &args, ostream &os) const {
	int m1, m2;
	if (args.size() != 2 || 
	    !parse_int(args[0], m1) || 
	    !parse_int(args[1], m2) ||
	    m1 < 0 || m1 >= classes.size() || m2 < 0 || m2 >= classes.size() || m1 == m2) 
	{
		os << "Specify 2 modes" << endl;
		return;
	}
	
	if (m1 > m2) {
		swap(m1, m2);
	}
	
	FOIL foil;
	if (context) {
		foil.set_problem(classes[m1]->mem_rel, classes[m2]->mem_rel, data.get_context_rels());
	} else {
		foil.set_problem(classes[m1]->mem_rel, classes[m2]->mem_rel, data.get_all_rels());
	}
	foil.dump_foil6(os);
}

void classifier::serialize(ostream &os) const {
	serializer(os) << pairs << classes << membership;
}

void classifier::unserialize(istream &is) {
	unserializer(is) >> pairs >> classes >> membership;
}

void classifier::pair_info::serialize(ostream &os) const {
	serializer(os) << cls_i << cls_j << clsfr;
}

void classifier::pair_info::unserialize(istream &is) {
	unserializer(is) >> cls_i >> cls_j >> clsfr;
}

void classifier::class_info::serialize(ostream &os) const {
	serializer(os) << mem_rel << stale;
}

void classifier::class_info::unserialize(istream &is) {
	unserializer(is) >> mem_rel >> stale;
}
