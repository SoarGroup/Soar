#ifndef LDA_H
#define LDA_H

#include <vector>
#include "mat.h"
#include "serializable.h"
#include "timer.h"

class nc_cls;

enum { NC_NONE, NC_DTREE, NC_LDA, NC_SIGN };

int get_num_classifier_type(const std::string &t);
std::string get_num_classifier_name(int t);

class num_classifier : public serializable {
public:
	num_classifier();
	num_classifier(const num_classifier &c);
	num_classifier(int t);
	~num_classifier();
	
	num_classifier &operator=(const num_classifier &c);
	
	void set_type(int t);
	
	/*
	 The data matrix will be modified! This is to avoid unnecessary copying.
	*/
	void learn(mat &data, const std::vector<int> &classes);
	int classify(const rvec &x) const;
	void inspect(std::ostream &os) const;
	
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);

private:
	int nc_type;
	nc_cls *cls;
	
	mutable timer_set timers;
};


#endif
