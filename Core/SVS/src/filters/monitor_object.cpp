#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"
#include "filter_table.h"

using namespace std;

typedef std::map<const sgnode*, double> vol_map;
typedef std::map<const sgnode*, vec3> pos_map;

class monitor_volume_filter : public typed_map_filter<double>
{
    public:
        monitor_volume_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
            : typed_map_filter<double>(root, si, input), scn(scn)
        {
        }
        
        bool compute(const filter_params* p, bool adding, double& res, bool& changed)
        {
            double newres;
            const sgnode* a;
            
            if (!get_filter_param(this, p, "a", a))
            {
                set_status("expecting parameter a");
                return false;
            }
            
            vec3 scale = a->get_trans('s');
            double curVol = scale[0] * scale[1] * scale[2];
            
            vol_map::const_iterator it = savedVolumes.find(a);
            if (it == savedVolumes.end())
            {
                savedVolumes[a] = curVol;
                newres = 1;
            }
            else
            {
                double savedVol = it->second;
                //std::cout << "VOL: " << curVol << ", " << savedVol << std::endl;
                if (savedVol < .000000001)
                {
                    newres = 1;
                }
                else
                {
                    newres = curVol / savedVol;
                }
            }
            
            changed = (res != newres);
            res = newres;
            return true;
        }
        
    private:
        vol_map savedVolumes;
        
        scene* scn;
};

class monitor_position_filter : public typed_map_filter<double>
{
    public:
        monitor_position_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
            : typed_map_filter<double>(root, si, input), scn(scn)
        {
        }
        
        bool compute(const filter_params* p, bool adding, double& res, bool& changed)
        {
            double newres;
            const sgnode* a;
            
            if (!get_filter_param(this, p, "a", a))
            {
                set_status("expecting parameter a");
                return false;
            }
            
            vec3 pos = a->get_trans('p');
            
            pos_map::const_iterator it = savedPositions.find(a);
            if (it == savedPositions.end())
            {
                savedPositions[a] = pos;
                newres = 0;
            }
            else
            {
                vec3 savedPos = it->second;
                vec3 diff = pos - savedPos;
                newres = sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
            }
            
            changed = (res != newres);
            res = newres;
            return true;
        }
        
    private:
        pos_map savedPositions;
        
        scene* scn;
};

filter* make_monitor_position_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new monitor_position_filter(root, si, scn, input);
}

filter* make_monitor_volume_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new monitor_volume_filter(root, si, scn, input);
}

filter_table_entry* monitor_position_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "monitor_position";
    e->parameters.push_back("a");
    e->ordered = false;
    e->allow_repeat = false;
    e->create = &make_monitor_position_filter;
    return e;
}

filter_table_entry* monitor_volume_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "monitor_volume";
    e->parameters.push_back("a");
    e->ordered = false;
    e->allow_repeat = false;
    e->create = &make_monitor_volume_filter;
    return e;
}

