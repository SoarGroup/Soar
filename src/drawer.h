#ifndef DRAWER_H
#define DRAWER_H

#include <fstream>
#include <string>
#include "sgnode.h"

class drawer {
public:
	static int POS, ROT, SCALE, COLOR, VERTS;
	static std::ofstream fifo;
	
	drawer(const std::string &sname);
	~drawer();
	
	void set_pos(const vec3 &p);
	void set_rot(const vec3 &r);
	void set_scale(const vec3 &s);
	void set_transforms(sgnode *n);
	void set_color(float r, float g, float b);
	void set_vertices(const ptlist &v);
	void set_vertices(sgnode *n);
	void reset_properties();
	void add(const std::string &name);
	void add(sgnode *n);
	void del(const std::string &name);
	void del(sgnode *n);
	void change(const std::string &name, int props);
	
private:
	std::string scene_name;
	vec3 pos, rot, scl, color;
	ptlist verts;
};

#endif
