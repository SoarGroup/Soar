/*
 * TclSoarLib.h
 *
 *  Created on: Nov 4, 2013
 *      Author: mazzin
 */

#ifndef TCLSOARLIB_H_
#define TCLSOARLIB_H_

#include "portability.h"
#include <string>
#include <sstream>
#include <iostream>

#include "sml_Client.h"
#include "sml_Names.h"
#include "sml_Utils.h"

#include "enums.h"
#include "soar_instance.h"
#include "debug.h"

#ifndef _TCL
#  ifndef MAC_OS_X
// On Windows this is Tcl_Tk/include/tcl.h
// On Linux this is the system tcl.h
#     include <tcl.h>
#  else
#     include <Tcl/tcl.h>
#  endif
#endif

// tcl.h defined panic to Tcl_panic.
#ifdef panic
#undef panic
#endif

// Some tcl interfaces were changed between tcl 8.3 and tcl 8.4
// these defines are intended to work around these interface changes.
#if (TCL_MAJOR_VERSION >= 8) && (TCL_MINOR_VERSION >= 4)
#   define TCL_CONST const
#else
#   define TCL_CONST
#endif
//

#ifdef __cplusplus
extern "C"
#endif
{
  extern "C" class EXPORT TclSoarLib
  {
    public:

      TclSoarLib(sml::Kernel* pKernel);
      void init_TclSoarLib();
      ~TclSoarLib();

      bool turnOn();
      bool turnOff();

      sml::Kernel* m_kernel;

    private:

      Tcl_Interp *m_interp;

      bool initialize_Master();
      bool initialize_Tcl_Interpreter();

      bool evaluateDirCommand (const std::string command);

      std::string& EscapeTclString(const char* in, std::string& out);
      int GlobalEval(const std::string& command, std::string& result);
      int GlobalDirEval(const std::string& command, std::string& result);
  };
} // extern "C"

#endif /* TCLSOARLIB_H_ */
