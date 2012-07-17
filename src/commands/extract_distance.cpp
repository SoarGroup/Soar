#include <iostream>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include <list>

using namespace std;

class extract_distance_command : public command {
public:
    extract_distance_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), state(state), a(NULL), b(NULL), axis(-1), res_root(NULL), first(true)
    {
	si = state->get_svs()->get_soar_interface();
    }
	
    ~extract_distance_command() {
	
    }
	
    string description() {
	return string("extract_distance");
    }
	
    bool update_drv() {
	
	if (first) {
	    if (a != NULL || b != NULL)
	    {
		std::cout << "CANNOTALREADY FIND A OR B" << std::endl;
		
	    }
	    get_nodes(state->get_svs()->get_soar_interface(), root,
		      state->get_scene());
		
	}
	if (a == NULL || b == NULL || axis == -1)
	{
	    std::cout << "CANNOT FIND A OR B or AXIS" << std::endl;
	    set_status("a,b incorrect filter syntax");
	}
	if (first)//|| changed())
	{
	    //std::cout << "we are hereherhehrehr" << std::endl;
	    first = false;
	    update_distance_result();
	}
	set_status("success");
	return true;
    }
	
    bool early() { return false; }
    
    void update_distance_result()
    {
	vec3 amin, amax, bmin, bmax, ac, bc;
	ptlist pa, pb;
	wme_list::iterator i;
	double dist;
	a->get_world_points(pa);
	b->get_world_points(pb);
	wme_list children;
	bbox ba(pa), bb(pb);
	ba.get_vals(amin, amax);
	bb.get_vals(bmin, bmax);
	ac = calc_centroid(pa);
	bc = calc_centroid(pb);
	
	/*
	std::cout << "A: " << ac[0] << " " << ac[1] << " " << ac[2] << " " 
		  << std::endl;
	std::cout << "B: " << bc[0] << " " << bc[1] << " " << bc[2] << " " 
		  << std::endl;
	*/
	if (amax[axis] <= bmin[axis])
	{
	    dist = abs(amax[axis] - bmin[axis]);
	}
	else if (bmax[axis] <= amin[axis])
	{
	    dist = abs(bmax[axis] - amin[axis]);
	}
	else if ((amax[axis] < bmax[axis] && amax[axis] > bmin[axis]) ||
		 (bmax[axis] < amax[axis] && bmax[axis] > amin[axis]) ||
		 (amax[axis] == bmax[axis]) || (bmin[axis] == amin[axis]))
	{
	    dist = 0.0; // hmm think about this abs(ac[axis] - bc[axis]);
	}
	else
	{
	    std::cout << "ERROR bad case!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	    dist = 0.0;
	}
	if (res_root == NULL) {
	    sym_wme_pair p;
	    p = si->make_id_wme(root, "result");
	    res_root = p.first;
	}
	si->get_child_wmes(res_root, children);
	for (i = children.begin(); i != children.end(); ++i) {
	    //if (!si->get_val(si->get_wme_attr(*i), pname)) {
	    //continue;
	    //}
	    si->remove_wme(*i);
	}
	si->make_wme(res_root, "distance", dist);//vec_dist(ac,bc));
    }
    
    double vec_dist(vec3 i, vec3 j)
    {
	double distance = 0;
	for (int c = 0; c < 3; c++)
	{
	    distance += pow(i[c] - j[c], 2);
	}
	return pow(distance, 0.5);
    }
    vec3 calc_centroid(const ptlist &pts) {
	vec3 c = vec3::Zero();
	ptlist::const_iterator i;

	for (i = pts.begin(); i != pts.end(); ++i) {
	    c += *i;

	}

	c /= pts.size();

	return c;
    }
    
    bool get_nodes(soar_interface *si, Symbol *root, scene *scn)
    {
	wme_list children, children2, params;
	wme_list::iterator i;
	wme_list::iterator j;
	string strval, pname, pname2, itype;
	Symbol* cval;
	Symbol* cval2;
	long intval;
	
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
	    if (!si->get_val(si->get_wme_attr(*i), pname)) {
		continue;
	    }
	    if ((pname.compare("a") == 0) && a == NULL)
	    {
		cval = si->get_wme_val(*i);
		si->get_child_wmes(cval, children2);
		for (j = children2.begin(); j != children2.end(); ++j) {
		    if (!si->get_val(si->get_wme_attr(*j), pname2)) {
			continue;
		    }
		    cval2 = si->get_wme_val(*j);
		    if (si->get_val(cval2, strval)) {
			if ((pname2.compare("name") == 0) && a == NULL) {
			    a = scn->get_node(strval);
			}
		    }
		}
	    }
	    else if ((pname.compare("b") == 0) && b == NULL)
	    {
		cval = si->get_wme_val(*i);
		si->get_child_wmes(cval, children2);
		for (j = children2.begin(); j != children2.end(); ++j) {
		    if (!si->get_val(si->get_wme_attr(*j), pname2)) {
			continue;
		    }
		    cval2 = si->get_wme_val(*j);
		    if (si->get_val(cval2, strval)) {
			if ((pname2.compare("name") == 0) && b == NULL) {
			    b = scn->get_node(strval);
			}
		    }
		}
	    }
	    else if ((pname.compare("axis") == 0) && axis == -1)
	    {
		cval = si->get_wme_val(*i);
		if (si->get_val(cval, intval)) {
		    axis = (int) intval;
		}
	    }
	}
	if (a != NULL && b != NULL && axis != -1)
	    return true;
	
	return false;
    }
        
    void clear_results() {
	//fltrs.clear();
    }
	
private:
    Symbol         *root;
    Symbol         *res_root;
    svs_state      *state;
    soar_interface *si;
    
    bool            first;
    int axis;
    sgnode *a;
    sgnode *b;
};

command *_make_extract_distance_command_(svs_state *state, Symbol *root) {
    return new extract_distance_command(state, root);
}
