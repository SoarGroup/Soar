/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file IgSKI_Test.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_TEST_H
#define IGSKI_TEST_H

#include "gSKI_Enumerations.h"

namespace gSKI
{
   class  ISymbol;
   struct Error;

   /**
    * @brief: Class used to represent a simple condition test.
    */
   class ITest
   {
   public:
      
      /**
       * @brief  Destructor for test.
       */
      virtual ~ITest(){}

      /**
       * @brief This returns the type of the test.
       *
       * @see egSKITestType
       *
       * @returns the test type. 
       */
      virtual egSKITestType GetType(Error* err = 0) const = 0;

      /**
       * @brief Returns the value being tested against.
       *
       * @returns The symbol which holds the value we are testing
       *          against.
       */
      virtual ISymbol*  GetValue(Error* err = 0) const = 0;
      
   };
}
#endif
