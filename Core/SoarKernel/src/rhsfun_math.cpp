#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  rhsfun_math.cpp
 *
 * =======================================================================
 *  Support routines for doing math in the RHS of productions.
 *  Need more comments here.  Nothing in soarkernel.h either.
 *  
 *  
 * =======================================================================
 */

#include "rhsfun_math.h"
#include "symtab.h"
#include "kernel.h"
#include "mem.h"
#include "print.h"
#include "lexer.h"
#include "rhsfun.h"

/* --------------------------------------------------------------------
                                Plus

   Takes any number of int_constant or float_constant arguments, and
   returns their sum.
-------------------------------------------------------------------- */

Symbol *plus_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Bool float_found;
  long i;
  float f = 0;
  Symbol *arg;
  cons *c;

  for (c=args; c!=NIL; c=c->rest) {
    arg = static_cast<symbol_union *>(c->first);
    if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
        (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
      print_with_symbols (thisAgent, "Error: non-number (%y) passed to + function\n",
                          arg);
      return NIL;
    }
  }

  i = 0;
  float_found = FALSE;
  while (args) {
    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) {
      if (float_found) f += arg->ic.value;
      else i += arg->ic.value;
    } else {
      if (float_found) f += arg->fc.value;
      else { float_found = TRUE; f = arg->fc.value + i; }
    }
    args = args->rest;
  }
  if (float_found) return make_float_constant (thisAgent, f);
  return make_int_constant (thisAgent, i);
}

/* --------------------------------------------------------------------
                                Times

   Takes any number of int_constant or float_constant arguments, and
   returns their product.
-------------------------------------------------------------------- */

Symbol *times_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Bool float_found;
  long i;
  float f = 0;
  Symbol *arg;
  cons *c;
  
  for (c=args; c!=NIL; c=c->rest) {
    arg = static_cast<symbol_union *>(c->first);
    if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
        (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
      print_with_symbols (thisAgent, "Error: non-number (%y) passed to * function\n",
                          arg);
      return NIL;
    }
  }

  i = 1;
  float_found = FALSE;
  while (args) {
    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) {
      if (float_found) f *= arg->ic.value;
      else i *= arg->ic.value;
    } else {
      if (float_found) f *= arg->fc.value;
      else { float_found = TRUE; f = arg->fc.value * i; }
    }
    args = args->rest;
  }
  if (float_found) return make_float_constant (thisAgent, f);
  return make_int_constant (thisAgent, i);
}

/* --------------------------------------------------------------------
                                Minus

   Takes one or more int_constant or float_constant arguments.
   If 0 arguments, returns NIL (error).
   If 1 argument (x), returns -x.
   If >=2 arguments (x, y1, ..., yk), returns x - y1 - ... - yk.
-------------------------------------------------------------------- */

Symbol *minus_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *arg;
  float f = 0;  /* For gcc -Wall */
  long i = 0;   /* For gcc -Wall */
  cons *c;
  Bool float_found;

  if (!args) {
    print (thisAgent, "Error: '-' function called with no arguments\n");
    return NIL;
  }
  
  for (c=args; c!=NIL; c=c->rest) {
    arg = static_cast<symbol_union *>(c->first);
    if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
        (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
      print_with_symbols (thisAgent, "Error: non-number (%y) passed to - function\n",
                          arg);
      return NIL;
    }
  }

  if (! args->rest) {
    /* --- only one argument --- */
    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE)
      return make_int_constant (thisAgent, - arg->ic.value);
    return make_float_constant (thisAgent, - arg->fc.value);
  }

  /* --- two or more arguments --- */
  arg = static_cast<symbol_union *>(args->first);
  float_found = FALSE;
  if (arg->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) i = arg->ic.value;
  else { float_found = TRUE; f = arg->fc.value; }
  for (c=args->rest; c!=NIL; c=c->rest) {
    arg = static_cast<symbol_union *>(c->first);
    if (arg->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) {
      if (float_found) f -= arg->ic.value;
      else i -= arg->ic.value;
    } else {
      if (float_found) f -= arg->fc.value;
      else { float_found = TRUE; f = i - arg->fc.value; }
    }
  }
 
  if (float_found) return make_float_constant (thisAgent, f);
  return make_int_constant (thisAgent, i);
}

/* --------------------------------------------------------------------
                     Floating-Point Division

   Takes one or more int_constant or float_constant arguments.
   If 0 arguments, returns NIL (error).
   If 1 argument (x), returns 1/x.
   If >=2 arguments (x, y1, ..., yk), returns x / y1 / ... / yk.
-------------------------------------------------------------------- */

Symbol *fp_divide_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *arg;
  float f;
  cons *c;

  if (!args) {
    print (thisAgent, "Error: '/' function called with no arguments\n");
    return NIL;
  }
  
  for (c=args; c!=NIL; c=c->rest) {
    arg = static_cast<symbol_union *>(c->first);
    if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
        (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
      print_with_symbols (thisAgent, "Error: non-number (%y) passed to / function\n",
                          arg);
      return NIL;
    }
  }

  if (! args->rest) {
    /* --- only one argument --- */
    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) f = (float)arg->ic.value;
    else f = arg->fc.value;
    if (f != 0.0) return make_float_constant (thisAgent, (float)(1.0 / f));
    print (thisAgent, "Error: attempt to divide ('/') by zero.\n");
    return NIL;
  }

  /* --- two or more arguments --- */
  arg = static_cast<symbol_union *>(args->first);
  if (arg->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) f = (float)arg->ic.value;
  else f = arg->fc.value;
  for (c=args->rest; c!=NIL; c=c->rest) {
    arg = static_cast<symbol_union *>(c->first);
    if (arg->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) {
      if (arg->ic.value) f /= arg->ic.value;
      else { print (thisAgent, "Error: attempt to divide ('/') by zero.\n"); return NIL; }
    } else {
      if (arg->fc.value != 0.0) f /= arg->fc.value;
      else { print (thisAgent, "Error: attempt to divide ('/') by zero.\n"); return NIL; }
    }
  }
  return make_float_constant (thisAgent, f);
}

/* --------------------------------------------------------------------
                     Integer Division (Quotient)

   Takes two int_constant arguments, and returns their quotient.
-------------------------------------------------------------------- */

Symbol *div_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *arg1, *arg2;

  arg1 = static_cast<symbol_union *>(args->first);
  arg2 = static_cast<symbol_union *>(args->rest->first);
  
  if (arg1->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: non-integer (%y) passed to div function\n",
                        arg1);
    return NIL;
  }
  if (arg2->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: non-integer (%y) passed to div function\n",
                        arg2);
    return NIL;
  }

  if (arg2->ic.value == 0) {
    print (thisAgent, "Error: attempt to divide ('div') by zero.\n");
    return NIL;
  }
  
  return make_int_constant (thisAgent, arg1->ic.value / arg2->ic.value);
 /* Warning: ANSI doesn't say precisely what happens if one or both of the
    two args is negative. */
}

/* --------------------------------------------------------------------
                          Integer Modulus

   Takes two int_constant arguments (x,y) and returns (x mod y), i.e.,
   the remainder after dividing x by y.
-------------------------------------------------------------------- */

Symbol *mod_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol *arg1, *arg2;

  arg1 = static_cast<symbol_union *>(args->first);
  arg2 = static_cast<symbol_union *>(args->rest->first);
  
  if (arg1->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: non-integer (%y) passed to mod function\n",
                        arg1);
    return NIL;
  }
  if (arg2->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: non-integer (%y) passed to mod function\n",
                        arg2);
    return NIL;
  }

  if (arg2->ic.value == 0) {
    print (thisAgent, "Error: attempt to divide ('mod') by zero.\n");
    return NIL;
  }
  
  return make_int_constant (thisAgent, arg1->ic.value % arg2->ic.value);
 /* Warning:  ANSI guarantees this does the right thing if both args are
    positive.  If one or both is negative, it only guarantees that
    (a/b)*b + a%b == a. */
}

/*
 * SIN_RHS_FUNCTION_CODE
 *
 * Returns as a float the sine of an angle measured in radians.
 */
Symbol *sin_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg;
    float  arg_value;

    if (!args) {
	print(thisAgent, "Error: 'sin' function called with no arguments\n");
	return NIL;
    }

    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
	arg_value = arg->fc.value;
    else if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
	arg_value = (float) arg->ic.value;
    else {
	print_with_symbols(thisAgent, "Error: 'sin' function called with non-numeric argument %y\n", arg);
	return NIL;
    }

    return make_float_constant(thisAgent, (float)sin(arg_value));
}


/*
 * COS_RHS_FUNCTION_CODE
 *
 * Returns as a float the cosine of an angle measured in radians.
 */
Symbol *cos_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg;
    float  arg_value;

    if (!args) {
	print(thisAgent, "Error: 'cos' function called with no arguments\n");
	return NIL;
    }

    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
	arg_value = arg->fc.value;
    else if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
	arg_value = (float) arg->ic.value;
    else {
	print_with_symbols(thisAgent, "Error: 'cos' function called with non-numeric argument %y\n", arg);
	return NIL;
    }
    return make_float_constant(thisAgent, (float)cos(arg_value));
}


/*
 * SQRT_RHS_FUNCTION_CODE
 *
 * Returns as a float the square root of its argument (integer or float).
 */
Symbol *sqrt_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg;
    float  arg_value;

    if (!args) {
	print(thisAgent, "Error: 'sqrt' function called with no arguments\n");
	return NIL;
    }

    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
	arg_value = arg->fc.value;
    else if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
	arg_value = (float)arg->ic.value;
    else {
	print_with_symbols(thisAgent, "Error: 'sqrt' function called with non-numeric argument %y\n", arg);
	return NIL;
    }
    return make_float_constant(thisAgent, (float)sqrt(arg_value));
}


/*
 * ATAN2_RHS_FUNCTION_CODE
 *
 * Returns as a float in radians the arctangent of (first_arg/second_arg)
 * which are floats or integers.
 */
Symbol *atan2_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg;
    cons *c;
    float  numer_value,
           denom_value;

    if (!args) {
	print(thisAgent, "Error: 'atan2' function called with no arguments\n");
	return NIL;
    }

    for (c=args; c!=NIL; c=c->rest) {
	arg = static_cast<symbol_union *>(c->first);
	if (   (arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE)
	    && (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
	    print_with_symbols (thisAgent, "Error: non-number (%y) passed to atan2\n",
				arg);
	    return NIL;
	}
    }

    if (!args->rest) {
	print(thisAgent, "Error: 'atan2' function called with only one argument\n");
	return NIL;
    }

    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
	numer_value = arg->fc.value;
    else
	numer_value = (float) arg->ic.value;

    c = args->rest;
    if (c->rest) {
	print(thisAgent, "Error: 'atan2' function called with more than two arguments.\n");
	return NIL;
    }
    arg = static_cast<symbol_union *>(c->first);
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
	denom_value = arg->fc.value;
    else
	denom_value = (float) arg->ic.value;

    return make_float_constant(thisAgent, (float)atan2(numer_value, denom_value));
}


/*
 * ABS_RHS_FUNCTION_CODE
 *
 * Returns the absolute value of a float as a float, of an int as an int.
 */
Symbol *abs_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg,
           *return_value;

    if (!args) {
	print(thisAgent, "Error: 'abs' function called with no arguments\n");
	return NIL;
    }

    arg = static_cast<symbol_union *>(args->first);
    if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
	return_value = make_float_constant(thisAgent, (float)fabs(arg->fc.value));
    else if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
	return_value = make_int_constant(thisAgent, (arg->ic.value<0) ? -arg->ic.value : arg->ic.value);
    else {
	print_with_symbols(thisAgent, "Error: 'abs' function called with non-numeric argument %y\n", arg);
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

Symbol *int_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol * sym;

  if (!args) {
    print (thisAgent, "Error: 'int' function called with no arguments.\n");
    return NIL;
  }

  if (args->rest) {
    print (thisAgent, "Error: 'int' takes exactly 1 argument.\n");
    return NIL;
  }

  sym = (Symbol *) args->first;
  if (sym->common.symbol_type == VARIABLE_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: variable (%y) passed to 'int' RHS function.\n",
			sym);
    return NIL;
  } else if (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: identifier (%y) passed to 'int' RHS function.\n",
			sym);
    return NIL;
  } else if (sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {
    long int_val;

    errno = 0;
    int_val = strtol(symbol_to_string (thisAgent, sym, FALSE, NIL, 0), NULL, 10);
    if (errno) {
      print (thisAgent, "Error: bad integer (%y) given to 'int' RHS function\n",
	     sym);
      return NIL;
    }
    return make_int_constant (thisAgent, int_val);
  } else if (sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
    symbol_add_ref(sym) ;
    return sym;
  } else if (sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
    double int_part;
    modf((double)sym->fc.value, &int_part);
    return make_int_constant(thisAgent, (long) int_part);
  }

  print (thisAgent, "Error: unknown symbol type (%y) given to 'int' RHS function\n",
	 sym);
  return NIL;
}


/* --------------------------------------------------------------------
                         float

   Casts the given symbol into an float.  If the symbol is a sym
   constant, a conversion is done.  If the symbol is an int, then
   the integer portion is converted.
-------------------------------------------------------------------- */

Symbol *float_rhs_function_code (agent* thisAgent, list *args, void* user_data) {
  Symbol * sym;

  if (!args) {
    print (thisAgent, "Error: 'float' function called with no arguments.\n");
    return NIL;
  }

  if (args->rest) {
    print (thisAgent, "Error: 'float' takes exactly 1 argument.\n");
    return NIL;
  }

  sym = (Symbol *) args->first;
  if (sym->common.symbol_type == VARIABLE_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: variable (%y) passed to 'float' RHS function.\n",
			sym);
    return NIL;
  } else if (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: identifier (%y) passed to 'float' RHS function.\n",
			sym);
    return NIL;
  } else if (sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {
    double float_val;

    errno = 0;
    float_val = my_strtod(symbol_to_string (thisAgent, sym, FALSE, NIL, 0), NULL, 10);
    if (errno) {
      print (thisAgent, "Error: bad float (%y) given to 'float' RHS function\n",
	     sym);
      return NIL;
    }
    return make_float_constant (thisAgent, (float)float_val);
  } else if (sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
    symbol_add_ref(sym) ;
    return sym;
  } else if (sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
    return make_float_constant(thisAgent, (float) sym->ic.value);
  }

  print (thisAgent, "Error: unknown symbol type (%y) given to 'float' RHS function\n",
	 sym);
  return NIL;
}

/* voigtjr 6/12/2007: added these built in functions on laird's request
these are straight out of the <8.6 kernel */

/***********************************************************
These RHS functions are used in the Quake-Soar agent.  They
are modified versions of the routines taken from TacAir-Soar.
*************************************************************/

/* "Normalizes" an integral heading to be between -180 and +180 */
long normalize_heading_int(long n)
{
    /* we need to make sure that -180 < value <= 180 so we modify */
    /*  the original rounded value using the fact that for heading, */
    /*  for any integer value of x, the following holds:            */
    /*            heading1 = x*360 + heading2                      */
    while (n <= -180)
        n += 360;
    while (n > 180)
        n -= 360;

    return n;
}

/* "Normalizes" a floating point heading to be between -180.0 and +180.0 */
float normalize_heading_float(float n)
{
    /* we need to make sure that -180 < value <= 180 so we modify */
    /*  the original rounded value using the fact that for heading, */
    /*  for any integer value of x, the following holds:            */
    /*            heading1 = x*360 + heading2                      */
    while (n <= -180.0)
        n += 360.0;
    while (n > 180.0)
        n -= 360.0;

    return n;
}

long round_off_heading_int(long n, long m)
{
    long unbounded_rounded;

    /* need to round the first (i_n) to the nearest second (i_m) */
    if (n < 0)
        unbounded_rounded = m * (long) ((n - (long) (m / 2)) / m);
    else
        unbounded_rounded = m * (long) ((n + (long) (m / 2)) / m);

    return unbounded_rounded;
}

float round_off_heading_float(float n, float m)
{
    float n_10, m_10, unbounded_rounded;
    double ip;
    double ip2;

    /* OK.  Both n and m can have tenths, so multiply by 10 and treat
       as integers */
    modf((n * 10.0), &ip);
    n_10 = (float) ip;
    modf((m * 10.0), &ip);
    m_10 = (float) ip;

    if (n_10 < 0.0) {

        modf((m_10 / 2.0), &ip2);
        modf(((n_10 - ip2) / m_10), &ip);
        unbounded_rounded = (float) (m_10 * ip);
    } else {

        modf((m_10 / 2.0), &ip2);
        modf(((n_10 + ip2) / m_10), &ip);
        unbounded_rounded = (float) (m_10 * ip);
    }

    /* Divide by 10 to get tenths back and return */
    return (float) (unbounded_rounded / 10.0);
}

Symbol *round_off_heading_air_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg;
    float n = 0, f_m = 0;
    long i_m = 0;
    cons *c;
    bool float_found = FALSE;

    if (!args) {
        print(thisAgent, "Error: 'round_off_heading' function called with no arguments\n");
        return NIL;
    }

    if (!args->rest) {
        /* --- only one argument --- */
        print(thisAgent, "Error: 'round_off_heading' function called with only one argument.\n");
        return NIL;
    }

    /* --- two or more arguments --- */
    arg = (Symbol *) args->first;
    if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        n = (float) arg->ic.value;
    else if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
        n = arg->fc.value;
    }

    c = args->rest;
    if (c->rest) {
        /* --- more than two arguments --- */
        print(thisAgent, "Error: 'round_off_heading' function called with more than two arguments.\n");
        return NIL;
    }
    arg = (Symbol *) c->first;
    if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        i_m = (long) arg->ic.value;
    else if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
        float_found = TRUE;
        f_m = arg->fc.value;
    }

    /* Now, deal with the arguments based on type and return result */
    if (float_found)
        return make_float_constant(thisAgent, normalize_heading_float(round_off_heading_float(n, f_m)));
    else
        return make_int_constant(thisAgent, normalize_heading_int(round_off_heading_int((long) n, i_m)));

}

/* code for round_off_heading */

/* --------------------------------------------------------------------
                                round_off

 Takes two numbers and returns the first rounded to the nearest second.
-------------------------------------------------------------------- */
Symbol *round_off_air_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg;
    float n = 0, f_m = 0;
    long i_m = 0;
    cons *c;
    bool float_found = FALSE;

    if (!args) {
        print(thisAgent, "Error: 'round_off' function called with no arguments\n");
        return NIL;
    }

    if (!args->rest) {
        /* --- only one argument --- */
        print(thisAgent, "Error: 'round_off' function called with only one argument.\n");
        return NIL;
    }

    /* --- two or more arguments --- */
    arg = (Symbol *) args->first;
    if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        n = (float) arg->ic.value;
    else if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
        n = arg->fc.value;
    }

    c = args->rest;
    if (c->rest) {
        /* --- more than two arguments --- */
        print(thisAgent, "Error: 'round_off' function called with more than two arguments.\n");
        return NIL;
    }
    arg = (Symbol *) c->first;
    if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
        i_m = arg->ic.value;
    else if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
        float_found = TRUE;
        f_m = arg->fc.value;
    }

    /* Now, deal with the arguments based on type and return result */
    if (float_found)
        return make_float_constant(thisAgent, round_off_heading_float(n, f_m));
    else
        return make_int_constant(thisAgent, round_off_heading_int((long) n, i_m));
}

typedef double float64;
#define PI 3.141592653589
#define PI_OVER_TWO (PI/2)
#define TWO_PI (PI*2)
#define RAD_TO_DEG(X) ((X*180)/PI)
#define X 0
#define Y 1
#define Z 2

void vector_from_to_position(float64 pos1[3], float64 pos2[3], float64 vector[3])
{
    vector[X] = pos2[X] - pos1[X];
    vector[Y] = pos2[Y] - pos1[Y];
    vector[Z] = pos2[Z] - pos1[Z];
}

void vec2_norm(float64 v[3], float64 r[3], int abort)
{
    float64 mag, mag2;
    mag2 = v[X] * v[X] + v[Y] * v[Y];
    mag = sqrt(mag2);
    if (!abort && (mag < 0.01)) {
        r[0] = 1.0;
        r[1] = 0.0;
        return;
    }
    r[0] = v[0] / mag;
    r[1] = v[1] / mag;
}

float64 convert_to_soar_angle(float64 heading_in_rads)
{
    double heading;

    /* Not only correct, but more efficient! */
    heading = heading_in_rads - PI_OVER_TWO;
    if (heading < 0.0)
        heading += TWO_PI;
    heading = TWO_PI - heading;
    if (heading > PI)
        heading -= TWO_PI;

    return heading;
}

void hrl_xydof_to_heading(float64 xydof[3], float64 * output)
{
    float64 heading_in_rads;

    heading_in_rads = convert_to_soar_angle(atan2(xydof[Y], xydof[X]));

    (*output) = heading_in_rads;
}

long air_soar_round_off_angle(long n, long m)
{
    long unbounded_rounded, bounded_rounded;

    /* need to round the first (n) to the nearest second (m) */
    if (n < 0)
        unbounded_rounded = m * (long) ((n - (long) (m / 2)) / m);
    else
        unbounded_rounded = m * (long) ((n + (long) (m / 2)) / m);

    /* we need to make sure that -180 < value <= 180. */

    bounded_rounded = (unbounded_rounded % 360);
    if (bounded_rounded > 180)
        bounded_rounded -= 360;
    if (bounded_rounded <= -180)
        bounded_rounded += 360;

    return bounded_rounded;
}

float64 bracket_rad_to_deg(float64 var)
{
    return (float64) air_soar_round_off_angle((long) RAD_TO_DEG(var), 1);
}

long convert(float64 flo)
{
    long tempx;
    tempx = (long) flo;
    return tempx;
}

long heading_to_point(long current_x, long current_y, long x, long y)
{
    float64 plane_pos[3], waypoint_pos[3], dir[3];
    float64 heading;

    plane_pos[0] = (float64) current_x;
    plane_pos[1] = (float64) current_y;
    plane_pos[2] = 0;

    waypoint_pos[0] = (float64) x;
    waypoint_pos[1] = (float64) y;
    waypoint_pos[2] = 0;

    vector_from_to_position(plane_pos, waypoint_pos, dir);
    vec2_norm(dir, dir, FALSE);
    hrl_xydof_to_heading(dir, &heading);

    return convert(bracket_rad_to_deg(heading));
}

/* --------------------------------------------------------------------
                                compute-heading

 Takes 4 args and returns integer heading from x1,y1 to x2,y2
-------------------------------------------------------------------- */

Symbol *compute_heading_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg;
    long current_x, current_y;
    long waypoint_x, waypoint_y;
    int count;
    cons *c;

    if (!args) {
        print(thisAgent, "Error: 'compute-heading' function called with no arguments\n");
        return NIL;
    }

    for (c = args; c != NIL; c = c->rest) {
        arg = (Symbol *) c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
            (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols(thisAgent, "Error: non-number (%y) passed to - compute-heading\n", arg);
            return NIL;
        }
    }

    count = 1;

    for (c = args->rest; c != NIL; c = c->rest) {
        arg = (Symbol *) c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
            (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols(thisAgent, "Error: non-number (%y) passed to compute-heading function.\n", arg);
            return NIL;
        } else {
            count++;
        }
    }

    if (count != 4) {
        print(thisAgent, "Error: 'compute-heading' takes exactly 4 arguments.\n");
        return NIL;
    }

    arg = (Symbol *) args->first;
    current_x = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ? arg->ic.value : (long) arg->fc.value;

    arg = (Symbol *) args->rest->first;
    current_y = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ? arg->ic.value : (long) arg->fc.value;

    arg = (Symbol *) args->rest->rest->first;
    waypoint_x = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ? arg->ic.value : (long) arg->fc.value;

    arg = (Symbol *) args->rest->rest->rest->first;
    waypoint_y = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ? arg->ic.value : (long) arg->fc.value;

    return make_int_constant(thisAgent, heading_to_point(current_x, current_y, waypoint_x, waypoint_y));
}

/* --------------------------------------------------------------------
                                compute-range

 Takes 4 args and returns integer range from x1,y1 to x2,y2
-------------------------------------------------------------------- */

Symbol *compute_range_rhs_function_code(agent* thisAgent, list *args, void* user_data)
{
    Symbol *arg;
    double current_x, current_y;
    double waypoint_x, waypoint_y;
    int count;
    cons *c;

    if (!args) {
        print(thisAgent, "Error: 'compute-range' function called with no arguments\n");
        return NIL;
    }

    for (c = args; c != NIL; c = c->rest) {
        arg = (Symbol *) c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE)
            && (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols(thisAgent, "Error: non-number (%y) passed to - compute-range\n", arg);
            return NIL;
        }
    }

    count = 1;

    for (c = args->rest; c != NIL; c = c->rest) {
        arg = (Symbol *) c->first;
        if ((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE)
            && (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
            print_with_symbols(thisAgent, "Error: non-number (%y) passed to compute-range function.\n", arg);
            return NIL;
        } else {
            count++;
        }
    }

    if (count != 4) {
        print(thisAgent, "Error: 'compute-range' takes exactly 4 arguments.\n");
        return NIL;
    }

    arg = (Symbol *) args->first;
    current_x = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ? (double) arg->ic.value : arg->fc.value;

    arg = (Symbol *) args->rest->first;
    current_y = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ? (double) arg->ic.value : arg->fc.value;

    arg = (Symbol *) args->rest->rest->first;
    waypoint_x = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ? (double) arg->ic.value : arg->fc.value;

    arg = (Symbol *) args->rest->rest->rest->first;
    waypoint_y = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ? (double) arg->ic.value : arg->fc.value;

    return make_int_constant(thisAgent, (long int) sqrt((current_x - waypoint_x)
                                             * (current_x - waypoint_x)
                                             + (current_y - waypoint_y)
                                             * (current_y - waypoint_y)));
}

/* ====================================================================

                  Initialize the Built-In RHS Math Functions

====================================================================
*/

void init_built_in_rhs_math_functions (agent* thisAgent)
{
	add_rhs_function (thisAgent, make_sym_constant (thisAgent, "+"), plus_rhs_function_code,
					-1, TRUE, FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant (thisAgent, "*"), times_rhs_function_code,
					-1, TRUE, FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant (thisAgent, "-"), minus_rhs_function_code,
					-1, TRUE, FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant (thisAgent, "/"), fp_divide_rhs_function_code,
					-1, TRUE, FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant (thisAgent, "div"), div_rhs_function_code,
					2, TRUE, FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant (thisAgent, "mod"), mod_rhs_function_code,
					2, TRUE, FALSE, 0);

	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "sin"),
			sin_rhs_function_code,
			1,
			TRUE,
			FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "cos"),
			cos_rhs_function_code,
			1,
			TRUE,
			FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "atan2"),
			atan2_rhs_function_code,
			2,
			TRUE,
			FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "sqrt"),
			sqrt_rhs_function_code,
			1,
			TRUE,
			FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "abs"),
			abs_rhs_function_code,
			1,
			TRUE,
			FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "int"),
			int_rhs_function_code,
			1,
			TRUE,
			FALSE, 0);
	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "float"),
			float_rhs_function_code,
			1,
			TRUE,
			FALSE, 0);

	/* voigtjr 6/12/2007: added these built in functions on laird's request
	these are straight out of the <8.6 kernel */
	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "round-off-heading"), 
		round_off_heading_air_rhs_function_code, 2, TRUE, FALSE, 0);

	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "round-off"), 
		round_off_air_rhs_function_code, 2, TRUE, FALSE, 0);

	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "compute-heading"), 
		compute_heading_rhs_function_code, 4, TRUE, FALSE, 0);

	add_rhs_function (thisAgent, make_sym_constant(thisAgent, "compute-range"), 
		compute_range_rhs_function_code, 4, TRUE, FALSE, 0);

}

void remove_built_in_rhs_math_functions (agent* thisAgent)
{
	// DJP-FREE: These used to call make_sym_constant, but the symbols must already exist and if we call make here again we leak a reference.
	remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "+"));
	remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "*"));
	remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "-"));
	remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "/"));
	remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "div"));
	remove_rhs_function (thisAgent, find_sym_constant (thisAgent, "mod"));
	remove_rhs_function (thisAgent, find_sym_constant(thisAgent, "sin"));
	remove_rhs_function (thisAgent, find_sym_constant(thisAgent, "cos"));
	remove_rhs_function (thisAgent, find_sym_constant(thisAgent, "atan2"));
	remove_rhs_function (thisAgent, find_sym_constant(thisAgent, "sqrt"));
	remove_rhs_function (thisAgent, find_sym_constant(thisAgent, "abs"));
	remove_rhs_function (thisAgent, find_sym_constant(thisAgent, "int"));
	remove_rhs_function (thisAgent, find_sym_constant(thisAgent, "float"));

	/* voigtjr 6/12/2007: added these built in functions on laird's request
	these are straight out of the <8.6 kernel */
	remove_rhs_function(thisAgent, find_sym_constant(thisAgent, "round-off-heading"));
	remove_rhs_function(thisAgent, find_sym_constant(thisAgent, "round-off"));
	remove_rhs_function(thisAgent, find_sym_constant(thisAgent, "compute-heading"));
	remove_rhs_function(thisAgent, find_sym_constant(thisAgent, "compute-range"));
}
