#include <cstdio>
#include <cassert>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <memory>

#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osgViewer/Viewer>
#include <osg/PositionAttitudeTransform>
#include <osgGA/TrackballManipulator>
#include <osgText/Font>
#include <osgText/Text>

#include "scene.h"

using namespace std;
using namespace osg;

void qhull(const vector<Vec3> &pts, vector<vector<int> > &facets) {
	FILE *p = popen("qhull i >/tmp/qhull", "w");
	fprintf(p, "3\n%d\n", pts.size());
	for (auto i = pts.begin(); i != pts.end(); ++i) {
		fprintf(p, "%f %f %f\n", (*i)[0], (*i)[1], (*i)[2]);
	}
	if (pclose(p) != 0) {
		cerr << "error executing qhull" << endl;
		exit(1);
	}
	
	ifstream output("/tmp/qhull");
	string line;
	getline(output, line);
	int nfacets;
	stringstream(line) >> nfacets;
	while (getline(output, line)) {
		stringstream ss(line);
		vector<int> facet;
		int x;
		while (ss >> x) {
			facet.push_back(x);
		}
		facets.push_back(facet);
	}
	assert (facets.size() == nfacets);
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

void split(const string &s, const string &delim, vector<string> &fields) {
	int start, end = 0;
	fields.clear();
	while (end < s.size()) {
		start = s.find_first_not_of(delim, end);
		if (start == string::npos) {
			return;
		}
		end = s.find_first_of(delim, start);
		if (end == string::npos) {
			end = s.size();
		}
		fields.push_back(s.substr(start, end - start));
	}
}

bool parse_vec3(vector<string> &f, int &start, Vec3 &x) {
	stringstream ss;
	if (start + 3 > f.size()) {
		start = f.size();
		return false;
	}
	for (int i = 0; i < 3; ++start, ++i) {
		ss << f[start] << endl;
		if (!(ss >> x[i])) {  // conversion failure
			return false;
		}
	}
	return true;
}

bool parse_verts(vector<string> &f, int &start, vector<Vec3> &verts) {
	int i;
	if (start >= f.size() || f[start] != "v") {
		return true;
	}
	start++;
	verts.clear();
	while (start < f.size()) {
		i = start;
		Vec3 v;
		if (!parse_vec3(f, start, v)) {
			return (i == start);  // end of list
		}
		verts.push_back(v);
	}
	return true;
}

bool parse_transforms(vector<string> &f, int &start, PositionAttitudeTransform &trans) {	
	Vec3 t;
	char type;
	
	while (start < f.size()) {
		if (f[start] != "p" && f[start] != "r" && f[start] != "s") {
			return true;
		}
		type = f[start][0];
		start++;
		if (!parse_vec3(f, start, t)) {
			return false;
		}
		switch (type) {
			case 'p':
				trans.setPosition(t);
				break;
			case 'r':
				trans.setAttitude(to_quaternion(t));
				break;
			case 's':
				trans.setScale(t);
				break;
			default:
				assert(false);
		}
	}
	return true;
}

node::node(const string &name) 
: name(name), trans(new PositionAttitudeTransform()), group(new Group)
{
	if (name != "world") {
		create_label();
	}
	trans->addChild(group);
}

node::node(const string &name, const vector<Vec3> &verts)
: name(name), trans(new PositionAttitudeTransform())
{
	create_label();
	if (verts.size() == 0) {
		group = new Group;
		trans->addChild(group);
	} else {
		leaf = new Geode;
		set_vertices(verts);
		scribe = new osgFX::Scribe;
		scribe->addChild(leaf);
		scribe->setWireframeColor(Vec4(0.0, 0.0, 0.0, 1.0));
		trans->addChild(scribe);
	}
}

void node::create_label() {
	ref_ptr<osgText::Text> txt = new osgText::Text;
	txt->setText(name);
	txt->setAxisAlignment(osgText::Text::SCREEN);
	txt->setColor(Vec4(0.f, 0.f, 0.f, 1.f));
    txt->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    txt->setCharacterSize(30);
    
	label = new Geode;
	ref_ptr<StateSet> ss = label->getOrCreateStateSet();
	ss->setMode(GL_DEPTH_TEST, StateAttribute::OFF);
       		ss->setRenderingHint( StateSet::TRANSPARENT_BIN );
	ss->setRenderBinDetails(11, "RenderBin");
	label->addDrawable(txt);
	trans->addChild(label);
}

void node::add_child(node &n) {
	assert(group.valid());
	group->addChild(n.trans);
}

void node::remove_child(node &n) {
	group->removeChild(n.trans.get());
}

void node::set_vertices(const vector<Vec3> &verts) {
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
		ref_ptr<DrawElementsUInt> triangles = new DrawElementsUInt(GL_TRIANGLES);
		ref_ptr<DrawElementsUInt> quads = new DrawElementsUInt(GL_QUADS);
		qhull(verts, facets);
		for (auto i = facets.begin(); i != facets.end(); ++i) {
			if (i->size() == 3) {
				copy(i->begin(), i->end(), back_inserter(*triangles));
			} else if (i->size() == 4) {
				copy(i->begin(), i->end(), back_inserter(*quads));
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
	leaf->removeDrawable(0);
	leaf->addDrawable(g);
}

bool node::is_group() {
	return group.valid();
}

scene::scene() {
	nodes["world"].reset(new node("world"));
	ref_ptr<PositionAttitudeTransform> r = nodes["world"]->trans;
	r->getOrCreateStateSet()->setMode(GL_LIGHTING, StateAttribute::OFF);
}

int scene::parse_update(vector<string> &f) {
	vector<Vec3> verts;
	int p;

	if (f.size() < 2) {
		return f.size();
	}
	
	auto i = nodes.find(f[0]);
	if (i == nodes.end()) {
		if (nodes.find(f[1]) == nodes.end() || !nodes[f[1]]->is_group()) {  // f[1] is parent name
			return 1;
		}
		p = 2;
	} else {
		p = 1;
	}
	
	if (!parse_verts(f, p, verts)) {
		return p;
	}
	
	node *n;
	if (i == nodes.end()) {
		n = new node(f[0], verts);
		nodes[f[0]].reset(n);
		nodes[f[1]]->add_child(*n);
		parents[f[0]] = f[1];
	} else {
		n = i->second.get();
		if (!verts.empty() && !n->is_group()) {
			n->set_vertices(verts);
		}
	}
	
	if (!parse_transforms(f, p, *(n->trans))) {
		return p;
	}
	
	return -1;
}

int scene::parse_del(vector<string> &f) {
	if (f.size() != 1) {
		return f.size();
	}
	auto i = nodes.find(f[0]);
	auto j = parents.find(f[0]);
	if (i == nodes.end() || j == parents.end()) {
		return 0;
	}
	nodes[j->second]->remove_child(*(i->second));
	return -1;
}

void scene::update(const string &s) {
	vector<string> fields;
	char cmd;
	int errfield;
	
	split(s, " \t\n", fields);
	
	if (fields.size() == 0) {
		return;
	}
	if (fields[0].size() != 1) {
		cerr << "expecting u or d at beginning of line '" << s << "'" << endl;
		return;
	}
	
	cmd = fields[0][0];
	fields.erase(fields.begin());
	
	switch(cmd) {
		case 'u':
			errfield = parse_update(fields);
			break;
		case 'd':
			errfield = parse_del(fields);
			break;
		default:
			cerr << "expecting u or d at beginning of line '" << s << "'" << endl;
	}
	
	if (errfield >= 0) {
		cerr << "error in field " << errfield + 1 << " of line '" << s << "' " << endl;
	}
}

Group* scene::get_root() {
	return nodes["world"]->trans.get();
}
