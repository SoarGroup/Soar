#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

/* Return whether two intervals [x1, x2] and [y1, y2] overlap */
inline bool overlap(float x1, float x2, float y1, float y2) {
	if (x1 <= y1) {
		return x2 >= y1;
	} else {
		return y2 >= x1;
	}
}

inline float min(float a, float b) {
	return a < b ? a : b;
}

inline float max(float a, float b) {
	return a < b ? b : a;
}

inline float between(float x, float a, float b) {
	if (x < a) {
		return a;
	}
	if (x > b) {
		return b;
	}
	return x;
}

class rect {
public:
	rect(float x, float y, float w, float h)
	: x1(x), y1(y), w(w), h(h), x2(x+w), y2(y+h)
	{}
	
	rect(const rect &r)
	: x1(r.x1), x2(r.x2), y1(r.y1), y2(r.y2), w(r.w), h(r.h)
	{}
	
	bool intersects(const rect &r) const {
		return overlap(x1, x2, r.x1, r.x2) && overlap(y1, y2, r.y1, r.y2);
	}
	
	void move(float dx, float dy) {
		x1 += dx;
		y1 += dy;
		x2 = x1 + w;
		y2 = y1 + h;
	}
	
	string get_sgel(bool add, const string &name) const {
		stringstream ss;
		
		if (add) {
			ss << "a " << name << " world v ";
			ss << "0 0 0 ";
			ss << x2 - x1 << " 0 0 ";
			ss << x2 - x1 << " " << y2 - y1 << " 0 ";
			ss << "0 " << y2 - y1 << " 0 ";
			ss << "0 0 1 ";
			ss << x2 - x1 << " 0 1 ";
			ss << x2 - x1 << " " << y2 - y1 << " 1 ";
			ss << "0 " << y2 - y1 << " 1 ";
		} else {
			ss << "c " << name << " ";
		}
		ss << "p " << x1 << " " << y1 << " 0";
		return ss.str();
	}
	
	float x1, y1, x2, y2, w, h;
};

class agent {
public:
	agent(const rect &body)
	: body(body)
	{}
	
	void update(map<string, float> &output, const vector<rect> &obstacles) {
		float dx, dy;

		dx = between(output["dx"], -1., 1.);
		dy = between(output["dy"], -1., 1.);
		
		rect next(body.x1 + dx, body.y1 + dy, body.w, body.h);
		
		/* naive collision resolution */
		vector<rect>::const_iterator i;
		for (i = obstacles.begin(); i != obstacles.end(); ++i) {
			if (next.intersects(*i)) {
				float xp, yp;
				if (dx > 0) {
					xp = max(next.x2 - i->x1, 0.);
				} else {
					xp = min(next.x1 - i->x2, 0.);
				}
				if (dy > 0) {
					yp = max(next.y2 - i->y1, 0.);
				} else {
					yp = min(next.y1 - i->y2, 0.);
				}
				// Resolve by reducing movement on the axis with least penetration
				if (fabs(xp) < fabs(yp)) {
					dx -= xp;
				} else {
					dy -= yp;
				}
			}
		}
		body.move(dx, dy);
	}
	
	string get_sgel(bool add) const {
		return body.get_sgel(add, "agent");
	}

	rect body;
};

class door {
public:
	door(const rect &slab, const rect &open_switch, const rect &close_switch, int travel, int dy)
	: slab(slab), open_switch(open_switch), close_switch(close_switch),
	  ymin(slab.y1), ymax(slab.y1 + travel), dy(dy)
	{}
	
	void update(const rect &agentrect) {
		if (open_switch.intersects(agentrect)) {
			cerr << "opening" << endl;
			slab.move(0., min(dy, ymax - slab.y1));
		} else if (close_switch.intersects(agentrect)) {
			cerr << "closing" << endl;
			slab.move(0., max(dy, ymin - slab.y1));
		}
	}
	
	string get_sgel(bool add, const string &name) const {
		stringstream ss;
		ss << slab.get_sgel(add, name + "/slab") << endl;
		if (add) {
			ss << open_switch.get_sgel(add, name + "/open") << endl;
			ss << close_switch.get_sgel(add, name + "/close");
		}
		return ss.str();
	}
	
	rect slab;
	rect open_switch;
	rect close_switch;
	float ymin, ymax, dy;
};

class scene {
public:
	scene(const rect &agentbody) : agnt(agentbody), added(false)
	{}
	
	void add_door(const rect &slab, const rect &open_switch, const rect &close_switch, int travel, int dy) {
		doors.push_back(door(slab, open_switch, close_switch, travel, dy));
	}
	
	void add_wall(const rect &w) {
		walls.push_back(w);
	}

	void update(map<string, float> &output) {
		vector<rect> obstacles;
		for (int i = 0; i < doors.size(); ++i) {
			obstacles.push_back(doors[i].slab);
		}
		for (int i = 0; i < walls.size(); ++i) {
			obstacles.push_back(walls[i]);
		}
		agnt.update(output, obstacles);
		for (int i = 0; i < doors.size(); ++i) {
			doors[i].update(agnt.body);
		}
	}

	string get_sgel() {
		stringstream ss;
		ss << agnt.get_sgel(!added) << endl;
		for (int i = 0; i < doors.size(); ++i) {
			stringstream n;
			n << "door" << i;
			ss << doors[i].get_sgel(!added, n.str()) << endl;
		}
		if (!added) {
			for (int i = 0; i < walls.size(); ++i) {
				stringstream n;
				n << "wall" << i;
				ss << walls[i].get_sgel(!added, n.str()) << endl;
			}
		}
		added = true;
		return ss.str();
	}
	
	agent agnt;
	vector<door> doors;
	vector<rect> walls;
	bool added;
};

int main() {
	scene s(rect(5., 10., 5., 5.));
	
	s.add_wall(rect(0., 0., 20., 1.));
	s.add_wall(rect(0., 20., 20., 1.));
	s.add_wall(rect(0., 0., 1., 20.));
	
	s.add_door(rect(20., 0., 1., 20.), rect(1., 1., 5., 5.), rect(1., 15., 5., 5.), 20., 1.);
	
	cout << s.get_sgel() << endl << "***" << endl;
	
	string line;
	map<string, float> output;
	while(getline(cin, line)) {
		stringstream ss(line);
		string name;
		float val;
		ss >> name;
		if (name == "***") {
			s.update(output);
			cout << s.get_sgel() << endl << "***" << endl;
			output.clear();
		} else {
			ss >> val;
			output[name] = val;
		}
	}
	return 0;
}
