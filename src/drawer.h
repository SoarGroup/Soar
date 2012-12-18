#ifndef DRAWER_H
#define DRAWER_H

#include <string>
#include "sgnode.h"
#include "ipcsocket.h"

class drawer {
public:
	enum change_type { 
		POS   = 1, 
		ROT   = 1 << 2, 
		SCALE = 1 << 3, 
		COLOR = 1 << 4, 
		SHAPE = 1 << 5,
	};
	
	drawer();
	~drawer();
	
	void add(const std::string &scn, const sgnode *n);
	void del(const std::string &scn, const sgnode *n);
	void change(const std::string &scn, const sgnode *n, int props);
	void delete_scene(const std::string &scn);
	void set_color(const std::string &name, double r, double g, double b);
	
private:
	ipcsocket sock;
};

#endif
