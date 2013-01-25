#include <vector>
#include <map>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>
#include "lda.h"
#include "nn.h"
#include "common.h"
#include "serialize.h"

using namespace std;
using namespace Eigen;

class nc_cls : public serializable {
public:
	virtual ~nc_cls() {}
	virtual void learn(mat &data, const std::vector<int> &classes) = 0;
	virtual int classify(const rvec &x) const = 0;
	virtual void inspect(std::ostream &os) const = 0;
	virtual void serialize(std::ostream &os) const = 0;
	virtual void unserialize(std::istream &is) = 0;
};

class LDA : public nc_cls {
public:
	LDA();
	
	void learn(mat &data, const std::vector<int> &classes);
	int classify(const rvec &x) const;
	bool project(const rvec &x, rvec &p) const;
	void inspect(std::ostream &os) const;
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:
	mat W, projected;
	rvec J;
	std::vector<int> classes, used_cols;
	bool degenerate;
	int degenerate_class;
};

class sign_classifier : public nc_cls {
public:
	sign_classifier();
	void learn(mat &data, const std::vector<int> &classes);
	int classify(const rvec &x) const;
	void inspect(std::ostream &os) const;
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);

private:
	int dim, sgn;
};

class dtree_classifier : public nc_cls {
public:
	dtree_classifier();
	void learn(mat &data, const std::vector<int> &classes);
	int classify(const rvec &x) const;
	void inspect(std::ostream &os) const;
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);

private:
	CvDTree *tree;
};

bool hasnan(const_mat_view m) {
	return (m.array() != m.array()).any();
}

#define BETA 1.0e-10

void clean_data(mat &data, vector<int> &nonuniform_cols) {
	del_uniform_cols(data, data.cols(), nonuniform_cols);
	data.conservativeResize(data.rows(), nonuniform_cols.size());
	/*
	 I used to add a small random offset to each element to try to fix numerical
	 stability. Now I add a constant BETA to the Sw matrix instead.

	mat rand_offsets = mat::Random(data.rows(), data.cols()) / 10000;
	data += rand_offsets;
	*/
}

int largest_class(const vector<int> &c) {
	map<int, int> counts;
	for (int i = 0; i < c.size(); ++i) {
		++counts[c[i]];
	}
	map<int, int>::iterator i;
	int largest_count = -1;
	int largest = -1;
	vector<int> used_cols;
	for (i = counts.begin(); i != counts.end(); ++i) {
		if (largest_count < i->second) {
			largest = i->first;
		}
	}
	return largest;
}

int get_num_classifier_type(const std::string &t) {
	if (t == "dtree") {
		return NC_DTREE;
	} else if (t == "lda") {
		return NC_LDA;
	} else if (t == "sign") {
		return NC_SIGN;
	}
	return NC_NONE;
}

string get_num_classifier_name(int t) {
	switch (t) {
	case NC_DTREE:
		return "dtree";
	case NC_LDA:
		return "lda";
	case NC_SIGN:
		return "sign";
	}
	return "";
}

num_classifier::num_classifier() : nc_type(NC_NONE), cls(NULL) {}

num_classifier::num_classifier(int t) : cls(NULL) {
	set_type(t);
}

num_classifier::~num_classifier() {
	delete cls;
}

void num_classifier::set_type(int t) {
	if (cls) {
		delete cls;
	}
	nc_type = t;
	switch (nc_type) {
	case NC_DTREE:
		cls = new dtree_classifier;
		break;
	case NC_LDA:
		cls = new LDA;
		break;
	case NC_SIGN:
		cls = new sign_classifier;
		break;
	default:
		assert(false);
	}
}

void num_classifier::learn(mat &data, const vector<int> &classes) {
	cls->learn(data, classes);
}

int num_classifier::classify(const rvec &x) const {
	return cls->classify(x);
}

void num_classifier::inspect(ostream &os) const {
	cls->inspect(os);
}

void num_classifier::serialize(ostream &os) const {
	serializer(os) << nc_type;
	cls->serialize(os);
}

void num_classifier::unserialize(istream &is) {
	unserializer(is) >> nc_type;
	set_type(nc_type);
	switch (nc_type) {
	case NC_DTREE:
		dynamic_cast<dtree_classifier*>(cls)->unserialize(is);
		break;
	case NC_LDA:
		dynamic_cast<LDA*>(cls)->unserialize(is);
		break;
	case NC_SIGN:
		dynamic_cast<sign_classifier*>(cls)->unserialize(is);
		break;
	default:
		assert(false);
	}
}

LDA::LDA() : degenerate(false), degenerate_class(9090909)
{}

void LDA::learn(mat &data, const vector<int> &cls) {
	classes = cls;
	clean_data(data, used_cols);
	
	if (data.cols() == 0) {
		LOG(WARN) << "Degenerate case, no useful classification data." << endl;
		degenerate = true;
		degenerate_class = largest_class(classes);
		return;
	}
	int d = data.cols();
	vector<rvec> class_means;
	vector<int> counts, cmem, norm_classes;
	
	// Remap classes to consecutive integers from 0
	for (int i = 0; i < classes.size(); ++i) {
		bool found = false;
		for (int j = 0; j < norm_classes.size(); ++j) {
			if (norm_classes[j] == classes[i]) {
				class_means[j] += data.row(i);
				++counts[j];
				cmem.push_back(j);
				found = true;
				break;
			}
		}
		if (!found) {
			norm_classes.push_back(classes[i]);
			class_means.push_back(data.row(i));
			counts.push_back(1);
			cmem.push_back(norm_classes.size() - 1);
		}
	}
	
	int C = norm_classes.size();
	
	/*
	 Degenerate case: fewer nonuniform dimensions in source
	 data than there are classes. Don't try to project. This then
	 becomes an ordinary NN classifier.
	*/
	if (d < C - 1) {
		W = mat::Identity(d, d);
		projected = data;
		return;
	}
	
	for (int i = 0; i < C; ++i) {
		class_means[i] /= counts[i];
	}
	
	mat Sw = mat::Zero(d, d);
	for (int i = 0; i < data.rows(); ++i) {
		rvec x = data.row(i) - class_means[cmem[i]];
		Sw += x.transpose() * x;
	}
	
	Sw.diagonal().array() += BETA;
	rvec all_mean = data.colwise().mean();
	mat Sb = mat::Zero(d, d);
	for (int i = 0; i < C; ++i) {
		rvec x = class_means[i] - all_mean;
		Sb += counts[i] * (x.transpose() * x);
	}
	
	mat Swi = Sw.inverse();
	mat S = Swi * Sb;
	assert(!hasnan(S));
	
	/*
	ofstream tmp("/tmp/swsb.ser"), tmp2("/tmp/swi.ser"), tmp3("/tmp/S.ser");
	::serialize(Sw, tmp);
	::serialize(Sb, tmp);
	tmp.close();
	::serialize(Swi, tmp2);
	tmp2.close();
	::serialize(S, tmp3);
	tmp3.close();
	*/
	
	EigenSolver<mat> e;
	e.compute(S);
	VectorXcd eigenvals = e.eigenvalues();
	MatrixXcd eigenvecs = e.eigenvectors();

	W.resize(d, C - 1);
	J.resize(C - 1);
	for (int i = 0; i < C - 1; ++i) {
		int best = -1;
		for (int j = 0; j < eigenvals.size(); ++j) {
			if (best < 0 || eigenvals(j).real() > eigenvals(best).real()) {
				best = j;
			}
		}
		J(i) = eigenvals(best).real();
		for (int j = 0; j < d; ++j) {
			W(j, i) = eigenvecs(j, best).real();
		}
		eigenvals(best) = complex<double>(-1, 0);
	}
	projected = data * W;
}

int LDA::classify(const rvec &x) const {
	rvec p;
	int best;
	
	if (!project(x, p))
		return degenerate_class;
	
	(projected.rowwise() - p).rowwise().squaredNorm().minCoeff(&best);
	return classes[best];
}

bool LDA::project(const rvec &x, rvec &p) const {
	if (degenerate)
		return false;
	
	if (used_cols.size() < x.size()) {
		rvec x1(used_cols.size());
		for (int i = 0; i < used_cols.size(); ++i) {
			x1(i) = x(used_cols[i]);
		}
		p = x1 * W;
	} else {
		p = x * W;
	}
	return true;
}

void LDA::inspect(ostream &os) const {
	if (degenerate) {
		os << "degenerate (" << degenerate_class << ")" << endl;
		return;
	}
	table_printer t;
	for (int i = 0; i < W.rows(); ++i) {
		t.add_row() << used_cols[i];
		for (int j = 0; j < W.cols(); ++j) {
			t << W(i, j);
		}
	}
	t.print(os);
}

void LDA::serialize(ostream &os) const {
	serializer(os) << W << projected << J << classes << used_cols << degenerate << degenerate_class;
}

void LDA::unserialize(istream &is) {
	unserializer(is) >> W >> projected >> J >> classes >> used_cols >> degenerate >> degenerate_class;
}

inline int sign(double x) { return x >= 0 ? 1 : -1; }

sign_classifier::sign_classifier() : dim(-1), sgn(0) {}

void sign_classifier::learn(mat &data, const vector<int> &classes) {
	cvec cls2(classes.size());
	for (int i = 0, iend = cls2.size(); i < iend; ++i) {
		cls2(i) = (classes[i] == 0 ? -1 : 1);
	}
	
	for (int i = 0, iend = data.rows(); i < iend; ++i) {
		for (int j = 0, jend = data.cols(); j < jend; ++j) {
			data(i, j) = sign(data(i, j));
		}
	}
	
	double best;
	dim = -1;
	for (int i = 0, iend = data.cols(); i < iend; ++i) {
		double dp = cls2.dot(data.col(i));
		if (dim == -1 || fabs(dp) > best) {
			dim = i;
			best = fabs(dp);
			sgn = sign(dp);
		}
	}
}

int sign_classifier::classify(const rvec &x) const {
	return max(sgn * sign(x(dim)), 0);
}

void sign_classifier::inspect(std::ostream &os) const {
	os << "dim: " << dim << " sign: " << sgn << endl;
}

void sign_classifier::serialize(ostream &os) const {
	serializer(os) << dim << sgn;
}

void sign_classifier::unserialize(istream &is) {
	unserializer(is) >> dim >> sgn;
}

dtree_classifier::dtree_classifier() {
	tree = new CvDTree();
}

void dtree_classifier::learn(mat &data, const vector<int> &classes) {
	static cv::Mat empty;
	static CvDTreeParams params;
	params.cv_folds = 10;
	params.min_sample_count = 10;
	
	cv::Mat cvdata(data.rows(), data.cols(), CV_64F, data.data()), cvfloatdata(data.rows(), data.cols(), CV_32F);
	cv::Mat cvclasses(classes);
	
	cvdata.convertTo(cvfloatdata, CV_32F);
	tree->train(cvfloatdata, CV_ROW_SAMPLE, cvclasses, empty, empty, empty, empty, params);
}

int dtree_classifier::classify(const rvec &x) const {
	cv::Mat cvx(1, x.size(), CV_64F, const_cast<double*>(x.data()));
	cv::Mat cvfloatx(1, x.size(), CV_32F);
	
	cvx.convertTo(cvfloatx, CV_32F);
	return tree->predict(cvfloatx)->value;
}

void print_tree(const CvDTreeNode *n, ostream &os) {
	const CvDTreeSplit *s = n->split;
	for (int i = 0; i < n->depth; ++i) {
		os << ' ';
	}
	os << s->var_idx << " <  " << s->ord.c;
	if (!n->left->left && !n->left->right) {
		os << " : " << n->left->value << endl;
	} else {
		os << endl;
		print_tree(n->left, os);
	}
	for (int i = 0; i < n->depth; ++i) {
		os << ' ';
	}
	os << s->var_idx << " >= " << s->ord.c;
	if (!n->right->left && !n->right->right) {
		os << " : " << n->right->value << endl;
	} else {
		os << endl;
		print_tree(n->right, os);
	}
}

void dtree_classifier::inspect(ostream &os) const {
	if (!tree->get_root()->split) {
		os << "constant: " << tree->get_root()->value << endl;
	} else {
		print_tree(tree->get_root(), os);
	}
}

void dtree_classifier::serialize(ostream &os) const {
	char temp[] = "dtree_serialize_XXXXXX";
	string line;
	
	mktemp(temp);
	assert(*temp != '\0');
	tree->save(temp);
	ifstream input(temp);
	
	os << "BEGIN_DTREE_CLASSIFIER" << endl;
	while (getline(input, line)) {
		os << line << endl;
	}
	os << endl << "END_DTREE_CLASSIFIER" << endl;
	unlink(temp);
}

void dtree_classifier::unserialize(std::istream &is) {
	char temp[] = "dtree_unserialize_XXXXXX";
	string line;
	
	mktemp(temp);
	assert(*temp != '\0');
	ofstream out(temp);
	
	while (getline(is, line) && line.empty())
		;
	assert(line == "BEGIN_DTREE_CLASSIFIER");
	while (getline(is, line)) {
		if (line == "END_DTREE_CLASSIFIER") {
			break;
		}
		out << line << endl;
	}
	assert(line == "END_DTREE_CLASSIFIER");
	out.close();
	
	tree->load(temp);
	unlink(temp);
}
