#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <cstdio>
#include "dtree.h"
#include "common.h"

using namespace std;

double entropy(const map<category, int> &counts, int total) {
	double ent = 0.;
	map<category, int>::const_iterator i;
	for (i = counts.begin(); i != counts.end(); ++i) {
		if (i->second > 0) {
			double p = ((double) i->second) / total;
			ent -= p * log2(p);
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

vector<double> chi2thresh;

void fill_chi2thresh() {
	double pv;
	char *end;
	string pval = get_option("chi2_pvalue");
	
	if (pval.empty()) {
		pv = 0.1;
	} else {
		pv = strtod(pval.c_str(), &end);
		assert(end != pval.c_str());
	}
	
	// Use scipy to calculate threshold values of the chi2 CDF
	stringstream ss;
	ss << "python -c 'from scipy.stats import chi2" << endl;
	ss << "for i in range(10): print chi2.ppf(" << pv << ", i+1)'" << endl;
	FILE *input = popen(ss.str().c_str(), "r");
	assert(input != NULL);
	
	char c;
	vector<char> buf;
	int i = 0;
	while (fread(&c, 1, 1, input) > 0) {
		if (!isspace(c)) {
			buf.push_back(c);
		} else if (buf.size() > 0) {
			double x = strtod(&buf[0], &end);
			assert(end != &buf[0]);
			chi2thresh.push_back(x);
			buf.clear();
		}
	}
	pclose(input);
}

class chi2test {
public:
	chi2test() : ttl(0) {
		if (chi2thresh.size() == 0) {
			fill_chi2thresh();
		}
	}
	
	void add(int rlabel, int clabel) {
		int i, j;
		for (i = 0; i < row_labels.size() && row_labels[i] != rlabel; ++i) ;
		if (i == row_labels.size()) {
			row_labels.push_back(rlabel);
			row_ttls.push_back(0);
		}
		for (j = 0; j < col_labels.size() && col_labels[j] != clabel; ++j) ;
		if (j == col_labels.size()) {
			col_labels.push_back(clabel);
			col_ttls.push_back(0);
		}
		
		int oldrows = counts.rows(), oldcols = counts.cols();
		counts.conservativeResize(row_labels.size(), col_labels.size());
		for (int k = oldrows; k < counts.rows(); ++k) {
			counts.row(k).setConstant(0.0);
		}
		for (int k = oldcols; k < counts.cols(); ++k) {
			counts.col(k).setConstant(0.0);
		}
		
		counts(i, j) += 1.0;
		++row_ttls[i];
		++col_ttls[j];
		++ttl;
	}
	
	double score() const {
		mat expected(row_labels.size(), col_labels.size());
		for (int i = 0; i < row_labels.size(); ++i) {
			double rp = row_ttls[i] / (double) ttl;
			for (int j = 0; j < col_labels.size(); ++j) {
				expected(i, j) = col_ttls[j] * rp;
			}
		}
		double s = ((counts - expected).array().square() / expected.array()).sum();
		return s;
	}
	
	// Returns true if data is likely to be dependent
	bool test() const {
		int df = (row_ttls.size() - 1) * (col_ttls.size() - 1);
		assert(df < chi2thresh.size());
		return score() > chi2thresh[df];
	}
	
private:
	vector<int> row_labels, col_labels;
	vector<int> row_ttls, col_ttls;
	int ttl;
	mat counts;
};

int node_id_counter = 0;

/*
 insts is the list of all instances. The tree will only maintain
 a reference to it, so it can grow after the tree is created. The
 insts_here variable indexes into this list.
*/
ID5Tree::ID5Tree(const vector<classifier_inst> &insts, int nattrs) 
: id(node_id_counter++), insts(insts), split_attr(-1), cat(90909)
{
	for (int i = 0; i < nattrs; ++i) {
		attrs_here.push_back(i);
	}
}

ID5Tree::ID5Tree(const vector<classifier_inst> &insts, const vector<int> &attrs) 
: id(node_id_counter++), insts(insts), attrs_here(attrs), split_attr(-1), cat(90909)
{ }

void ID5Tree::expand() {
	assert(left.get() == NULL && right.get() == NULL && split_attr != -1 && attrs_here.size() > 0);
	vector<int> attrs, linsts, rinsts;
	remove_copy(attrs_here.begin(), attrs_here.end(), back_inserter(attrs), split_attr);
	assert(attrs.size() == attrs_here.size() - 1);

	left.reset(new ID5Tree(insts, attrs));
	right.reset(new ID5Tree(insts, attrs));
	vector<int>::const_iterator i;
	for (i = insts_here.begin(); i != insts_here.end(); ++i) {
		if (insts[*i].attrs[split_attr]) {
			linsts.push_back(*i);
		} else {
			rinsts.push_back(*i);
		}
	}
	left->batch_update(linsts);
	right->batch_update(rinsts);
}

void ID5Tree::update_tree(int i) {
	if (expanded() && (left->empty() || right->empty())) {
		shrink();
	}
	
	if (i >= 0) {
		assert(find(insts_here.begin(), insts_here.end(), i) == insts_here.end());
		insts_here.push_back(i);
		update_counts_new(i);
	}
	
	if (cats_all_same()) {
		cat = insts[insts_here[0]].cat;
		shrink();
	} else if (attrs_here.size() == 0 || attrs_all_same()) {
		cat = best_cat();
		shrink();
	} else {
		int best_split = choose_split();
		if (best_split == -1) {
			split_attr = -1;
			cat = best_cat();
			shrink();
		} else if (!expanded()) {
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

void ID5Tree::reset_counts() {
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
		update_counts_new(*i);
	}
}

/*
 Update counts after adding instance i.
*/
void ID5Tree::update_counts_new(int i) {
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
void ID5Tree::update_counts_change(int i, category old) {
	assert(find(insts_here.begin(), insts_here.end(), i) != insts_here.end());
	assert(is_unique(attrs_here));
	
	--ttl_counts[old];
	++ttl_counts[insts[i].cat];
	vector<int>::iterator j;
	for (j = attrs_here.begin(); j != attrs_here.end(); ++j) {
		val_counts &c = av_counts[*j];
		if (insts[i].attrs[*j]) {
			--c.true_counts[old];
			assert(c.true_counts[old] >= 0);
			++c.true_counts[insts[i].cat];
		} else {
			--c.false_counts[old];
			assert(c.false_counts[old] >= 0);
			++c.false_counts[insts[i].cat];
		}
	}
	
	if (expanded()) {
		if (insts[i].attrs[split_attr]) {
			left->update_counts_change(i, old);
		} else {
			right->update_counts_change(i, old);
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
		const classifier_inst &inst = insts[*i];
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
	double highgain = 0.0;
	int attr = -1;
	vector<int>::const_iterator i;
	for (i = attrs_here.begin(); i != attrs_here.end(); ++i) {
		if (gains[*i] > highgain) {
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
	int ninsts = insts_here.size();
	gains.clear();
	double curr_ent = entropy(ttl_counts, ninsts);
	vector<int>::const_iterator i;
	for (i = attrs_here.begin(); i != attrs_here.end(); ++i) {
		val_counts &c = av_counts[*i];
		double ptrue = c.ttl_true / (double) ninsts;
		double pfalse = c.ttl_false / (double) ninsts;
		double true_ent = entropy(c.true_counts, c.ttl_true);
		double false_ent = entropy(c.false_counts, c.ttl_false);
		gains[*i] = curr_ent - (ptrue * true_ent + pfalse * false_ent);
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

const ID5Tree *ID5Tree::get_matched_node(const attr_vec &attrs) const {
	if (!expanded()) {
		return this;
	}
	if (attrs[split_attr]) {
		return left->get_matched_node(attrs);
	}
	return right->get_matched_node(attrs);
}

void ID5Tree::get_all_splits(vector<int> &splits) const {
	if (split_attr < 0) {
		return;
	}
	splits.push_back(split_attr);
	left->get_all_splits(splits);
	right->get_all_splits(splits);
}

void ID5Tree::print_graphviz(ostream &os) const {
	stringstream lss;
	if (split_attr >= 0) {
		lss << split_attr;
	} else {
		lss << cat << " (";
		map<category, int>::const_iterator i;
		for (i = ttl_counts.begin(); i != ttl_counts.end(); ++i) {
			lss << i->first << ":" << i->second << ", ";
		}
		lss << ")";
	}
	
	os << (intptr_t) this << " [label=\"" << lss.str() << "\"]" << endl;
	
	if (split_attr >= 0) {
		os << (intptr_t) this << " -> " << (intptr_t) left.get() << " [label=\"1\"];" << endl;
		left->print_graphviz(os);
		os << (intptr_t) this << " -> " << (intptr_t) right.get() << " [label=\"0\"];" << endl;
		right->print_graphviz(os);
	}
}

void ID5Tree::print(const string &prefix, ostream &os) const {
	if (!expanded()) {
		os << " " << "N" << id << " [" << cat << " (";
		map<category, int>::const_iterator i;
		for (i = ttl_counts.begin(); i != ttl_counts.end(); ++i) {
			os << i->first << ":" << i->second << " ";
		}
		os << ") ]";
	} else {
		string newpref = prefix + "|   ";
		os << endl << prefix << "N" << id;
		os << endl << prefix << split_attr << " = 1:";
		left->print(newpref, os);
		os << endl << prefix << split_attr << " = 0:";
		right->print(newpref, os);
	}
}

bool ID5Tree::cli_inspect(int nid, ostream &os) const {
	if (id == nid) {
		map<category, int>::const_iterator i;
		for (i = ttl_counts.begin(); i != ttl_counts.end(); ++i) {
			os << "class " << i->first << endl;
			for (int j = 0; j < insts_here.size(); ++j) {
				if (insts[insts_here[j]].cat == i->first) {
					os << insts_here[j] << ' ';
				}
			}
			os << endl;
		}
		return true;
	}
	if (!expanded()) {
		return false;
	}
	if (left->cli_inspect(nid, os)) {
		return true;
	}
	return right->cli_inspect(nid, os);
}

void ID5Tree::batch_update(const vector<int> &new_insts) {
	insts_here = new_insts;
	reset_counts();
	if (insts_here.empty()) {
		return;
	}
	if (cats_all_same()) {
		cat = insts[insts_here[0]].cat;
	} else if (attrs_here.size() == 0 || attrs_all_same()) {
		cat = best_cat();
	} else {
		split_attr = choose_split();
		if (split_attr == -1) {
			cat = best_cat();
		} else {
			expand();
		}
	}
}

void ID5Tree::batch_update() {
	vector<int> all_insts;
	for (int i = 0; i < insts.size(); ++i) {
		all_insts.push_back(i);
	}
	batch_update(all_insts);
}

category ID5Tree::best_cat() {
	int c = -1;
	map<category, int>::const_iterator i;
	for (i = ttl_counts.begin(); i != ttl_counts.end(); ++i) {
		if (c == -1 || ttl_counts[c] < i->second) {
			c = i->first;
		}
	}
	return c;
}

void ID5Tree::prune() {
	chi2test chi;
	if (!expanded()) {
		return;
	}
	
	for (int i = 0; i < left->insts_here.size(); ++i) {
		chi.add(0, insts[left->insts_here[i]].cat);
	}
	for (int i = 0; i < right->insts_here.size(); ++i) {
		chi.add(1, insts[right->insts_here[i]].cat);
	}
	if (!chi.test()) {
		cat = best_cat();
		shrink();
	} else {
		left->prune();
		right->prune();
	}
}

void ID5Tree::get_instances(vector<int> &i) const {
	i = insts_here;
}

