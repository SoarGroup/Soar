#include "debug.h"
#include "soar_instance.h"
#include "print.h"

#define DEBUG_BUFFER_SIZE 5000

static Output_Manager* thisOutput_Manager = NULL;

//#define test_print_function(format, args...) dprint (DT_DEBUG, format , ##args)
//#define test_print_function(format, args...) Output_Manager::Get_OM().debug_print_sf (DT_DEBUG, format , ##args)
//#define test_print_function(format, args...) Output_Manager::Get_OM().debug_print_sf_noprefix (DT_DEBUG, format , ##args)
//#define test_print_function(format, args...) Output_Manager::Get_OM().print_sf (format , ##args)

//#define test_print_function(format, args...) print (debug_agent, format , ##args)
//#define test_print_function(format, args...) print_with_symbols (debug_agent, format , ##args)
#define test_print_function(format, args...) print_old (debug_agent, format , ##args)

//#define test_print_function(format, args...) Output_Manager::Get_OM().printa_sf(debug_agent, format , ##args)
//#define test_print_function(format, args...) thisOutput_Manager->printa_sf(debug_agent, format , ##args)

//#define test_print_function(format, args...) dprint_y (DT_DEBUG, format , ##args)
//#define test_print_function(format, args...) dprint_macro (DT_DEBUG, format , ##args)

void test_print_speed()
{
    agent* debug_agent = Soar_Instance::Get_Soar_Instance().Get_Default_Agent();
    if (!debug_agent)
    {
        return;
    }
    thisOutput_Manager = &(Output_Manager::Get_OM());

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID02  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID03  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID04  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID05  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID06  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID07  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID08  = make_new_identifier(debug_agent, 'Z', 1, 1);
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
        test_print_function("This is just a plain string.\n");
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Identifier: %s\n", newID01->to_string());
        test_print_function("Identifier: %s\n", newID02->to_string());
        test_print_function("Identifier: %s\n", newID03->to_string());
        test_print_function("Identifier: %s\n", newID04->to_string());
        test_print_function("Identifier: %s\n", newID05->to_string());
        test_print_function("Identifier: %s\n", newID06->to_string());
        test_print_function("Identifier: %s\n", newID07->to_string());
        test_print_function("Identifier: %s\n", newID08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Identifiers: %s %s %s %s %s %s %s %s\n",
            newID01->to_string(), newID02->to_string(), newID03->to_string(), newID04->to_string(),
            newID05->to_string(), newID06->to_string(), newID07->to_string(), newID08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("String: %s\n", newStr01->to_string());
        test_print_function("String: %s\n", newStr02->to_string());
        test_print_function("String: %s\n", newStr03->to_string());
        test_print_function("String: %s\n", newStr04->to_string());
        test_print_function("String: %s\n", newStr05->to_string());
        test_print_function("String: %s\n", newStr06->to_string());
        test_print_function("String: %s\n", newStr07->to_string());
        test_print_function("String: %s\n", newStr08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Strings: %s %s %s %s %s %s %s %s\n",
            newStr01->to_string(), newStr02->to_string(), newStr03->to_string(), newStr04->to_string(),
            newStr05->to_string(), newStr06->to_string(), newStr07->to_string(), newStr08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Variable: %s\n", newVar01->to_string());
        test_print_function("Variable: %s\n", newVar02->to_string());
        test_print_function("Variable: %s\n", newVar03->to_string());
        test_print_function("Variable: %s\n", newVar04->to_string());
        test_print_function("Variable: %s\n", newVar05->to_string());
        test_print_function("Variable: %s\n", newVar06->to_string());
        test_print_function("Variable: %s\n", newVar07->to_string());
        test_print_function("Variable: %s\n", newVar08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Variables: %s %s %s %s %s %s %s %s\n",
            newVar01->to_string(), newVar02->to_string(), newVar03->to_string(), newVar04->to_string(),
            newVar05->to_string(), newVar06->to_string(), newVar07->to_string(), newVar08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Integer: %s\n", newInt01->to_string());
        test_print_function("Integer: %s\n", newInt02->to_string());
        test_print_function("Integer: %s\n", newInt03->to_string());
        test_print_function("Integer: %s\n", newInt04->to_string());
        test_print_function("Integer: %s\n", newInt05->to_string());
        test_print_function("Integer: %s\n", newInt06->to_string());
        test_print_function("Integer: %s\n", newInt07->to_string());
        test_print_function("Integer: %s\n", newInt08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Integers: %s %s %s %s %s %s %s %s\n",
            newInt01->to_string(), newInt02->to_string(), newInt03->to_string(), newInt04->to_string(),
            newInt05->to_string(), newInt06->to_string(), newInt07->to_string(), newInt08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Float: %s\n", newFloat01->to_string());
        test_print_function("Float: %s\n", newFloat02->to_string());
        test_print_function("Float: %s\n", newFloat03->to_string());
        test_print_function("Float: %s\n", newFloat04->to_string());
        test_print_function("Float: %s\n", newFloat05->to_string());
        test_print_function("Float: %s\n", newFloat06->to_string());
        test_print_function("Float: %s\n", newFloat07->to_string());
        test_print_function("Float: %s\n", newFloat08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Floats: %s %s %s %s %s %s %s %s\n",
            newFloat01->to_string(), newFloat02->to_string(), newFloat03->to_string(), newFloat04->to_string(),
            newFloat05->to_string(), newFloat06->to_string(), newFloat07->to_string(), newFloat08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Identifier: %s\n", newID01->to_string());
        test_print_function("String: %s\n", newStr01->to_string());
        test_print_function("Variable: %s\n", newVar01->to_string());
        test_print_function("Integer: %s\n", newInt01->to_string());
        test_print_function("Float: %s\n", newFloat01->to_string());

        test_print_function("Identifier: %s\n", newID02->to_string());
        test_print_function("String: %s\n", newStr02->to_string());
        test_print_function("Variable: %s\n", newVar02->to_string());
        test_print_function("Integer: %s\n", newInt02->to_string());
        test_print_function("Float: %s\n", newFloat02->to_string());

        test_print_function("Identifier: %s\n", newID03->to_string());
        test_print_function("String: %s\n", newStr03->to_string());
        test_print_function("Variable: %s\n", newVar03->to_string());
        test_print_function("Integer: %s\n", newInt03->to_string());
        test_print_function("Float: %s\n", newFloat03->to_string());

        test_print_function("Identifier: %s\n", newID04->to_string());
        test_print_function("String: %s\n", newStr04->to_string());
        test_print_function("Variable: %s\n", newVar04->to_string());
        test_print_function("Integer: %s\n", newInt04->to_string());
        test_print_function("Float: %s\n", newFloat04->to_string());

        test_print_function("Identifier: %s\n", newID05->to_string());
        test_print_function("String: %s\n", newStr05->to_string());
        test_print_function("Variable: %s\n", newVar05->to_string());
        test_print_function("Integer: %s\n", newInt05->to_string());
        test_print_function("Float: %s\n", newFloat05->to_string());

        test_print_function("Identifier: %s\n", newID06->to_string());
        test_print_function("String: %s\n", newStr06->to_string());
        test_print_function("Variable: %s\n", newVar06->to_string());
        test_print_function("Integer: %s\n", newInt06->to_string());
        test_print_function("Float: %s\n", newFloat06->to_string());

        test_print_function("Identifier: %s\n", newID07->to_string());
        test_print_function("String: %s\n", newStr07->to_string());
        test_print_function("Variable: %s\n", newVar07->to_string());
        test_print_function("Integer: %s\n", newInt07->to_string());
        test_print_function("Float: %s\n", newFloat07->to_string());

        test_print_function("Identifier: %s\n", newID08->to_string());
        test_print_function("String: %s\n", newStr08->to_string());
        test_print_function("Variable: %s\n", newVar08->to_string());
        test_print_function("Integer: %s\n", newInt08->to_string());
        test_print_function("Float: %s\n", newFloat08->to_string());
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Mixed: %s %s %s %s %s\n", newID01->to_string(), newStr01->to_string(),
            newVar01->to_string(), newInt01->to_string(), newFloat01->to_string());
        test_print_function("Mixed: %s %s %s %s %s\n", newID02->to_string(), newStr02->to_string(),
            newVar02->to_string(), newInt02->to_string(), newFloat01->to_string());
        test_print_function("Mixed: %s %s %s %s %s\n", newID03->to_string(), newStr03->to_string(),
            newVar03->to_string(), newInt03->to_string(), newFloat01->to_string());
        test_print_function("Mixed: %s %s %s %s %s\n", newID04->to_string(), newStr04->to_string(),
            newVar04->to_string(), newInt04->to_string(), newFloat01->to_string());
        test_print_function("Mixed: %s %s %s %s %s\n", newID05->to_string(), newStr05->to_string(),
            newVar05->to_string(), newInt05->to_string(), newFloat01->to_string());
        test_print_function("Mixed: %s %s %s %s %s\n", newID06->to_string(), newStr06->to_string(),
            newVar06->to_string(), newInt06->to_string(), newFloat01->to_string());
        test_print_function("Mixed: %s %s %s %s %s\n", newID07->to_string(), newStr07->to_string(),
            newVar07->to_string(), newInt07->to_string(), newFloat01->to_string());
        test_print_function("Mixed: %s %s %s %s %s\n", newID08->to_string(), newStr08->to_string(),
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

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID02  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID03  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID04  = make_new_identifier(debug_agent, 'M', 1, 1);
    Symbol* newID05  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID06  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID07  = make_new_identifier(debug_agent, 'Z', 1, 1);
    Symbol* newID08  = make_new_identifier(debug_agent, 'Z', 1, 1);
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
        test_print_function("This is just a plain string.\n");
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Identifier: %y\n", newID01);
        test_print_function("Identifier: %y\n", newID02);
        test_print_function("Identifier: %y\n", newID03);
        test_print_function("Identifier: %y\n", newID04);
        test_print_function("Identifier: %y\n", newID05);
        test_print_function("Identifier: %y\n", newID06);
        test_print_function("Identifier: %y\n", newID07);
        test_print_function("Identifier: %y\n", newID08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Identifiers: %y %y %y %y %y %y %y %y\n",
            newID01, newID02, newID03, newID04,
            newID05, newID06, newID07, newID08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("String: %y\n", newStr01);
        test_print_function("String: %y\n", newStr02);
        test_print_function("String: %y\n", newStr03);
        test_print_function("String: %y\n", newStr04);
        test_print_function("String: %y\n", newStr05);
        test_print_function("String: %y\n", newStr06);
        test_print_function("String: %y\n", newStr07);
        test_print_function("String: %y\n", newStr08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Strings: %y %y %y %y %y %y %y %y\n",
            newStr01, newStr02, newStr03, newStr04,
            newStr05, newStr06, newStr07, newStr08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Variable: %y\n", newVar01);
        test_print_function("Variable: %y\n", newVar02);
        test_print_function("Variable: %y\n", newVar03);
        test_print_function("Variable: %y\n", newVar04);
        test_print_function("Variable: %y\n", newVar05);
        test_print_function("Variable: %y\n", newVar06);
        test_print_function("Variable: %y\n", newVar07);
        test_print_function("Variable: %y\n", newVar08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Variables: %y %y %y %y %y %y %y %y\n",
            newVar01, newVar02, newVar03, newVar04,
            newVar05, newVar06, newVar07, newVar08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Integer: %y\n", newInt01);
        test_print_function("Integer: %y\n", newInt02);
        test_print_function("Integer: %y\n", newInt03);
        test_print_function("Integer: %y\n", newInt04);
        test_print_function("Integer: %y\n", newInt05);
        test_print_function("Integer: %y\n", newInt06);
        test_print_function("Integer: %y\n", newInt07);
        test_print_function("Integer: %y\n", newInt08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Integers: %y %y %y %y %y %y %y %y\n",
            newInt01, newInt02, newInt03, newInt04,
            newInt05, newInt06, newInt07, newInt08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Float: %y\n", newFloat01);
        test_print_function("Float: %y\n", newFloat02);
        test_print_function("Float: %y\n", newFloat03);
        test_print_function("Float: %y\n", newFloat04);
        test_print_function("Float: %y\n", newFloat05);
        test_print_function("Float: %y\n", newFloat06);
        test_print_function("Float: %y\n", newFloat07);
        test_print_function("Float: %y\n", newFloat08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Floats: %y %y %y %y %y %y %y %y\n",
            newFloat01, newFloat02, newFloat03, newFloat04,
            newFloat05, newFloat06, newFloat07, newFloat08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Identifier: %y\n", newID01);
        test_print_function("String: %y\n", newStr01);
        test_print_function("Variable: %y\n", newVar01);
        test_print_function("Integer: %y\n", newInt01);
        test_print_function("Float: %y\n", newFloat01);

        test_print_function("Identifier: %y\n", newID02);
        test_print_function("String: %y\n", newStr02);
        test_print_function("Variable: %y\n", newVar02);
        test_print_function("Integer: %y\n", newInt02);
        test_print_function("Float: %y\n", newFloat02);

        test_print_function("Identifier: %y\n", newID03);
        test_print_function("String: %y\n", newStr03);
        test_print_function("Variable: %y\n", newVar03);
        test_print_function("Integer: %y\n", newInt03);
        test_print_function("Float: %y\n", newFloat03);

        test_print_function("Identifier: %y\n", newID04);
        test_print_function("String: %y\n", newStr04);
        test_print_function("Variable: %y\n", newVar04);
        test_print_function("Integer: %y\n", newInt04);
        test_print_function("Float: %y\n", newFloat04);

        test_print_function("Identifier: %y\n", newID05);
        test_print_function("String: %y\n", newStr05);
        test_print_function("Variable: %y\n", newVar05);
        test_print_function("Integer: %y\n", newInt05);
        test_print_function("Float: %y\n", newFloat05);

        test_print_function("Identifier: %y\n", newID06);
        test_print_function("String: %y\n", newStr06);
        test_print_function("Variable: %y\n", newVar06);
        test_print_function("Integer: %y\n", newInt06);
        test_print_function("Float: %y\n", newFloat06);

        test_print_function("Identifier: %y\n", newID07);
        test_print_function("String: %y\n", newStr07);
        test_print_function("Variable: %y\n", newVar07);
        test_print_function("Integer: %y\n", newInt07);
        test_print_function("Float: %y\n", newFloat07);

        test_print_function("Identifier: %y\n", newID08);
        test_print_function("String: %y\n", newStr08);
        test_print_function("Variable: %y\n", newVar08);
        test_print_function("Integer: %y\n", newInt08);
        test_print_function("Float: %y\n", newFloat08);
    }
    for (int i=0; i< num_iterations; ++i)
    {
        test_print_function("Mixed: %y %y %y %y %y\n", newID01, newStr01,
            newVar01, newInt01, newFloat01);
        test_print_function("Mixed: %y %y %y %y %y\n", newID02, newStr02,
            newVar02, newInt02, newFloat01);
        test_print_function("Mixed: %y %y %y %y %y\n", newID03, newStr03,
            newVar03, newInt03, newFloat01);
        test_print_function("Mixed: %y %y %y %y %y\n", newID04, newStr04,
            newVar04, newInt04, newFloat01);
        test_print_function("Mixed: %y %y %y %y %y\n", newID05, newStr05,
            newVar05, newInt05, newFloat01);
        test_print_function("Mixed: %y %y %y %y %y\n", newID06, newStr06,
            newVar06, newInt06, newFloat01);
        test_print_function("Mixed: %y %y %y %y %y\n", newID07, newStr07,
            newVar07, newInt07, newFloat01);
        test_print_function("Mixed: %y %y %y %y %y\n", newID08, newStr08,
            newVar08, newInt08, newFloat01);
    }
}
