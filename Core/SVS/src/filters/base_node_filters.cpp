
#include "filters/base_node_filters.h"

#include <iostream>
using namespace std;

bool node_test_filter::compute(const filter_params* p, bool& out)
{
    sgnode* a = NULL;
    sgnode* b = NULL;

    if (!get_filter_param(this, p, "a", a)
            || !get_filter_param(this, p, "b", b))
    {
        set_status("Need nodes a and b as input");
        return false;
    }
    out = test(a, b, p);
    return true;
}


bool node_test_select_filter::compute(const filter_params* p, sgnode*& out, bool& select)
{
    sgnode* a = NULL;
    sgnode* b = NULL;
    if (!get_filter_param(this, p, "a", a)
            || !get_filter_param(this, p, "b", b))
    {
        set_status("Need nodes a and b as input");
        return false;
    }
    out = b;
    if (test(a, b, p) == select_true)
    {
        select = true;
    }
    else
    {
        select = false;
    }
    return true;
}


bool node_comparison_filter::compute(const filter_params* p, double& out)
{
    sgnode* a = NULL;
    sgnode* b = NULL;
    if (!get_filter_param(this, p, "a", a)
            || !get_filter_param(this, p, "b", b))
    {
        set_status("Need nodes a and b as input");
        return false;
    }
    out = comp(a, b, p);
    return true;
}

bool node_comparison_select_filter::compute(const filter_params* p, sgnode*& out, bool& select)
{
	sgnode* a = NULL;
	sgnode* b = NULL;
    if (!get_filter_param(this, p, "a", a)
            || !get_filter_param(this, p, "b", b))
    {
        set_status("Need nodes a and b as input");
        return false;
    }
    
    double sel_min;
    if (!get_filter_param(this, p, "min", sel_min))
    {
        sel_min = range_min;
    }
    
    double sel_max;
    if (!get_filter_param(this, p, "max", sel_max))
    {
        sel_max = range_max;
    }
    
    double res = comp(a, b, p);
    
    out = b;
    select = (sel_min <= res && res <= sel_max);
    return true;
}


bool node_comparison_rank_filter::rank(const filter_params* p, double& r)
{
	sgnode* a = NULL;
	sgnode* b = NULL;
    if (!get_filter_param(this, p, "a", a)
            || !get_filter_param(this, p, "b", b))
    {
        set_status("Need nodes a and b as input");
        return false;
    }
    
    r = comp(a, b, p);
    return true;
}

bool node_evaluation_filter::compute(const filter_params* p, double& out)
{
	sgnode* a = NULL;
    if (!get_filter_param(this, p, "a", a))
    {
        set_status("Need node a input");
        return false;
    }
    out = eval(a, p);
    return true;
}


bool node_evaluation_select_filter::compute(const filter_params* p, sgnode*& out, bool& select)
{
	sgnode* a = NULL;
    if (!get_filter_param(this, p, "a", a))
    {
        set_status("Need node a input");
        return false;
    }
    
    double sel_min;
    if (!get_filter_param(this, p, "min", sel_min))
    {
        sel_min = range_min;
    }
    
    double sel_max;
    if (!get_filter_param(this, p, "max", sel_max))
    {
        sel_max = range_max;
    }
    
    double res = eval(a, p);
    out = a;
    select = (sel_min <= res && res <= sel_max);
    return true;
}


bool node_evaluation_rank_filter::rank(const filter_params* p, double& r)
{
	sgnode* a = NULL;
    if (!get_filter_param(this, p, "a", a))
    {
        set_status("Need node a as input");
        return false;
    }
    
    r = eval(a, p);
    return true;
}

