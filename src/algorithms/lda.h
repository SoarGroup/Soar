#ifndef LDA_H
#define LDA_H

#include <vector>
#include "mat.h"
#include "serializable.h"

class num_classifier : public serializable {
public:
	num_classifier() {}
	virtual ~num_classifier() {}
	
	/*
	 The data matrix will be modified! This is to avoid unnecessary copying.
	*/
	virtual void learn(mat &data, const std::vector<int> &classes) {}
	virtual int classify(const rvec &x) const { return 0; }
	virtual void inspect(std::ostream &os) const {}
	
	virtual void serialize(std::ostream &os) const {}
	virtual void unserialize(std::istream &is) {}
};

class LDA : public num_classifier {
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

class sign_classifier : public num_classifier {
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

#endif
