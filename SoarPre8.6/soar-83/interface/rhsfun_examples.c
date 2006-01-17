/***********************************************************

  file:  rhsfun_examples.c


These RHS functions are used in the Quake-Soar agent.  They
are modified versions of the routines taken from TacAir-Soar.
*************************************************************/


#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include "soar.h"


#include "rhsfun_examples.h"


/* M_PI sometimes isn't picked up from math.h */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/* "Normalizes" an integral heading to be between -180 and +180 */
long normalize_heading_int(long n)
{
    /* we need to make sure that -180 < value <= 180 so we modify */
    /*  the original rounded value using the fact that for heading, */
    /*  for any integer value of x, the following holds:            */
    /*            heading1 = x*360 + heading2                      */
    while (n <= -180) n += 360;
    while (n > 180) n -= 360;

    return n;
}

/* "Normalizes" a floating point heading to be between -180.0 and +180.0 */
float normalize_heading_float(float n)
{
    /* we need to make sure that -180 < value <= 180 so we modify */
    /*  the original rounded value using the fact that for heading, */
    /*  for any integer value of x, the following holds:            */
    /*            heading1 = x*360 + heading2                      */
    while (n <= -180.0) n += 360.0;
    while (n > 180.0) n -= 360.0;

    return n;
}

long round_off_heading_int(long n, long m)
{
    long unbounded_rounded;

    /* need to round the first (i_n) to the nearest second (i_m) */
    if (n < 0)
	unbounded_rounded = m * (long)((n - (long)(m/2)) / m);
    else
	unbounded_rounded = m * (long)((n + (long)(m/2)) / m);

    return unbounded_rounded;
}

float round_off_heading_float(float n, float m)
{
    float n_10,
          m_10,
          unbounded_rounded;
    double ip;
    double ip2;

    /* OK.  Both n and m can have tenths, so multiply by 10 and treat
       as integers */
    modf((n*10.0), &ip);
    n_10 = (float)ip;
    modf((m*10.0), &ip);
    m_10 = (float)ip;

    if (n_10 < 0.0) {

	modf((m_10/2.0), &ip2);
	modf(((n_10 - ip2) / m_10), &ip);
	unbounded_rounded = (float)(m_10 * ip); 
    }
    else {

	modf((m_10/2.0), &ip2);
	modf(((n_10 + ip2) / m_10), &ip);
	unbounded_rounded = (float)(m_10 * ip);  
    }

    /* Divide by 10 to get tenths back and return */
    return  (float)(unbounded_rounded / 10.0);
}

Symbol *round_off_heading_air_rhs_function_code (list *args) {
  Symbol *arg;
  float n, f_m;
  long i_m;
  cons *c;
  bool float_found = FALSE;

  if (!args) {
    print ("Error: 'round_off_heading' function called with no arguments\n");
    return NIL;
  }

  if (! args->rest) {
    /* --- only one argument --- */
    print ("Error: 'round_off_heading' function called with only one argument.\n");
    return NIL;
  }

  /* --- two or more arguments --- */
  arg = args->first;
  if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
      n = (float) arg->ic.value;
  else if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
      n = arg->fc.value;
  }

  c = args->rest;
  if (c->rest) {
    /* --- more than two arguments --- */
    print("Error: 'round_off_heading' function called with more than two arguments.\n");
    return NIL;
  }
  arg = c->first;
  if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
      i_m = (long) arg->ic.value;
  else if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
      float_found = TRUE;
      f_m = arg->fc.value;
  }

  /* Now, deal with the arguments based on type and return result */
  if (float_found)
      return make_float_constant( normalize_heading_float( round_off_heading_float(n, f_m)));
  else
      return make_int_constant( normalize_heading_int( round_off_heading_int((long)n, i_m)));

}

/* code for round_off_heading */

/* --------------------------------------------------------------------
                                round_off

 Takes two numbers and returns the first rounded to the nearest second.
-------------------------------------------------------------------- */
Symbol *round_off_air_rhs_function_code (list *args) {
  Symbol *arg;
  float n, f_m;
  long i_m;
  cons *c;
  bool float_found = FALSE;

  if (!args) {
    print ("Error: 'round_off' function called with no arguments\n");
    return NIL;
  }

  if (! args->rest) {
    /* --- only one argument --- */
    print ("Error: 'round_off' function called with only one argument.\n");
    return NIL;
  }

  /* --- two or more arguments --- */
  arg = args->first;
  if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
      n = (float)arg->ic.value;
  else if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
      n = arg->fc.value;
  }

  c = args->rest;
  if (c->rest) {
    /* --- more than two arguments --- */
    print("Error: 'round_off' function called with more than two arguments.\n");
    return NIL;
  }
  arg = c->first;
  if (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
      i_m = arg->ic.value;
  else if (arg->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
      float_found = TRUE;
      f_m = arg->fc.value;
  }

  /* Now, deal with the arguments based on type and return result */
  if (float_found)
      return make_float_constant(round_off_heading_float(n, f_m));
  else
      return make_int_constant(round_off_heading_int((long)n, i_m));
}



typedef double float64;
#define PI 3.141592653589
#define PI_OVER_TWO (PI/2)
#define TWO_PI (PI*2)
#define RAD_TO_DEG(X) ((X*180)/PI)
#define X 0
#define Y 1
#define Z 2

void vector_from_to_position(pos1,pos2,vector)
float64 pos1[3];
float64 pos2[3];
float64 vector[3];
{
	vector[X] = pos2[X] - pos1[X];
	vector[Y] = pos2[Y] - pos1[Y];
	vector[Z] = pos2[Z] - pos1[Z];
}

void vec2_norm(v, r, abort)
float64 v[3], r[3];
int abort;
{
	float64 mag,mag2;
	mag2 = v[X] * v[X] + v[Y] * v[Y];
	mag  = sqrt(mag2);
	if (!abort && (mag < 0.01))
	{
		r[0] = 1.0;
		r[1] = 0.0;
		return; 
	}
	r[0] = v[0]/mag;
	r[1] = v[1]/mag; 
}

float64 convert_to_soar_angle (float64 heading_in_rads)
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

void hrl_xydof_to_heading(xydof,output)
float64 xydof[3];
float64 *output;
{
    float64 heading_in_rads;
	
    heading_in_rads = convert_to_soar_angle(atan2(xydof[Y], xydof[X]));
	
    (*output) = heading_in_rads;
}

long air_soar_round_off_angle (long n, long m)
{
	long unbounded_rounded, bounded_rounded;
	
	/* need to round the first (n) to the nearest second (m) */
	if (n < 0)
		unbounded_rounded = m * (long)((n - (long)(m/2)) / m);
	else
		unbounded_rounded = m * (long)((n + (long)(m/2)) / m);
	
	/* we need to make sure that -180 < value <= 180. */
	
	bounded_rounded = (unbounded_rounded % 360);
	if (bounded_rounded > 180)
		bounded_rounded -= 360;
	if (bounded_rounded <= -180)
		bounded_rounded += 360;
	
	return bounded_rounded;
}

float64 bracket_rad_to_deg (float64 var)
{
  return (float64) air_soar_round_off_angle ((long) RAD_TO_DEG(var), 1);
}

long convert (flo)
float64 flo;
{
	long tempx;
	tempx = (long) flo;
	return tempx;
}

long
heading_to_point (long current_x, long current_y, long x, long y)
{
  float64 plane_pos[3], waypoint_pos[3], dir[3];
  float64 heading;

  plane_pos[0] = (float64) current_x;
  plane_pos[1] = (float64) current_y;
  plane_pos[2] = 0;

  waypoint_pos[0] = (float64) x;
  waypoint_pos[1] = (float64) y;
  waypoint_pos[2] = 0;

  vector_from_to_position (plane_pos, waypoint_pos, dir);
  vec2_norm(dir, dir, FALSE);
  hrl_xydof_to_heading(dir, &heading);

  return convert(bracket_rad_to_deg(heading));
}

/* --------------------------------------------------------------------
                                compute-heading

 Takes 4 args and returns integer heading from x1,y1 to x2,y2
-------------------------------------------------------------------- */

Symbol *compute_heading_rhs_function_code(list *args) {
  Symbol *arg;
  long current_x, current_y;
  long waypoint_x, waypoint_y;
  int count;
  cons *c;
  
  if (!args) {
    print ("Error: 'compute-heading' function called with no arguments\n");
    return NIL;
  }
  
  for(c = args; c != NIL; c = c->rest) {
    arg = c->first;
    if((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
       (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
      print_with_symbols("Error: non-number (%y) passed to - compute-heading\n", arg);
      return NIL;
    }
  }
  
  count = 1;
  
  for(c = args->rest; c != NIL; c = c->rest) {
    arg = c->first;
    if((arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) &&
       (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
      print_with_symbols ("Error: non-number (%y) passed to compute-heading function.\n", arg);
      return NIL;
    }
    else {
      count++;
    }
  }
  
  if(count != 4) {
    print("Error: 'compute-heading' takes exactly 4 arguments.\n");
    return NIL;
  }

  arg = args->first;
  current_x = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ?
              arg->ic.value : (long)arg->fc.value;

  arg = args->rest->first;
  current_y = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ?
              arg->ic.value : (long)arg->fc.value;

  arg = args->rest->rest->first;
  waypoint_x = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ?
               arg->ic.value : (long)arg->fc.value;

  arg = args->rest->rest->rest->first;
  waypoint_y = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ?
               arg->ic.value : (long)arg->fc.value;

  return make_int_constant (heading_to_point(current_x, current_y,
					     waypoint_x, waypoint_y));
}

/* --------------------------------------------------------------------
                                compute-range

 Takes 4 args and returns integer range from x1,y1 to x2,y2
-------------------------------------------------------------------- */


Symbol *compute_range_rhs_function_code (list *args) {
  Symbol *arg;
  double current_x, current_y;
  double waypoint_x, waypoint_y;
  int count;
  cons *c;

  if (!args) {
    print ("Error: 'compute-range' function called with no arguments\n");
    return NIL;
  }

  for (c=args; c!=NIL; c=c->rest) {
    arg = c->first;
    if (   (arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE)
	&& (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
      print_with_symbols ("Error: non-number (%y) passed to - compute-range\n",
                          arg);
      return NIL;
    }
  }

  count = 1;

  for (c = args->rest; c != NIL; c = c->rest)
    {
      arg = c->first;
      if (   (arg->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE)
	  && (arg->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) {
	print_with_symbols ("Error: non-number (%y) passed to compute-range function.\n", arg);
	return NIL;
      } else {
	count++;
      }
    }

  if (count != 4) {
    print ("Error: 'compute-range' takes exactly 4 arguments.\n");
    return NIL;
  }

  arg = args->first;
  current_x = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ?
              (double) arg->ic.value : arg->fc.value;

  arg = args->rest->first;
  current_y = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ?
              (double) arg->ic.value : arg->fc.value;

  arg = args->rest->rest->first;
  waypoint_x = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ?
               (double) arg->ic.value : arg->fc.value;

  arg = args->rest->rest->rest->first;
  waypoint_y = (arg->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ?
               (double) arg->ic.value : arg->fc.value;

  return make_int_constant ((long int)sqrt(    (current_x - waypoint_x)
					     * (current_x - waypoint_x)
					   +   (current_y - waypoint_y)
					     * (current_y - waypoint_y)));
}

 
/*****************************************************************************/

void add_bot_rhs_functions (agent *a) 
{
  add_rhs_function (make_sym_constant("round-off-heading"),
		    round_off_heading_air_rhs_function_code,
		    2,
		    TRUE,
		    FALSE);
  add_rhs_function (make_sym_constant("round-off"),
		    round_off_air_rhs_function_code,
		    2,
		    TRUE,
		    FALSE);
   
  
  add_rhs_function (make_sym_constant("compute-heading"),
		    compute_heading_rhs_function_code,
		    4,
		    TRUE,
		    FALSE);
  
 
  add_rhs_function (make_sym_constant("compute-range"),
		    compute_range_rhs_function_code,
		    4,
		    TRUE,
		    FALSE);
}

void remove_bot_rhs_functions (agent *a) 
{
  remove_rhs_function (make_sym_constant("round-off-heading"));
  remove_rhs_function (make_sym_constant("round-off"));
  remove_rhs_function (make_sym_constant("compute-heading"));
  remove_rhs_function (make_sym_constant("compute-range"));
}


