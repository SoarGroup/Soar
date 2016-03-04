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
#include <run_soar.h>
#include "portability.h"
#include "kernel.h"
#include "ebc.h"
#include "decide.h"
#include "callback.h"
#include "io_link.h"
#include "lexer.h"
#include "mem.h"
#include "print.h"
#include "agent.h"
#include "production.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "instantiation.h"
#include "symbol.h"
#include "working_memory.h"
#ifndef NO_SVS
#include "svs_interface.h"
#endif
