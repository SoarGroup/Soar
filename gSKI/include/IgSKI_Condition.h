/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_condition.h 
*********************************************************************
* created:	   6/13/2002   12:18
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_CONDITION_H
#define IGSKI_CONDITION_H

namespace gSKI {

   struct Error;
   class ITestSet;

   /**
    * @brief The abstract base class for Conditions
    *
    * This is a simple interface that offers read only
    * access to the text of a condition and whether or
    * not it is negated.
    */
   class ICondition
   {
   public:
      /**
       * @brief Destructor for the ICondition
       *
       * This function insures that the destructor in the most derived
       * class is called when it is destroyed.  This will always be 
       * neccesary because this is a pure virtual base class.
       */
      virtual ~ICondition(){};

      /**
       * @brief Retrieves the text of the condition
       *
       * The text returned by GetText is <b>NOT</b> guaranteed to
       * be a textural match of the one enter in to the production.
       * It may, or may not be the same.  It will be functionally
       * equivalent.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The text of the condition as a C style string
       */
      virtual const char* GetText(Error* err = 0) = 0;

      /**
       * @brief Returns the negated status of the condition
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns true if the condition is negated.  false if it is not
       */
      virtual bool IsNegated(Error* err = 0) = 0;

      /**
       * @brief Indicates if the condition is a state or not.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns true if the condition is a state test.
       */
      virtual bool IsStateCondition(Error* err = 0) = 0;

      /**
       * @brief Indicates if the condition is an impasse test or not.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns true if the condition is a state test.
       */
      virtual bool IsImpasseCondition(Error* err = 0) = 0;

      /**
       * @brief Returns the Test for the Identifier.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       */
      virtual ITestSet* GetIdTest(Error* err = 0) = 0;

      /**
       * @brief Returns the Test for the Attribute.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       */
      virtual ITestSet* GetAttrTest(Error* err = 0) = 0;

      /**
       * @brief Returns the Test for the Value.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       */
      virtual ITestSet* GetValTest(Error* err = 0) = 0;
   };
}
#endif
