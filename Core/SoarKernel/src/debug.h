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

/* --  Comment out the following line for release versions or if you don't
 *     need these utilities. -- */
// #define SOAR_DEBUG_UTLITIES

#ifdef SOAR_DEBUG_UTLITIES

#include <portability.h>
#include "soar_module.h"

#define DEBUG_USE_STDERR_TRACE
//#define DEBUG_EPMEM_SQL
//#define DEBUG_EPMEM_WME_ADD


extern void debug_print(const char *format, ...);
extern void debug_print_db_err();
extern void debug_init_db( agent *my_agent);
extern void debug_print_epmem_table(const char *table_name);
extern void debug_print_smem_table(const char *table_name);

/**
 * @brief Contains the parameters for the debug command
 */
class debug_param_container: public soar_module::param_container
{
    public:

        // storage
        soar_module::boolean_param *epmem_commands, *smem_commands, *sql_commands;

        debug_param_container( agent *new_agent ): soar_module::param_container( new_agent )
        {
            epmem_commands = new soar_module::boolean_param("epmem", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
            smem_commands = new soar_module::boolean_param("smem", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
            sql_commands = new soar_module::boolean_param("sql", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
            add(epmem_commands);
            add(smem_commands);
            add(sql_commands);
            debug_init_db(new_agent);
        }
};

#else
class debug_param_container: public soar_module::param_container{
    public:
        debug_param_container( agent *new_agent ): soar_module::param_container( new_agent ) {}
};
#endif


#endif

