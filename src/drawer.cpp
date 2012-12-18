#include <iostream>
#include <cstdlib>
#include "drawer.h"
#include "common.h"

using namespace std;
const char *viewer_sock = "/tmp/viewer";

ostream &write_vec3(ostream &os, const vec3 &v) {
	os << v(0) << " " << v(1) << " " << v(2);
	return os;
}

drawer::drawer() {
	sock.connect(viewer_sock);
}

drawer::~drawer() {
	sock.disconnect();
}

void drawer::add(const string &scn, const sgnode *n) {
	if (!sock.connected() || !n->get_parent()) {
		return;
	}
	change(scn, n, SHAPE | POS | ROT | SCALE);
}

void drawer::del(const string &scn, const sgnode *n) {
	if (!sock.connected()) {
		return;
	}
	
	stringstream ss;
	ss << scn << " -" << n->get_name() << endl;
	sock.send(ss.str());
}

void drawer::change(const string &scn, const sgnode *n, int props) {
	if (!sock.connected()) {
		return;
	}
	
	vec3 p, r, s;
	vec4 q;
	stringstream ss;

	n->get_world_trans(p, r, s);
	q = n->get_quaternion();
	ss << "+" << scn << " +" << n->get_name() << " ";
	if (props & SHAPE) {
		string shape;
		n->get_shape_sgel(shape);
		ss << " " << shape << " ";
	}
	if (props & POS) {
		ss << " p ";
		write_vec3(ss, p);
	}
	if (props & ROT) {
		ss << " r " << q(0) << " " << q(1) << " " << q(2) << " " << q(3) << " ";
	}
	if (props & SCALE) {
		ss << " s ";
		write_vec3(ss, s);
	}
	ss << endl;
	sock.send(ss.str());
}

void drawer::delete_scene(const string &scn) {
	sock.send(string("-") + scn + "\n");
}

void drawer::set_color(const string &name, double r, double g, double b) {
	stringstream ss;
	ss << "* " << name << " " << r << " " << g << " " << b << endl;
	sock.send(ss.str());
}
