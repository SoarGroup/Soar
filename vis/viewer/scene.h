#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <map>
#include <string>
#include <osg/Group>
#include <osg/Geode>
#include <osg/PositionAttitudeTransform>
#include <osgFX/Scribe>

class node {
public:
	osg::ref_ptr<osg::PositionAttitudeTransform> trans;
	osg::ref_ptr<osg::Group> group;
	osg::ref_ptr<osg::Geode> leaf;
	osg::ref_ptr<osg::Geode> label;
	osg::ref_ptr<osg::Group> axes;
	osg::ref_ptr<osgFX::Scribe> scribe;
	std::string name;
	std::string parent;
	
	node(const std::string &name, const std::string &parent);
	void make_polyhedron(const std::vector<osg::Vec3> &verts);
	void make_sphere(double radius);
	void make_group();
	
	void add_child(node &n);
	void remove_child(node &n);
	bool is_group();
	void toggle_axes();
	void toggle_wireframe();
	void toggle_fill();
	
private:
	void create_label();
	void create_axes();
};

class scene {
public:
	scene();
	osg::Group *get_root();
	void update(const std::vector<std::string> &fields);
	void toggle_axes();
	void toggle_wireframe();
	void toggle_fill();
	void update_grid(double cx, double cy, double cfar);
	
private:
	int parse_add(std::vector<std::string> &f, std::string &error);
	int parse_change(std::vector<std::string> &f, std::string &error);
	int parse_del(std::vector<std::string> &f, std::string &error);
	
	typedef std::map<std::string, node*> node_table;
	
	node_table nodes;
	osg::ref_ptr<osg::Geode> grid;
};

#endif
