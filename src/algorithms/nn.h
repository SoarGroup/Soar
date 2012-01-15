#ifndef NN_H
#define NN_H

#include <queue>
#include <vector>
#include "common.h"

typedef std::pair<double, int> di_pair;
typedef std::priority_queue<di_pair> di_queue;

class nearest_neighbor {
public:
	virtual void query(const floatvec &q, int k, di_queue &nn) = 0;
};

class brute_nn : public nearest_neighbor {
public:
	brute_nn(std::vector<floatvec> *points);
	void query(const floatvec &q, int k, di_queue &nn);
	
private:
	std::vector<floatvec> *points;
};

class balltree : public nearest_neighbor {
public:
	balltree(int ndim, int leafsize, std::vector<floatvec> *pts, const std::vector<int> &inds);
	~balltree();
	
	void query(const floatvec &q, int k, di_queue &nn);
	
private:
	void distsq_to(const floatvec &q, floatvec &dout);
	void update_ball();
	void split();
	void linear_scan(const floatvec &q, int k, di_queue &nn);
	
	balltree *left, *right, *parent;
	floatvec center;
	double radius;
	std::vector<int> inds;
	std::vector<floatvec> *pts;
	int ndim;
	int leafsize;
	
	int pruned;
};

#endif
