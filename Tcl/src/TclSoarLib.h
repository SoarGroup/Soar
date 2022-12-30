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
#include <thread>
#include <mutex>
#include <condition_variable>

#include "sml_Client.h"
#include "sml_Names.h"
#include "sml_Utils.h"

#include "enums.h"
#include "soar_instance.h"

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

class EXPORT TclSoarLib
{
    public:
        TclSoarLib(sml::Kernel* pKernel);
        ~TclSoarLib();

        // Handles a message made to the tclsoarlib
        // Options are:
        //   'on'  - start listening for soar events
        //   'off' - stop listening for soar events
        //   'create agent_name' - creates a new slave interpreter of the given name
        //   'destroy agent_name' - destroys the slave interpreter of the given name
        bool handle_message(std::string message);

        static const int CREATE_TCL_SLAVE = 0;
        static const int DESTROY_TCL_SLAVE = 1;
        static const int SHUTDOWN_TCL_THREAD = 2;

    private:
        sml::Kernel* m_kernel;

        bool turnOn();
        bool turnOff();

        // The thread which creates and destroys all interpreters
        std::thread lib_thread;
        std::mutex interp_mutex;
        bool thread_ready;
        friend void* launch_tcl(void* lib_ptr);

        // Managing message passing to the thread (sending commands)
        std::mutex command_mutex;
        std::condition_variable cv;
        bool new_command = false;
        int command_type;
        std::string command_info;
        bool finished_command = false;
        void send_thread_command(int type, std::string info);

        // The tcl interpreter and initialization functions
        Tcl_Interp* m_interp;

        bool initialize_Master();
        bool initialize_Tcl_Interpreter();

        static const char *tclRHS( sml::smlRhsEventId id, void *pData, sml::Agent *pAgent, char const *pFunc, char const *pArg, int *bufSize, char *buf );

        // Methods to send commands to the tcl interpreter
        bool evaluateDirCommand(const std::string command);
        std::string& EscapeTclString(const char* in, std::string& out);
        int GlobalEval(const std::string& command, std::string& result);
        int GlobalDirEval(const std::string& command, std::string& result);
};

} // extern "C"


#endif /* TCLSOARLIB_H_ */
