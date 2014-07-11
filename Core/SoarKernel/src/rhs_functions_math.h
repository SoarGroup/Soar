/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  rhs_functions_math.h
 *
 * =======================================================================
 *  used only by rhsfun.cpp ...  explicitly add to file and drop this one?
 *
 * =======================================================================
 */

#ifndef rhs_functions_math_H
#define rhs_functions_math_H


typedef struct agent_struct agent;

extern void init_built_in_rhs_math_functions(agent* thisAgent);
extern void remove_built_in_rhs_math_functions(agent* thisAgent);

#endif
