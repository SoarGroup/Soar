#include <iostream>
#include <cstdlib>
#include "drawer.h"
#include "common.h"

using namespace std;

void write_ptlist(std::ostream &os, const ptlist &l) {
	copy(l.begin(), l.end(), ostream_iterator<vec3>(os, " "));
}

drawer::drawer() 
: scl(1., 1., 1.)
{
	string path = get_option("display");
	if (path.empty()) {
		path = "/tmp/viewer";
	}
	sock.connect(path);
}

drawer::~drawer() {
	sock.disconnect();
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
}

void drawer::add(const string &scn, const string &name) {
	if (!sock.connected()) {
		return;
	}
	stringstream ss;
	ss << scn << " a " << name << " world v ";
	write_ptlist(ss, verts);
	ss << " p " << pos << " r " << rot << " s " << scl << endl;
	sock.send_line(ss.str());
}

void drawer::add(const string &scn, sgnode *n) {
	ptlist pts;
	n->get_local_points(pts);
	set_pos(n->get_trans('p'));
	set_rot(n->get_trans('r'));
	set_scale(n->get_trans('s'));
	set_vertices(pts);
	add(scn, n->get_name());
}

void drawer::del(const string &scn, const string &name) {
	if (!sock.connected()) {
		return;
	}
	
	stringstream ss;
	ss << scn << " d " << name << endl;
	sock.send_line(ss.str());
}

void drawer::del(const string &scn, sgnode *n) {
	del(scn, n->get_name());
}

void drawer::change(const string &scn, const string &name, int props) {
	if (!sock.connected()) {
		return;
	}
	
	stringstream ss;
	ss << scn << " c " << name;
	if (props & VERTS) {
		ss << " v ";
		write_ptlist(ss, verts);
	}
	if (props & POS) {
		ss << " p " << pos;
	}
	if (props & ROT) {
		ss << " r " << rot;
	}
	if (props & SCALE) {
		ss << " s " << scl;
	}
	ss << endl;
	sock.send_line(ss.str());
}

