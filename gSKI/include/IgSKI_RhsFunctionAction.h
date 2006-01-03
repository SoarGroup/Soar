/********************************************************************
* @file igski_rhsfunctionaction.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#ifndef IGSKI_RHS_FUNCTION_ACTION_HEADER
#define IGSKI_RHS_FUNCTION_ACTION_HEADER

#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"

namespace gSKI {

   struct Error;
   class  IActionElement;

   /** 
    * @brief Definition of the rhs function action interface
    *
    * The Right hand side function action interface is used to 
    *  access the parsed rhs function on the rhs of a production.
    *
    * Right hand side functions come in two major flavors:
    *  @li Stand alone functions - These do not return symbols and 
    *          do not create wme preferences.  They are not part
    *          of IRhsAction objects.
    *  @li Nested functions - These are part of either the attribute
    *          or value of IRhsAction objects and return a symbol
    *          used to form a wme preference.
    *
    * Use this interface to get the name and parameters for a particular
    *   call to a rhs function in a production.
    */
   class IRhsFunctionAction {

   public:

      /**
       * @brief Destructor
       *
       * The destructor cleans up all parameter action elements.  They are owned
       *  by the rhs function action object, so you should not hold pointers to
       *  its action elements when this object has been destroyed
       */
      virtual ~IRhsFunctionAction() {}

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
      virtual const char*             GetName(Error* err) = 0;

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
      virtual int                     GetNumParameters(Error* err) const = 0;


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
      virtual tIActionElementIterator* GetParameterList(Error* err) = 0;

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
      virtual bool                    IsStandAlone(Error* err) const = 0;

   };
}

#endif

