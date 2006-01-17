/*************************************************************************
 *
 *  file:  rhsfun_math.c
 *
 * =======================================================================
 *  Support routines for doing math in the RHS of productions.
 *  BUGBUG more comments here.  Nothing in soarkernel.h either.
 *  
 *  
 * =======================================================================
 *
 * Copyright 1995-2004 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */

#include "soarkernel.h"
#include <math.h>
#include <errno.h>
#include "rhsfun.h"

/* --------------------------------------------------------------------
                                Plus

   Takes any number of int_constant or float_constant arguments, and
   returns their sum.
-------------------------------------------------------------------- */

Symbol *plus_rhs_function_code(list * args)
{
    bool float_found;
    long i;
    float f = 0;
    Symbol *arg;
    cons *c;

    for (c = args; c != NIL; c = c->rest) {
        arg = c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
            (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols("Error: non-number (%y) passed to + function\n", arg);
            return NIL;
        }
    }

    i = 0;
    float_found = FALSE;
    while (args) {
        arg = args->first;
        if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
            if (float_found)
                f += arg->ic.value;
            else
                i += arg->ic.value;
        } else {
            if (float_found)
                f += arg->fc.value;
            else {
                float_found = TRUE;
                f = arg->fc.value + i;
            }
        }
        args = args->rest;
    }
    if (float_found)
        return make_float_constant(f);
    return make_int_constant(i);
}

/* --------------------------------------------------------------------
                                Times

   Takes any number of int_constant or float_constant arguments, and
   returns their product.
-------------------------------------------------------------------- */

Symbol *times_rhs_function_code(list * args)
{
    bool float_found;
    long i;
    float f = 0;
    Symbol *arg;
    cons *c;

    for (c = args; c != NIL; c = c->rest) {
        arg = c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
            (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols("Error: non-number (%y) passed to * function\n", arg);
            return NIL;
        }
    }

    i = 1;
    float_found = FALSE;
    while (args) {
        arg = args->first;
        if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
            if (float_found)
                f *= arg->ic.value;
            else
                i *= arg->ic.value;
        } else {
            if (float_found)
                f *= arg->fc.value;
            else {
                float_found = TRUE;
                f = arg->fc.value * i;
            }
        }
        args = args->rest;
    }
    if (float_found)
        return make_float_constant(f);
    return make_int_constant(i);
}

/* --------------------------------------------------------------------
                                Minus

   Takes one or more int_constant or float_constant arguments.
   If 0 arguments, returns NIL (error).
   If 1 argument (x), returns -x.
   If >=2 arguments (x, y1, ..., yk), returns x - y1 - ... - yk.
-------------------------------------------------------------------- */

Symbol *minus_rhs_function_code(list * args)
{
    Symbol *arg;
    float f = 0;                /* For gcc -Wall */
    long i = 0;                 /* For gcc -Wall */
    cons *c;
    bool float_found;

    if (!args) {
        print("Error: '-' function called with no arguments\n");
        return NIL;
    }

    for (c = args; c != NIL; c = c->rest) {
        arg = c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
            (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols("Error: non-number (%y) passed to - function\n", arg);
            return NIL;
        }
    }

    if (!args->rest) {
        /* --- only one argument --- */
        arg = args->first;
        if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
            return make_int_constant(-arg->ic.value);
        return make_float_constant(-arg->fc.value);
    }

    /* --- two or more arguments --- */
    arg = args->first;
    float_found = FALSE;
    if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        i = arg->ic.value;
    else {
        float_found = TRUE;
        f = arg->fc.value;
    }
    for (c = args->rest; c != NIL; c = c->rest) {
        arg = c->first;
        if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
            if (float_found)
                f -= arg->ic.value;
            else
                i -= arg->ic.value;
        } else {
            if (float_found)
                f -= arg->fc.value;
            else {
                float_found = TRUE;
                f = i - arg->fc.value;
            }
        }
    }

    if (float_found)
        return make_float_constant(f);
    return make_int_constant(i);
}

/* --------------------------------------------------------------------
                     Floating-Point Division

   Takes one or more int_constant or float_constant arguments.
   If 0 arguments, returns NIL (error).
   If 1 argument (x), returns 1/x.
   If >=2 arguments (x, y1, ..., yk), returns x / y1 / ... / yk.
-------------------------------------------------------------------- */

Symbol *fp_divide_rhs_function_code(list * args)
{
    Symbol *arg;
    float f;
    cons *c;

    if (!args) {
        print("Error: '/' function called with no arguments\n");
        return NIL;
    }

    for (c = args; c != NIL; c = c->rest) {
        arg = c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
            (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols("Error: non-number (%y) passed to / function\n", arg);
            return NIL;
        }
    }

    if (!args->rest) {
        /* --- only one argument --- */
        arg = args->first;
        if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
            f = (float) arg->ic.value;
        else
            f = arg->fc.value;
        if (f != 0.0)
            return make_float_constant((float) (1.0 / f));
        print("Error: attempt to divide ('/') by zero.\n");
        return NIL;
    }

    /* --- two or more arguments --- */
    arg = args->first;
    if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        f = (float) arg->ic.value;
    else
        f = arg->fc.value;
    for (c = args->rest; c != NIL; c = c->rest) {
        arg = c->first;
        if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
            if (arg->ic.value)
                f /= arg->ic.value;
            else {
                print("Error: attempt to divide ('/') by zero.\n");
                return NIL;
            }
        } else {
            if (arg->fc.value != 0.0)
                f /= arg->fc.value;
            else {
                print("Error: attempt to divide ('/') by zero.\n");
                return NIL;
            }
        }
    }
    return make_float_constant(f);
}

/* --------------------------------------------------------------------
                     Integer Division (Quotient)

   Takes two int_constant arguments, and returns their quotient.
-------------------------------------------------------------------- */

Symbol *div_rhs_function_code(list * args)
{
    Symbol *arg1, *arg2;

    arg1 = args->first;
    arg2 = args->rest->first;

    if (arg1->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) {
        print_with_symbols("Error: non-integer (%y) passed to div function\n", arg1);
        return NIL;
    }
    if (arg2->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) {
        print_with_symbols("Error: non-integer (%y) passed to div function\n", arg2);
        return NIL;
    }

    if (arg2->ic.value == 0) {
        print("Error: attempt to divide ('div') by zero.\n");
        return NIL;
    }

    return make_int_constant(arg1->ic.value / arg2->ic.value);
    /* Warning: ANSI doesn't say precisely what happens if one or both of the
       two args is negative. */
}

/* --------------------------------------------------------------------
                          Integer Modulus

   Takes two int_constant arguments (x,y) and returns (x mod y), i.e.,
   the remainder after dividing x by y.
-------------------------------------------------------------------- */

Symbol *mod_rhs_function_code(list * args)
{
    Symbol *arg1, *arg2;

    arg1 = args->first;
    arg2 = args->rest->first;

    if (arg1->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) {
        print_with_symbols("Error: non-integer (%y) passed to mod function\n", arg1);
        return NIL;
    }
    if (arg2->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) {
        print_with_symbols("Error: non-integer (%y) passed to mod function\n", arg2);
        return NIL;
    }

    if (arg2->ic.value == 0) {
        print("Error: attempt to divide ('mod') by zero.\n");
        return NIL;
    }

    return make_int_constant(arg1->ic.value % arg2->ic.value);
    /* Warning:  ANSI guarantees this does the right thing if both args are
       positive.  If one or both is negative, it only guarantees that
       (a/b)*b + a%b == a. */
}

/*
 * SIN_RHS_FUNCTION_CODE
 *
 * Returns as a float the sine of an angle measured in radians.
 */
Symbol *sin_rhs_function_code(list * args)
{
    Symbol *arg;
    float arg_value;

    if (!args) {
        print("Error: 'sin' function called with no arguments\n");
        return NIL;
    }

    arg = args->first;
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
        arg_value = arg->fc.value;
    else if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        arg_value = (float) arg->ic.value;
    else {
        print_with_symbols("Error: 'sin' function called with non-numeric argument %y\n", arg);
        return NIL;
    }

    return make_float_constant((float) sin(arg_value));
}

/*
 * COS_RHS_FUNCTION_CODE
 *
 * Returns as a float the cosine of an angle measured in radians.
 */
Symbol *cos_rhs_function_code(list * args)
{
    Symbol *arg;
    float arg_value;

    if (!args) {
        print("Error: 'cos' function called with no arguments\n");
        return NIL;
    }

    arg = args->first;
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
        arg_value = arg->fc.value;
    else if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        arg_value = (float) arg->ic.value;
    else {
        print_with_symbols("Error: 'cos' function called with non-numeric argument %y\n", arg);
        return NIL;
    }
    return make_float_constant((float) cos(arg_value));
}

/*
 * SQRT_RHS_FUNCTION_CODE
 *
 * Returns as a float the square root of its argument (integer or float).
 */
Symbol *sqrt_rhs_function_code(list * args)
{
    Symbol *arg;
    float arg_value;

    if (!args) {
        print("Error: 'sqrt' function called with no arguments\n");
        return NIL;
    }

    arg = args->first;
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
        arg_value = arg->fc.value;
    else if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        arg_value = (float) arg->ic.value;
    else {
        print_with_symbols("Error: 'sqrt' function called with non-numeric argument %y\n", arg);
        return NIL;
    }
    return make_float_constant((float) sqrt(arg_value));
}

/*
 * ATAN2_RHS_FUNCTION_CODE
 *
 * Returns as a float in radians the arctangent of (first_arg/second_arg)
 * which are floats or integers.
 */
Symbol *atan2_rhs_function_code(list * args)
{
    Symbol *arg;
    cons *c;
    float numer_value, denom_value;

    if (!args) {
        print("Error: 'atan2' function called with no arguments\n");
        return NIL;
    }

    for (c = args; c != NIL; c = c->rest) {
        arg = c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE)
            && (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols("Error: non-number (%y) passed to atan2\n", arg);
            return NIL;
        }
    }

    if (!args->rest) {
        print("Error: 'atan2' function called with only one argument\n");
        return NIL;
    }

    arg = args->first;
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
        numer_value = arg->fc.value;
    else
        numer_value = (float) arg->ic.value;

    c = args->rest;
    if (c->rest) {
        print("Error: 'atan2' function called with more than two arguments.\n");
        return NIL;
    }
    arg = c->first;
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
        denom_value = arg->fc.value;
    else
        denom_value = (float) arg->ic.value;

    return make_float_constant((float) atan2(numer_value, denom_value));
}

/*
 * ABS_RHS_FUNCTION_CODE
 *
 * Returns the absolute value of a float as a float, of an int as an int.
 */
Symbol *abs_rhs_function_code(list * args)
{
    Symbol *arg, *return_value;

    if (!args) {
        print("Error: 'abs' function called with no arguments\n");
        return NIL;
    }

    arg = args->first;
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
        return_value = make_float_constant((float) fabs(arg->fc.value));
    else if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        return_value = make_int_constant((arg->ic.value < 0) ? -arg->ic.value : arg->ic.value);
    else {
        print_with_symbols("Error: 'abs' function called with non-numeric argument %y\n", arg);
        return NIL;
    }
    return return_value;
}

/* --------------------------------------------------------------------
                         int

   Casts the given symbol into an integer.  If the symbol is a sym
   constant, a conversion is done.  If the symbol is a float, then
   the integer portion is returned.
-------------------------------------------------------------------- */

Symbol *int_rhs_function_code(list * args)
{
    Symbol *sym;

    if (!args) {
        print("Error: 'int' function called with no arguments.\n");
        return NIL;
    }

    if (args->rest) {
        print("Error: 'int' takes exactly 1 argument.\n");
        return NIL;
    }

    sym = (Symbol *) args->first;
    if (sym->common.symbol_type == VARIABLE_SYMBOL_TYPE) {
        print_with_symbols("Error: variable (%y) passed to 'int' RHS function.\n", sym);
        return NIL;
    } else if (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) {
        print_with_symbols("Error: identifier (%y) passed to 'int' RHS function.\n", sym);
        return NIL;
    } else if (sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {
        long int_val;

        errno = 0;
        int_val = strtol(symbol_to_string(sym, FALSE, NIL, 0), NULL, 10);
        if (errno) {
            print("Error: bad integer (%y) given to 'int' RHS function\n", sym);
            return NIL;
        }
        return make_int_constant(int_val);
    } else if (sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
        return sym;
    } else if (sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
        double int_part;
        modf((double) sym->fc.value, &int_part);
        return make_int_constant((long) int_part);
    }

    print("Error: unknown symbol type (%y) given to 'int' RHS function\n", sym);
    return NIL;
}

/* --------------------------------------------------------------------
                         float

   Casts the given symbol into an float.  If the symbol is a sym
   constant, a conversion is done.  If the symbol is an int, then
   the integer portion is converted.
-------------------------------------------------------------------- */

Symbol *float_rhs_function_code(list * args)
{
    Symbol *sym;

    if (!args) {
        print("Error: 'float' function called with no arguments.\n");
        return NIL;
    }

    if (args->rest) {
        print("Error: 'float' takes exactly 1 argument.\n");
        return NIL;
    }

    sym = (Symbol *) args->first;
    if (sym->common.symbol_type == VARIABLE_SYMBOL_TYPE) {
        print_with_symbols("Error: variable (%y) passed to 'float' RHS function.\n", sym);
        return NIL;
    } else if (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) {
        print_with_symbols("Error: identifier (%y) passed to 'float' RHS function.\n", sym);
        return NIL;
    } else if (sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {
        double float_val;

        errno = 0;
        /*float_val = strtod(symbol_to_string (sym, FALSE, NIL,0), NULL, 10); */
        float_val = strtod(symbol_to_string(sym, FALSE, NIL, 0), NULL);
        if (errno) {
            print("Error: bad float (%y) given to 'float' RHS function\n", sym);
            return NIL;
        }
        return make_float_constant((float) float_val);
    } else if (sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
        return sym;
    } else if (sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
        return make_float_constant((float) sym->ic.value);
    }

    print("Error: unknown symbol type (%y) given to 'float' RHS function\n", sym);
    return NIL;
}

/* ====================================================================

                  Initialize the Built-In RHS Math Functions

====================================================================
*/

void init_built_in_rhs_math_functions(void)
{
    add_rhs_function(make_sym_constant("+"), plus_rhs_function_code, -1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("*"), times_rhs_function_code, -1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("-"), minus_rhs_function_code, -1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("/"), fp_divide_rhs_function_code, -1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("div"), div_rhs_function_code, 2, TRUE, FALSE);
    add_rhs_function(make_sym_constant("mod"), mod_rhs_function_code, 2, TRUE, FALSE);

    add_rhs_function(make_sym_constant("sin"), sin_rhs_function_code, 1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("cos"), cos_rhs_function_code, 1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("atan2"), atan2_rhs_function_code, 2, TRUE, FALSE);
    add_rhs_function(make_sym_constant("sqrt"), sqrt_rhs_function_code, 1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("abs"), abs_rhs_function_code, 1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("int"), int_rhs_function_code, 1, TRUE, FALSE);
    add_rhs_function(make_sym_constant("float"), float_rhs_function_code, 1, TRUE, FALSE);

}

void remove_built_in_rhs_math_functions(void)
{
    remove_rhs_function(make_sym_constant("+"));
    remove_rhs_function(make_sym_constant("*"));
    remove_rhs_function(make_sym_constant("-"));
    remove_rhs_function(make_sym_constant("/"));
    remove_rhs_function(make_sym_constant("div"));
    remove_rhs_function(make_sym_constant("mod"));
    remove_rhs_function(make_sym_constant("sin"));
    remove_rhs_function(make_sym_constant("cos"));
    remove_rhs_function(make_sym_constant("atan2"));
    remove_rhs_function(make_sym_constant("sqrt"));
    remove_rhs_function(make_sym_constant("abs"));
    remove_rhs_function(make_sym_constant("int"));
    remove_rhs_function(make_sym_constant("float"));
}
