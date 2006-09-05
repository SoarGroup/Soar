/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_actionelement.h 
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#ifndef GSKI_ACTION_ELEMENT_HEADER
#define GSKI_ACTION_ELEMENT_HEADER

#include "gSKI_Enumerations.h"
#include "gSKI_Error.h"
#include "IgSKI_Symbol.h"

typedef struct agent_struct agent;

namespace gSKI {

   class gSymbol;
   class RhsFunctionAction;

   typedef char* rhs_value;

   /**
    * @brief Defines the interface for an action element
    *
    * An action element describes one part of an action triplet.
    *  It can be either a constant symbol (string, double, int),
    *  a variable symbol (<a>, <var1>, etc) or a RhsFunctionAction
    *  that generates one of these two types of symbols.
    *
    * An action element is used internally by the kernel to
    *  create a single symbol for a new WME when a production
    *  fires.
    *
    */
   class ActionElement
   {
   public:
      
      /**
       * @brief Constructor for action element
       * 
       * There are two constructors depending on the type
       *  of action element you want to create.
       *
       * Copy operations also supplied (needed to do ref count
       *  on symbol correctly).
       */
      //{
      ActionElement();
      ActionElement(agent* a, rhs_value rhsval);
      ActionElement(const ActionElement& old) 
      { 
         m_val.sym = 0;    // need this to ensure no cleanup before creation
         *this = old; 
      }
      //}

      /** 
       * @brief Copy operator (guarrantees proper symbol ref counting)
       */
      ActionElement& operator=(const ActionElement& rhs);

      /** 
       * @brief Sets the value of this action element from a soar rhs_value
       *
       * Call this to set the action element's values if you used the default
       *   constructor.
       */
      void SetValues(agent* a, rhs_value rhsval);

      /**
       * @brief Destructor
       *
       * The destructor cleans up the data it references.  It owns
       *  the symbol or rhs function element it contains, so you
       *  should not hold references to a symbol or rhs function action
       *  after the owning element is destroyed.
       */
     ~ActionElement();

      /**
       * @brief  Returns the type of data stored by this action element
       *
       * Action elements can store either a symbol or RhsFunctionAction.
       *  ALWAYS call this method BEFORE accessing the data with either
       *  GetFunction or GetSymbol, each of these functions will return
       *  0 if you call them when the element is not a function or
       *  symbol respectively.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return The type of data stored in this action element.  The value
       *          will be either gSKI_ACTION_SYMBOL, or gSKI_ACTION_FUNCTION.
       */
     egSKIActionElementType      GetType(Error* err) const;

      /** 
       * @brief Returns the value of this element as a RhsFunctionAction
       *
       * Call this method to retrieve the data of this element as a 
       *  right hand side function action.  Make sure you call 
       *  GetType before calling this method to ensure that the
       *  type stored is actually a RHS function.
       *
       * Possible Errors:
       *   @li gSKIERR_ELEMENT_NOT_FUNCTION if the action element is not
       *               a rhs function.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to the RhsFunctionAction referenced by this
       *          element.  The object returned is owned by the element
       *          and should NOT be deleted by the client.  If an error
       *          occurs (e.g. the type of this element is not 
       *          gSKI_ACTION_FUNCTION) 0 is returned.
       */
     const RhsFunctionAction*   GetFunction(Error* err) const;

      /** 
       * @brief Returns the value of this element as a symbol
       *
       * Call this method to retrieve the data of this element as a 
       *  symbol.  Make sure you call GetType before calling this method
       *  to ensure that the type stored is actually a symbol.
       *
       * Possible Errors:
       *   @li gSKIERR_ELEMENT_NOT_SYMBOL if the action element is not
       *               a symbol.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to the symbol referenced by this
       *          element.  The object returned is owned by the element
       *          and should NOT be deleted by the client.  If an error
       *          occurs (e.g. the type of this element is not 
       *          gSKI_ACTION_SYMBOL) 0 is returned.
       */
      const ISymbol*              GetSymbol(Error* err) const;

   private:

      /** 
       * @brief private member to clean up old memory use.
       */
      void cleanup();

   private:

      /** Stores the actual symbol or rhs function */
      union Value {
         gSymbol*             sym;
         RhsFunctionAction*  func;
      } m_val;

      /** Stores the type of this action element */
      egSKIActionElementType  m_type;
   };
}

#endif

