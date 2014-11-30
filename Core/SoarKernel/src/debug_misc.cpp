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
#include "test.h"
#include "output_manager.h"
#include <string>
#include <iostream>
#include <sstream>

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
    dprint(DT_DEBUG, "Starting tests: ");
    dprint_test(DT_DEBUG, (*dest));
    dprint_noprefix(DT_DEBUG, "\n");
    dprint(DT_DEBUG, "Looking for %y.  Comparing against...", sym);
    c = (*dest)->data.conjunct_list;
    while (c)
    {
        dprint_test(DT_DEBUG, static_cast<test>(c->first));
        if (static_cast<test>(c->first)->data.referent == sym)
        {
            dprint_noprefix(DT_DEBUG, "<-- FOUND\n");
            c = delete_test_from_conjunct(debug_agent, dest, c);
            dprint_noprefix(DT_DEBUG, "...after deletion: ");
            dprint_test(DT_DEBUG, (*dest));
            dprint_noprefix(DT_DEBUG, "\n");
        }
        else
        {
            c = c->rest;
        }
    }
    dprint(DT_DEBUG, "Final tests: ");
    dprint_test(DT_DEBUG, (*dest));
    dprint_noprefix(DT_DEBUG, "\n");
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
extern void test_print_speed();
extern void test_print_speed_y();
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
            dprint_wmes(DT_DEBUG, true);
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
