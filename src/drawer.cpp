#include <iostream>
#include <cstdlib>
#include "drawer.h"

using namespace std;

int drawer::POS = 1;
int drawer::ROT = 1 << 2;
int drawer::SCALE = 1 << 3;
int drawer::COLOR = 1 << 4;
int drawer::VERTS = 1 << 5;

bool drawer::fifo_open = false;
ofstream drawer::fifo;

void write_ptlist(std::ostream &os, const ptlist &l) {
	copy(l.begin(), l.end(), ostream_iterator<vec3>(os, " "));
}

drawer::drawer(const string &sname) 
: scene_name(sname), scl(1., 1., 1.), color(0., 1., 0.)
{
	if (!fifo_open) {
		char *path = getenv("SVS_DISPLAY_PIPE");
		if (path != NULL && access(path, W_OK) == 0) {
			fifo.open(path);
		}
	}
}

drawer::~drawer() {
	// fifo.close();
}

void drawer::set_pos(const vec3 &p) {
	pos = p;
}

void drawer::set_rot(const vec3 &r) {
	rot = r;
}

void drawer::set_scale(const vec3 &s) {
	scl = s;
}

void drawer::set_transforms(sgnode *n) {
	set_pos(n->get_trans('p'));
	set_rot(n->get_trans('r'));
	set_scale(n->get_trans('s'));
}

void drawer::set_color(float r, float g, float b) {
	color[0] = r;
	color[1] = g;
	color[2] = b;
}

void drawer::set_vertices(const ptlist &v) {
	verts = v;
}

void drawer::set_vertices(sgnode *n) {
	ptlist pts;
	n->get_local_points(pts);
	verts = pts;
}

void drawer::reset_properties() {
	pos << 0, 0, 0;
	rot << 0, 0, 0;
	scl << 1, 1, 1;
	color << 0, 1, 0;
}

void drawer::add(const string &name) {
	if (!fifo_open) return;
	
	fifo << scene_name << " u " << name << " world v ";
	write_ptlist(fifo, verts);
	fifo << " p " << pos << " r " << rot << " s " << scl << endl;
	fifo.flush();
}

void drawer::add(sgnode *n) {
	ptlist pts;
	n->get_local_points(pts);
	set_pos(n->get_trans('p'));
	set_rot(n->get_trans('r'));
	set_scale(n->get_trans('s'));
	set_vertices(pts);
	add(n->get_name());
}

void drawer::del(const string &name) {
	if (!fifo_open) return;
	
	fifo << scene_name << " d " << name << endl;
	fifo << scene_name << " d " << name << "_label" << endl;
	fifo.flush();
}

void drawer::del(sgnode *n) {
	del(n->get_name());
}

void drawer::change(const string &name, int props) {
	if (!fifo_open) return;
	
	fifo << scene_name << " u " << name;
	if (props & VERTS) {
		fifo << " v ";
		write_ptlist(fifo, verts);
	}
	if (props & POS) {
		fifo << " p " << pos;
	}
	if (props & ROT) {
		fifo << " r " << rot;
	}
	if (props & SCALE) {
		fifo << " s " << scl;
	}
	fifo << endl;
	/*
	if (props & COLOR) {
		fifo << " c " << color;
	}
	if (props & POS) {
		fifo << scene_name << " t " << name << "_label " << pos << " " << name << endl;
	}
	*/
	fifo.flush();
}

