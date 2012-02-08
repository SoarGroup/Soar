#ifndef NN_H
#define NN_H

#include <queue>
#include <vector>
#include "common.h"

typedef std::pair<double, int> di_pair;
typedef std::priority_queue<di_pair> di_queue;

class nearest_neighbor {
public:
	virtual void query(const rvec &q, int k, di_queue &nn) = 0;
};

class brute_nn : public nearest_neighbor {
public:
	brute_nn(std::vector<rvec> *points);
	void query(const rvec &q, int k, di_queue &nn);
	
private:
	std::vector<rvec> *points;
};

class balltree : public nearest_neighbor {
public:
	balltree(int ndim, int leafsize, std::vector<rvec> *pts, const std::vector<int> &inds);
	~balltree();
	
	void query(const rvec &q, int k, di_queue &nn);
	
private:
	void distsq_to(const rvec &q, rvec &dout);
	void update_ball();
	void split();
	void linear_scan(const rvec &q, int k, di_queue &nn);
	
	balltree *left, *right, *parent;
	rvec center;
	double radius;
	std::vector<int> inds;
	std::vector<rvec> *pts;
	int ndim;
	int leafsize;
	
	int pruned;
};

#endif
