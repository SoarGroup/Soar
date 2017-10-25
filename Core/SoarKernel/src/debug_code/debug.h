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

#include "kernel.h"

void debug_set_mode_info(trace_mode_info mode_info[num_trace_modes], bool pEnabled);
void initialize_debug_trace(trace_mode_info mode_info[num_trace_modes]);
void debug_trace_set(int dt_num, bool pEnable);
void debug_trace_on();
void debug_trace_off();
#endif

