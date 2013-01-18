#include <iostream>
#include <cstdlib>
#include "drawer.h"
#include "common.h"

using namespace std;

ostream &write_vec3(ostream &os, const vec3 &v) {
	os << v(0) << " " << v(1) << " " << v(2);
	return os;
}

drawer::drawer() : error(false) {}

drawer::drawer(const string &path) : error(false) {
	sock.set_address(path);
}

void drawer::set_address(const string &path) {
	sock.set_address(path);
	error = false;
}

void drawer::add(const string &scn, const sgnode *n) {
	if (error || !n->get_parent()) {
		return;
	}
	change(scn, n, SHAPE | POS | ROT | SCALE);
}

void drawer::del(const string &scn, const sgnode *n) {
	if (error) {
		return;
	}
	
	stringstream ss;
	ss << scn << " -" << n->get_name() << endl;
	if (!sock.send(ss.str()))
		error = true;
}

void drawer::change(const string &scn, const sgnode *n, int props) {
	if (error) {
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
	if (!sock.send(ss.str()))
		error = true;
}

void drawer::delete_scene(const string &scn) {
	if (error)
		return;
	
	if (!sock.send(string("-") + scn + "\n"))
		error = true;
}

void drawer::set_color(const string &name, double r, double g, double b) {
	stringstream ss;
	
	if (error)
		return;
	ss << "* " << name << " c " << r << " " << g << " " << b << endl;
	if (!sock.send(ss.str()))
		error = true;
}

void drawer::set_pos(const string &name, double x, double y, double z) {
	stringstream ss;
	
	if (error)
		return;
	ss << "* " << name << " p " << x << " " << y << " " << z << endl;
	if (!sock.send(ss.str()))
		error = true;
}
