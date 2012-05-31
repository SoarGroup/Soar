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
	node(const std::string &name, const std::string &parent, const std::vector<osg::Vec3> &verts);
	void add_child(node &n);
	void remove_child(node &n);
	void set_vertices(const std::vector<osg::Vec3> &verts);
	bool is_group();
	void toggle_axes();
	
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
	void update_grid(double cx, double cy, double cfar);
	
private:
	int parse_add(std::vector<std::string> &f);
	int parse_change(std::vector<std::string> &f);
	int parse_del(std::vector<std::string> &f);
	
	typedef std::map<std::string, node*> node_table;
	
	node_table nodes;
	osg::ref_ptr<osg::Geode> grid;
};

#endif
