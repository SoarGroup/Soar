/*
 * TclSoarLib.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: mazzin
 */

#include "TclSoarLib.h"

#include <iostream>
#include <thread>

#ifdef __cplusplus
extern "C"
#endif
{
using namespace sml;
using namespace std;

// Tcl Thread
// This thread takes care of creating and deleting all tcl interpreters
// Tcl will cause a crash if an interpreter is not deleted by the same thread that created it

void* launch_tcl(void* lib_ptr){
    //cout << "TclThread: Started" << endl;
    TclSoarLib* tcl_lib = (TclSoarLib*)lib_ptr;

    /****** INITIALIZATION ******/
    unique_lock<mutex> interp_lock(tcl_lib->interp_mutex);
    {
        tcl_lib->m_interp = Tcl_CreateInterp();
        Tcl_FindExecutable(0);

        //cout << "TclThread: Created Interpreter" << endl;

        if (!tcl_lib->initialize_Tcl_Interpreter() || !tcl_lib->initialize_Master())
        {
            tcl_lib->m_interp = NULL;
        } 
        else if (tcl_lib->m_kernel)
        {
            std::string output;
            tcl_lib->GlobalEval("smlstartup", output);
        }
    }
    interp_lock.unlock();

    //cout << "TclThread: Initialized Interpreter" << endl;
    // Lets the Library know it has finished the initialization
    tcl_lib->thread_ready = true;
    tcl_lib->cv.notify_one();

    if(tcl_lib->m_interp == NULL)
    {
        exit(EXIT_FAILURE);
    }

    /****** COMMAND HANDLING ******/

    bool exit = false;
    while(!exit)
    {
        // Wait until there is a new command (tcl_lib->new_command == true)
        unique_lock<mutex> cmd_lock(tcl_lib->command_mutex);
        tcl_lib->cv.wait(cmd_lock, [tcl_lib]{ return tcl_lib->new_command; });
        tcl_lib->new_command = false;

        // Process the command
        std::string command;
        std::string result;
        switch(tcl_lib->command_type)
        {
            case TclSoarLib::CREATE_TCL_SLAVE:
                //cout << "TclThread: create slave " << tcl_lib->command_info << endl;
                command = "createSlave \"" + tcl_lib->command_info + "\"";
                interp_lock.lock();
                {
                    tcl_lib->GlobalEval(command, result);
                }
                interp_lock.unlock();
                break;

            case TclSoarLib::DESTROY_TCL_SLAVE:
                //cout << "TclThread: destroy slave " << tcl_lib->command_info << endl;
                command = "destroySlave \"" + tcl_lib->command_info + "\"";
                interp_lock.lock();
                {
                    tcl_lib->GlobalEval(command, result);
                }
                interp_lock.unlock();
                break;

            case TclSoarLib::SHUTDOWN_TCL_THREAD:
                //cout << "TclThread: SHUTDOWN received" << endl;
                exit = true;
                break;
        }

        // Let the messenger know that the command was finished
        tcl_lib->finished_command = true;
        cmd_lock.unlock();
        tcl_lib->cv.notify_one();
    }

    /****** SHUTDOWN ******/
    interp_lock.lock();
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
    interp_lock.unlock();

    //cout << "TclThread: Exiting" << endl;
    return NULL;
}

TclSoarLib::TclSoarLib(Kernel* myKernel) :
    m_kernel(0),
    m_interp(0)
{
    m_kernel = myKernel;

    // Create the new thread
    thread_ready = false;
    unique_lock<mutex> lock(command_mutex);
    lib_thread = thread(&launch_tcl, (void*)this);

		myKernel->AddRhsFunction( "tcl", tclRHS, this );

    // Wait until the new thread has finished initialization
    cv.wait(lock, [this]{ return thread_ready; });
}

TclSoarLib::~TclSoarLib()
{
    //cout << "TclSoarLib::~TclSoarLib()" << endl;
    //cout << "   Waiting for TclThread to exit" << endl;

    // Tell the thread to shutdown
    send_thread_command(SHUTDOWN_TCL_THREAD, "");

    // Wait until the thread exits
    lib_thread.join();

    m_kernel = 0;

    //cout << "TclSoarLib destroyed" << endl;
}

std::string TclSoarLib::tclRHS( sml::smlRhsEventId, void *pData, sml::Agent *pAgent, char const *pFunc, char const *pArg )
{
	TclSoarLib *pLib = (TclSoarLib *)pData;

	std::string comm(pArg);
	std::string res;

	pLib->GlobalEval( comm, res);
		
	return res;
}

void TclSoarLib::send_thread_command(int type, string info){
    if (m_interp)
    {
        // Set up the command to be sent to the thread
        unique_lock<mutex> cmd_lock(command_mutex);
        command_type = type;
        command_info = info;
        new_command = true;
        cmd_lock.unlock();

        // Notify the thread that there is a new command
        cv.notify_one();

        // Wait until the thread is finished executing the command
        cmd_lock.lock();
        cv.wait(cmd_lock, [this]{ return finished_command; });
        finished_command = false;
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
    return TCL_OK;
}

int TclSoarLib::GlobalEval(const std::string& command, std::string& result)
{
    int ret_val = TCL_OK;
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
    return ret_val;
}

bool TclSoarLib::initialize_Tcl_Interpreter()
{
    /* -- Only needed if compiling with TCL_MEM_DEBUG if you want access to tcl memory command -- */
    //  Tcl_InitMemory(m_interp);
    bool ret_val = true;

    if (Tcl_Init(m_interp) != TCL_OK)
    {
        Tcl_DeleteInterp(m_interp);
        ret_val = false;
    }
    
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
    lock_guard<mutex> lock(interp_mutex);
    return evaluateDirCommand("createCallbackHandlers");
}

bool TclSoarLib::turnOff()
{
    lock_guard<mutex> lock(interp_mutex);
    return evaluateDirCommand("removeCallbackHandlers");
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

						for ( int i = libDir.length() - 1; i >= 0; i-- )
						{
							if (libDir[i] == '\"')
								libDir.erase(i,1);
						}

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
