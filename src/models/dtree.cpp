#include <iostream>
#include <sstream>
#include <algorithm>
#include <math.h>
#include <assert.h>
#include "dtree.h"
#include "common.h"

using namespace std;
int dbgc = 0;

double entropy(const map<category, int> &counts, int total) {
	double ent = 0.;
	map<category, int>::const_iterator i;
	for (i = counts.begin(); i != counts.end(); ++i) {
		if (i->second > 0) {
			double p = ((double) i->second) / total;
			ent += -p * log2(p);
		}
	}
	return ent;
}

template<typename T>
void vec_remove_item(vector<T> &v, const T &item) {
	typename vector<T>::iterator i = remove(v.begin(), v.end(), item);
	assert(i != v.end());
	v.erase(i, v.end());
}

template<typename T>
bool is_unique(const vector<T> &v) {
	vector<T> t;
	copy(v.begin(), v.end(), back_inserter(t));
	sort(t.begin(), t.end());
	t.erase(unique(t.begin(), t.end()), t.end());
	return t.size() == v.size();
}

void DTreeInst::save(ostream &os) const {
	os << cat << endl;
	save_vector(attrs, os);
}

void DTreeInst::load(istream &is) {
	is >> cat;
	load_vector(attrs, is);
}

ID3Tree::ID3Tree(const vector<DTreeInst> &insts)
: split_attr(-1)
{
	vector<int> attrs;
	for (int i = 0; i < insts[0].attrs.size(); ++i) {
		attrs.push_back(i);
	}
	learn_rec(insts, attrs);
}

int ID3Tree::classify(const attr_vec &attrs) const {
	if (split_attr < 0) {
		return cat;
	}
	if (attrs[split_attr]) {
		return left->classify(attrs);
	}
	return right->classify(attrs);
}

void ID3Tree::output(const vector<string> &attr_names) const {
	output_rec("", attr_names);
}

ID3Tree::ID3Tree()
: split_attr(-1), cat(90909) // debug val
{ }

int ID3Tree::choose_attrib(const vector<DTreeInst> &insts, const vector<int> &attrs) {
	int highattrib = -1, ninsts = insts.size();
	double highgain, curr_ent;
	map<category, int> ttl_counts;
	for (int j = 0; j < ninsts; ++j) {
		++ttl_counts[insts[j].cat];
	}
	curr_ent = entropy(ttl_counts, ninsts);
	
	std::vector<int>::const_iterator i;
	for (i = attrs.begin(); i != attrs.end(); ++i) {
		map<category, int> true_counts, false_counts;
		int ttl_true = 0, ttl_false = 0;
		for (int j = 0; j < ninsts; ++j) {
			if (insts[j].attrs[*i]) {
				++true_counts[insts[j].cat];
				++ttl_true;
			} else {
				++false_counts[insts[j].cat];
				++ttl_false;
			}
		}
		
		double true_ent = entropy(true_counts, ninsts) * ((double) ttl_true) / ninsts;
		double false_ent = entropy(false_counts, ninsts) * ((double) ttl_false) / ninsts;
		double gain = curr_ent - (true_ent + false_ent);
		if (highattrib == -1 || gain > highgain) {
			highattrib = *i;
			highgain = gain;
		}
	}
	return highattrib;
}

void ID3Tree::learn_rec(const vector<DTreeInst> &insts, const vector<int> &attrs) {
	assert(insts.size() > 0);
	
	cat = insts[0].cat;
	bool same_cat = true;
	for (int i = 0; i < insts.size(); ++i) {
		if (cat != insts[i].cat) {
			same_cat = false;
			break;
		}
	}
	if (same_cat) {
		return;
	}
	
	bool same_attrs = true;
	for (int i = 0; i < insts.size(); ++i) {
		if (insts[i].attrs != insts[0].attrs) {
			same_attrs = false;
			break;
		}
	}
	
	if (attrs.size() == 0 || same_attrs) {
		map<category, int> counts;
		for (int i = 0; i < insts.size(); ++i) {
			int c = insts[i].cat;
			if (counts.find(c) == counts.end()) {
				counts[c] = 1;
			} else {
				++counts[c];
			}
			if ((counts.size() == 1) || (counts[cat] < counts[c])) {
				cat = c;
			}
		}
		return;
	}
	
	split_attr = choose_attrib(insts, attrs);
	vector<attr_vec> lattrs, rattrs;
	vector<DTreeInst> linsts, rinsts;
	for (int i = 0; i < insts.size(); ++i) {
		if (insts[i].attrs[split_attr]) {
			linsts.push_back(insts[i]);
		} else {
			rinsts.push_back(insts[i]);
		}
	}
	
	vector<int> child_attrs(attrs);
	child_attrs.erase(child_attrs.begin() + split_attr);
	if (lattrs.size() > 0) {
		left.reset(new ID3Tree());
		left->learn_rec(linsts, child_attrs);
	} else {
		left.reset();
	}
	
	if (rattrs.size() > 0) {
		right.reset(new ID3Tree());
		right->learn_rec(rinsts, child_attrs);
	} else {
		right.reset();
	}
}

void ID3Tree::output_rec(const string &prefix, const vector<string> &attrib_names) const {
	if (split_attr < 0) {
		cout << prefix << " : " << cat << endl;
	} else {
		stringstream lss, rss;
		lss << prefix << " " << attrib_names[split_attr] << ":t";
		rss << prefix << " " << attrib_names[split_attr] << ":f";
		left->output_rec(lss.str(), attrib_names);
		right->output_rec(rss.str(), attrib_names);
	}
}

/*
 insts is the list of all instances. The tree will only maintain
 a reference to it, so it can grow after the tree is created. The
 insts_here variable indexes into this list.
*/
ID5Tree::ID5Tree(const vector<DTreeInst> &insts) 
: insts(insts), split_attr(-1), cat(90909)
{
	assert(insts.size() > 0);
	int nattrs = insts[0].attrs.size();
	attrs_here.reserve(nattrs);
	for (int i = 0; i < nattrs; ++i) {
		attrs_here.push_back(i);
	}
}

ID5Tree::ID5Tree(const vector<DTreeInst> &insts, const vector<int> &attrs) 
: insts(insts), attrs_here(attrs), split_attr(-1), cat(90909)
{ }

void ID5Tree::expand() {
	assert(left.get() == NULL && right.get() == NULL && split_attr != -1 && attrs_here.size() > 0);
	vector<int> attrs;
	remove_copy(attrs_here.begin(), attrs_here.end(), back_inserter(attrs), split_attr);
	assert(attrs.size() == attrs_here.size() - 1);

	left.reset(new ID5Tree(insts, attrs));
	right.reset(new ID5Tree(insts, attrs));
	vector<int>::const_iterator i;
	for (i = insts_here.begin(); i != insts_here.end(); ++i) {
		if (insts[*i].attrs[split_attr]) {
			left->update_tree(*i);
		} else {
			right->update_tree(*i);
		}
	}
}

void ID5Tree::update_tree(int i) {
	if (expanded() && (left->empty() || right->empty())) {
		shrink();
	}
	
	if (i >= 0) {
		assert(find(insts_here.begin(), insts_here.end(), i) == insts_here.end());
		insts_here.push_back(i);
		update_counts(i);
	}
	
	if (cats_all_same()) {
		cat = insts[insts_here[0]].cat;
		shrink();
	} else if (attrs_here.size() == 0 || attrs_all_same()) {
		map<category, int>::const_iterator i;
		for (i = ttl_counts.begin(); i != ttl_counts.end(); ++i) {
			if (ttl_counts[cat] < i->second) {
				cat = i->first;
			}
		}
		shrink();
	} else {
		int best_split = choose_split();
		if (!expanded()) {
			split_attr = best_split;
			expand();
		} else {
			if (best_split != split_attr) {
				pull_up(best_split);
			}
			if (i < 0) {
				left->update_tree(-1);
				right->update_tree(-1);
			} else if (insts[i].attrs[split_attr]) {
				left->update_tree(i);
				right->update_tree(-1);
			} else {
				left->update_tree(-1);
				right->update_tree(i);
			}
		}
	}
}

void ID5Tree::shrink() {
	left.reset();
	right.reset();
	split_attr = -1;
}

/*
 Restructure tree so that the current node splits on attribute i.
*/
void ID5Tree::pull_up(int i) {
	assert(find(attrs_here.begin(), attrs_here.end(), i) != attrs_here.end());
	if (split_attr == i) {
		return;
	} else if (!expanded()) {
		split_attr = i;
		expand();
	} else {
		left->pull_up(i);
		right->pull_up(i);
		assert(left->expanded() && right->expanded());
		
		/*
		 Children now split on i and current node splits on
		 j. When we reverse this the left->right and right->left
		 subsubtrees will switch places, but none of the
		 subsubtrees have to be recomputed.
		*/
		vec_remove_item(left->attrs_here, i);
		vec_remove_item(right->attrs_here, i);
		left->attrs_here.push_back(split_attr);
		right->attrs_here.push_back(split_attr);
		
		assert(is_unique(left->attrs_here));
		assert(is_unique(right->attrs_here));
		
		left->split_attr = split_attr;
		right->split_attr = split_attr;
		split_attr = i;
		
		swap(left->right, right->left);
		left->pull_up_repair();
		right->pull_up_repair();
	}
}

void ID5Tree::pull_up_repair() {
	/*
	 The set of instances is just the union of the sets of instances
	 at the children.
	*/
	insts_here.clear();
	copy(left->insts_here.begin(), left->insts_here.end(), back_inserter(insts_here));
	copy(right->insts_here.begin(), right->insts_here.end(), back_inserter(insts_here));
	
	update_counts_from_children();
}

void ID5Tree::update_all_counts() {
	ttl_counts.clear();
	av_counts.clear();
	vector<int>::iterator i;
	
	/*
	 This is just so that the map actually has all the entries,
	 even though all counts may be 0
	*/
	for (i = attrs_here.begin(); i != attrs_here.end(); ++i) {
		av_counts[*i];
	}
	for (i = insts_here.begin(); i != insts_here.end(); ++i) {
		update_counts(*i);
	}
}

/*
 Update counts after adding instance i.
*/
void ID5Tree::update_counts(int i) {
	++ttl_counts[insts[i].cat];
	vector<int>::iterator j;
	for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
		val_counts &c = av_counts[*j];
		if (insts[i].attrs[*j]) {
			++c.ttl_true;
			++c.true_counts[insts[i].cat];   //  initialized to 0 if it doesn't exist in the map
		} else {
			++c.ttl_false;
			++c.false_counts[insts[i].cat];
		}
	}
}


/*
 Update counts after the category of instance i changes from old to its
 current value. Assumes that counts were correct before the change.
*/
void ID5Tree::update_category(int i, category old) {
	assert(find(insts_here.begin(), insts_here.end(), i) != insts_here.end());
	assert(is_unique(attrs_here));
	
	int cat = insts[i].cat;
	--ttl_counts[old];
	++ttl_counts[cat];
	vector<int>::iterator j;
	for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
		val_counts &c = av_counts[*j];
		if (insts[i].attrs[*j]) {
			--c.true_counts[old];
			assert(c.true_counts[old] >= 0);
			++c.true_counts[cat];
		} else {
			--c.false_counts[old];
			assert(c.false_counts[old] >= 0);
			++c.false_counts[cat];
		}
	}
	
	if (expanded()) {
		if (insts[i].attrs[split_attr]) {
			left->update_category(i, old);
		} else {
			right->update_category(i, old);
		}
	}
	
	//assert(validate_counts());
}

/*
 Assuming that the counts for each (attrib, value, class) triple is
 correct in each child, reconstruct the counts in this node by summing
 the child counts.
*/
void ID5Tree::update_counts_from_children() {
	ttl_counts.clear();
	av_counts.clear();
	
	//left->update_all_counts();
	//right->update_all_counts();
	map<category, int>::iterator i;
	for (i = left->ttl_counts.begin(); i != left->ttl_counts.end(); ++i) {
		ttl_counts[i->first] += i->second;
		av_counts[split_attr].ttl_true += i->second;
		av_counts[split_attr].true_counts[i->first] = i->second;
	}
	for (i = right->ttl_counts.begin(); i != right->ttl_counts.end(); ++i) {
		ttl_counts[i->first] += i->second;
		av_counts[split_attr].ttl_false += i->second;
		av_counts[split_attr].false_counts[i->first] = i->second;
	}
	
	vector<int>::iterator j;
	for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
		if (*j == split_attr) {
			continue;
		}
		val_counts &ccounts = av_counts[*j];
		val_counts &lcounts = left->av_counts[*j];
		val_counts &rcounts = right->av_counts[*j];
		
		ccounts.ttl_true = lcounts.ttl_true + rcounts.ttl_true;
		ccounts.ttl_false = lcounts.ttl_false + rcounts.ttl_false;
		map<category, int>::const_iterator k;
		for (k = lcounts.true_counts.begin(); k != lcounts.true_counts.end(); ++k) {
			ccounts.true_counts[k->first] += k->second;
		}
		for (k = rcounts.true_counts.begin(); k != rcounts.true_counts.end(); ++k) {
			ccounts.true_counts[k->first] += k->second;
		}
		for (k = lcounts.false_counts.begin(); k != lcounts.false_counts.end(); ++k) {
			ccounts.false_counts[k->first] += k->second;
		}
		for (k = rcounts.false_counts.begin(); k != rcounts.false_counts.end(); ++k) {
			ccounts.false_counts[k->first] += k->second;
		}
	}
}

bool ID5Tree::validate_counts() {
	vector<int>::iterator i, j;
	map<category, int>::iterator k;
	map<int, val_counts> ref_av_counts;
	map<category, int> ref_ttl_counts;
	
	/* clear all zero count entries */
	for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
		val_counts &counts = av_counts[*j];
		
		k = counts.true_counts.begin();
		while (k != counts.true_counts.end()) {
			if (k->second == 0) {
				counts.true_counts.erase(k++);
			} else {
				++k;
			}
		}
		k = counts.false_counts.begin();
		while (k != counts.false_counts.end()) {
			if (k->second == 0) {
				counts.false_counts.erase(k++);
			} else {
				++k;
			}
		}
	}
	
	for (i = insts_here.begin(); i != insts_here.end(); ++i) {
		const DTreeInst &inst = insts[*i];
		++ref_ttl_counts[inst.cat];
		for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
			val_counts &c = ref_av_counts[*j];
			if (inst.attrs[*j]) {
				++c.ttl_true;
				++c.true_counts[inst.cat];
			} else {
				++c.ttl_false;
				++c.false_counts[inst.cat];
			}
		}
	}
	
	for (k = ttl_counts.begin(); k != ttl_counts.end(); ) {
		if (k->second == 0) {
			ttl_counts.erase(k++);
		} else {
			++k;
		}
	}
	
	assert(ref_ttl_counts == ttl_counts);
	assert(ref_av_counts == av_counts);
	
	for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
		val_counts &counts = av_counts[*j];
		int ttl_true = 0, ttl_false = 0;
		
		for (k = counts.true_counts.begin(); k != counts.true_counts.end(); ++k) {
			assert (k->second + counts.false_counts[k->first] == ttl_counts[k->first]);
			ttl_true += k->second;
		}
		for (k = counts.false_counts.begin(); k != counts.false_counts.end(); ++k) {
			ttl_false += k->second;
		}
		assert (counts.ttl_true == ttl_true && counts.ttl_false == ttl_false);
	}
	if (!expanded()) {
		return true;
	}
	for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
		if (*j == split_attr) {
			continue;
		}
		val_counts &counts = av_counts[*j];
		val_counts &lcounts = left->av_counts[*j];
		val_counts &rcounts = right->av_counts[*j];
		
		for (k = counts.true_counts.begin(); k != counts.true_counts.end(); ++k) {
			if (k->second != lcounts.true_counts[k->first] + rcounts.true_counts[k->first]) {
				return false;
			}
		}
		for (k = counts.false_counts.begin(); k != counts.false_counts.end(); ++k) {
			if (k->second != lcounts.false_counts[k->first] + rcounts.false_counts[k->first]) {
				return false;
			}
		}
	}
	return true;
}

/*
 Choose the best attribute to split on based on current entropy gain
 estimates.
*/
int ID5Tree::choose_split() {
	update_gains();
	double highgain;
	int attr = -1;
	vector<int>::const_iterator i;
	for (i = attrs_here.begin(); i != attrs_here.end(); ++i) {
		if (attr == -1 || gains[*i] > highgain) {
			attr = *i;
			highgain = gains[*i];
		}
	}
	return attr;
}

/*
 Update the entropy gain expected from splitting on each attribute.
*/
void ID5Tree::update_gains() {
	//update_all_counts();
	gains.clear();
	double curr_ent = entropy(ttl_counts, insts_here.size());
	vector<int>::const_iterator i;
	for (i = attrs_here.begin(); i != attrs_here.end(); ++i) {
		val_counts &c = av_counts[*i];
		double true_ent = entropy(c.true_counts, c.ttl_true) * ((double) c.ttl_true) / insts_here.size();
		double false_ent = entropy(c.false_counts, c.ttl_false) * ((double) c.ttl_false) / insts_here.size();
		gains[*i] = curr_ent - (true_ent + false_ent);
	}
}

void ID5Tree::output(const vector<string> &attr_names) const {
	output_rec("", attr_names);
}

void ID5Tree::output_rec(const string &prefix, const vector<string> &attr_names) const {
	if (split_attr < 0) {
		cout << prefix << " : " << cat << endl;
	} else {
		stringstream lss, rss;
		lss << prefix << " " << attr_names[split_attr] << ":t";
		rss << prefix << " " << attr_names[split_attr] << ":f";
		left->output_rec(lss.str(), attr_names);
		right->output_rec(rss.str(), attr_names);
	}
}

bool ID5Tree::expanded() const {
	return split_attr != -1;
}

bool ID5Tree::empty() const {
	return insts_here.size() == 0;
}

bool ID5Tree::cats_all_same() const {
	if (insts_here.size() < 2) {
		return true;
	}
	vector<int>::const_iterator i;
	for (i = insts_here.begin(); i != insts_here.end(); ++i) {
		if (insts[*i].cat != insts[insts_here[0]].cat) {
			return false;
		}
	}
	return true;
}

bool ID5Tree::attrs_all_same() const {
	if (insts_here.size() < 2) {
		return true;
	}
	vector<int>::const_iterator i;
	for (i = insts_here.begin(); i != insts_here.end(); ++i) {
		vector<int>::const_iterator j;
		for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
			if (insts[*i].attrs[*j] != insts[insts_here[0]].attrs[*j]) {
				DATAVIS("'attrs all same' false" << endl)
				return false;
			}
		}
	}
	DATAVIS("'attrs all same' true" << endl)
	return true;
}

int ID5Tree::size() const {
	if (!expanded()) {
		return 1;
	}
	return 1 + left->size() + right->size();
}

int ID5Tree::classify(const attr_vec &attrs) const {
	if (!expanded()) {
		return cat;
	}
	if (attrs[split_attr]) {
		return left->classify(attrs);
	}
	return right->classify(attrs);
}

void ID5Tree::get_all_splits(vector<int> &splits) const {
	if (split_attr < 0) {
		return;
	}
	splits.push_back(split_attr);
	left->get_all_splits(splits);
	right->get_all_splits(splits);
}

void ID5Tree::save(ostream &os) const {
	save_vector(insts_here, os);
	save_vector(attrs_here, os);
	os << split_attr << endl;
	if (split_attr >= 0) {
		left->save(os);
		right->save(os);
	} else {
		os << cat << endl;
	}
}

void ID5Tree::load(istream &is) {
	load_vector(insts_here, is);
	load_vector(attrs_here, is);
	is >> split_attr;
	if (split_attr >= 0) {
		left.reset(new ID5Tree(insts));
		left->load(is);
		right.reset(new ID5Tree(insts));
		right->load(is);
	} else {
		is >> cat;
	}
	update_all_counts();
}

void ID5Tree::print_graphviz(ostream &os) const {
	stringstream lss;
	if (split_attr >= 0) {
		lss << split_attr;
	} else {
		lss << cat;
	}
	
	os << (int) this << " [label=\"" << lss.str() << "\"]" << endl;
	
	if (split_attr >= 0) {
		os << (int) this << " -> " << (int) left.get() << " [label=\"0\"];" << endl;
		left->print_graphviz(os);
		os << (int) this << " -> " << (int) right.get() << " [label=\"1\"];" << endl;
		right->print_graphviz(os);
	}
}
