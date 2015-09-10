#include "debug.h"
#include "agent.h"
#include "test.h"
#include "variablization_manager.h"
#include "episodic_memory.h"
#include "soar_module.h"
#include "lexer.h"
#include "soar_instance.h"
#include "print.h"
#include "test.h"
#include "output_manager.h"
#include <string>
#include <iostream>
#include <sstream>
#include "sml_Names.h"
#include "stats.h"

/* -- Just a simple function that can be called from the debug command.  Something to put random code for testing/debugging -- */
extern void test_print_speed();
extern void test_print_speed_y();
using namespace sml;


inline size_t strnmove2(char** s1, const char* s2, size_t n) {
    size_t m = 0;
    if ( n > 0) {
        m = strlen(s2);
        if (m+1 > n) {
            m = n-1;
        }
        memmove(*s1, s2, m);
        (*s1)[m]=0;
    } else {
        return n;
    }
    *s1 = &((*s1)[m]);
    return n-m-1;
}

void test_strnmove(char* buf1)
{
    size_t buffer_left = 0;

    buffer_left = 15;
    char* start = &(buf1[0]);

    strcpy(buf1, "");
    std::cout << "Before move |" << buf1 << "| " << buffer_left << std::endl;
    buffer_left = strnmove2(&(buf1), "Hellosfasdfsfsfdfsfsfdsfdsfd", buffer_left);
    std::cout << "Buffer:     |" << buf1 << "| " << buffer_left << std::endl;
    std::cout << "Starting buffer: |" << start << "| " << buffer_left << std::endl;
    buffer_left = strnmove2(&buf1, " Mazin Assanie the first", buffer_left);
    std::cout << "Buffer:     |" << buf1 << "| " << buffer_left << std::endl;
    std::cout << "Starting buffer: |" << start << "| " << buffer_left << std::endl;
    buffer_left = strnmove2(&buf1, " Mazin Assanie the first", buffer_left);
    std::cout << "Buffer:     |" << buf1 << "| " << buffer_left << std::endl;
    std::cout << "Starting buffer: |" << start << "| " << buffer_left << std::endl;

}

void debug_test(int type)
{
    agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
    if (!thisAgent)
    {
        return;
    }

    switch (type)
    {
        case 1:
            print_internal_symbols(thisAgent);
            dprint_identifiers(DT_DEBUG);
            break;
        case 2:
            /* -- Print all instantiations -- */
            dprint_all_inst(DT_DEBUG);
            break;
        case 3:
        {
            /* -- Print all wme's -- */
            dprint(DT_DEBUG, "%8");
            break;
        }
        case 4:
            break;

        case 5:
        {

            break;
        }
        case 6:
        {
            dprint_variablization_tables(DT_DEBUG);
            dprint_variablization_tables(DT_DEBUG, 1);
            dprint_o_id_tables(DT_DEBUG);
            dprint_attachment_points(DT_DEBUG);
            dprint_constraints(DT_DEBUG);
            dprint_merge_map(DT_DEBUG);
            dprint_ovar_to_o_id_map(DT_DEBUG);
            dprint_o_id_substitution_map(DT_DEBUG);
            dprint_o_id_to_ovar_debug_map(DT_DEBUG);
            dprint_tables(DT_DEBUG);
            break;
        }
        case 7:
            break;
        case 8:
        {
            break;
        }
        case 9:
            test_print_speed_y();
            break;
    }
}
void debug_test_structs()
{
    agent* debug_agent = Output_Manager::Get_OM().get_default_agent();
    if (!debug_agent)
    {
        return;
    }

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
//    test blankTest = NULL;


    test dest, add_me;

    /* Test 1 - Bug in last version */
    //  dest = copy_test(debug_agent, idEqTest01);
    //  add_test(debug_agent, &blankTest, dest, varEqTest01);
    //  deallocate_test(debug_agent, dest);

    dest = copy_test(debug_agent, idEqTest01);
    add_me = copy_test(debug_agent, idEqTest02);
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
    dprint(DT_DEBUG, "Starting tests: %t\n", (*dest));
    dprint(DT_DEBUG, "Looking for %y.  Comparing against...", sym);
    c = (*dest)->data.conjunct_list;
    while (c)
    {
        dprint(DT_DEBUG, "%t", static_cast<test>(c->first));
        if (static_cast<test>(c->first)->data.referent == sym)
        {
            dprint_noprefix(DT_DEBUG, "<-- FOUND\n");
            c = delete_test_from_conjunct(debug_agent, dest, c);
            dprint_noprefix(DT_DEBUG, "...after deletion: %t\n", (*dest));
        }
        else
        {
            c = c->rest;
        }
    }
    dprint(DT_DEBUG, "Final tests: %t\n", (*dest));
}

void debug_test_delete_conjuncts()
{

    agent* debug_agent = Output_Manager::Get_OM().get_default_agent();
    if (!debug_agent)
    {
        return;
    }

    dprint(DT_DEBUG, "Delete conjunct test.  Creating tests...\n");

    Symbol* newID01  = make_new_identifier(debug_agent, 'M', 1, 1);
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

