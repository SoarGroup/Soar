// Debug.h: interface for the Debug functions
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUG_H__95F3147A_5B92_4B9F_AAA4_7E699929B115__INCLUDED_)
#define AFX_DEBUG_H__95F3147A_5B92_4B9F_AAA4_7E699929B115__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Need to disable some warnings to compile VC++'s STL headers at warning level 4.
#ifdef _MSC_VER
#pragma warning(push,3)
#pragma warning(disable: 4018)
#endif

#include <vector>
typedef std::vector<char const*> StackTrace ;

#ifdef _MSC_VER
#pragma warning(pop)
#pragma warning(disable: 4514)
#endif

void PrintDebugMethod(long indent, char const* pMethodName, char const* pStr) ;
void PrintDebug(char const* pStr) ;
void PrintDebugFormat(char const* pFormat, ...) ;

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

		m_stackTraceDisplayPos = m_stackTrace.size() ;
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

#endif // !defined(AFX_DEBUG_H__95F3147A_5B92_4B9F_AAA4_7E699929B115__INCLUDED_)
