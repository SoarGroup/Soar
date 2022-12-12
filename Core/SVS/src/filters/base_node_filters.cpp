
#include "filters/base_node_filters.h"

#include <iostream>

void node_select_range_filter::set_range_from_params(const filter_params* p){
    double sel_min;
    if (get_filter_param(this, p, "min", sel_min))
    {
			range_min = sel_min;
    }

    double sel_max;
    if (get_filter_param(this, p, "max", sel_max))
    {
			range_max = sel_max;
    }

		std::string incl_min;
		if(get_filter_param(this, p, "include_min", incl_min))
		{
			include_min = (incl_min == "false" ? false : true);
		}

		std::string incl_max;
		if(get_filter_param(this, p, "include_max", incl_max))
		{
			include_max = (incl_max == "false" ? false : true);
		}
}

bool node_select_range_filter::falls_in_range(double val){
	if(include_min && val < range_min)
		return false;
	if(!include_min && val <= range_min)
		return false;
	if(include_max && val > range_max)
		return false;
	if(!include_max && val >= range_max)
		return false;
	return true;
}


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

		set_range_from_params(p);

		double res = comp(a, b, p);
		out = b;
		select = falls_in_range(res);
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

		set_range_from_params(p);

    double res = eval(a, p);
		out = a;
		select = falls_in_range(res);
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

