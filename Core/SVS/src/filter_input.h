/*****************************************************************
 *
 * File SVS/src/filter_input.h
 *
 * Contains:
 *   type  filter_params = vector< pair<string, filter_val*> >
 *   class filter_input
 *     class null_filter_input
 *     class concat_filter_input
 *     class product_filter_input
 *
 * filter_params is a list of (name, value) pairs consisting
 *   of one logical input into a filter
 *
 * the filter_input takes the inputs into a filter and creates
 *   a set of filter_params that the filter will act upon and
 *   create an output for each one
 *
 * null_filter_input - produces no input into a filter
 * concat_filter_input - puts all inputs together in one filter_params item
 * product_filter_input - creates a cartesian product of all inputs
 *                        and creates a filter_params for each one
 *
 *****************************************************************/
#ifndef FILTER_INPUT_H
#define FILTER_INPUT_H

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <sstream>
#include <iterator>

#include "common.h"
#include "change_tracking_list.h"

#include "filter_val.h"
class filter;

/*
 A filter parameter set represents one complete input into a filter. It's
 just a list of pairs <parameter name, value>.
*/
typedef std::vector<std::pair<std::string, filter_val*> > filter_params;

/*
 Each filter takes a number of input parameters. Each of those parameters
 is in the form of an output list. The derived classes of this abstract
 base class are responsible for combining those separate output lists into
 a single list of parameter sets. For example, the output parameter set
 could be the Cartesian product of all elements in each input list.

 I'm assuming that this class owns the memory of the filters that are
 added to it.
*/
class filter_input : public change_tracking_list<filter_params>
{
    public:
        struct param_info
        {
            std::string name;
            filter* in_fltr;
        };
        
        typedef std::vector<param_info>        input_table;
        
        virtual ~filter_input();
        
        bool update();
        void add_param(std::string name, filter* f);
        
        virtual void combine(const input_table& inputs) = 0;
        
        virtual void clear();
        
    private:
        input_table input_info;
};


class null_filter_input : public filter_input
{
    public:
        void combine(const input_table& inputs) {}
};

/*
 Input class that just concatenates all separate lists into
 a single parameter set list, with each parameter set being a single
 element.
*/
class concat_filter_input : public filter_input
{
    public:
        void combine(const input_table& inputs);
        
    private:
        void reset_sub();
        void clear_sub();
        
        std::map<filter_val*, filter_params*> val2params;
};

/*
 Input class that takes the Cartesian product of all input lists.
*/
class product_filter_input : public filter_input
{
    public:
        void combine(const input_table& inputs);
        
    private:
        void gen_new_combinations(const input_table& inputs);
        void erase_param_set(filter_params* s);
        void reset_sub();
        void clear_sub();
        
        typedef std::list<filter_params*> param_set_list;

        typedef std::map<filter_val*, param_set_list > val2param_map;
        val2param_map val2params;
        
        //void printVal2Params()
};


#endif
