/////////////////////////////////////////////////////////////////
// KernelHeaders file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : February 2007
//
// Simple point for collecting together the headers required for
// external interface to the Soar kernel (not KernelSML but the internal
// kernel library).
//
/////////////////////////////////////////////////////////////////

// NOTE: Requires that portability.h be loaded first
#include <portability.h>

#include "kernel.h" 
#include "init_soar.h"
#include "mem.h"
#include "lexer.h"
#include "chunk.h"
#include "callback.h"
#include "agent.h"
#include "init_soar.h"
#include "rhsfun.h"
#include "production.h" // for struct multi_attributes
#include "print.h"      // for symboltostring
#include "decide.h"
#include "recmem.h"
#include "symtab.h"
#include "io_soar.h"
#include "wmem.h"
