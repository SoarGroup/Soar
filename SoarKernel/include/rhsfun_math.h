/*************************************************************************
 *
 *  file:  rhsfun_math.h
 *
 * =======================================================================
 *  used only by rhsfun.cpp ...  explicitly add to file and drop this one?
 *  
 * =======================================================================
 *
 * Copyright (c) 1995-1999 Carnegie Mellon University,
 *                         The Regents of the University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.
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
