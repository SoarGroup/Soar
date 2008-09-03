#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_test.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/

//
// gSKI HEaders
#include "gSKI_Test.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Symbol.h"
#include "gSKI_EnumRemapping.h"

// Misc Headers
#include "MegaAssert.h"
#include <stdio.h>

//
// Soar Headers
#include "print.h"
#include "gdatastructs.h"

namespace gSKI 
{
   /*
   ==================================
 _____        _
|_   _|__ ___| |_
  | |/ _ Y __| __|
  | |  __|__ \ |_
  |_|\___|___/\__|
   ==================================
   */
   Test::Test(test thisTest, agent* a) : m_agent(a)
   {  
      //
      // This routine should never handle conjunction or disjunction tests.
      // those are handled by the TestSet.
      if(!IsSimpleTest(thisTest))
      {
         m_type = gSKI_OTHER;
         MegaAssert(false, "Error: Conjunction of Disjunction passed to the Simple Test Constructor.");
         return;
      }

      //
      // We check to see if this test is blank.  It should never happen,
      // but we check anyway.
      if(test_is_blank_test(thisTest))
      {
         MegaAssert(false, "This is a blank test!  This should not happen.");
         return;
      }

      //
      // The :: distinguished the gSKI Symbol from the Soar Symbol.
      // This happens to be the Soar symbol
      ::Symbol*   value;

      //
      // If we have a complex test here, we know it is a value 
      // comparison that is not EQUALS.  This is not really a problem.
      // We just need to get the type and thesymbol out of the complex
      // test structure.
      if(test_is_complex_test(thisTest))
      {
         complex_test* ct = complex_test_from_test(thisTest);

         m_type =  EnumRemappings::ReMapTestType(ct->type);
         value = ct->data.referent;
      } 
      else 
      {
         m_type = gSKI_EQUAL; 
         value = referent_of_equality_test(thisTest);
      }
      //
      // Create a new Symbol of the appropriate type.
      // NOTE: Because we are looking at the parse tree, this
      //  symbol can never be an object
      //
      // Correction:  Justifications can have ID Symbols.  grrr.
      //
      //MegaAssert(value->sc.common_symbol_info.symbol_type != IDENTIFIER_SYMBOL_TYPE, "Parse symbol should not be a WMO!");
      m_symbol = new gSymbol(m_agent, value, 0, false);
   }

   /*
   ==================================
 /\/|____        _
|/\/_   _|__ ___| |_
     | |/ _ Y __| __|
     | |  __|__ \ |_
     |_|\___|___/\__|
   ==================================
   */
   Test::~Test()
   {
      m_symbol->Release();
   }

   /*
   ==================================
  ____      _  _____
 / ___| ___| ||_   _|   _ _ __   ___
| |  _ / _ \ __|| || | | | '_ \ / _ \
| |_| |  __/ |_ | || |_| | |_) |  __/
 \____|\___|\__||_| \__, | .__/ \___|
                    |___/|_|
   ==================================
   */
   egSKITestType Test::GetType( Error* err) const
   {
      return m_type;
   }

   /*
   ==================================
 ___     ____  _                 _     _____        _
|_ _|___/ ___|(_)_ __ ___  _ __ | | __|_   _|__ ___| |_
 | |/ __\___ \| | '_ ` _ \| '_ \| |/ _ \| |/ _ Y __| __|
 | |\__ \___) | | | | | | | |_) | |  __/| |  __|__ \ |_
|___|___/____/|_|_| |_| |_| .__/|_|\___||_|\___|___/\__|
                          |_|
   ==================================
   */
   bool Test::IsSimpleTest(test thisTest)
   {
      if(test_is_complex_test(thisTest))
      {
         complex_test* ct = complex_test_from_test(thisTest);
         egSKITestType type =  EnumRemappings::ReMapTestType(ct->type);

         if((type == gSKI_CONJUNCTION) || ( type == gSKI_DISJUNCTION))
            return false;
      }
      return true;

   }

    /*
   ==================================
  ____      _ __     __    _
 / ___| ___| |\ \   / /_ _| |_   _  ___
| |  _ / _ \ __\ \ / / _` | | | | |/ _ \
| |_| |  __/ |_ \ V / (_| | | |_| |  __/
 \____|\___|\__| \_/ \__,_|_|\__,_|\___|
   ==================================
   */
   ISymbol* Test::GetValue( Error* err) const
  {
     return m_symbol;
  }


    /*
   ==================================
  ____      _  _____         _
 / ___| ___| ||_   _|____  _| |_
| |  _ / _ \ __|| |/ _ \ \/ / __|
| |_| |  __/ |_ | |  __/>  <| |_
 \____|\___|\__||_|\___/_/\_\\__|
   ==================================
   */
  std::string Test::GetText()
  {
     std::string testText;
     switch(m_type)
     {
        case gSKI_EQUAL:
           testText += "";
           break;
        case gSKI_GREATER_THAN:
           testText += "> ";
           break;
        case gSKI_LESS_THAN:
           testText += "< ";
           break;
        case gSKI_GREATER_OR_EQUAL:
           testText += ">= ";
           break;
        case gSKI_LESS_THAN_OR_EQUAL:
           testText += "<= ";
           break;
        case gSKI_NOT_EQUAL:
           testText += "<> ";
           break;
        case gSKI_DISJUNCTION:
           testText += "disjunction: ";
           break;
        case gSKI_CONJUNCTION:
           testText += "conjunction: ";
           break;
        case gSKI_OTHER:
           testText += "Uh Oh! ";
           break;
     }

     testText += gSymbol::ConvertSymbolToString(m_symbol);
     
     return testText;
  }


}
