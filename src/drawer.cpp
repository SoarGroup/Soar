#include <iostream>
#include <cstdlib>
#include "drawer.h"
#include "common.h"

using namespace std;

ostream &write_vec3(ostream &os, const vec3 &v) {
	os << v(0) << " " << v(1) << " " << v(2);
	return os;
}

void write_shape_string(const sgnode *n, ostream &os) {
	const convex_node *cn = dynamic_cast<const convex_node *>(n);
	if (cn) {
		const ptlist &pts = cn->get_verts();
		os << "v ";
		for (int i = 0; i < pts.size(); ++i) {
			write_vec3(os, pts[i]) << " ";
		}
		return;
	}
}

drawer::drawer() {
	string path = get_option("display");
	if (path.empty()) {
		path = "/tmp/viewer";
	}
	sock.connect(path);
	if (sock.connected()) {
		sock.send("clear\n");
	}
}

drawer::~drawer() {
	sock.disconnect();
}

void drawer::add(const string &scn, const sgnode *n) {
	if (!sock.connected() || !n->get_parent()) {
		return;
	}
	
	stringstream ss;
	string shape_str;
	n->get_shape_sgel(shape_str);
	
	ss << scn << " a " << n->get_name() << " " << n->get_parent()->get_name() << " " << shape_str << " p ";
	write_vec3(ss, n->get_trans('p')) << " r ";
	write_vec3(ss, n->get_trans('r')) << " s ";
	write_vec3(ss, n->get_trans('s')) << endl;
	
	sock.send(ss.str());
}

void drawer::del(const string &scn, const sgnode *n) {
	if (!sock.connected()) {
		return;
	}
	
	stringstream ss;
	ss << scn << " d " << n->get_name() << endl;
	sock.send(ss.str());
}

void drawer::change(const string &scn, const sgnode *n, int props) {
	if (!sock.connected()) {
		return;
	}
	
	stringstream ss;
	ss << scn << " c " << n->get_name() << " ";
	if (props & SHAPE) {
		write_shape_string(n, ss);
	}
	if (props & POS) {
		ss << " p ";
		write_vec3(ss, n->get_trans('p'));
	}
	if (props & ROT) {
		ss << " r ";
		write_vec3(ss, n->get_trans('r'));
	}
	if (props & SCALE) {
		ss << " s ";
		write_vec3(ss, n->get_trans('s'));
	}
	ss << endl;
	sock.send(ss.str());
}
