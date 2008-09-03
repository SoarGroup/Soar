/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

#ifndef GSKI_RHSFUNCTION_H
#define GSKI_RHSFUNCTION_H

#include "IgSKI_Iterator.h"

namespace gSKI {

   class ISymbol;
   class ISymbolFactory;

   /** 
    * A class that implements a RHS function in Soar.
    *
    * Inherit from this interface to define your own RHS functions.
    * You can add RHS functions to the kernel by calling AddRhsFunction and
    *  you can remove them by calling RemoveRhsFunction.
    *
    */
	class RhsFunction {
	public:

      /**
		 * Virtual destructor for usual reasons of correct cleanup of derived classes	
		 */
		virtual ~RhsFunction() {}

      /** 
       * Returns the name of the RHS function.
       *
       * All Rhs functions must have unique names so that they can be identified in Soar
       *
       * @return The name of the RHS function as a 0 terminated string
       */
      virtual const char*  GetName() const = 0;

      /** 
       * Gets the number of parameters expected for this RHS function
       *
       * @return Returns the number of parameters expected by this RHS function.  If the function
       *             can take an unlimitted number of parameters, this will return an integer
       *             < 0.
       */
      virtual int GetNumExpectedParameters() const = 0;

      /** 
       * Returns true if the RHS function returns a value other than 0 from Execute
       *
       * @return true if something other than 0 is ever returned from Execute
       */
      virtual bool         IsValueReturned() const = 0;

      /** 
       * Executes the RHS function given the set of symbols
       *
       * You should NOT release either the iterator or the symbol values that are passed in.
       * Because this is a callback, the calling method will release them.  However, if you clone
       *  or otherwise copy any values, you are responsible for releasing the copies.
       *
       * @param pArguments Pointer to an iterator of symbol arguments.  This list is the list of
       *         the arguments that Soar is passing to the RHS function.
       * @param pSymbolFactory A symbol factory that can be used to create a symbol to return
       *         as the return value of the method.  
       * @return A symbol representing the result of the RHS function execution.  This may be 0
       *         if the RHS function returns nothing.
       */
		virtual ISymbol*     Execute(tISymbolIterator* pArguments, ISymbolFactory* pSymbolFactory) = 0;
	};

} // Namespace

#endif

