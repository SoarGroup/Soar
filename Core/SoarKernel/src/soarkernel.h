/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* =======================================================================
 *
 *                         soarkernel.h
 *
 * =======================================================================
 */

#ifndef _SOAR_H_INCLUDED
#define _SOAR_H_INCLUDED

/* =====================================================================*/

#include "kernel.h"

/* =====================================================================*/

/* -------------------------------------------------- */
/*              Names of Rete Structures              */
/* (only pointers to these are used outside the rete) */
/* -------------------------------------------------- */

struct token_struct;
struct rete_node_struct;
struct node_varnames_struct;

#include "mem.h"
#include "lexer.h"
#include "symtab.h"
#include "gdatastructs.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "production.h"
#include "gsysparam.h"
#include "init_soar.h"
#include "wmem.h"
#include "tempmem.h"
#include "decide.h"
#include "consistency.h"
#include "parser.h"
#include "print.h"
#include "reorder.h"
#include "recmem.h"
#include "backtrace.h"
#include "chunk.h"
#include "osupport.h"
#include "rete.h"
#include "trace.h"
#include "callback.h"
#include "io_soar.h"
#include "exploration.h"
#include "reinforcement_learning.h"
#include "episodic_memory.h"
#include "semantic_memory.h"
#include "explain.h"
#include "agent.h"

#endif /* _SOAR_H_INCLUDED */
