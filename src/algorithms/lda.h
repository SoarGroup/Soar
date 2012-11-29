#ifndef LDA_H
#define LDA_H

#include <vector>
#include "mat.h"
#include "serializable.h"

class LDA : public serializable {
public:
	LDA();
	
	/*
	 The data matrix will be modified! This is to avoid unnecessary copying.
	*/
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


#endif
