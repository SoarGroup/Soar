#ifndef LWR_H
#define LWR_H

#include <iostream>
#include "mat.h"
#include "serializable.h"

class LWR : public serializable
{
    public:
        LWR(int nnbrs, double noise_var, bool alloc);
        ~LWR();
        
        void learn(const rvec& x, const rvec& y);
        bool predict(const rvec& x, rvec& y);
        void load(std::istream& is);
        void save(std::ostream& os) const;
        
        int size() const
        {
            return examples.size();
        }
        int xsize() const
        {
            return xsz;
        }
        int ysize() const
        {
            return ysz;
        }
        
        void serialize(std::ostream& os) const;
        void unserialize(std::istream& is);
        
    private:
        void normalize();
        
        class data : public serializable
        {
            public:
                rvec const* x;
                rvec const* y;
                rvec xnorm;
                
                void serialize(std::ostream& os) const;
                void unserialize(std::istream& is);
        };
        
        int xsz, ysz, nnbrs;
        std::vector<data*> examples;
        std::vector<rvec*> xnptrs;
        rvec xmin, xmax, xrange;
        bool normalized, alloc;
        double noise_var;
};

#endif
