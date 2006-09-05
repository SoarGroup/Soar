/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

// CmdRhsFunction.h
#ifndef _CMD_RHS_FUNCTION_H_
#define _CMD_RHS_FUNCTION_H_

/*
  This rhs function allows a client to implement arbitrary behavior
  where the implementation of the function is within the client (not gSKI or the kernel).
*/

#include "gSKI_RhsFunction.h"
#include "gSKI_Kernel.h"

class CmdRhsFunction: public gSKI::RhsFunction
{
private:
	gSKI::Kernel* m_Kernel ;
	gSKI::Agent*  m_Agent ;

 public:
   CmdRhsFunction(gSKI::Kernel* kernel):m_Kernel(kernel), m_Agent(0) { }

   void SetAgent(gSKI::Agent* pAgent) { m_Agent = pAgent ; }
   const char* GetName() const { return "cmd"; }
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

	  bool first = true ;

	  // Get the argument string (the sum of all arguments on the line with space separators)
      for(;pArguments->IsValid(); pArguments->Next())
	  {
		  // Insert spaces between the arguments
		  if (first) first = false ;
		  else ostr << " " ;

         appendSymbolToStream(ostr, pArguments->GetVal());
      }

	  std::string argument = ostr.str() ;

	  // The size of this buffer limits the largest string value we can return, 1k should do it.
	  // (The client is passed this length so we can safely change it later if we wish).
	  const int kMaxReturnLength = 1024; 
	  char buffer[kMaxReturnLength + 1] ;
	  buffer[0] = 0 ;	// Make it null terminated just in case somebody returns true without filling it in...
	  bool commandLine = true ;	// exec commands aren't handled by the command line processor (they're handled by a client user function)

	  bool haveResult = m_Kernel->FireRhsNotification(m_Agent, commandLine, functionName.c_str(), argument.c_str(), kMaxReturnLength, buffer) ;

	  if (!haveResult)
	  {
         std::cerr << "No client responded to the rhs user function " << functionName << " with a value"
                   << std::endl;
	     return NULL ;
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
