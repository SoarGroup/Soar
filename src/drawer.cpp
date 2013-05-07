#include <iostream>
#include <cstdlib>
#include "drawer.h"
#include "common.h"

using namespace std;

ostream &write_vec3(ostream &os, const vec3 &v) {
	os << v(0) << " " << v(1) << " " << v(2);
	return os;
}

drawer *get_drawer() {
	static drawer d;
	return &d;
}

drawer::drawer() : connected(false) {}

void drawer::connect(const string &path) {
	connected = sock.connect(path);
}

void drawer::disconnect() {
	if (connected) {
		sock.disconnect();
	}
	connected = false;
}

void drawer::add(const string &scn, const sgnode *n) {
	if (!connected || !n->get_parent()) {
		return;
	}
	change(scn, n, SHAPE | POS | ROT | SCALE);
}

void drawer::del(const string &scn, const sgnode *n) {
	if (!connected) {
		return;
	}
	
	stringstream ss;
	ss << scn << " -" << n->get_name() << endl;
	send(ss.str());
}

void drawer::change(const string &scn, const sgnode *n, int props) {
	if (!connected) {
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
	send(ss.str());
}

void drawer::delete_scene(const string &scn) {
	if (!connected)
		return;
	
	send(string("-") + scn + "\n");
}

void drawer::set_color(const string &name, double r, double g, double b) {
	if (!connected) {
		return;
	}
	
	stringstream ss;
	ss << "* " << name << " c " << r << " " << g << " " << b << endl;
	send(ss.str());
}

void drawer::set_pos(const string &name, double x, double y, double z) {
	if (!connected) {
		return;
	}
		
	stringstream ss;
	ss << "* " << name << " p " << x << " " << y << " " << z << endl;
	send(ss.str());
}

void drawer::send(const string &s) {
	if (!connected) {
		return;
	}
	if (s[s.size()-1] != '\n') {
		connected = sock.send(s + '\n');
	} else {
		connected = sock.send(s);
	}
}
