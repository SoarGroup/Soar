/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

// ExecRhsFunction.h
#ifndef _EXEC_RHS_FUNCTION_H_
#define _EXEC_RHS_FUNCTION_H_

/*
  This rhs function allows a client to implement arbitrary behavior
  where the implementation of the function is within the client (not gSKI or the kernel).
*/

#include "IgSKI_RhsFunction.h"
#include "IgSKI_Kernel.h"

class ExecRhsFunction: public gSKI::IRhsFunction
{
private:
	gSKI::IKernel* m_Kernel ;
	gSKI::IAgent*  m_Agent ;

 public:
	 ExecRhsFunction(gSKI::IKernel* kernel):m_Kernel(kernel), m_Agent(0) { }

   void SetAgent(gSKI::IAgent* pAgent) { m_Agent = pAgent ; }
   const char* GetName() const { return "exec"; }
   int GetNumExpectedParameters() const { return (gSKI_PARAM_NUM_VARIABLE); }
   bool IsValueReturned() const { return true; }

   gSKI::ISymbol* Execute(gSKI::tISymbolIterator* pArguments,
                          gSKI::ISymbolFactory* pSymbolFactory) {

      std::ostringstream ostr;

	  // Didn't pass a function name to "cmd"
	  if (!pArguments->IsValid())
	  {
         std::cerr << GetName() << " should be followed by a function name " 
                   << std::endl;

		  return NULL ;
	  }

	  // Get the function name
	  std::string functionName = pArguments->GetVal()->GetString() ;
	  pArguments->Next() ;

	  // Get the argument string.
	  // We've decided for "exec" to just concatenate all arguments together without inserting
	  // spaces.  This allows for powerful operations (such as constructing an argument out of pieces).
      for(;pArguments->IsValid(); pArguments->Next())
	  {
         appendSymbolToStream(ostr, pArguments->GetVal());
      }

	  std::string argument = ostr.str() ;

	  // The size of this buffer limits the largest string value we can return, 1k should do it.
	  // (The client is passed this length so we can safely change it later if we wish).
	  const int kMaxReturnLength = 1024; 
	  char buffer[kMaxReturnLength + 1] ;
	  buffer[0] = 0 ;	// Make it null terminated just in case somebody returns true without filling it in...
	  bool commandLine = false ;	// exec commands aren't handled by the command line processor (they're handled by a client user function)

	  bool haveResult = m_Kernel->FireRhsNotification(m_Agent, commandLine, functionName.c_str(), argument.c_str(), kMaxReturnLength, buffer) ;

	  if (!haveResult)
	  {
         std::cerr << "No client responded to the rhs user function " << functionName << " with a value"
                   << std::endl;

		 // Returning NULL will generate an assertion -- so instead we'll return an explicit error string
		 // which will then appear inside the agent.
		 // return NULL ;
		 std::string errorMsg = "Error executing " ;
		 errorMsg += functionName ;
		 return pSymbolFactory->CreateStringSymbol(errorMsg.c_str());
	  }

	  // Create a new string symbol for the result.
      return pSymbolFactory->CreateStringSymbol(buffer);
   }

   void appendSymbolToStream(std::ostringstream& ostr, gSKI::ISymbol* psym) {
      if ( !psym ) {
         std::cerr << "Concat function was sent a null symbol! " 
                   << "Ignoring it..."
                   << std::endl;
	  } else {
         ostr << psym->GetString();
      }
   }

};


#endif
