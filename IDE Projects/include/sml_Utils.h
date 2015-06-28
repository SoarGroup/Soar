#ifndef SML_UTILS_H
#define SML_UTILS_H

/////////////////////////////////////////////////////////////////
// Utility header
//
// Author: Jonathan Voigt, Bob Marinier
// Date  : June 2007
//
// This header collects some useful code used throughout Soar.
//
/////////////////////////////////////////////////////////////////

#include "Export.h"

namespace sml
{

// Silences unreferenced formal parameter warning
#define unused(x) (void)(x)

// sanity check helpers
#define CHECK(x)            { assert(x) ; if (!(x)) return ; }
#define CHECK_RET_FALSE(x)  { assert(x) ; if (!(x)) return false ; }
#define CHECK_RET_ZERO(x)   { assert(x) ; if (!(x)) return 0 ; }
#define CHECK_RET_NULL(x)   { assert(x) ; if (!(x)) return NULL ; }


/////////////////////////////////////////////////////////////////////
// Function name  : Sleep
//
// Argument       : int secs
// Argument       : int msecs
// Return type    : void
//
// Description    : Sleep for the specified seconds and milliseconds
//
/////////////////////////////////////////////////////////////////////
    void EXPORT Sleep(int secs, int msecs);
    
/////////////////////////////////////////////////////////////////////
// Function name  : ReportSystemErrorMessage
//
// Return type    : void
//
// Description    : Get the text of the most recent system error
//
/////////////////////////////////////////////////////////////////////
    void EXPORT ReportSystemErrorMessage();
    
/////////////////////////////////////////////////////////////////
// Debug stuff
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Some handy methods for displaying debug output and for
// tracking entry and exit from functions.
//
/////////////////////////////////////////////////////////////////

// Have to comment this in before we can turn on dump of method entry and exit (currently used in socket code)
//#define DEBUG_CALLS

    void EXPORT PrintDebugMethod(char const* pMethodName, char const* pStr) ;
    void EXPORT PrintDebug(char const* pStr) ;
    void EXPORT PrintDebugFormat(char const* pFormat, ...) ;
    
#ifdef DEBUG_CALLS
    
#include <vector>
    typedef std::vector<char const*> StackTrace ;
#define CTDEBUG_ENTER_METHOD(methodName) CTDebugEnterMethod myEnterMethod3PNY(methodName)
    
    class EXPORT CTDebugEnterMethod
    {
        public:
            // Constructor which is called when you enter a method
            CTDebugEnterMethod(const char* pszMethodName)
            {
                m_stackTrace.push_back(pszMethodName) ;
                
                if (m_bPrintOnEnterMethod)
                {
                    PrintDebugMethod(pszMethodName, "Entered");
                }
                
                m_nCurrentNestLevel++;
            }
            
            // Destructor is called when you exit the method
            ~CTDebugEnterMethod()
            {
                if (m_stackTraceDisplayPos > 0 && m_stackTraceDisplayPos >= (long)(m_stackTrace.size()))
                {
                    m_stackTraceDisplayPos-- ;
                }
                
                if (m_bPrintOnExitMethod)
                {
                    char const* pMethodName = m_stackTrace.back() ;
                    PrintDebugMethod(pMethodName, "Exiting");
                }
                
                m_stackTrace.pop_back() ;
                
                m_nCurrentNestLevel--;
            }
            
            static void PrintStackTrace()
            {
                if (!m_bShowStackTrace)
                {
                    return ;
                }
                
                for (long i = m_stackTraceDisplayPos ; i != (long)m_stackTrace.size() ; i++)
                {
                    char const* pMethodName = m_stackTrace[i] ;
                    PrintDebugMethod(pMethodName, "Entered") ;
                }
                
                m_stackTraceDisplayPos = (int)m_stackTrace.size() ;
            }
            
            // Get the current nest level
            static int  GetCurrentNestLevel() ;
            
            // Get the current method
            static const char*  GetCurrentMethodName()
            {
                return (!m_bShowStackTrace && m_stackTrace.size() > 0) ? m_stackTrace.back() : "" ;
            }
            
            // Should we be indenting the output?
            static bool IsOutputIndented()
            {
                return m_bIndentOutput ;
            }
            
        protected:
            // This value increases as we enter methods and decreases
            // as we exit methods.
            static int          m_nCurrentNestLevel;
            
            // Pointer to last method we entered
//  static const char*  m_pCurrentMethodName ;

            // Control whether we print out method names as enter/exit method
            static bool         m_bPrintOnEnterMethod ;
            static bool         m_bPrintOnExitMethod ;
            
            // Control whether output is indented
            static bool         m_bIndentOutput ;
            
            // Decide whether output will show previous stack trace
            static bool         m_bShowStackTrace ;
            
            // The current stack of function names
            static StackTrace   m_stackTrace ;
            
            // Pointer into stack trace to last item already printed.  Points to end() if none have been printed.
            static long m_stackTraceDisplayPos ;
    };
    
#else   // DEBUG_CALLS
#define CTDEBUG_ENTER_METHOD(methodName)
#endif  // DEBUG_CALLS
    
///// End debug stuff
////////////////////////////

} // namespace sml

#endif // SML_UTILS_H

