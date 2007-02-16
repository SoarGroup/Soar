/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_error.h 
*********************************************************************
* created:	   6/19/2002   11:41
*
* purpose: 
*********************************************************************/

#ifndef GSKI_ERROR_H
#define GSKI_ERROR_H

#include "gSKI_ErrorIds.h"
#include <string.h>

namespace gSKI {

   /** List of error messages
    *
    * You must add to this list every time a new error number is entered.
    *  The order of these messages should be the same as the order of the
    *  error numbers
    */
   static const char *gSKI_errorText[gSKIERR_MAXERRNO] =
   {
      "Success!",
      "No Error was returned because 0 was passed in.",
      "File not found!",
      "Bad File Format!",
      "Can not add production, it already exists.  Excise first.",
      "Production does not exist!",
      "Unable to save, disk full!",
      "File permissions prohibit saving file!",
      "The given pointer is invalid",
      "The given pattern in invalid",
      "The specified agent does not exist within the context of this Agent Manager",
      "The action element is a symbol, but you are accessing it as a rhs function.",
      "The action element is a rhs function, but you are accessing it as a symbol.",
      "The symbol you are dereferencing has no string value.  Cannot dereference.",
      "The symbol you are dereferencing has no integer value.  Cannot dereference.",
      "The symbol you are dereferencing has no double value.  Cannot dereference.",
      "The symbol you are dereferencing has no object value.  Cannot dereference.",
      "This action does not create a binary preference.  There is no binary referent.",
      "Agent is already running.  Cannot run again.",
      "The requested operation is not implemented in gSKI yet.",
      "Agent is stopped.  Cannnot continue.  Use continue to start an agent when it is interrupted.",
      "Agent is halted. You need to reinitialize before it can run again.",
      "There are no agents in the run list to run.  Add at least one agent to the manager's run list first.",
      "Unable to add the given production",
	  "A duplicate production already exists",
      "A RHS function with the given name already exists. Remove the old one first before adding the new one.",
      "The given RHS function is not currently registered with this agent; therefore, it cannot be removed."
   };

   /**
    * @brief Function for creating and setting an error value.
    *
    * @param e The error struct we are return the values in.
    * @param id The identifier of the error.
    *
    * @returns a copy of the new Error value.
    */
   inline void SetErrorExtended(Error *e, tgSKIErrId id, const char *ExtendedMsg){
      if(e != 0)
      {
         e->Id = id;
         strncpy( e->ExtendedMsg, ExtendedMsg, (gSKI_EXTENDED_ERROR_MESSAGE_LENGTH - 1) );
         e->Text = gSKI_errorText[id];
      }
   }

   /**
    * @brief Function for creating and setting an error value.
    *
    * @param e The error struct we are putting the values in.
    * @param id The identifier of the error.
    *
    * @returns a copy of the new Error value.
    */
   inline void SetError(Error *e, tgSKIErrId id){
      if(e != 0)
      {
         SetErrorExtended(e, id, "");
      }
   }

   /**
    * @brief clears error from the error structure
    *
    * This is a simple wrapper around SetError to set the
    * structure to no-error.
    *
    * @param e The error struct being cleared.
    */
   inline void ClearError(Error *e){
      SetError(e,gSKIERR_NONE);
   }

   /**
    * @brief helper function for error handling.
    */
   inline bool isError(Error err)
   {
      return(err.Id != gSKIERR_NONE);
   }

}
#endif
