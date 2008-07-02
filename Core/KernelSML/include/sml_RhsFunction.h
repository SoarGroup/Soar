/////////////////////////////////////////////////////////////////
// RhsFunction class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2007
//
// Support for right hand side functions.
/////////////////////////////////////////////////////////////////

#ifndef RHS_FUNCTION_H
#define RHS_FUNCTION_H

#include <vector>

typedef union symbol_union Symbol;
typedef struct agent_struct agent;
typedef struct cons_struct list;

namespace sml {

class AgentSML ;

class RhsFunction
{
protected:
	// The agent
	AgentSML* m_pAgentSML ;

public:
	static Symbol* RhsFunctionCallback(agent* thisAgent, list* args, void* user_data) ;
	static const int kPARAM_NUM_VARIABLE = -1 ;

	RhsFunction(AgentSML* pAgent) { m_pAgentSML = pAgent ; }

	virtual ~RhsFunction() { }

	/** 
	* Returns the name of the RHS function.
	*
	* All Rhs functions must have unique names so that they can be identified in Soar
	*/
	virtual const char*  GetName() const = 0;

	/** 
	* Gets the number of parameters expected for this RHS function
	*/
	virtual int GetNumExpectedParameters() const = 0;

	/** 
	* Returns true if the RHS function returns a value other than 0 from Execute
	*/
	virtual bool         IsValueReturned() const = 0;

      /** 
       * Executes the RHS function given the set of symbols
       *
       * You should NOT release the symbol values that are passed in.
       * Because this is a callback, the calling method will release them.  However, if you clone
       *  or otherwise copy any values, you are responsible for releasing the copies.
       */
	  virtual Symbol* Execute(std::vector<Symbol*>* pArguments) = 0;
} ;

class InterruptRhsFunction: public RhsFunction
{
public:
	InterruptRhsFunction(AgentSML* pAgent):RhsFunction(pAgent) { } 

	const char* GetName() const { return "interrupt"; }
	int GetNumExpectedParameters() const { return 0; }
	bool IsValueReturned() const { return false; }

	virtual Symbol* Execute(std::vector<Symbol*>* pArguments) ;
};

class ConcatRhsFunction: public RhsFunction
{
 public:
	 ConcatRhsFunction(AgentSML* pAgent):RhsFunction(pAgent) { } 

   const char* GetName() const { return "concat"; }
   int GetNumExpectedParameters() const { return (kPARAM_NUM_VARIABLE); }
   bool IsValueReturned() const { return true; }

   virtual Symbol* Execute(std::vector<Symbol*>* pArguments) ;
};

class ExecRhsFunction: public RhsFunction
{
 public:
   ExecRhsFunction(AgentSML* pAgent):RhsFunction(pAgent) { } 

   const char* GetName() const { return "exec"; }
   int GetNumExpectedParameters() const { return (kPARAM_NUM_VARIABLE); }
   bool IsValueReturned() const { return true; }

   virtual Symbol* Execute(std::vector<Symbol*>* pArguments) ;
};

class CmdRhsFunction: public RhsFunction
{
 public:
   CmdRhsFunction(AgentSML* pAgent):RhsFunction(pAgent) { } 

   const char* GetName() const { return "cmd"; }
   int GetNumExpectedParameters() const { return (kPARAM_NUM_VARIABLE); }
   bool IsValueReturned() const { return true; }

   virtual Symbol* Execute(std::vector<Symbol*>* pArguments) ;
};


}

#endif
