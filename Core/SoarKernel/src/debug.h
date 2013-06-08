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

// Comment out the following line for release versions or if you don't want
#define SOAR_DEBUG_UTLITIES
#ifdef SOAR_DEBUG_UTLITIES

#include <portability.h>
#include "soar_module.h"

/**
 * @brief Contains the parameters for the memory consolidation code
 */
class debug_param_container: public soar_module::param_container
{
    public:

        // storage
        soar_module::boolean_param *debug_setting_1;

        debug_param_container( agent *new_agent ): soar_module::param_container( new_agent )
        {
            debug_setting_1           = new soar_module::boolean_param("setting1", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
            add(debug_setting_1);
        }
};

extern void debug_init_db( agent *my_agent);

#endif
#endif

