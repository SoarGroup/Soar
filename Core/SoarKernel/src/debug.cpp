/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
                       debug.cpp

   @brief debug.cpp provides some utility functions for inspecting and
          manipulating the data structures of the Soar kernel at run
          time.

------------------------------------------------------------------ */

#include "debug.h"
#include "debug_defines.h"
#include "agent.h"
#include "episodic_memory.h"
#include "soar_module.h"
#include "soar_instance.h"
#include "lexer.h"
#include "test.h"

debug_param_container::debug_param_container(agent* new_agent): soar_module::param_container(new_agent)
{
    epmem_commands = new soar_module::boolean_param("epmem", off, new soar_module::f_predicate<boolean>());
    smem_commands = new soar_module::boolean_param("smem", off, new soar_module::f_predicate<boolean>());
    sql_commands = new soar_module::boolean_param("sql", off, new soar_module::f_predicate<boolean>());
    add(epmem_commands);
    add(smem_commands);
    add(sql_commands);
}
#ifdef SOAR_DEBUG_UTILITIES

#include "sqlite3.h"

#define DEBUG_BUFFER_SIZE 5000

/* -- Just a simple function that can be called from the debug command.  Something to put random code for testing/debugging -- */
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
//      print_internal_symbols(debug_agent);
            break;
        case 2:
            break;
        case 3:
        {
//      Symbol *newSym  = find_identifier(debug_agent, 'S', 1);
            break;
        }
        case 4:
            break;
            
        case 5:
            break;
            
        case 6:
            break;
            
    }
}



#endif

