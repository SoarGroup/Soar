#ifndef LWR_H
#define LWR_H

#include <vector>
#include <fstream>
#include "model.h"
#include "common.h"
#include "nn.h"

class lwr : public model {
public:
	lwr(int nnbrs, const std::string &logpath, bool test);
	~lwr();
	void learn(scene *scn, const floatvec &x, const floatvec &y, float dt);
	bool predict(const floatvec &x, floatvec &y);
	std::string get_type() const { return "lwr"; }
	int get_input_size() const { return xsize; }
	int get_output_size() const { return ysize; }

	bool load_file(const char *file);
	int size() const;
	
private:
	void normalize();
	
	int xsize, ysize, nnbrs;
	std::vector<std::pair<floatvec, floatvec> > examples;
	std::vector<floatvec> xnorm;
	floatvec xmin, xmax, xrange;
	bool normalized;
	nearest_neighbor *nn;
	std::ofstream *log;
	bool test;
};

#endif
