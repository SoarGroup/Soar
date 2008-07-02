#include <portability.h>

/////////////////////////////////////////////////////////////////
// RhsFunction class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2007
//
// Support for right hand side functions.
/////////////////////////////////////////////////////////////////

#include "sml_RhsFunction.h"

#include "sml_Utils.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"

#include "KernelHeaders.h"

#include <iostream>

using namespace sml ;

// This method is called by the kernel, which in turn calls the Execute method of the RhsFunction.
Symbol* RhsFunction::RhsFunctionCallback(agent* thisAgent, list* args, void* user_data)
{
	// Since we registered this callback, we know what the user data is.
	RhsFunction* rhsFunction = static_cast<RhsFunction*>(user_data);

	// Prepare arguments

	// List of symbols wrapped in gSymbols
	std::vector<Symbol*> symVector;
	for(; args != NIL; args = args->rest)
		symVector.push_back(static_cast<Symbol*>(args->first));

	Symbol* pSoarReturn = 0;

	// Check to make sure we have the right number of arguments.   
	if( (rhsFunction->GetNumExpectedParameters() == kPARAM_NUM_VARIABLE) ||
	  ((int)symVector.size() == rhsFunction->GetNumExpectedParameters()) )
	{
		 // Actually make the call.  We can do the dynamic cast because we passed in the
		 //  symbol factory and thus know how the symbol was created.
		 Symbol* pReturn = rhsFunction->Execute(&symVector);

		 // Return the result, assuming it is not NIL
		 if(rhsFunction->IsValueReturned() == true)
		 {
			// There should be a return value
			assert(pReturn != 0);
			if(pReturn != 0)
			{
			   // Return the result
			   pSoarReturn = pReturn;
			}
			else
			{
			   // We have to return something to prevent a crash, so we return an error code
			   pSoarReturn = make_sym_constant(thisAgent, "error_expected_rhs_function_to_return_value_but_it_did_NOT");
			}
		 }
		 else
		 {
			// Expected that the rhs function would not return a value, but it did.  Return value ignored.
			assert(pReturn == 0);
		 }

		// In any case, we are done using the return value
		//if(pReturn != 0)
		//{
		//	symbol_remove_ref(thisAgent, pReturn) ;
		//}
	}
	else
	{
		// Wrong number of arguments passed to RHS function
		assert(false) ;

		// We can return anything we want to soar; we return an error message so at least the problem is obvious.
		if(rhsFunction->IsValueReturned() == true)
			pSoarReturn = make_sym_constant(thisAgent, "error_wrong_number_of_args_passed_to_rhs_function");
	}

	return pSoarReturn;
}

Symbol* sml::InterruptRhsFunction::Execute(std::vector<Symbol*>* pArguments)
{
	unused(pArguments) ;

	// BADBAD?  Should this be just an interrupt on this agent?  Existing semantics are for all agents so doing that here too.
	m_pAgentSML->GetKernelSML()->InterruptAllAgents(sml_STOP_AFTER_SMALLEST_STEP) ;
	return 0 ;
}

Symbol* sml::ConcatRhsFunction::Execute(std::vector<Symbol*>* pArguments)
{
      std::ostringstream ostr;

	  for (std::vector<Symbol*>::iterator iter = pArguments->begin() ; iter != pArguments->end() ; iter++)
	  {
		  Symbol* pSymbol = *iter ;

		if ( !pSymbol ) {
		 std::cerr << "Concat function was sent a null symbol! " 
				   << "Ignoring it..."
				   << std::endl;
		} else {
			ostr << symbol_to_string( m_pAgentSML->GetSoarAgent(), pSymbol, true, 0, 0 );
		}
	  }

	  char const* pResultStr = ostr.str().c_str() ;
	  Symbol* pResult = make_sym_constant(m_pAgentSML->GetSoarAgent(), pResultStr) ;
	  return pResult ;
}

Symbol* sml::CmdRhsFunction::Execute(std::vector<Symbol*>* pArguments)
{
	std::ostringstream ostr;

	// Didn't pass a function name to "cmd"
	if (pArguments->size() == 0)
	{
	 std::cerr << GetName() << " should be followed by a command name " 
			   << std::endl;

	  return NULL ;
	}

	bool first = false ;

	// Get the command line string.
	  for (std::vector<Symbol*>::iterator iter = pArguments->begin() ; iter != pArguments->end() ; iter++)
	  {
			Symbol* pSymbol = *iter ;

			// Insert spaces between the arguments
			if (first) first = false ;
			else ostr << " " ;

			if ( !pSymbol ) {
			 std::cerr << "Concat function was sent a null symbol! " 
					   << "Ignoring it..."
					   << std::endl;
			} else {
				ostr << symbol_to_string( m_pAgentSML->GetSoarAgent(), pSymbol, true, 0, 0 );
			}
	  }

	std::string argument = ostr.str() ;

	std::string result = m_pAgentSML->ExecuteCommandLine(argument) ;

	Symbol* pResult = make_sym_constant(m_pAgentSML->GetSoarAgent(), result.c_str()) ;
	return pResult ;
}

Symbol* sml::ExecRhsFunction::Execute(std::vector<Symbol*>* pArguments)
{
	std::ostringstream ostr;

	// Didn't pass a function name to "exec"
	if (pArguments->size() == 0)
	{
	 std::cerr << GetName() << " should be followed by a function name " 
			   << std::endl;

	  return NULL ;
	}

	std::string function = symbol_to_string( m_pAgentSML->GetSoarAgent(), pArguments->front(), true, 0, 0 );

	// Get the command line string.
	// We've decided for "exec" to just concatenate all arguments together without inserting
	// spaces.  This allows for powerful operations (such as constructing an argument out of pieces).
	  for (std::vector<Symbol*>::iterator iter = (++pArguments->begin()) ; iter != pArguments->end() ; iter++)
	  {
			Symbol* pSymbol = *iter ;

			if ( !pSymbol ) {
				std::cerr << "Concat function was sent a null symbol! " 
					   << "Ignoring it..."
					   << std::endl;
			} else {
				ostr << symbol_to_string( m_pAgentSML->GetSoarAgent(), pSymbol, true, 0, 0 );
			}
	  }

	std::string argument = ostr.str() ;

	std::string result ;
	bool ok = m_pAgentSML->GetKernelSML()->FireRhsEvent(m_pAgentSML, smlEVENT_RHS_USER_FUNCTION, function, argument, &result) ;

	if (!ok)
	{
		result = std::string("Error: Nobody was registered to implement rhs function ") + function ;
	}

	Symbol* pResult = make_sym_constant(m_pAgentSML->GetSoarAgent(), result.c_str()) ;
	return pResult ;
}
