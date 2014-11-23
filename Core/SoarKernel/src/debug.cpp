/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
                       debug.cpp

   @brief debug.cpp provides some utility functions for inspecting and
          manipulating the data structures of the Soar kernel at run
          time.

------------------------------------------------------------------ */

#include "debug.h"
#include "debug_defines.h"
#include "agent.h"
#include "test.h"
#include "variablization_manager.h"
#include "episodic_memory.h"
#include "soar_module.h"
#include "lexer.h"
#include "soar_instance.h"
#include "print.h"
#include "output_manager.h"
#include <string>
#include <iostream>
#include <sstream>


/* -- For stack trace printing (works on Mac.  Not sure about other platforms) --*/
#ifdef DEBUG_MAC_STACKTRACE
#include <execinfo.h>
#include <cxxabi.h>
#include "debug_trace.h"
#endif

using namespace soar_module;

debug_param_container::debug_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    epmem_commands = new soar_module::boolean_param("epmem", off, new soar_module::f_predicate<boolean>());
    smem_commands = new soar_module::boolean_param("smem", off, new soar_module::f_predicate<boolean>());
    sql_commands = new soar_module::boolean_param("sql", off, new soar_module::f_predicate<boolean>());
    use_new_chunking = new soar_module::boolean_param("chunk", on, new soar_module::f_predicate<boolean>());

    add(epmem_commands);
    add(smem_commands);
    add(sql_commands);
    add(use_new_chunking);
}

#include "sqlite3.h"

#define DEBUG_BUFFER_SIZE 5000

static Output_Manager* thisOutput_Manager = NULL;

//#define dprint_testspeed(format, args...) dprint (DT_DEBUG, format , ##args)
#define dprint_testspeed(format, args...) dprint_y (DT_DEBUG, format , ##args)
//#define dprint_testspeed(format, args...) Output_Manager::Get_OM().printv_y (format , ##args)

//#define dprint_testspeed(format, args...) dprint_macro (DT_DEBUG, format , ##args)

//#define dprint_testspeed(format, args...) print (debug_agent, format , ##args)
//#define dprint_testspeed(format, args...) print_with_symbols (debug_agent, format , ##args)
//#define dprint_testspeed(format, args...) print_old (debug_agent, format , ##args)

//#define dprint_testspeed(format, args...) Output_Manager::Get_OM().printv_agent(debug_agent, format , ##args)
//#define dprint_testspeed(format, args...) thisOutput_Manager->printv_agent(debug_agent, format , ##args)

void test_print_speed()
{
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }
    thisOutput_Manager = &(Output_Manager::Get_OM());

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID02  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID03  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID04  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID05  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID06  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID07  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID08  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newStr01 = make_str_constant(debug_agent, "attr1");
    Symbol* newStr02 = make_str_constant(debug_agent, "attr2");
    Symbol* newStr03 = make_str_constant(debug_agent, "attr3");
    Symbol* newStr04 = make_str_constant(debug_agent, "attr4");
    Symbol* newStr05 = make_str_constant(debug_agent, "str1");
    Symbol* newStr06 = make_str_constant(debug_agent, "str2");
    Symbol* newStr07 = make_str_constant(debug_agent, "str3");
    Symbol* newStr08 = make_str_constant(debug_agent, "str4");
    Symbol* newVar01 = make_variable(debug_agent, "var1");
    Symbol* newVar02 = make_variable(debug_agent, "var2");
    Symbol* newVar03 = make_variable(debug_agent, "var3");
    Symbol* newVar04 = make_variable(debug_agent, "var4");
    Symbol* newVar05 = make_variable(debug_agent, "var5");
    Symbol* newVar06 = make_variable(debug_agent, "var6");
    Symbol* newVar07 = make_variable(debug_agent, "var7");
    Symbol* newVar08 = make_variable(debug_agent, "var8");
    Symbol* newInt01 = make_int_constant(debug_agent, 1);
    Symbol* newInt02 = make_int_constant(debug_agent, 2);
    Symbol* newInt03 = make_int_constant(debug_agent, 3);
    Symbol* newInt04 = make_int_constant(debug_agent, 4);
    Symbol* newInt05 = make_int_constant(debug_agent, 5);
    Symbol* newInt06 = make_int_constant(debug_agent, 6);
    Symbol* newInt07 = make_int_constant(debug_agent, 7);
    Symbol* newInt08 = make_int_constant(debug_agent, 8);
    Symbol* newFloat01 = make_float_constant(debug_agent, 1.1231);
    Symbol* newFloat02 = make_float_constant(debug_agent, 2.3);
    Symbol* newFloat03 = make_float_constant(debug_agent, 3.3);
    Symbol* newFloat04 = make_float_constant(debug_agent, 4.1783000421);
    Symbol* newFloat05 = make_float_constant(debug_agent, 5.5555);
    Symbol* newFloat06 = make_float_constant(debug_agent, 6.66);
    Symbol* newFloat07 = make_float_constant(debug_agent, 7.1234567890);
    Symbol* newFloat08 = make_float_constant(debug_agent, 8.00000000001);

    uint64_t num_iterations = 10000;

    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("This is just a plain string.\n");
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Identifier: %s\n", newID01->to_string());
        dprint_testspeed("Identifier: %s\n", newID02->to_string());
        dprint_testspeed("Identifier: %s\n", newID03->to_string());
        dprint_testspeed("Identifier: %s\n", newID04->to_string());
        dprint_testspeed("Identifier: %s\n", newID05->to_string());
        dprint_testspeed("Identifier: %s\n", newID06->to_string());
        dprint_testspeed("Identifier: %s\n", newID07->to_string());
        dprint_testspeed("Identifier: %s\n", newID08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Identifiers: %s %s %s %s %s %s %s %s\n",
            newID01->to_string(), newID02->to_string(), newID03->to_string(), newID04->to_string(),
            newID05->to_string(), newID06->to_string(), newID07->to_string(), newID08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("String: %s\n", newStr01->to_string());
        dprint_testspeed("String: %s\n", newStr02->to_string());
        dprint_testspeed("String: %s\n", newStr03->to_string());
        dprint_testspeed("String: %s\n", newStr04->to_string());
        dprint_testspeed("String: %s\n", newStr05->to_string());
        dprint_testspeed("String: %s\n", newStr06->to_string());
        dprint_testspeed("String: %s\n", newStr07->to_string());
        dprint_testspeed("String: %s\n", newStr08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Strings: %s %s %s %s %s %s %s %s\n",
            newStr01->to_string(), newStr02->to_string(), newStr03->to_string(), newStr04->to_string(),
            newStr05->to_string(), newStr06->to_string(), newStr07->to_string(), newStr08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Variable: %s\n", newVar01->to_string());
        dprint_testspeed("Variable: %s\n", newVar02->to_string());
        dprint_testspeed("Variable: %s\n", newVar03->to_string());
        dprint_testspeed("Variable: %s\n", newVar04->to_string());
        dprint_testspeed("Variable: %s\n", newVar05->to_string());
        dprint_testspeed("Variable: %s\n", newVar06->to_string());
        dprint_testspeed("Variable: %s\n", newVar07->to_string());
        dprint_testspeed("Variable: %s\n", newVar08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Variables: %s %s %s %s %s %s %s %s\n",
            newVar01->to_string(), newVar02->to_string(), newVar03->to_string(), newVar04->to_string(),
            newVar05->to_string(), newVar06->to_string(), newVar07->to_string(), newVar08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Integer: %s\n", newInt01->to_string());
        dprint_testspeed("Integer: %s\n", newInt02->to_string());
        dprint_testspeed("Integer: %s\n", newInt03->to_string());
        dprint_testspeed("Integer: %s\n", newInt04->to_string());
        dprint_testspeed("Integer: %s\n", newInt05->to_string());
        dprint_testspeed("Integer: %s\n", newInt06->to_string());
        dprint_testspeed("Integer: %s\n", newInt07->to_string());
        dprint_testspeed("Integer: %s\n", newInt08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Integers: %s %s %s %s %s %s %s %s\n",
            newInt01->to_string(), newInt02->to_string(), newInt03->to_string(), newInt04->to_string(),
            newInt05->to_string(), newInt06->to_string(), newInt07->to_string(), newInt08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Float: %s\n", newFloat01->to_string());
        dprint_testspeed("Float: %s\n", newFloat02->to_string());
        dprint_testspeed("Float: %s\n", newFloat03->to_string());
        dprint_testspeed("Float: %s\n", newFloat04->to_string());
        dprint_testspeed("Float: %s\n", newFloat05->to_string());
        dprint_testspeed("Float: %s\n", newFloat06->to_string());
        dprint_testspeed("Float: %s\n", newFloat07->to_string());
        dprint_testspeed("Float: %s\n", newFloat08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Floats: %s %s %s %s %s %s %s %s\n",
            newFloat01->to_string(), newFloat02->to_string(), newFloat03->to_string(), newFloat04->to_string(),
            newFloat05->to_string(), newFloat06->to_string(), newFloat07->to_string(), newFloat08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Identifier: %s\n", newID01->to_string());
        dprint_testspeed("String: %s\n", newStr01->to_string());
        dprint_testspeed("Variable: %s\n", newVar01->to_string());
        dprint_testspeed("Integer: %s\n", newInt01->to_string());
        dprint_testspeed("Float: %s\n", newFloat01->to_string());

        dprint_testspeed("Identifier: %s\n", newID02->to_string());
        dprint_testspeed("String: %s\n", newStr02->to_string());
        dprint_testspeed("Variable: %s\n", newVar02->to_string());
        dprint_testspeed("Integer: %s\n", newInt02->to_string());
        dprint_testspeed("Float: %s\n", newFloat02->to_string());

        dprint_testspeed("Identifier: %s\n", newID03->to_string());
        dprint_testspeed("String: %s\n", newStr03->to_string());
        dprint_testspeed("Variable: %s\n", newVar03->to_string());
        dprint_testspeed("Integer: %s\n", newInt03->to_string());
        dprint_testspeed("Float: %s\n", newFloat03->to_string());

        dprint_testspeed("Identifier: %s\n", newID04->to_string());
        dprint_testspeed("String: %s\n", newStr04->to_string());
        dprint_testspeed("Variable: %s\n", newVar04->to_string());
        dprint_testspeed("Integer: %s\n", newInt04->to_string());
        dprint_testspeed("Float: %s\n", newFloat04->to_string());

        dprint_testspeed("Identifier: %s\n", newID05->to_string());
        dprint_testspeed("String: %s\n", newStr05->to_string());
        dprint_testspeed("Variable: %s\n", newVar05->to_string());
        dprint_testspeed("Integer: %s\n", newInt05->to_string());
        dprint_testspeed("Float: %s\n", newFloat05->to_string());

        dprint_testspeed("Identifier: %s\n", newID06->to_string());
        dprint_testspeed("String: %s\n", newStr06->to_string());
        dprint_testspeed("Variable: %s\n", newVar06->to_string());
        dprint_testspeed("Integer: %s\n", newInt06->to_string());
        dprint_testspeed("Float: %s\n", newFloat06->to_string());

        dprint_testspeed("Identifier: %s\n", newID07->to_string());
        dprint_testspeed("String: %s\n", newStr07->to_string());
        dprint_testspeed("Variable: %s\n", newVar07->to_string());
        dprint_testspeed("Integer: %s\n", newInt07->to_string());
        dprint_testspeed("Float: %s\n", newFloat07->to_string());

        dprint_testspeed("Identifier: %s\n", newID08->to_string());
        dprint_testspeed("String: %s\n", newStr08->to_string());
        dprint_testspeed("Variable: %s\n", newVar08->to_string());
        dprint_testspeed("Integer: %s\n", newInt08->to_string());
        dprint_testspeed("Float: %s\n", newFloat08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Mixed: %s %s %s %s %s\n", newID01->to_string(), newStr01->to_string(),
            newVar01->to_string(), newInt01->to_string(), newFloat01->to_string());
        dprint_testspeed("Mixed: %s %s %s %s %s\n", newID02->to_string(), newStr02->to_string(),
            newVar02->to_string(), newInt02->to_string(), newFloat01->to_string());
        dprint_testspeed("Mixed: %s %s %s %s %s\n", newID03->to_string(), newStr03->to_string(),
            newVar03->to_string(), newInt03->to_string(), newFloat01->to_string());
        dprint_testspeed("Mixed: %s %s %s %s %s\n", newID04->to_string(), newStr04->to_string(),
            newVar04->to_string(), newInt04->to_string(), newFloat01->to_string());
        dprint_testspeed("Mixed: %s %s %s %s %s\n", newID05->to_string(), newStr05->to_string(),
            newVar05->to_string(), newInt05->to_string(), newFloat01->to_string());
        dprint_testspeed("Mixed: %s %s %s %s %s\n", newID06->to_string(), newStr06->to_string(),
            newVar06->to_string(), newInt06->to_string(), newFloat01->to_string());
        dprint_testspeed("Mixed: %s %s %s %s %s\n", newID07->to_string(), newStr07->to_string(),
            newVar07->to_string(), newInt07->to_string(), newFloat01->to_string());
        dprint_testspeed("Mixed: %s %s %s %s %s\n", newID08->to_string(), newStr08->to_string(),
            newVar08->to_string(), newInt08->to_string(), newFloat01->to_string());
    }

}

void test_print_speed_y()
{

    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }
    thisOutput_Manager = &(Output_Manager::Get_OM());

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID02  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID03  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID04  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID05  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID06  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID07  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID08  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newStr01 = make_str_constant(debug_agent, "attr1");
    Symbol* newStr02 = make_str_constant(debug_agent, "attr2");
    Symbol* newStr03 = make_str_constant(debug_agent, "attr3");
    Symbol* newStr04 = make_str_constant(debug_agent, "attr4");
    Symbol* newStr05 = make_str_constant(debug_agent, "str1");
    Symbol* newStr06 = make_str_constant(debug_agent, "str2");
    Symbol* newStr07 = make_str_constant(debug_agent, "str3");
    Symbol* newStr08 = make_str_constant(debug_agent, "str4");
    Symbol* newVar01 = make_variable(debug_agent, "var1");
    Symbol* newVar02 = make_variable(debug_agent, "var2");
    Symbol* newVar03 = make_variable(debug_agent, "var3");
    Symbol* newVar04 = make_variable(debug_agent, "var4");
    Symbol* newVar05 = make_variable(debug_agent, "var5");
    Symbol* newVar06 = make_variable(debug_agent, "var6");
    Symbol* newVar07 = make_variable(debug_agent, "var7");
    Symbol* newVar08 = make_variable(debug_agent, "var8");
    Symbol* newInt01 = make_int_constant(debug_agent, 1);
    Symbol* newInt02 = make_int_constant(debug_agent, 2);
    Symbol* newInt03 = make_int_constant(debug_agent, 3);
    Symbol* newInt04 = make_int_constant(debug_agent, 4);
    Symbol* newInt05 = make_int_constant(debug_agent, 5);
    Symbol* newInt06 = make_int_constant(debug_agent, 6);
    Symbol* newInt07 = make_int_constant(debug_agent, 7);
    Symbol* newInt08 = make_int_constant(debug_agent, 8);
    Symbol* newFloat01 = make_float_constant(debug_agent, 1.1231);
    Symbol* newFloat02 = make_float_constant(debug_agent, 2.3);
    Symbol* newFloat03 = make_float_constant(debug_agent, 3.3);
    Symbol* newFloat04 = make_float_constant(debug_agent, 4.1783000421);
    Symbol* newFloat05 = make_float_constant(debug_agent, 5.5555);
    Symbol* newFloat06 = make_float_constant(debug_agent, 6.66);
    Symbol* newFloat07 = make_float_constant(debug_agent, 7.1234567890);
    Symbol* newFloat08 = make_float_constant(debug_agent, 8.00000000001);

    uint64_t num_iterations = 10000;

    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("This is just a plain string.\n");
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Identifier: %y\n", newID01);
        dprint_testspeed("Identifier: %y\n", newID02);
        dprint_testspeed("Identifier: %y\n", newID03);
        dprint_testspeed("Identifier: %y\n", newID04);
        dprint_testspeed("Identifier: %y\n", newID05);
        dprint_testspeed("Identifier: %y\n", newID06);
        dprint_testspeed("Identifier: %y\n", newID07);
        dprint_testspeed("Identifier: %y\n", newID08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Identifiers: %y %y %y %y %y %y %y %y\n",
            newID01, newID02, newID03, newID04,
            newID05, newID06, newID07, newID08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("String: %y\n", newStr01);
        dprint_testspeed("String: %y\n", newStr02);
        dprint_testspeed("String: %y\n", newStr03);
        dprint_testspeed("String: %y\n", newStr04);
        dprint_testspeed("String: %y\n", newStr05);
        dprint_testspeed("String: %y\n", newStr06);
        dprint_testspeed("String: %y\n", newStr07);
        dprint_testspeed("String: %y\n", newStr08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Strings: %y %y %y %y %y %y %y %y\n",
            newStr01, newStr02, newStr03, newStr04,
            newStr05, newStr06, newStr07, newStr08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Variable: %y\n", newVar01);
        dprint_testspeed("Variable: %y\n", newVar02);
        dprint_testspeed("Variable: %y\n", newVar03);
        dprint_testspeed("Variable: %y\n", newVar04);
        dprint_testspeed("Variable: %y\n", newVar05);
        dprint_testspeed("Variable: %y\n", newVar06);
        dprint_testspeed("Variable: %y\n", newVar07);
        dprint_testspeed("Variable: %y\n", newVar08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Variables: %y %y %y %y %y %y %y %y\n",
            newVar01, newVar02, newVar03, newVar04,
            newVar05, newVar06, newVar07, newVar08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Integer: %y\n", newInt01);
        dprint_testspeed("Integer: %y\n", newInt02);
        dprint_testspeed("Integer: %y\n", newInt03);
        dprint_testspeed("Integer: %y\n", newInt04);
        dprint_testspeed("Integer: %y\n", newInt05);
        dprint_testspeed("Integer: %y\n", newInt06);
        dprint_testspeed("Integer: %y\n", newInt07);
        dprint_testspeed("Integer: %y\n", newInt08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Integers: %y %y %y %y %y %y %y %y\n",
            newInt01, newInt02, newInt03, newInt04,
            newInt05, newInt06, newInt07, newInt08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Float: %y\n", newFloat01);
        dprint_testspeed("Float: %y\n", newFloat02);
        dprint_testspeed("Float: %y\n", newFloat03);
        dprint_testspeed("Float: %y\n", newFloat04);
        dprint_testspeed("Float: %y\n", newFloat05);
        dprint_testspeed("Float: %y\n", newFloat06);
        dprint_testspeed("Float: %y\n", newFloat07);
        dprint_testspeed("Float: %y\n", newFloat08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Floats: %y %y %y %y %y %y %y %y\n",
            newFloat01, newFloat02, newFloat03, newFloat04,
            newFloat05, newFloat06, newFloat07, newFloat08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Identifier: %y\n", newID01);
        dprint_testspeed("String: %y\n", newStr01);
        dprint_testspeed("Variable: %y\n", newVar01);
        dprint_testspeed("Integer: %y\n", newInt01);
        dprint_testspeed("Float: %y\n", newFloat01);

        dprint_testspeed("Identifier: %y\n", newID02);
        dprint_testspeed("String: %y\n", newStr02);
        dprint_testspeed("Variable: %y\n", newVar02);
        dprint_testspeed("Integer: %y\n", newInt02);
        dprint_testspeed("Float: %y\n", newFloat02);

        dprint_testspeed("Identifier: %y\n", newID03);
        dprint_testspeed("String: %y\n", newStr03);
        dprint_testspeed("Variable: %y\n", newVar03);
        dprint_testspeed("Integer: %y\n", newInt03);
        dprint_testspeed("Float: %y\n", newFloat03);

        dprint_testspeed("Identifier: %y\n", newID04);
        dprint_testspeed("String: %y\n", newStr04);
        dprint_testspeed("Variable: %y\n", newVar04);
        dprint_testspeed("Integer: %y\n", newInt04);
        dprint_testspeed("Float: %y\n", newFloat04);

        dprint_testspeed("Identifier: %y\n", newID05);
        dprint_testspeed("String: %y\n", newStr05);
        dprint_testspeed("Variable: %y\n", newVar05);
        dprint_testspeed("Integer: %y\n", newInt05);
        dprint_testspeed("Float: %y\n", newFloat05);

        dprint_testspeed("Identifier: %y\n", newID06);
        dprint_testspeed("String: %y\n", newStr06);
        dprint_testspeed("Variable: %y\n", newVar06);
        dprint_testspeed("Integer: %y\n", newInt06);
        dprint_testspeed("Float: %y\n", newFloat06);

        dprint_testspeed("Identifier: %y\n", newID07);
        dprint_testspeed("String: %y\n", newStr07);
        dprint_testspeed("Variable: %y\n", newVar07);
        dprint_testspeed("Integer: %y\n", newInt07);
        dprint_testspeed("Float: %y\n", newFloat07);

        dprint_testspeed("Identifier: %y\n", newID08);
        dprint_testspeed("String: %y\n", newStr08);
        dprint_testspeed("Variable: %y\n", newVar08);
        dprint_testspeed("Integer: %y\n", newInt08);
        dprint_testspeed("Float: %y\n", newFloat08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        dprint_testspeed("Mixed: %y %y %y %y %y\n", newID01, newStr01,
            newVar01, newInt01, newFloat01);
        dprint_testspeed("Mixed: %y %y %y %y %y\n", newID02, newStr02,
            newVar02, newInt02, newFloat01);
        dprint_testspeed("Mixed: %y %y %y %y %y\n", newID03, newStr03,
            newVar03, newInt03, newFloat01);
        dprint_testspeed("Mixed: %y %y %y %y %y\n", newID04, newStr04,
            newVar04, newInt04, newFloat01);
        dprint_testspeed("Mixed: %y %y %y %y %y\n", newID05, newStr05,
            newVar05, newInt05, newFloat01);
        dprint_testspeed("Mixed: %y %y %y %y %y\n", newID06, newStr06,
            newVar06, newInt06, newFloat01);
        dprint_testspeed("Mixed: %y %y %y %y %y\n", newID07, newStr07,
            newVar07, newInt07, newFloat01);
        dprint_testspeed("Mixed: %y %y %y %y %y\n", newID08, newStr08,
            newVar08, newInt08, newFloat01);
    }
}

void debug_test_structs()
{
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID02  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID03  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID04  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newID05  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID06  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID07  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newID08  = make_new_identifier(debug_agent, 'Z', 1, NIL);
    Symbol* newStr01 = make_str_constant(debug_agent, "attr1");
    Symbol* newStr02 = make_str_constant(debug_agent, "attr2");
    Symbol* newStr03 = make_str_constant(debug_agent, "attr3");
    Symbol* newStr04 = make_str_constant(debug_agent, "attr4");
    Symbol* newStr05 = make_str_constant(debug_agent, "str1");
    Symbol* newStr06 = make_str_constant(debug_agent, "str2");
    Symbol* newStr07 = make_str_constant(debug_agent, "str3");
    Symbol* newStr08 = make_str_constant(debug_agent, "str4");
    Symbol* newVar01 = make_variable(debug_agent, "var1");
    Symbol* newVar02 = make_variable(debug_agent, "var2");
    Symbol* newVar03 = make_variable(debug_agent, "var3");
    Symbol* newVar04 = make_variable(debug_agent, "var4");
    Symbol* newVar05 = make_variable(debug_agent, "var5");
    Symbol* newVar06 = make_variable(debug_agent, "var6");
    Symbol* newVar07 = make_variable(debug_agent, "var7");
    Symbol* newVar08 = make_variable(debug_agent, "var8");
    Symbol* newInt01 = make_int_constant(debug_agent, 1);
    Symbol* newInt02 = make_int_constant(debug_agent, 2);
    Symbol* newInt03 = make_int_constant(debug_agent, 3);
    Symbol* newInt04 = make_int_constant(debug_agent, 4);
    Symbol* newInt05 = make_int_constant(debug_agent, 5);
    Symbol* newInt06 = make_int_constant(debug_agent, 6);
    Symbol* newInt07 = make_int_constant(debug_agent, 7);
    Symbol* newInt08 = make_int_constant(debug_agent, 8);

    test idEqTest01 = make_test(debug_agent, newID01, EQUALITY_TEST);
    test idEqTest02 = make_test(debug_agent, newID02, EQUALITY_TEST);
    test idEqTest03 = make_test(debug_agent, newID03, EQUALITY_TEST);
    test idEqTest04 = make_test(debug_agent, newID04, EQUALITY_TEST);
    test idEqTest05 = make_test(debug_agent, newID05, EQUALITY_TEST);
    test idEqTest06 = make_test(debug_agent, newID06, EQUALITY_TEST);
    test idEqTest07 = make_test(debug_agent, newID07, EQUALITY_TEST);
    test idEqTest08 = make_test(debug_agent, newID08, EQUALITY_TEST);
    test strEqTest01 = make_test(debug_agent, newStr01, EQUALITY_TEST);
    test strEqTest02 = make_test(debug_agent, newStr02, EQUALITY_TEST);
    test strEqTest03 = make_test(debug_agent, newStr03, EQUALITY_TEST);
    test strEqTest04 = make_test(debug_agent, newStr04, EQUALITY_TEST);
    test strEqTest05 = make_test(debug_agent, newStr05, EQUALITY_TEST);
    test strEqTest06 = make_test(debug_agent, newStr06, EQUALITY_TEST);
    test strEqTest07 = make_test(debug_agent, newStr07, EQUALITY_TEST);
    test strEqTest08 = make_test(debug_agent, newStr08, EQUALITY_TEST);
    test varEqTest01 = make_test(debug_agent, newVar01, EQUALITY_TEST);
    test varEqTest02 = make_test(debug_agent, newVar02, EQUALITY_TEST);
    test varEqTest03 = make_test(debug_agent, newVar03, EQUALITY_TEST);
    test varEqTest04 = make_test(debug_agent, newVar04, EQUALITY_TEST);
    test varEqTest05 = make_test(debug_agent, newVar05, EQUALITY_TEST);
    test varEqTest06 = make_test(debug_agent, newVar06, EQUALITY_TEST);
    test varEqTest07 = make_test(debug_agent, newVar07, EQUALITY_TEST);
    test varEqTest08 = make_test(debug_agent, newVar08, EQUALITY_TEST);
    test intEqTest01 = make_test(debug_agent, newInt01, EQUALITY_TEST);
    test intEqTest02 = make_test(debug_agent, newInt02, EQUALITY_TEST);
    test intEqTest03 = make_test(debug_agent, newInt03, EQUALITY_TEST);
    test intEqTest04 = make_test(debug_agent, newInt04, EQUALITY_TEST);
    test intEqTest05 = make_test(debug_agent, newInt05, EQUALITY_TEST);
    test intEqTest06 = make_test(debug_agent, newInt06, EQUALITY_TEST);
    test intEqTest07 = make_test(debug_agent, newInt07, EQUALITY_TEST);
    test intEqTest08 = make_test(debug_agent, newInt08, EQUALITY_TEST);
    test blankTest = make_blank_test();


    test dest, add_me;

    /* Test 1 - Bug in last version */
    //  dest = copy_test(debug_agent, idEqTest01);
    //  add_test(debug_agent, &blankTest, dest, varEqTest01);
    //  deallocate_test(debug_agent, dest);

    dest = copy_test(debug_agent, idEqTest01);
    dest->original_test = copy_test(debug_agent, varEqTest01);
    add_me = copy_test(debug_agent, idEqTest02);
    add_me->original_test = copy_test(debug_agent, varEqTest01);
    add_test(debug_agent, &dest, add_me);
    add_test(debug_agent, &dest, idEqTest03);

    deallocate_test(debug_agent, dest);
    deallocate_test(debug_agent, idEqTest01);
    deallocate_test(debug_agent, idEqTest02);
    deallocate_test(debug_agent, idEqTest03);
    deallocate_test(debug_agent, idEqTest04);
    deallocate_test(debug_agent, idEqTest05);
    deallocate_test(debug_agent, idEqTest06);
    deallocate_test(debug_agent, idEqTest07);
    deallocate_test(debug_agent, idEqTest08);
    deallocate_test(debug_agent, strEqTest01);
    deallocate_test(debug_agent, strEqTest02);
    deallocate_test(debug_agent, strEqTest03);
    deallocate_test(debug_agent, strEqTest04);
    deallocate_test(debug_agent, strEqTest05);
    deallocate_test(debug_agent, strEqTest06);
    deallocate_test(debug_agent, strEqTest07);
    deallocate_test(debug_agent, strEqTest08);
    deallocate_test(debug_agent, varEqTest01);
    deallocate_test(debug_agent, varEqTest02);
    deallocate_test(debug_agent, varEqTest03);
    deallocate_test(debug_agent, varEqTest04);
    deallocate_test(debug_agent, varEqTest05);
    deallocate_test(debug_agent, varEqTest06);
    deallocate_test(debug_agent, varEqTest07);
    deallocate_test(debug_agent, varEqTest08);
    deallocate_test(debug_agent, intEqTest01);
    deallocate_test(debug_agent, intEqTest02);
    deallocate_test(debug_agent, intEqTest03);
    deallocate_test(debug_agent, intEqTest04);
    deallocate_test(debug_agent, intEqTest05);
    deallocate_test(debug_agent, intEqTest06);
    deallocate_test(debug_agent, intEqTest07);
    deallocate_test(debug_agent, intEqTest08);
}

void debug_test_find_delete_sym(agent* debug_agent, test* dest, Symbol* sym)
{
    ::list* c;

    dprint_test(DT_DEBUG, (*dest), true, false, false, "Starting tests: ", "\n");
    dprint(DT_DEBUG, "Looking for %s.  Comparing against...", sym->to_string());
    c = (*dest)->data.conjunct_list;
    while (c)
    {
        dprint_test(DT_DEBUG, static_cast<test>(c->first), true, false, false, "", ", ");
        if (static_cast<test>(c->first)->data.referent == sym)
        {
            dprint_noprefix(DT_DEBUG, "<-- FOUND\n");
            c = delete_test_from_conjunct(debug_agent, dest, c);
            dprint_test(DT_DEBUG, (*dest), true, false, false, "...after deletion: ", "\n");
        }
        else
        {
            c = c->rest;
        }
    }
    dprint_test(DT_DEBUG, (*dest), true, false, false, "Final tests: ", "\n");

}

void debug_test_delete_conjuncts()
{

    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }

    dprint(DT_DEBUG, "Delete conjunct test.  Creating tests...\n");

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, NIL);
    Symbol* newStr01 = make_str_constant(debug_agent, "attr1");
    Symbol* newInt01 = make_int_constant(debug_agent, 1);
    Symbol* newInt02 = make_int_constant(debug_agent, 2);

    test dest = NULL;
    add_test(debug_agent, &dest, make_test(debug_agent, newID01, EQUALITY_TEST));
    add_test(debug_agent, &dest, make_test(debug_agent, newStr01, EQUALITY_TEST));
    add_test(debug_agent, &dest, make_test(debug_agent, newInt01, GREATER_OR_EQUAL_TEST));
    add_test(debug_agent, &dest, make_test(debug_agent, newInt02, LESS_TEST));
    add_test(debug_agent, &dest, make_test(debug_agent, NULL, GOAL_ID_TEST));

    debug_test_find_delete_sym(debug_agent, &dest, NULL);
    debug_test_find_delete_sym(debug_agent, &dest, newInt01);
    debug_test_find_delete_sym(debug_agent, &dest, newID01);
    debug_test_find_delete_sym(debug_agent, &dest, newStr01);

    dprint(DT_DEBUG, "Deallocating tests and finishing...\n");
    deallocate_test(debug_agent, dest);

    symbol_remove_ref(debug_agent, newID01);
    symbol_remove_ref(debug_agent, newStr01);
    symbol_remove_ref(debug_agent, newInt01);
    symbol_remove_ref(debug_agent, newInt02);

//    print_internal_symbols(debug_agent);

}
/* -- Just a simple function that can be called from the debug command.  Something to put random code for testing/debugging -- */
void debug_test(int type)
{
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }
    switch (type)
    {
        case 1:
            print_internal_symbols(debug_agent);
            dprint_identifiers(DT_DEBUG);
            break;
        case 2:
            debug_agent->variablizationManager->print_tables();
            break;
        case 3:
        {
            Symbol* newSym  = find_identifier(debug_agent, 'S', 3);
            dprint(DT_DEBUG, "S1 refcount %d\n", newSym->reference_count);
            break;
        }
        case 4:
            debug_test_delete_conjuncts();
            break;

        case 5:
            dprint_all_inst(DT_DEBUG);
            break;

        case 6:
            dprint_wmes(DT_DEBUG);
            break;

        case 7:
            dprint(DT_DEBUG, "Trying dprintf with no arguments.");
            dprint(DT_DEBUG, "Trying dprintf with 1 string argument %s", "MAZIN");
            break;
        case 8:
            test_print_speed();
            break;
        case 9:
            test_print_speed_y();
            break;
    }
}

void dprint_sym(char* sym_string)
{
    Symbol* newSym = NULL;
    if (sym_string)
    {
        bool found = false;
        bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
        bool rereadable;
        std::string convertStr(sym_string);
        std::stringstream convert(convertStr);
        int newInt;
        double newFloat;

        agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
        if (!debug_agent)
        {
            return;
        }

        soar::Lexer::determine_possible_symbol_types_for_string(sym_string,
                static_cast<size_t>(strlen(sym_string)),
                &possible_id,
                &possible_var,
                &possible_sc,
                &possible_ic,
                &possible_fc,
                &rereadable);

        if (possible_id)
        {
            newSym = find_identifier(debug_agent, toupper(sym_string[0]), strtol(&sym_string[1], NULL, 10));
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_var)
        {
            newSym = find_variable(debug_agent, sym_string);
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_sc)
        {
            newSym = find_str_constant(debug_agent, sym_string);
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_ic)
        {
            if (convert >> newInt)
            {
                newSym = find_int_constant(debug_agent, newInt);
            }
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_fc)
        {
            if (convert >> newFloat)
            {
                newSym = find_float_constant(debug_agent, newFloat);
            }
            if (newSym)
            {
                found = true;
            }
        }
    }
    if (newSym)
    {
        dprint(DT_DEBUG,
               "%s:\n"
               "  type     = %s\n"
               "  refcount = %d\n"
               "  tc_num   = %d\n",
               newSym->to_string(),
               newSym->type_string(),
               newSym->reference_count,
               newSym->tc_num);
    }
    else
    {
        dprint(DT_DEBUG, "No symbol %s found.\n", sym_string);
    }
}
bool check_symbol(agent* thisAgent, Symbol* sym, const char* message)
{
#ifdef DEBUG_CHECK_SYMBOL
    std::string strName(sym->to_string());
    if (strName == DEBUG_CHECK_SYMBOL)
    {
        //    dprint(DT_DEBUG, "%sFound %s(%lld) | %s\n", message, strName.c_str(), sym->reference_count, get_refcount_stacktrace_string().c_str());
        dprint(DT_DEBUG, "%sFound %s(%lld) | %s\n", message, strName.c_str(), sym->reference_count, "");
        return true;
    }
#endif
    return false;
}

bool check_symbol_in_test(agent* thisAgent, test t, const char* message)
{
#ifdef DEBUG_CHECK_SYMBOL
    cons* c;
    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            if (t->type == EQUALITY_TEST)
            {
                if (static_cast<test>(c->first)->original_test)
                {
                    return (check_symbol(thisAgent, static_cast<test>(c->first)->data.referent, message) || check_symbol(thisAgent, static_cast<test>(c->first)->original_test->data.referent, message));
                }
                else
                {
                    return (check_symbol(thisAgent, static_cast<test>(c->first)->data.referent, message));
                }
            }
        }
    }
    else if (t->type == EQUALITY_TEST)
    {
        if (t->original_test)
        {
            return (check_symbol(thisAgent, t->data.referent, message) || check_symbol_in_test(thisAgent, t->original_test, message));
        }
        else
        {
            return (check_symbol(thisAgent, t->data.referent, message));
        }
    }
#endif
    return false;
}

#ifdef DEBUG_TRACE_REFCOUNT_INVENTORY

#include "output_manager.h"

void debug_store_refcount(Symbol* sym, bool isAdd)
{
    std::string caller_string = get_stacktrace(isAdd ? "add_ref" : "remove_ref");
    debug_agent->outputManager->store_refcount(sym, caller_string.c_str() , isAdd);
}

#endif

#ifdef DEBUG_MAC_STACKTRACE
std::string get_stacktrace(const char* prefix)
{
    // storage array for stack trace data
    // you can change the size of the array to increase the depth of
    // the stack summarized in the string returned
    void* addrlist[7];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0)
    {
        return std::string("<empty, possibly corrupt>");
    }

    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);
    std::string return_string;
    if (prefix)
    {
        return_string += prefix;
        return_string +=  " | ";
    }
    // iterate over the returned symbol lines. skip the first two
    for (int i = 2; i < addrlen; i++)
    {
        return_string += tracey::demangle(std::string(symbollist[i]));
        if (i < (addrlen - 1))
        {
            return_string += " | ";
        }
    }
    free(funcname);
    free(symbollist);
    return return_string;
}
#endif
