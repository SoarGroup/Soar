#ifndef LDA_H
#define LDA_H

#include <vector>
#include "common.h"

class LDA_NN_Classifier {
public:
	LDA_NN_Classifier(const_mat_view data, const std::vector<int> &membership);
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
	std::vector<int> membership;
};


#endif
