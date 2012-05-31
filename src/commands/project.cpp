#include <iostream>
#include "command.h"
#include "filter.h"
#include "svs.h"
#include <list>

using namespace std;

class project_command : public command {
public:
    project_command(svs_state *state, Symbol *root)
	: command(state, root), root(root), state(state), a(NULL), b(NULL), res_root(NULL), first(true)
    {
	si = state->get_svs()->get_soar_interface();
    }
	
    ~project_command() {
/*
	if (a != NULL)
	    delete a;
	if (b != NULL)
	    delete b;
*/
	if (!fltrs.empty()) {
	    fltrs.clear();
	}
    }
	
    string description() {
	return string("projection");
    }
	
    bool update() {
	
	if (first) {
	    if (!fltrs.empty()) {
		fltrs.clear();
	    }
	    
	    std::list<string> tlist;
	    if (!get_filter_types(state->get_svs()->get_soar_interface(), root,
				  tlist))
	    {
		set_status("incorrect filter syntax");
		return false;
	    }
	    std::list<string>::iterator i = tlist.begin();
	    while (i != tlist.end())
	    {
		filter *f = parse_project_filter_spec(
		    state->get_svs()->get_soar_interface(), root, 
		    state->get_scene(), *i, "");
		if (f == NULL)
		{
		    set_status("incorrect filter syntax");
		    return false;
		}
	
		fltrs.push_back(f);
		i++;
	    }

	}
	if (a == NULL || b == NULL)
	{
	    std::cout << "CANNOT FIND A OR B" << std::endl;
	    set_status("a,b incorrect filter syntax");
	}
	if (first)
	{
	    first =false;
	    update_position_result();
	}

	return true;
    }
	
    bool early() { return false; }
    
    void update_position_result()
    {
	std::list<filter*>::iterator i;
	int axis, comp;
	int axes[3] = {-10, -10, -10};
	for (i = fltrs.begin(); i != fltrs.end(); ++i) {
	    filter *f = (filter *) *i;
	    axis = f->getAxis();
	    comp = f->getComp();
	    if (axis >= 0 && comp > -3)
	    {
		axes[axis] = comp;
	    }
	}
	vec3 amin, amax, bmin, bmax, ac, bc;
	ptlist pa, pb;
	
	a->get_world_points(pa);
	b->get_world_points(pb);
	
	bbox ba(pa), bb(pb);
	ba.get_vals(amin, amax);
	bb.get_vals(bmin, bmax);
	ac = calc_centroid(pa);
	bc = calc_centroid(pb);
	    
	double pos[3];
	for (int i = 0; i < 3; i++)
	{
	    //unspecified along axes or aligned
	    if (axes[i] == -10 || axes[i] == 0)
	    {
		//as default use the b's center location
		pos[i] = bc[i];
	    }
	    else if (axes[i] == 1)
	    {
		// width of both objects + 10% positive
		double dist = (vec_dist(bc, bmax) + vec_dist(ac, amax));
		pos[i] = bc[i] + dist;
	    }
	    else if (axes[i] == -1)
	    {
		// width of both objects + 10% negative
		double dist = (vec_dist(bc, bmax) + vec_dist(ac, amax));
		pos[i] = bc[i] - dist;
	    }
	}
	
	if (res_root == NULL) {
	    sym_wme_pair p;
	    p = si->make_id_wme(root, "result");
	    res_root = p.first;
	}
	
	si->make_wme(res_root, "x", pos[0]);
	si->make_wme(res_root, "y", pos[1]);
	si->make_wme(res_root, "z", pos[2]);
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
	vec3 c;
	ptlist::const_iterator i;
	for (i = pts.begin(); i != pts.end(); ++i) {
	    c += *i;
	}
		c /= pts.size();
		return c;
    }
    
    filter *parse_project_filter_spec(soar_interface *si, Symbol *root, 
				      scene *scn, string ftype, 
				      string parent_pname) {
	wme_list children, params;
	wme_list::iterator i;
	Symbol* cval;
	string strval, pname, itype;
	long intval;
	float floatval;
	filter_input *input;
	bool fail;
	filter *f;
	
	fail = false;
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
	    if (!si->get_val(si->get_wme_attr(*i), pname)) {
		continue;
	    }
	    cval = si->get_wme_val(*i);
	    if (pname == "type") {
		
	    } else if (pname == "input-type") {
		if (!si->get_val(cval, itype)) {
		    return NULL;
		}
	    } else if (pname != "status" && pname != "result") {
		params.push_back(*i);
	    }
	}
	
	if (itype == "concat") {
	    input = new concat_filter_input();
	} else if (params.size() == 0) {
	    input = new null_filter_input();
	} else {
	    input = new product_filter_input();
	}
	
	for (i = params.begin(); i != params.end(); ++i) {
	    if (!si->get_val(si->get_wme_attr(*i), pname)) {
		continue;
	    }
	    cval = si->get_wme_val(*i);
	    if (si->get_val(cval, strval)) {
		input->add_param(pname, new const_filter<string>(strval));
		if ((parent_pname.compare("a") == 0) && 
		    a == NULL && (pname.compare("name") == 0))
		{
		    a = scn->get_node(strval);
		}
		if ((parent_pname.compare("b") == 0) && 
		    b == NULL && (pname.compare("name") == 0))
		{
		    b = scn->get_node(strval);
		}
	    } else if (si->get_val(cval, intval)) {
		input->add_param(pname, new const_filter<int>(intval));
	    } else if (si->get_val(cval, floatval)) {
		input->add_param(pname, new const_filter<float>(floatval));
	    } else {
		filter *cf;
		// must be identifier
		if ((cf = parse_project_filter_spec(si, cval, scn, ftype, pname)) 
		    == NULL) {
		    fail = true;
		    break;
		}
		input->add_param(pname, cf);
	    }	    
	    
	}
	
	if (!fail) {
	    f = get_filter_table().make_filter(ftype, scn, input);
	}
	
	if (fail || ftype == "" || f == NULL) {
	    delete input;
	    return NULL;
	}
	return f;
    }
    
    bool get_filter_types(soar_interface *si, Symbol *root, 
			  std::list<string> &tlist) {
	wme_list children;
	wme_list::iterator i;
	Symbol* cval;
	string pname, ftype;
	bool pass = false;
		
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
	    if (!si->get_val(si->get_wme_attr(*i), pname)) {
		continue;
	    }
	    cval = si->get_wme_val(*i);
	    if (pname == "type") {
		if (!si->get_val(cval, ftype)) {
		    continue;
		}
		pass = true;
		tlist.push_back(ftype);
	    }
	}
	return pass;
    }
    
    void clear_results() {
	fltrs.clear();
    }
	
private:
    Symbol         *root;
    Symbol         *res_root;
    svs_state      *state;
    soar_interface *si;
    //filter         *fltr;
    std::list<filter*>   fltrs;
    //filter_result  *res;
    bool            first;
    sgnode *a;
    sgnode *b;
    //std::map<filter_val*, wme*> res2wme;
};

command *_make_project_command_(svs_state *state, Symbol *root) {
    return new project_command(state, root);
}
