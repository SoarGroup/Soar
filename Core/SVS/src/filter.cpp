#include "filter.h"

#include <sstream>
#include <iterator>
#include <utility>
#include <iostream>

#include "scene.h"
#include "sgnode.h"

using namespace std;

/**********************************************
 * member functions for filter_val_c<sgnode*>
 ***********************************************/

void filter_val_c<const sgnode*>::get_rep(map<string, string>& rep) const
{
    rep.clear();
    rep[""] = v->get_name();
}

string filter_val_c<const sgnode*>::toString() const
{
    stringstream ss;
    ss << v;
    return ss.str();
}


/*******
 * filter
 ********/

  filter::filter(Symbol* root, soar_interface* si, filter_input* in)
  : root(root), si(si), status_wme(NULL), input(in)
{
  if(input == NULL){
    input = new null_filter_input();
  }
  if(root && si){
    si->find_child_wme(root, "status", status_wme);
  }
}

filter::~filter(){
  delete input;
}

void filter::set_status(const std::string& msg){
  if(status == msg){
    return;
  }
  status = msg;
  if(status_wme){
    si->remove_wme(status_wme);
  }
  if(root && si){
    status_wme = si->make_wme(root, si->get_common_syms().status, status);
  }
}

bool filter::update()
{
    if (!input->update())
    {
        set_status("Errors in input");
        clear_output();
        return false;
    }
    
    if (!update_outputs())
    {
        input->reset();
        clear_output();
        return false;
    }
    set_status("success");
    input->clear_changes();
    return true;
}
