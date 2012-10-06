#ifndef LDA_H
#define LDA_H

#include <vector>
#include "mat.h"

class LDA {
public:
	/*
	 The data matrix will be modified! This is to avoid unnecessary copying.
	*/
	LDA(mat &data, const std::vector<int> &classes);
	int classify(const rvec &x) const;
	
	const mat &getW() const {
		return W;
	}
	
	const rvec &getJ() const {
		return J;
	}
	
private:
	mat W, projected;
	rvec J;
	std::vector<int> classes, used_cols;
	bool degenerate;
	int degenerate_class;
};


#endif
