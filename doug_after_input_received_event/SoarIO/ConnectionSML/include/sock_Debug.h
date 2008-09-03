/////////////////////////////////////////////////////////////////
// Debug class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Some handy methods for displaying debug output and for
// tracking entry and exit from functions.
// 
/////////////////////////////////////////////////////////////////

#ifndef CT_DEBUG_H
#define CT_DEBUG_H

// Have to comment this in before we can turn on dump of method entry and exit in socket code
//#define DEBUG_CALLS

void PrintDebugMethod(long indent, char const* pMethodName, char const* pStr) ;
void PrintDebug(char const* pStr) ;
void PrintDebugFormat(char const* pFormat, ...) ;

#ifdef DEBUG_CALLS

#include <vector>
typedef std::vector<char const*> StackTrace ;
#define CTDEBUG_ENTER_METHOD(methodName) CTDebugEnterMethod myEnterMethod3PNY(methodName)

class CTDebugEnterMethod
{
public:
	// Constructor which is called when you enter a method
	CTDebugEnterMethod(const char* pszMethodName)
	{
		m_stackTrace.push_back(pszMethodName) ;

		if (m_bPrintOnEnterMethod)
			PrintDebugMethod(GetCurrentNestLevel(), pszMethodName, "Entered");

		m_nCurrentNestLevel++;
	}

	// Destructor is called when you exit the method
	~CTDebugEnterMethod()
	{
		if (m_stackTraceDisplayPos > 0 && m_stackTraceDisplayPos >= (long)(m_stackTrace.size()))
			m_stackTraceDisplayPos-- ;

		if (m_bPrintOnExitMethod)
		{
			char const* pMethodName = m_stackTrace.back() ;
			PrintDebugMethod(GetCurrentNestLevel(), pMethodName, "Exiting");
		}

		m_stackTrace.pop_back() ;

		m_nCurrentNestLevel--;
	}

	static void PrintStackTrace()
	{
		if (!m_bShowStackTrace)
			return ;

		for (long i = m_stackTraceDisplayPos ; i != (long)m_stackTrace.size() ; i++)
		{
			char const* pMethodName = m_stackTrace[i] ;
			PrintDebugMethod(i,pMethodName, "Entered") ;
		}

		m_stackTraceDisplayPos = (int)m_stackTrace.size() ;
	}

	// Get the current nest level
	static int	GetCurrentNestLevel() ;

	// Get the current method
	static const char*	GetCurrentMethodName() { return (!m_bShowStackTrace && m_stackTrace.size() > 0) ? m_stackTrace.back() : "" ; }

	// Should we be indenting the output?
	static bool IsOutputIndented()			   { return m_bIndentOutput ; }

protected:
	// This value increases as we enter methods and decreases
	// as we exit methods.
	static int			m_nCurrentNestLevel;

	// Pointer to last method we entered
//	static const char*	m_pCurrentMethodName ;

	// Control whether we print out method names as enter/exit method
	static bool			m_bPrintOnEnterMethod ;
	static bool			m_bPrintOnExitMethod ;

	// Control whether output is indented
	static bool			m_bIndentOutput ;

	// Decide whether output will show previous stack trace
	static bool			m_bShowStackTrace ;

	// The current stack of function names
	static StackTrace	m_stackTrace ;

	// Pointer into stack trace to last item already printed.  Points to end() if none have been printed.
	static long m_stackTraceDisplayPos ;
};

#else	// DEBUG_CALLS
#define CTDEBUG_ENTER_METHOD(methodName)
#endif	// DEBUG_CALLS

#endif 
