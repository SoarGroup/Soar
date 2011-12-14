#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <osg/Group>
#include <osg/Geode>
#include <osg/PositionAttitudeTransform>
#include <osgFX/Scribe>

class node {
public:
	osg::ref_ptr<osg::PositionAttitudeTransform> trans;
	osg::ref_ptr<osg::PositionAttitudeTransform> wiretrans;
	osg::ref_ptr<osg::Group> group;
	osg::ref_ptr<osg::Geode> leaf;
	osg::ref_ptr<osg::Geode> label;
	osg::ref_ptr<osgFX::Scribe> scribe;
	std::string name;
	
	node(const std::string &name);	
	node(const std::string &name, const std::vector<osg::Vec3> &verts);
	void create_label();
	void add_child(node &n);
	void remove_child(node &n);
	void set_vertices(const std::vector<osg::Vec3> &verts);
	bool is_group();
};

class scene {
public:
	scene();
	osg::Group *get_root();
	void update(const std::string &s);
	
private:
	int parse_update(std::vector<std::string> &f);
	int parse_del(std::vector<std::string> &f);
	
	typedef std::unique_ptr<node> node_ptr;
	
	std::map<std::string, node_ptr > nodes;
	std::map<std::string, std::string > parents;
};

#endif
