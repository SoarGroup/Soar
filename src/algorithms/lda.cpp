#include <vector>
#include <map>
#include "lda.h"
#include "nn.h"
#include "common.h"
#include "serialize.h"

using namespace std;
using namespace Eigen;

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

