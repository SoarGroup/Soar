#include <vector>
#include <map>
#include "lda.h"
#include "nn.h"
#include "common.h"

using namespace std;
using namespace Eigen;

bool hasnan(const_mat_view m) {
	return (m.array() != m.array()).any();
}

void clean_data(mat &data, vector<int> &nonstatic_cols) {
	del_static_cols(data, data.cols(), nonstatic_cols);
	data.conservativeResize(data.rows(), nonstatic_cols.size());
	mat rand_offsets = mat::Random(data.rows(), data.cols()) / 10000;
	data += rand_offsets;
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

LDA_NN_Classifier::LDA_NN_Classifier(const_mat_view data, const vector<int> &used_rows, const vector<int> &classes)
: classes(classes), degenerate(false), degenerate_class(9090909)
{
	mat cleaned;
	pick_rows(data, used_rows, cleaned);
	clean_data(cleaned, used_cols);
	
	if (cleaned.cols() == 0) {
		LOG(WARN) << "Degenerate case, no useful classification data." << endl;
		degenerate = true;
		degenerate_class = largest_class(classes);
		return;
	}
	int d = cleaned.cols();
	vector<rvec> class_means;
	vector<int> counts, cmem, norm_classes;
	
	// Remap classes to consecutive integers from 0
	for (int i = 0; i < classes.size(); ++i) {
		bool found = false;
		for (int j = 0; j < norm_classes.size(); ++j) {
			if (norm_classes[j] == classes[i]) {
				class_means[j] += cleaned.row(i);
				++counts[j];
				cmem.push_back(j);
				found = true;
				break;
			}
		}
		if (!found) {
			norm_classes.push_back(classes[i]);
			class_means.push_back(cleaned.row(i));
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
		projected = cleaned;
		return;
	}
	
	for (int i = 0; i < C; ++i) {
		class_means[i] /= counts[i];
	}
	
	mat Sw = mat::Zero(d, d);
	for (int i = 0; i < cleaned.rows(); ++i) {
		rvec x = cleaned.row(i) - class_means[cmem[i]];
		Sw += x.transpose() * x;
	}
	
	rvec all_mean = cleaned.colwise().mean();
	mat Sb = mat::Zero(d, d);
	for (int i = 0; i < C; ++i) {
		rvec x = class_means[i] - all_mean;
		Sb += counts[i] * (x.transpose() * x);
	}
	
	mat S = Sw.inverse() * Sb;
	assert(!hasnan(S));
	
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
	projected = cleaned * W;
}

int LDA_NN_Classifier::classify(const rvec &x) const {
	if (degenerate) {
		return degenerate_class;
	}
	rvec p;
	if (used_cols.size() < x.size()) {
		rvec x1(used_cols.size());
		for (int i = 0; i < used_cols.size(); ++i) {
			x1(i) = x(used_cols[i]);
		}
		p = x1 * W;
	} else {
		p = x * W;
	}
	int best;
	(projected.rowwise() - p).rowwise().squaredNorm().minCoeff(&best);
	return classes[best];
}
