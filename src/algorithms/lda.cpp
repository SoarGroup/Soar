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

LDA_NN_Classifier::LDA_NN_Classifier(const_mat_view data, const vector<int> &membership)
: membership(membership)
{
	int d = data.cols();
	vector<rvec> class_means;
	vector<int> counts, cmem, classes;
	
	// Remap classes to consecutive integers from 0
	for (int i = 0; i < membership.size(); ++i) {
		bool found = false;
		for (int j = 0; j < classes.size(); ++j) {
			if (classes[j] == membership[i]) {
				class_means[j] += data.row(i);
				++counts[j];
				cmem.push_back(j);
				found = true;
				break;
			}
		}
		if (!found) {
			classes.push_back(membership[i]);
			class_means.push_back(data.row(i));
			counts.push_back(1);
			cmem.push_back(classes.size() - 1);
		}
	}
	
	int C = classes.size();
	
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
	
	rvec all_mean = data.colwise().mean();
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
	projected = data * W;
}

int LDA_NN_Classifier::classify(const rvec &x) const {
	rvec p = x * W;
	int best;
	(projected.rowwise() - p).rowwise().squaredNorm().minCoeff(&best);
	return membership[best];
}
