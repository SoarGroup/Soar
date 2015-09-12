#include "portability.h"

#include "unittest.h"
#include <ctime>

#include "simplelistener.h"
#include "misc.h"

bool g_Cancel = false;

#ifdef _WIN32
BOOL WINAPI handle_ctrlc(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT)
    {
        g_Cancel = true;
        return TRUE;
    }
    
    return FALSE;
}
#endif // _WIN32

bool g_NoRemote = true;

#ifdef __APPLE__
#include "assert.hpp"
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>

static bool AmIBeingDebugged(void)
// Returns true if the current process is being debugged (either
// running under the debugger or has a debugger attached post facto).
{
    int                 junk;
    int                 mib[4];
    struct kinfo_proc   info;
    size_t              size;
    
    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.
    
    info.kp_proc.p_flag = 0;
    
    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.
    
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    
    // Call sysctl.
    
    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    assert(junk == 0);
    
    // We're being debugged if the P_TRACED flag is set.
    
    return ((info.kp_proc.p_flag & P_TRACED) != 0);
}
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    //_crtBreakAlloc = 6077;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    SetConsoleCtrlHandler(handle_ctrlc, TRUE);
#endif // _WIN32
    
    bool debugged = false;
    
#ifdef __APPLE__
    if (AmIBeingDebugged())
    {
        debugged = true;
    }
#elif _WIN32
    debugged = IsDebuggerPresent() == TRUE;
#endif
    
    if (!debugged)
    {
        set_working_directory_to_executable_path();
    }
    
    for (int index = 1; index < argc; ++index)
    {
        std::string argument(argv[index]);
        if (argument == "--listener")
        {
            SimpleListener simpleListener(600);
            return simpleListener.run();
            
        }
        else if (argument == "--remote")
        {
            std::cout << "Running tests with remote." << std::endl;
            g_NoRemote = false;
        }
        else
        {
            std::cerr << "Unknown argument " << argument << " ignored." << std::endl;
        }
    }
 
    std::cout << "WARNING: These tests are deprecated.  These are not complete.  Please make any additions to the Prototype-UnitTesting framework." << std::endl;
   
    srand(static_cast<unsigned>(time(NULL)));
    
    //--- Create the event manager and test controller
    CPPUNIT_NS::TestResult controller;
    
    //--- Add a listener that colllects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);
    
    //--- Add a listener that print dots as test run.
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);
    
    //--- Add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
    runner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(controller);
    
    CPPUNIT_NS::CompilerOutputter outputter(&result, std::cerr);
    outputter.write();
    
#ifdef _MSC_VER
    if (IsDebuggerPresent())
    {
        std::cout << "Press enter to continue..." << std::endl;
        std::cin.get();
    }
#endif
	
	std::cout << "WARNING: These tests are deprecated.  These are not complete.  Please make any additions to the Prototype-UnitTesting framework." << std::endl;
	
    return result.wasSuccessful() ? 0 : 1;
}
