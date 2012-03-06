#ifndef LWR_H
#define LWR_H

#include <iostream>
#include "nn.h"
#include "common.h"

class LWR {
public:
	LWR(int nnbrs);
	
	void learn(const rvec &x, const rvec &y);
	bool predict(const rvec &x, rvec &y);
	void load(std::istream &is);
	void save(std::ostream &os) const;
	
	int size() const { return examples.size(); }
	int xsize() const { return xsz; }
	int ysize() const { return ysz; }
	
private:
	void normalize();
	
	int xsz, ysz, nnbrs;
	std::vector<std::pair<rvec, rvec> > examples;
	std::vector<rvec> xnorm;
	rvec xmin, xmax, xrange;
	bool normalized;
	nearest_neighbor *nn;
};

#endif
