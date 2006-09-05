/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

// ConcatRhsFunction.h
#ifndef _CONCAT_RHS_FUNCTION_H_
#define _CONCAT_RHS_FUNCTION_H_

/* 
   A simple rhs function for concatenating symbols into a string symbol.
*/

#include "gSKI_RhsAction.h"
#include "gSKI_RhsFunction.h"
#include "gSKI_RhsFunctionAction.h"
#include "IgSKI_SymbolFactory.h"
#include "IgSKI_Symbol.h"

#include <sstream>
#include <iostream>

class ConcatRhsFunction: public gSKI::RhsFunction
{
 public:
   const char* GetName() const { return "concat"; }
   int GetNumExpectedParameters() const { return (gSKI_PARAM_NUM_VARIABLE); }
   bool IsValueReturned() const { return true; }

   gSKI::ISymbol* Execute(gSKI::tISymbolIterator* pArguments,
                          gSKI::ISymbolFactory* pSymbolFactory) {

      std::ostringstream ostr;

      for(;pArguments->IsValid(); pArguments->Next()) {
         appendSymbolToStream(ostr, pArguments->GetVal());
      }

      return pSymbolFactory->CreateStringSymbol(ostr.str().c_str());
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
