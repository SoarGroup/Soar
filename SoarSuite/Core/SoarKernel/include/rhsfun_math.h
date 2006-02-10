/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  rhsfun_math.h
 *
 * =======================================================================
 *  used only by rhsfun.cpp ...  explicitly add to file and drop this one?
 *  
 * =======================================================================
 */

#ifndef RHSFUN_MATH_H
#define RHSFUN_MATH_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef struct agent_struct agent;
typedef struct kernel_struct Kernel;

extern void init_built_in_rhs_math_functions (agent* thisAgent);
extern void remove_built_in_rhs_math_functions (agent* thisAgent);

#ifdef __cplusplus
}
#endif

#endif
