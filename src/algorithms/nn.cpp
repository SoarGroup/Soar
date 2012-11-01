#include <assert.h>
#include <iostream>
#include <algorithm>
#include <queue>
#include "nn.h"

using namespace std;

inline bool possibly_better(double d, int k, const di_queue &nn) {
	return nn.size() < k || (nn.size() > 0 && d < nn.top().first);
}

void di_queue_to_vec(di_queue &q, vector<int> &indexes, rvec &dists) {
	indexes.reserve(q.size());
	dists.resize(q.size());
	for (int i = 0; i < dists.size(); ++i) {
		dists(i) = q.top().first;
		indexes.push_back(q.top().second);
		q.pop();
	}
}

void brute_nearest_neighbor(const_mat_view data, const rvec &q, int k, vector<int> &indexes, rvec &dists) {
	di_queue nn;
	for (int i = 0; i < data.rows(); ++i) {
		double d = (q - data.row(i)).squaredNorm();
		if (possibly_better(d, k, nn)) {
			nn.push(std::make_pair(d, i));
			if (nn.size() > k) {
				nn.pop();
			}
		}
	}
	di_queue_to_vec(nn, indexes, dists);
}

void brute_nearest_neighbor(const vector<rvec*> &data, const rvec &q, int k, vector<int> &indexes, rvec &dists) {
	di_queue nn;
	for (int i = 0; i < data.size(); ++i) {
		double d = (q - *data[i]).squaredNorm();
		if (possibly_better(d, k, nn)) {
			nn.push(std::make_pair(d, i));
			if (nn.size() > k) {
				nn.pop();
			}
		}
	}
	di_queue_to_vec(nn, indexes, dists);
}

balltree::balltree(int ndim, int leafsize, vector<rvec> *pts, const vector<int> &inds)
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

void balltree::distsq_to(const rvec &q, rvec &dout) {
	vector<int>::const_iterator i;
	for (int i = 0; i < inds.size(); ++i) {
		dout[i] = (q - (*pts)[inds[i]]).squaredNorm();
	}
}

void balltree::update_ball() {
	if (left != NULL) {
		rvec dir = (left->center - right->center).normalized();
		rvec p1 = left->center + dir * left->radius;
		rvec p2 = right->center - dir * right->radius;
		center = (p1 + p2) / 2.0;
		radius = (center - p1).norm();
	} else {
		vector<int>::const_iterator i;
		rvec dists(inds.size());
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
	rvec dc(inds.size()), dl(inds.size()), dr(inds.size());
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

void balltree::linear_scan(const rvec &q, int k, di_queue &nn) {
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

void balltree::query(const rvec &q, int k, di_queue &nn) {
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

