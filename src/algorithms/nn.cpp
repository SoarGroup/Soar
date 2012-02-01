#include <assert.h>
#include <iostream>
#include <algorithm>
#include "nn.h"

using namespace std;

inline bool possibly_better(double d, int k, const di_queue &nn) {
	return nn.size() < k || (nn.size() > 0 && d < nn.top().first);
}

brute_nn::brute_nn(std::vector<evec> *points) : points(points) {}

void brute_nn::query(const evec &q, int k, di_queue &nn) {
	for (int i = 0; i < points->size(); ++i) {
		double d = (q - points->at(i)).squaredNorm();
		if (possibly_better(d, k, nn)) {
			nn.push(std::make_pair(d, i));
			if (nn.size() > k) {
				nn.pop();
			}
		}
	}
}


balltree::balltree(int ndim, int leafsize, vector<evec> *pts, const vector<int> &inds)
: ndim(ndim), leafsize(leafsize), pts(pts), inds(inds), left(NULL), right(NULL), parent(NULL), pruned(0)
{
	if (this->inds.size() == 0) {
		for (int i = 0; i < pts->size(); ++i) {
			this->inds.push_back(i);
		}
	}
	if (this->inds.size() > leafsize) {
		split();
	} else {
		update_ball();
	}
}

balltree::~balltree() {
	delete left;
	delete right;
}

void balltree::distsq_to(const evec &q, evec &dout) {
	vector<int>::const_iterator i;
	for (int i = 0; i < inds.size(); ++i) {
		dout[i] = (q - (*pts)[inds[i]]).squaredNorm();
	}
}

void balltree::update_ball() {
	if (left != NULL) {
		evec dir = (left->center - right->center).normalized();
		evec p1 = left->center + dir * left->radius;
		evec p2 = right->center - dir * right->radius;
		center = (p1 + p2) / 2.0;
		radius = (center - p1).norm();
	} else {
		vector<int>::const_iterator i;
		evec dists(inds.size());
		int furthest;
		center.setZero();
		for (i = inds.begin(); i != inds.end(); ++i) {
			center += (*pts)[*i];
		}
		center /= inds.size();
		distsq_to(center, dists);
		radius = sqrt(dists.maxCoeff());
	}
}

void balltree::split() {
	evec dc(inds.size()), dl(inds.size()), dr(inds.size());
	vector<int> linds, rinds;
	int i, li, ri;
	
	assert(left == NULL && right == NULL);
	
	update_ball();
	distsq_to(center, dc);
	dc.maxCoeff(&li);                  // furthest from center
	distsq_to((*pts)[inds[li]], dl);
	dl.maxCoeff(&ri);                  // furthest from li
	distsq_to((*pts)[inds[ri]], dr);
	for (i = 0; i < dl.size(); ++i) {
		if (dl[i] < dr[i]) {
			linds.push_back(inds[i]);
		} else {
			rinds.push_back(inds[i]);
		}
	}
	left = new balltree(ndim, leafsize, pts, linds);
	left->parent = this;
	right = new balltree(ndim, leafsize, pts, rinds);
	right->parent = this;
	update_ball();
}

void balltree::linear_scan(const evec &q, int k, di_queue &nn) {
	vector<int>::const_iterator i;
	double d;
	for (i = inds.begin(); i != inds.end(); ++i) {
		d = (q - (*pts)[*i]).squaredNorm();
		if (possibly_better(d, k, nn)) {
			nn.push(make_pair(d, *i));
			if (nn.size() > k) {
				nn.pop();
			}
		}
	}
}

void balltree::query(const evec &q, int k, di_queue &nn) {
	double dmin = pow(max((center - q).norm() - radius, 0.0), 2);
	if (nn.size() >= k && dmin >= nn.top().first) {
		pruned += inds.size();
		return;
	}
	
	if (inds.size() <= leafsize) { // right == NULL for sure
		linear_scan(q, k, nn);
		return;
	}
	
	left->query(q, k, nn);
	right->query(q, k, nn);
}

