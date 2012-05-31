#include <cstdlib>
#include <cassert>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/Material>
#include <osgText/Font>
#include <osgText/Text>

#include "scene.h"

using namespace std;
using namespace osg;

const char *FONT = "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf";
const double AXIS_RADIUS = 0.005;
const double AXIS_LEN = 1.0;
const double GRIDSIZE = 10;

/*
 Execute qhull to calculate the convex hull of pts.
*/
int qhull(const vector<Vec3> &pts, vector<vector<int> > &facets) {
	char *end;
	
	FILE *p = popen("qhull i >/tmp/qhull", "w");
	fprintf(p, "3\n%ld\n", pts.size());
	for (int i = 0; i < pts.size(); ++i) {
		fprintf(p, "%f %f %f\n", pts[i][0], pts[i][1], pts[i][2]);
	}
	int ret = pclose(p);
	if (ret != 0) {
		return ret;
	}
	
	ifstream output("/tmp/qhull");
	string line;
	getline(output, line);
	int nfacets = strtol(line.c_str(), &end, 10);
	if (*end != '\0') {
		return 1;
	}
	while (getline(output, line)) {
		const char *start = line.c_str();
		vector<int> facet;
		while (true) {
			int x = strtol(start, &end, 10);
			if (end == start) {
				break;
			}
			facet.push_back(x);
			start = end;
		}
		facets.push_back(facet);
	}
	assert (facets.size() == nfacets);
	return 0;
}

Quat to_quaternion(const Vec3 &rpy) {
	double halfroll = rpy[0] / 2;
	double halfpitch = rpy[1] / 2;
	double halfyaw = rpy[2] / 2;

	double sin_r2 = sin( halfroll );
	double sin_p2 = sin( halfpitch );
	double sin_y2 = sin( halfyaw );

	double cos_r2 = cos( halfroll );
	double cos_p2 = cos( halfpitch );
	double cos_y2 = cos( halfyaw );

	return Quat(sin_r2 * cos_p2 * cos_y2 - cos_r2 * sin_p2 * sin_y2,
	            cos_r2 * sin_p2 * cos_y2 + sin_r2 * cos_p2 * sin_y2,
	            cos_r2 * cos_p2 * sin_y2 - sin_r2 * sin_p2 * cos_y2,
	            cos_r2 * cos_p2 * cos_y2 + sin_r2 * sin_p2 * sin_y2);
}

node::node(const string &name, const string &parent) 
: name(name), parent(parent), trans(new PositionAttitudeTransform()), 
  group(new Group), scribe(new osgFX::Scribe), leaf(new Geode)
{
	trans->addChild(group);
	create_label();
	create_axes();
	scribe->addChild(leaf);
	scribe->setWireframeColor(Vec4(0.0, 0.0, 0.0, 1.0));
}

void node::make_polyhedron(const vector<Vec3> &verts) {
	ref_ptr<Geometry> g = new Geometry;
	ref_ptr<Vec3Array> v = new Vec3Array;
	copy(verts.begin(), verts.end(), back_inserter(*v));
	g->setVertexArray(v);
	
	if (verts.size() == 1) {
		g->addPrimitiveSet(new DrawArrays(GL_POINTS, 0, 1));
	} else if (verts.size() == 2) {
		g->addPrimitiveSet(new DrawArrays(GL_LINES, 0, 2));
	} else if (verts.size() == 3) {
		g->addPrimitiveSet(new DrawArrays(GL_TRIANGLES, 0, 3));
	} else {
		vector<vector<int> > facets;
		if (qhull(verts, facets) != 0) {
			cerr << "error executing qhull" << endl;
			exit(1);
		}
		ref_ptr<DrawElementsUInt> triangles = new DrawElementsUInt(GL_TRIANGLES);
		ref_ptr<DrawElementsUInt> quads = new DrawElementsUInt(GL_QUADS);
		for (int i = 0; i < facets.size(); ++i) {
			if (facets[i].size() == 3) {
				copy(facets[i].begin(), facets[i].end(), back_inserter(*triangles));
			} else if (facets[i].size() == 4) {
				copy(facets[i].begin(), facets[i].end(), back_inserter(*quads));
			} else {
				assert(false);
			}
		}
		
		if (!triangles->empty()) {
			g->addPrimitiveSet(triangles);
		}
		if (!quads->empty()) {
			g->addPrimitiveSet(quads);
		}
	}
	if (leaf->getNumDrawables() == 1) {
		leaf->setDrawable(0, g);
	} else {
		leaf->addDrawable(g);
	}
	trans->setChild(0, scribe);
}

void node::make_sphere(double radius) {
	ShapeDrawable* d = new ShapeDrawable(new Sphere( Vec3(0,0,0), radius));
	if (leaf->getNumDrawables() == 1) {
		leaf->setDrawable(0, d);
	} else {
		leaf->addDrawable(d);
	}
	trans->setChild(0, scribe);
}

void node::make_group() {
	trans->setChild(0, group);
}

void node::create_axes() {
	axes = new Group;
	for (int i = 0; i < 3; ++i) {
		ref_ptr<Geode> g = new Geode;
		ref_ptr<Material> m = new Material;
		ref_ptr<Cylinder> c;
		
		switch (i) {
		case 0:
			c = new Cylinder(Vec3(0.5, 0, 0), AXIS_RADIUS, AXIS_LEN);
			c->setRotation(Quat(PI / 2, Vec3(0, 1, 0)));
			m->setDiffuse( Material::FRONT, Vec4(1, 0, 0, 1));
			break;
		case 1:
			c = new Cylinder(Vec3(0, 0.5, 0), AXIS_RADIUS, AXIS_LEN);
			c->setRotation(Quat(PI / 2, Vec3(1, 0, 0)));
			m->setDiffuse( Material::FRONT, Vec4(0, 1, 0, 1));
			break;
		case 2:
			c = new Cylinder(Vec3(0, 0, 0.5), AXIS_RADIUS, AXIS_LEN);
			m->setDiffuse( Material::FRONT, Vec4(0, 0, 1, 1));
			break;
		}
		g->addDrawable(new ShapeDrawable(c));
		g->getOrCreateStateSet()->setAttribute(m, StateAttribute::ON | StateAttribute::OVERRIDE);
		axes->addChild(g);
	}
	trans->addChild(axes);
}

void node::create_label() {
	ref_ptr<osgText::Text> txt = new osgText::Text;
	txt->setText(name);
	txt->setFont(FONT);
	txt->setAxisAlignment(osgText::Text::SCREEN);
	txt->setColor(Vec4(1.f, 0.f, 0.f, 1.f));
    txt->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    txt->setCharacterSize(24);
    
	label = new Geode;
	ref_ptr<StateSet> ss = label->getOrCreateStateSet();
	ss->setMode(GL_DEPTH_TEST, StateAttribute::OFF);
	/*
	ss->setRenderingHint( StateSet::TRANSPARENT_BIN );
	ss->setRenderBinDetails(11, "RenderBin");
	*/
	label->addDrawable(txt);
	trans->addChild(label);
}

void node::add_child(node &n) {
	group->addChild(n.trans);
}

void node::remove_child(node &n) {
	group->removeChild(n.trans.get());
}

bool node::is_group() {
	return trans->getChild(0) == group;
}

void node::toggle_axes() {
	if (axes) {
		axes->setNodeMask(~axes->getNodeMask());
	}
}

void node::toggle_wireframe() {
	scribe->setEnabled(!scribe->getEnabled());
}

scene::scene() {
	node *w = new node("world", "");
	assert(w->is_group());
	nodes["world"] = w;
	ref_ptr<PositionAttitudeTransform> r = w->trans;
	//r->getOrCreateStateSet()->setMode(GL_LIGHTING, StateAttribute::OFF);
	
	grid = new Geode;
	ref_ptr<StateSet> ss = grid->getOrCreateStateSet();
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	w->group->addChild(grid);
	update_grid(0.0, 0.0, 100.0);
}

void scene::update_grid(double cx, double cy, double cfar) {
	double xmin = ((int) ((cx - cfar) / GRIDSIZE) - 1) * GRIDSIZE;
	double xmax = ((int) ((cx + cfar) / GRIDSIZE) + 1) * GRIDSIZE;
	double ymin = ((int) ((cy - cfar) / GRIDSIZE) - 1) * GRIDSIZE;
	double ymax = ((int) ((cy + cfar) / GRIDSIZE) + 1) * GRIDSIZE;
	
	ref_ptr<Vec3Array> verts = new Vec3Array;
	for(double x = xmin; x <= xmax; x += GRIDSIZE) {
		verts->push_back(Vec3(x, ymin, 0.0));
		verts->push_back(Vec3(x, ymax, 0.0));
	}
	for(double y = ymin; y <= ymax; y += GRIDSIZE) {
		verts->push_back(Vec3(xmin, y, 0.0));
		verts->push_back(Vec3(xmax, y, 0.0));
	}
	
	ref_ptr<Geometry> g = new Geometry;
	g->setVertexArray(verts);
	g->addPrimitiveSet(new DrawArrays(GL_LINES, 0, verts->size()));
	ref_ptr<Vec4Array> colors = new Vec4Array;
	colors->push_back(Vec4(0.5, 0.5, 0.5, 1.0));
	g->setColorArray(colors);
	g->setColorBinding(Geometry::BIND_OVERALL);
	
	grid->removeDrawable(0);
	grid->addDrawable(g);
}

bool parse_vec3(vector<string> &f, int &p, Vec3 &x) {
	if (p + 3 > f.size()) {
		p = f.size();
		return false;
	}
	for (int i = 0; i < 3; ++i) {
		char *end;
		x[i] = strtod(f[p + i].c_str(), &end);
		if (*end != '\0') {
			p += i;
			return false;
		}
	}
	p += 3;
	return true;
}

bool parse_verts(vector<string> &f, int &p, vector<Vec3> &verts) {
	while (p < f.size()) {
		Vec3 v;
		int old = p;
		if (!parse_vec3(f, p, v)) {
			if (p > old) {
				return false;
			}
			break;
		}
		verts.push_back(v);
	}
	return true;
}

bool parse_mods(vector<string> &f, int &p, node *n, string &error) {
	Vec3 v3;
	char t;
	char *end;
	double radius;
	vector<Vec3> verts;
	
	while (p < f.size()) {
		t = f[p++][0];
		switch (t) {
		case 'p':
		case 'r':
		case 's':
			if (!parse_vec3(f, p, v3)) {
				error = "expecting three numbers after ";
				error += t;
				return false;
			}
			if (t == 'p') {
				n->trans->setPosition(v3);
			} else if (t == 'r') {
				n->trans->setAttitude(to_quaternion(v3));
			} else {
				n->trans->setScale(v3);
			}
			break;
		
		case 'b':
			if (p >= f.size()) {
				error = "expecting a radius after b";
				return false;
			}
			radius = strtod(f[p++].c_str(), &end);
			if (*end != '\0') {
				error = "expecting a radius after b";
				return false;
			}
			n->make_sphere(radius);
			break;
		
		case 'v':
			if (!parse_verts(f, p, verts)) {
				error = "expecting N numbers after v, divisible by 3";
				return false;
			}
			n->make_polyhedron(verts);
			break;
			
		default:
			error = "unknown attribute ";
			error += t;
			return false;
		}
	}
	return true;
}

// f[0] is node name, f[1] is parent name
int scene::parse_add(vector<string> &f, string &error) {
	if (f.size() < 2) {
		error = "expecting <node name> <parent name>";
		return f.size();
	}
	
	if (nodes.find(f[0]) != nodes.end()) {
		error = "node already exists";
		return 0;
	}
	if (nodes.find(f[1]) == nodes.end() || !nodes[f[1]]->is_group()) {
		error = "parent not found or not a group";
		return 1;
	}
	
	node *n = new node(f[0], f[1]);
	
	int p = 2;
	if (!parse_mods(f, p, n, error)) {
		delete n;
		return p;
	}
	nodes[f[0]] = n;
	nodes[f[1]]->add_child(*n);
	return -1;
}

int scene::parse_change(vector<string> &f, string &error) {
	if (f.size() < 1) {
		error = "expecting <node name>";
		return f.size();
	}
	
	if (nodes.find(f[0]) == nodes.end()) {
		error = "node not found";
		return 0;
	}
	
	node *n = nodes[f[0]];
	int p = 1;
	if (!parse_mods(f, p, n, error)) {
		return p;
	}
	return -1;
}

int scene::parse_del(vector<string> &f, string &error) {
	if (f.size() != 1) {
		error = "expecting <node name>";
		return f.size();
	}
	if (f[0] == "world") {
		error = "cannot delete world node";
		return 0;
	}
	if (nodes.find(f[0]) == nodes.end()) {
		error = "node not found";
		return 0;
	}

	node *n = nodes[f[0]];
	nodes[n->parent]->remove_child(*n);
	
	// remove n and all offspring from nodes map
	vector<string> offspring;
	offspring.push_back(f[0]);
	for (int i = 0; i < offspring.size(); ++i) {
		delete nodes[offspring[i]];
		nodes.erase(offspring[i]);
		// look for children
		map<string, node*>::iterator j;
		for (j = nodes.begin(); j != nodes.end(); ++j) {
			if (j->second->parent == offspring[i]) {
				offspring.push_back(j->first);
			}
		}
		
	}
	
	return -1;
}

void scene::update(const vector<string> &fields) {
	char cmd;
	int errfield;
	
	if (fields.size() == 0) {
		return;
	}
	if (fields[0].size() != 1 || fields[0].find_first_of("acd") != 0) {
		cerr << "known commands are a, c, or d" << endl;
		return;
	}
	
	cmd = fields[0][0];
	vector<string> rest;
	for (int i = 1; i < fields.size(); ++i) {
		rest.push_back(fields[i]);
	}
	string error;
	switch(cmd) {
		case 'a':
			errfield = parse_add(rest, error);
			break;
		case 'c':
			errfield = parse_change(rest, error);
			break;
		case 'd':
			errfield = parse_del(rest, error);
			break;
		default:
			return;
	}
	
	if (errfield >= 0) {
		cerr << "error in field " << errfield + 2 << endl;
		cerr << error << endl;
	}
}

Group* scene::get_root() {
	return nodes["world"]->trans.get();
}

void scene::toggle_axes() {
	node_table::iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		i->second->toggle_axes();
	}
}

void scene::toggle_wireframe() {
	node_table::iterator i;
	for (i = nodes.begin(); i != nodes.end(); ++i) {
		i->second->toggle_wireframe();
	}
}
