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
		VERTS = 1 << 5,
	};
	
	drawer();
	~drawer();
	
	void set_pos(const vec3 &p);
	void set_rot(const vec3 &r);
	void set_scale(const vec3 &s);
	void set_transforms(sgnode *n);
	void set_color(float r, float g, float b);
	void set_vertices(const ptlist &v);
	void set_vertices(sgnode *n);
	void reset_properties();
	void add(const std::string &scn, const std::string &name);
	void add(const std::string &scn, sgnode *n);
	void del(const std::string &scn, const std::string &name);
	void del(const std::string &scn, sgnode *n);
	void change(const std::string &scn, const std::string &name, int props);
	
private:
	std::string scene_name;
	vec3 pos, rot, scl, color;
	ptlist verts;
	ipcsocket sock;
};

#endif
