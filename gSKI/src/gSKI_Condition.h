/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_Condition.h 
*********************************************************************
* created:	   6/13/2002   12:18
*
* purpose: 
*********************************************************************/
#ifndef GSKI_CONDITION_H
#define GSKI_CONDITION_H

#include "IgSKI_Condition.h"

typedef struct condition_struct condition;
typedef struct agent_struct agent;
typedef char * test;

namespace gSKI {
   class  TestSet;
   class  Test;
   struct Error;

   /**
    * @brief The abstract base class for Conditions
    *
    * This is a simple interface that offers read only
    * access to the text of a condition and whether or
    * not it is negated.
    */
   class Condition : public ICondition
   {
   public:
      /**
       * @brief: Constuctor for the Condition
       */
      Condition(condition *cond, agent* a);

      /**
       * @brief Destructor for the Condition
       *
       * This function insures that the destructor in the most derived
       * class is called when it is destroyed.  This will always be 
       * neccesary because this is a purebase class.
       */
     virtual ~Condition();

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
     const char* GetText(Error* err = 0);

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
     bool IsNegated(Error* err = 0);

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
      virtual bool IsStateCondition(Error* err = 0);

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
      virtual bool IsImpasseCondition(Error* err = 0);

      /**
       * @brief Returns the Test for the Identifier.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       */
      ITestSet* GetIdTest(Error* err = 0);

      /**
       * @brief Returns the Test for the Attribute.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       */
      ITestSet* GetAttrTest(Error* err = 0);

      /**
       * @brief Returns the Test for the Value.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       */
      ITestSet* GetValTest(Error* err = 0);


   private:
      condition*        m_condition;
      agent*            m_agent;

      TestSet*          m_idTest;
      TestSet*          m_attrTest;
      TestSet*          m_valTest;




   };
}
#endif
