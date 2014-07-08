/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
             debug_print.cpp

   @brief debugPrint.cpp provides some printing functions that are
          not used from the kernel but can be used for debugging,
          for example by being called from gdb.

------------------------------------------------------------------ */

#include "debug.h"

#ifdef SOAR_DEBUG_UTILITIES
#include "print.h"
#include "agent.h"
#include "instantiations.h"
#include "rete.h"
#include "reorder.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "output_manager.h"
#include "prefmem.h"
#include "wmem.h"
#include "soar_instance.h"
#include "test.h"

inline void dprint_string(TraceMode mode, const char *message, bool noPrefix=false)
{
    Output_Manager::Get_OM().print_debug(message, mode, noPrefix);
  }

void dprint (TraceMode mode, const char *format, ...) {

  if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;

  va_list args;
  char buf[PRINT_BUFSIZE];

  va_start (args, format);
  vsprintf (buf, format, args);
  va_end (args);

  dprint_string(mode, buf);

}

void dprint_noprefix (TraceMode mode, const char *format, ...) {

  if (!Output_Manager::Get_OM().debug_mode_enabled(mode)) return;

  va_list args;
  char buf[PRINT_BUFSIZE];

  va_start (args, format);
  vsprintf (buf, format, args);
  va_end (args);

  dprint_string(mode, buf, true);
}

#endif
