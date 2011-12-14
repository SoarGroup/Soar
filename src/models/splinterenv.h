/*
 The main class is splinterenv, which can both be used as a model in SVS
 as well as an environment simulator. The idea is to have the same code
 for both so that SVS has a guaranteed perfect model of the environment.
*/

#ifndef SPLINTERENV_H
#define SPLINTERENV_H

#include <iostream>
#include <sstream>
#include <string>
#include <chipmunk/chipmunk.h>
#include "splinter.h"
#include "common.h"

class box {
public:
	box(cpSpace *spc, const std::string &name, float px, float py, float rz, float mass, float width, float length)
	: space(spc), name(name)
	{
		body = cpBodyNew(mass, cpMomentForBox(mass, width, length));
		shape = cpBoxShapeNew(body, width, length);
		cpBodySetPos(body, cpv(px, py));
		cpSpaceAddBody(spc, body);
		cpSpaceAddShape(spc, shape);
		
		const char *attribs[] = {":px", ":py", ":vx", ":vy", ":fx", ":fy", ":rz", ":av", ":t" };
		int len = sizeof(attribs) / sizeof(char*);
		for (int i = 0; i < len; ++i) {
			slots.push_back(name + attribs[i]);
		}
	}

	~box() {
		cpSpaceRemoveBody(space, body);
		cpSpaceRemoveShape(space, shape);
		cpShapeFree(shape);
		cpBodyFree(body);
	}
	
	std::string get_name() {
		return name;
	}
	
	virtual void update(float lvolt, float rvolt) {
		cpVect v = cpBodyGetVel(body);
		v.x *= 0.5;
		v.y *= 0.5;
		cpBodySetVel(body, v);
		cpBodySetAngVel(body, cpBodyGetAngVel(body) * 0.5);
	}
	
	virtual void get_sgel(std::ostream &os, bool first) {
		cpVect pos = cpBodyGetPos(body);
		cpVect vel = cpBodyGetVel(body);
		cpVect force = cpBodyGetForce(body);
		
		if (first) {
			std::stringstream vs1, vs2;
			int n = cpPolyShapeGetNumVerts(shape);
			for (int i = 0; i < n; ++i) {
				cpVect v = cpPolyShapeGetVert(shape, i);
				vs1 << v.x << " " << v.y << " -.5 ";
				vs2 << v.x << " " << v.y << " .5 ";
			}
			os << "a " << name << " world v " << vs1.str() + vs2.str();
		} else {
			os << "c " << name;
		}
		os << " p " << pos.x << " " << pos.y << " 0.0";
		os << " r 0.0 0.0 " << cpBodyGetAngle(body) << std::endl;
		
		os << "p " << name << " vx " << vel.x << std::endl;
		os << "p " << name << " vy " << vel.y << std::endl;
		os << "p " << name << " fx " << force.x << std::endl;
		os << "p " << name << " fy " << force.y << std::endl;
		os << "p " << name << " av " << cpBodyGetAngVel(body) << std::endl;
		os << "p " << name << " t " << cpBodyGetTorque(body) << std::endl;
	}
	
	virtual void get_disp(std::ostream &os) {
		int n = cpPolyShapeGetNumVerts(shape);
		os << name << std::endl;
		for (int i = 0; i < n; ++i) {
			cpVect p = cpBodyLocal2World(body, cpPolyShapeGetVert(shape, i));
			os << p.x << " " << p.y << std::endl;
		}
		os << "end" << std::endl;
	}
	
	virtual bool set_state(const floatvec &x, int offset) {
		if (offset + size() > x.size()) {
			return false;
		}
		cpBodySetPos(   body, cpv(x[offset],   x[offset+1]));
		cpBodySetVel(   body, cpv(x[offset+2], x[offset+3]));
		cpBodySetForce( body, cpv(x[offset+4], x[offset+5]));
		cpBodySetAngle( body, x[offset+6]);
		cpBodySetAngVel(body, x[offset+7]);
		cpBodySetTorque(body, x[offset+8]);
		return true;
	}
	
	void append_state(std::vector<float> &s) const {
		cpVect pos = cpBodyGetPos(body), vel = cpBodyGetVel(body), force = cpBodyGetForce(body);
		float a = cpBodyGetAngle(body), av = cpBodyGetAngVel(body), t = cpBodyGetTorque(body);
		s.push_back(pos.x);
		s.push_back(pos.y);
		s.push_back(vel.x);
		s.push_back(vel.y);
		s.push_back(force.x);
		s.push_back(force.y);
		s.push_back(a);
		s.push_back(av);
		s.push_back(t);
	}
	
	void append_slots(std::vector<std::string> &s) const {
		std::copy(slots.begin(), slots.end(), std::back_inserter(s));
	}
	
	virtual int size() const {
		return slots.size();
	}
	
protected:
	std::vector<std::string> slots;
	std::string name, vertstr;
	cpSpace *space;
	cpBody *body;
	cpShape *shape;
};

class splinter : public box {
public:
	splinter(cpSpace *spc, const std::string &name, float px, float py, float rz)
	: box(spc, name, px, py, rz, 1.0, 1.0, 2.0), lrps(0.), rrps(0.)
	{}
	
	void get_sgel(std::ostream &os, bool first) {
		box::get_sgel(os, first);
		cpVect vel = cpBodyGetVel(body);
		
		// dynamic props
		os << "p " << name << " lrps "  << lrps << std::endl;
		os << "p " << name << " rrps " << rrps << std::endl;
	}
	
	bool set_state(const floatvec &x, int offset) {
		if (offset + size() > x.size()) {
			return false;
		}
		box::set_state(x, offset);
		int offset2 = offset + box::size();
		lrps = x[offset2];
		rrps = x[offset2 + 1];
		return true;
	}
	
	void append_state(std::vector<float> &s) {
		box::append_state(s);
		s.push_back(lrps);
		s.push_back(rrps);
	}
	
	void update(float lvolt, float rvolt) {
		cpVect pos = cpBodyGetPos(body);
		cpVect vel = cpBodyGetVel(body);
		float px = pos.x, py = pos.y, vx = vel.x, vy = vel.y;
		float rz = cpBodyGetAngle(body), rvz = cpBodyGetAngVel(body);
		
		splinter_update(px, py, vx, vy, rz, rvz, lrps, rrps, lvolt, rvolt);
		
		cpBodySetVel(body, cpv(vx, vy));
		cpBodySetAngVel(body, rvz);
	}
	
	void append_slots(std::vector<std::string> &s) const {
		box::append_slots(s);
		s.push_back(name + ":lrps");
		s.push_back(name + ":rrps");
	}
	
	int size() const {
		return box::size() + 2;
	}
	
private:
	float lrps, rrps;
};

class splinterenv {
public:
	splinterenv() : first(true) {
		space = cpSpaceNew();
		splntr = new splinter(space, "splinter", 0.0, 0.0, 0.0);
		add_block("box1", 1.3, 1.3, 0.0, 2.0, 1.0, 1.0);
	}
	
	~splinterenv() {
		std::vector<box*>::iterator i;
		delete splntr;
		for (i = blocks.begin(); i != blocks.end(); ++i) {
			delete *i;
		}
		cpSpaceFree(space);
	}
	
	void step(float lvolt, float rvolt) {
		std::vector<box*>::iterator i;
		splntr->update(lvolt, rvolt);
		for (i = blocks.begin(); i != blocks.end(); ++i) {
			(**i).update(lvolt, rvolt);
		}
		for (int j = 0; j < 100; ++j) {
			cpSpaceStep(space, 1. / 100.);
		}
	}
	
	void get_disp(std::ostream &os) {
		std::vector<box*>::iterator i;
		splntr->get_disp(os);
		for (i = blocks.begin(); i != blocks.end(); ++i) {
			(**i).get_disp(os);
		}
	}
	
	void get_sgel(std::ostream &os) {
		std::vector<box*>::iterator i;
		splntr->get_sgel(os, first);
		for (i = blocks.begin(); i != blocks.end(); ++i) {
			(**i).get_sgel(os, first);
		}
		first = false;
	}
	
	void get_flat_state(floatvec &x) {
		std::vector<box*>::iterator i;
		std::vector<float> p;
		
		splntr->append_state(p);
		for (i = blocks.begin(); i != blocks.end(); ++i) {
			(**i).append_state(p);
		}
		x = p;
	}
	
	bool predict(const floatvec &x, floatvec &y) {
		int len, offset;
		std::vector<box*>::iterator i;
		
		if (blocks.size() == 0) {
			len = 2 + splntr->size();
		} else {
			len = 2 + splntr->size() + blocks.size() * blocks[0]->size();
		}
		if (x.size() != len) {
			return false;
		}
		
		if (!splntr->set_state(x, 2)) {
			return false;
		}
		
		offset = 2 + splntr->size();
		for (i = blocks.begin(); i != blocks.end(); ++i) {
			if (!(**i).set_state(x, offset)) {
				return false;
			}
			offset += (**i).size();
		}
		
		step(x[0], x[1]);
		
		get_flat_state(y);
		return true;
	}
	
	int get_input_size() const {
		return 2 + get_output_size();
	}
	
	int get_output_size() const {
		std::vector<box*>::const_iterator i;
		int s = 0;
		s += splntr->size();
		for (i = blocks.begin(); i != blocks.end(); ++i) {
			s += (**i).size();
		}
		return s;
	}
	
private:
	void add_block(const std::string &name, float px, float py, float rz, float mass, float width, float length) {
		blocks.push_back(new box(space, name, px, py, rz, mass, width, length));
	}
	
	bool first;
	cpSpace *space;
	splinter *splntr;
	std::vector<box*> blocks;
};

#endif
