/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_rhsfunctionaction.h 
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#ifndef GSKI_RHS_FUNCTION_ACTION_HEADER
#define GSKI_RHS_FUNCTION_ACTION_HEADER

#include "gSKI_ActionElement.h"
#include "gSKI_Error.h"
#include "IgSKI_Iterator.h"

#include <string>
#include <vector>

typedef struct cons_struct list;

namespace gSKI {

   /** 
    * @brief Definition of the rhs function action interface
    *
    * The Right hand side function action interface is used to 
    *  access the parsed rhs function on the rhs of a production.
    *
    * Right hand side functions come in two major flavors:
    *  @li Stand alone functions - These do not return symbols and 
    *          do not create wme preferences.  They are not part
    *          of RhsAction objects.
    *  @li Nested functions - These are part of either the attribute
    *          or value of RhsAction objects and return a symbol
    *          used to form a wme preference.
    *
    * Use this interface to get the name and parameters for a particular
    *   call to a rhs function in a production.
    */
   class RhsFunctionAction
   {

   public:

      /**
       * @brief Constructs a function call action from soar data
       *
       * Default and copy constructors are also provided.
       *
       * @param funcall_list List containing the function name (first element)
       *              and all parameters (rhs_value)
       */
      //{
      RhsFunctionAction();
      RhsFunctionAction(agent* a, list* funcall_list);
      RhsFunctionAction(const RhsFunctionAction& old) { *this = old; }
      //}

      /** 
       * @brief Sets the values of this RhsFunctionAction from a soar funcall_list
       *
       * Call this method when you have used the default constructor to create
       *  your RhsFunctionAction object.
       *
       */
      void SetValues(agent* a, list* funcall_list);

      /** 
       * @brief copy operator
       */
      RhsFunctionAction& operator=(const RhsFunctionAction& rhs);

      /**
       * @brief Destructor
       *
       * The destructor cleans up all parameter action elements.  They are owned
       *  by the rhs function action object, so you should not hold pointers to
       *  its action elements when this object has been destroyed
       */
     ~RhsFunctionAction();

      /**
       * @brief Gets the Rhs function name
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A c-style string containing the name of the rhs function.
       *          This pointer will never be 0.
       */
     const char*             GetName(Error* err);

      /**
       * @brief Gets the number of parameters this rhs function has
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return Returns the number of parameters that should be passed
       *          to this rhs function.  If this value is < 0, the function
       *          can take any number of parameters.
       */
     int                     GetNumParameters(Error* err) const;


      /** 
       * @brief Get the list of parameter action elements.
       *
       * Call this method to obtain an iterator into the list of all of the
       *  parameters for this function.  These parameters are action elements
       *  so they can be symbols (variable or constant) or other rhs functions.
       *  This allows for infinite recursion of rhs functions.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to an iterator into the list of all action elements
       *           used as parameters to this function.  You do not need to
       *           release any of the action elements.
       */
     tIActionElementIterator* GetParameterList(Error* err);

      /**
       * @brief Get whether or not this function is standalone
       *
       * Call this method to find out whether this function is a stand alone
       *  function (meaning, it is its own action and does not create
       *  preferences for wmes or return a symbol), or if it is a nested
       *  rhs function (meaning it returns a symbol used for other rhs
       *  functions or to create wme preference.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if this is a stand alone function, false if it is a
       *          nested function.
       *
       */ 
     bool                    IsStandAlone(Error* err) const;

   private:

      /** 
       * @brief Cleans up memory used by the parameters 
       */
      void cleanup();

   private:

	   /**
	    * Typedef for the container used to store rhs function parameters.
	    */
	   //{
      typedef std::vector<ActionElement*> tParamVec;
      typedef tParamVec::iterator         tParamVecIt;
      typedef tParamVec::const_iterator   tParamVecCIt;
	   //}

      /** 
       * Iterator for parameters (external to class)
       */
      //typedef gSKI::ObjectToPtrIterator<ActionElement*, tParamVec> tParamIterator; 

      /** Name of the rhs function */
      std::string       m_name;

      /** Number of parameters passed to this rhs function (-1 for infinite) */
      int               m_numParams;

      /** true if this rhs function is standalone */
      bool              m_standAlone;

      /** List of parameters (as action elements) */
      tParamVec         m_parameters;
   };
}

#endif

