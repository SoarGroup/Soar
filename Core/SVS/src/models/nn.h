#ifndef NN_H
#define NN_H

#include <vector>
#include <queue>
#include <utility>
#include "mat.h"

typedef std::pair<double, int> di_pair;
typedef std::priority_queue<di_pair> di_queue;

void brute_nearest_neighbor(const std::vector<rvec*>& data, const rvec& q, int k, std::vector<int>& indexes, rvec& dists);
void brute_nearest_neighbor(const_mat_view data, const rvec& q, int k, std::vector<int>& indexes, rvec& dists);

class balltree
{
    public:
        balltree(int ndim, int leafsize, std::vector<rvec>* pts, const std::vector<int>& inds);
        ~balltree();
        
        void query(const rvec& q, int k, di_queue& nn);
        
    private:
        void distsq_to(const rvec& q, rvec& dout);
        void update_ball();
        void split();
        void linear_scan(const rvec& q, int k, di_queue& nn);
        
        balltree* left, *right, *parent;
        rvec center;
        double radius;
        std::vector<int> inds;
        std::vector<rvec>* pts;
        int ndim;
        int leafsize;
        
        int pruned;
};

#endif
