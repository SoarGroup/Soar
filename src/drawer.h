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
	
	void set_address(const std::string &addr);
	void add(const std::string &scn, const sgnode *n);
	void del(const std::string &scn, const sgnode *n);
	void change(const std::string &scn, const sgnode *n, int props);
	void delete_scene(const std::string &scn);
	void send(const std::string &s);
	
	// these are all hacks
	void set_color(const std::string &name, double r, double g, double b);
	void set_pos(const std::string &name, double x, double y, double z);
	
private:
	bool error;
	ipcsocket sock;
	
	drawer();
	friend drawer *get_drawer();
};

drawer *get_drawer();

#endif
