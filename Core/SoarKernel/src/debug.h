/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
					   debug.h

   @brief debug.h provides some utility functions for inspecting and
   	   	  manipulating the data structures of the Soar kernel at run
   	   	  time.

   	   	  (Not much here now.  Will move some other utility stuff from
   	   	   experimental chunking and memory consolidation branches
   	   	   later.)

------------------------------------------------------------------ */

#ifndef SOARDEBUG_H
#define SOARDEBUG_H

#include "portability.h"
#include "kernel.h"
#include "soar_module.h"
#include "Export.h"

//#define SOAR_DEBUG_UTILITIES

#ifdef SOAR_DEBUG_UTILITIES

#ifdef DEBUG_EPMEM_SQL
static void profile_sql(void *context, const char *sql, sqlite3_uint64 ns)
{ fprintf(stderr, "Execution Time of %llu ms for: %s\n", ns / 1000000, sql);}
static void trace_sql( void* /*arg*/, const char* query ) { fprintf(stderr, "Query: %s\n", query );}
#endif
#endif

extern EXPORT void dprint (TraceMode mode, const char *format, ... );
extern EXPORT void dprint_noprefix (TraceMode mode, const char *format, ... );

extern void debug_test(int type=1);
/**
 * @brief Contains the parameters for the debug command
 */
class debug_param_container: public soar_module::param_container
{
  public:

    soar_module::boolean_param *epmem_commands, *smem_commands, *sql_commands;

    debug_param_container( agent *new_agent );

};

#endif

