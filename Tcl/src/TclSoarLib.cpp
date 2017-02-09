/*
 * TclSoarLib.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: mazzin
 */

#include "TclSoarLib.h"

#include <iostream>
#include <pthread.h>

#ifdef __cplusplus
extern "C"
#endif
{
using namespace sml;
using namespace std;

void* launch_tcl(void* lib_ptr){
    /****** INITIALIZATION ******/
    //cout << "TclThread: Started" << endl;
    TclSoarLib* tcl_lib = (TclSoarLib*)lib_ptr;

    pthread_mutex_lock(&tcl_lib->interp_mutex);
    {
        tcl_lib->m_interp = Tcl_CreateInterp();
        Tcl_FindExecutable(0);

        //cout << "TclThread: Created Interpreter" << endl;

        if (!tcl_lib->initialize_Tcl_Interpreter() || !tcl_lib->initialize_Master())
        {
            pthread_mutex_unlock(&tcl_lib->interp_mutex);
            exit(EXIT_FAILURE);
            return NULL;
        }

        if (tcl_lib->m_kernel)
        {
            std::string output;
            tcl_lib->GlobalEval("smlstartup", output);
        }
    }
    //cout << "TclThread: Initialized Interpreter" << endl;
    pthread_mutex_unlock(&tcl_lib->interp_mutex);

	// Signals the end of initialization
	pthread_barrier_wait(&tcl_lib->interp_barrier);

    /****** COMMAND HANDLING ******/

    bool exit = false;
    while(!exit)
    {
        // Wait until there is a new command
        pthread_barrier_wait(&tcl_lib->interp_barrier);

        std::string command;
        std::string result;
        switch(tcl_lib->thread_command_info.command)
        {
            case CREATE_TCL_SLAVE:
                //cout << "TclThread: create slave " << tcl_lib->thread_command_info.info << endl;
                command = "createSlave \"" + tcl_lib->thread_command_info.info + "\"";
                tcl_lib->GlobalEval(command, result);
                break;
            case DESTROY_TCL_SLAVE:
                //cout << "TclThread: destroy slave " << tcl_lib->thread_command_info.info << endl;
                command = "destroySlave \"" + tcl_lib->thread_command_info.info + "\"";
                tcl_lib->GlobalEval(command, result);
                break;
            case SHUTDOWN_TCL_THREAD:
                //cout << "TclThread: SHUTDOWN received" << endl;
                exit = true;
                break;
        }

        // Let the messenger know that the command was finished
        pthread_barrier_wait(&tcl_lib->interp_barrier);
    }

    /****** SHUTDOWN ******/

    pthread_mutex_lock(&tcl_lib->interp_mutex);
    {
        if (tcl_lib->m_interp)
        {
            if (tcl_lib->m_kernel)
            {
                std::string output;
                tcl_lib->GlobalEval("smlshutdown", output);
            }
            //cout << "TclThread: Deleting Tcl Interpreter" << endl;
            Tcl_DeleteInterp(tcl_lib->m_interp);
            tcl_lib->m_interp = 0;
        }
    }
    pthread_mutex_unlock(&tcl_lib->interp_mutex);

    //cout << "TclThread: Exiting" << endl;
    return NULL;
}

TclSoarLib::TclSoarLib(Kernel* myKernel) :
    m_kernel(0),
    m_interp(0)
{
    m_kernel = myKernel;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&interp_mutex, &mutex_attr);
	pthread_barrier_init(&interp_barrier, NULL, 2);
    pthread_create(&lib_thread, NULL, launch_tcl, (void*)this);

	pthread_barrier_wait(&interp_barrier);
}

TclSoarLib::~TclSoarLib()
{
    //cout << "TclSoarLib::~TclSoarLib()" << endl;
    //cout << "   Waiting for TclThread to exit" << endl;

    // Tell the thread to shutdown
    send_thread_command(SHUTDOWN_TCL_THREAD, "");

    // Wait until the thread exits
    pthread_join(lib_thread, NULL);
    pthread_mutex_destroy(&interp_mutex);
	pthread_barrier_destroy(&interp_barrier);

    m_kernel = 0;

    //cout << "TclSoarLib destroyed" << endl;
}

void TclSoarLib::send_thread_command(TclThreadCommand command, std::string info){
    if (m_interp)
    {
        thread_command_info.command = command;
        thread_command_info.info = info;
        // Wakes the tcl thread up and allows it to process the command
        pthread_barrier_wait(&interp_barrier);
        // Wait until the tcl thread is finished processing the command
        pthread_barrier_wait(&interp_barrier);
    }
}

bool TclSoarLib::handle_message(std::string message)
{
    if(message == "on")
    {
        return turnOn();
    }
    else if(message == "off")
    {
        return turnOff();
    } 
    else if(message.find("create") == 0 && message.size() >= 8)
    {
        send_thread_command(CREATE_TCL_SLAVE, message.substr(7));
        return true;
    } 
    else if(message.find("destroy") == 0 && message.size() >= 9)
    {
        send_thread_command(DESTROY_TCL_SLAVE, message.substr(8));
        return true;
    }
    return false;
}

std::string& TclSoarLib::EscapeTclString(const char* in, std::string& out)
{
    for (const char* c = in; *c != '\0'; ++c)
    {
        if (*c == '\\')
        {
            out += '\\';
        }
        out += *c;
    }
    return out;
}

int TclSoarLib::GlobalDirEval(const std::string& command, std::string& result)
{
    int ret_val = TCL_OK;
    pthread_mutex_lock(&interp_mutex);
    {
        if (!m_interp)
        {
            ret_val = TCL_ERROR;
        }
        else
        {
        
            string tcl_cmd_string;
            
            EscapeTclString(command.c_str(), tcl_cmd_string);
            if (Tcl_Eval(m_interp, (char*) tcl_cmd_string.c_str()) != TCL_OK)
            {
                tcl_cmd_string.erase();
                result = Tcl_GetStringResult(m_interp);
                ret_val = TCL_ERROR;
            }
            else 
            {
                tcl_cmd_string.erase();
                result = Tcl_GetStringResult(m_interp);
            }
        }
    }
    pthread_mutex_unlock(&interp_mutex);
    return TCL_OK;
}

int TclSoarLib::GlobalEval(const std::string& command, std::string& result)
{
    int ret_val = TCL_OK;
    pthread_mutex_lock(&interp_mutex);
    {
        if (!m_interp)
        {
            ret_val = TCL_ERROR;
        }
        else
        {
        
            if (Tcl_Eval(m_interp, (char*) command.c_str()) != TCL_OK)
            {
                result = Tcl_GetStringResult(m_interp);
                std::cout << "TclSoarLib::GlobalEval(" << command << ") ERROR" << std::endl;
                std::cout << result << std::endl;
                ret_val = TCL_ERROR;
            }
            else
            {
                result = Tcl_GetStringResult(m_interp);
            }
        }
    }
    pthread_mutex_unlock(&interp_mutex);
    return ret_val;
}

bool TclSoarLib::initialize_Tcl_Interpreter()
{
    /* -- Only needed if compiling with TCL_MEM_DEBUG if you want access to tcl memory command -- */
    //  Tcl_InitMemory(m_interp);
    bool ret_val = true;
    pthread_mutex_lock(&interp_mutex);
    {
        if (Tcl_Init(m_interp) != TCL_OK)
        {
            Tcl_DeleteInterp(m_interp);
            ret_val = false;
        }
    }
    pthread_mutex_unlock(&interp_mutex);
    
    return ret_val;
}
bool TclSoarLib::evaluateDirCommand(const string command)
{
    string tcl_cmd_string, result_string;
    
    EscapeTclString(command.c_str(), tcl_cmd_string);
    if (GlobalEval(tcl_cmd_string, result_string) != TCL_OK)
    {
        tcl_cmd_string.erase();
        return false;
    }
    
    tcl_cmd_string.erase();
    return true;
}

bool TclSoarLib::turnOn()
{
    pthread_mutex_lock(&interp_mutex);
    bool res = (evaluateDirCommand("createCallbackHandlers"));
    pthread_mutex_unlock(&interp_mutex);
    return res;
}

bool TclSoarLib::turnOff()
{
    pthread_mutex_lock(&interp_mutex);
    bool res = (evaluateDirCommand("removeCallbackHandlers"));
    pthread_mutex_unlock(&interp_mutex);
    return res;
}

// Test if a path exists and is not a directory.
bool isFile(const char* path)
{
#ifdef _WIN32
    DWORD a = GetFileAttributes(path);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return (stat(path, &st) == 0 && !S_ISDIR(st.st_mode));
#endif
}
// Test if a path exists and is not a directory.
bool isDir(const char* path)
{
#ifdef _WIN32
    DWORD a = GetFileAttributes(path);
    return a != INVALID_FILE_ATTRIBUTES && !(a & !FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

bool TclSoarLib::initialize_Master()
{
    string smlTclDir,  libDir, masterFilePath, result_string;
    
    if (((GlobalDirEval("pwd", libDir) != TCL_OK) ||
            (GlobalDirEval("file join [pwd] tcl", smlTclDir) != TCL_OK) ||
            (GlobalDirEval("file join [pwd] tcl master.tcl", masterFilePath) != TCL_OK)))
    {
        GlobalEval("puts {Error finding tcl scripts.}", result_string);
        return false;
    }
    else
    {
        if (!(isDir(libDir.c_str()) && isDir(smlTclDir.c_str()) && isFile(masterFilePath.c_str())))
        {
            libDir = getenv("SOAR_HOME");
            if (libDir.size() == 0)
            {
                GlobalEval("puts {Unable to find tcl scripts under current directory or SOAR_HOME, which is not currently set.}", result_string);
                return false;
            }
            
            smlTclDir = libDir;
            if (smlTclDir.find_last_of("/\\") != smlTclDir.size() - 1)
            {
                smlTclDir += '/';
            }
            smlTclDir += "tcl";
            
            /* -- Normalize directory for any cross-platform differences-- */
            string normalizeCmd("file normalize ");
            normalizeCmd += smlTclDir;
            if (GlobalDirEval(normalizeCmd.c_str(), smlTclDir) != TCL_OK)
            {
                return false;
            }
            
            masterFilePath = smlTclDir;
            masterFilePath += "/master.tcl";
            if (!(isDir(libDir.c_str()) && isDir(smlTclDir.c_str()) && isFile(masterFilePath.c_str())))
            {
                GlobalEval("puts {Unable to find tcl scripts under SOAR_HOME}", result_string);
                return false;
            }
        }
    }
    
    if (!evaluateDirCommand("source \"" + smlTclDir + "/dirstack.tcl\""))
    {
        GlobalEval("puts {Unable to find tcl scripts under current directory or SOAR_HOME.}", result_string);
        return false;
    }
    if (!evaluateDirCommand("pushd \"" + smlTclDir + "\""))
    {
        GlobalEval("puts {Unable to find tcl scripts under current directory or SOAR_HOME.}", result_string);
        return false;
    }
    
    if (GlobalDirEval("source master.tcl", result_string) != TCL_OK)
    {
        GlobalEval("puts {Unable to find tcl scripts under current directory or SOAR_HOME.}", result_string);
        return false;
    }
    if (GlobalDirEval("initializeMaster", result_string) != TCL_OK)
    {
        GlobalEval("puts {Error initializing master tcl interpreter}", result_string);
        return false;
    }
    
    if (!evaluateDirCommand("popd"))
    {
        GlobalEval("puts {Error initializing master tcl interpreter}", result_string);
        return false;
    }
    
    return true;
}

} // #endif extern "C"
