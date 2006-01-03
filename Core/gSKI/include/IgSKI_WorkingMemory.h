/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_workingmemory.h 
*********************************************************************
* created:	   6/13/2002   14:54
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_WORKINGMEMORY_H
#define IGSKI_WORKINGMEMORY_H

#include "IgSKI_WorkingMemoryView.h"

namespace gSKI {

   // Forward declarations 
   class ISymbol;
   class ISymbolFactory;
   class IWMObject;
   class IWme;
   class IWorkingMemoryListener ;
   struct Error;

   /** 
    * @brief The main interface for access to the various aspects of the
    *         Soar agent's working memory.
    *
    * This interface handles all the details involved with adding, removing
    * and replacing Wme's in a Soar agent's working memory. This interface
    * also provides access to the ISymbolFactory interface for creating 
    * Soar Symbols.
    */
   class IWorkingMemory: public IWorkingMemoryView {
   public:
       
     /**
      * @brief Virtual Destructor
      * 
      * Including a virtual destructor for the usual C++ safety reasons.
      */     
     virtual ~IWorkingMemory() {}

     /**
      * @brief Returns a pointer to the Agent that owns this WorkingMemory
      *         interface
      *
      * This method returns a pointer to the agent that owns this WorkingMemory
      * object.
      *
      * @param  pErr Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return a pointer to the agent that owns the WorkingMemory 
      *          (this pointer can never be NULL)
      */
     virtual IAgent* GetAgent(Error * err = 0) const = 0;

     /**
      * @brief Adds a Wme using the old-style symbol syntax
      *
      * This method adds a Wme to an agent's working memory using the old-style
      * symbol triplet syntax (ID, Attribute, Value Symbols). These symbols
      * can be obtained from the ISymbolFactory interface or from existing
      * existing Wme's. The idSymbol must be of type ID, while the 
      * attrSymbol and valSymbol may be of any symbol type. If the idSymbol
      * is not of type ID or the ID symbol does not currently exist in 
      * working memory then a 0 Wme pointer will be returned.
      *
      * @param idSymbol A symbol representing the ID of the Wme
      * @param attrSymbol A symbol representing the attribute of the Wme
      * @param valSymbol A symbol representing the value of the Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *             is not 0 this structure is filled with extended error
      *             information.  If it is 0 (the default) extended error
      *             information is not returned.
      *
      * @return A pointer to the newly created Wme or NULL if the 
      *          specified ID doesn't exist.
      */
     virtual IWme* AddWme(const ISymbol* idSymbol, 
                          const ISymbol* attrSymbol,
                          const ISymbol* valSymbol,
                          Error* err = 0) = 0;

     /**
      * @brief Adds a Wme with an integer value
      *
      * This method adds a Wme with an integer value to an agent's working 
      * memory. The Wme is added as a child of the specified wmObject. If
      * the wmObject is invalid (NULL or no longer exists in the agent's 
      * working memory) no Wme will be added and a NULL pointer will be 
      * returned. The same is true if the attribute string is NULL.
      *
      * @param wmObject a pointer to the WMObject that corresponds to this Wme
      * @param attr a string representing the attribute for the new Wme
      * @param intValue the integer value for the new Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the newly created Wme or NULL if the wmObject
      *          is invalid or the attr string is NULL
      */
     virtual IWme* AddWmeInt(IWMObject* wmObject,
                             const char* attr,
                             int intValue,
                             Error* err = 0) = 0;

     /**
      * @brief Adds a Wme with a double value
      *
      * This method adds a Wme with a double value to an agent's working 
      * memory. The Wme is added as a child of the specified wmObject. If
      * the wmObject is invalid (NULL or no longer exists in the agent's 
      * working memory) no Wme will be added and a NULL pointer will be 
      * returned. The same is true if the attribute string is NULL.
      *
      * @param wmObject a pointer to the WMObject that corresponds to this Wme
      * @param attr a string representing the attribute for the new Wme
      * @param dValue the double value for the new Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the newly created Wme or NULL if the wmObject
      *          is invalid or the attr string is NULL
      */
     virtual IWme* AddWmeDouble(IWMObject* wmObject,
                                const char* attr,
                                double dValue,
                                Error* err = 0) = 0;

     /**
      * @brief Adds a Wme with a string value
      *
      * This method adds a Wme with a string value to an agent's working 
      * memory. The Wme is added as a child of the specified wmObject. If
      * the wmObject is invalid (NULL or no longer exists in the agent's 
      * working memory) no Wme will be added and a NULL pointer will be 
      * returned. The same is true if the attribute or value string is NULL.
      *
      * @param wmObject a pointer to the WMObject that corresponds to this Wme
      * @param attr a string representing the attribute for the new Wme
      * @param value the string value for the new Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the newly created Wme or NULL if the wmObject
      *          is invalid or the attr string is NULL
      */
     virtual IWme* AddWmeString(IWMObject* wmObject,
                                const char* attr,
                                const char* value,
                                Error* err = 0) = 0;

     /**
      * @brief Adds a Wme with a value that is a copy of the specified object
      *
      * This method adds a Wme whose value is a copy of the specified object.
      * This is a deep copy which includes creating Wme's for the Wme's of the
      * copied object.
      *
      * The copy is constructed using the current state of the specified 
      * object which may not necessarily correspond with earlier states of 
      * the object (since the objects can be modified internally by the
      * Soar agent).
      *
      * @param wmObject a pointer to the WMObject that is the parent of this 
      *         Wme
      * @param attr a string representing the attribute for the new Wme
      * @param value a pointer to the object to copy as the new value of this
      *         Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the newly created Wme or NULL if the wmObject
      *           is invalid, the attr string is NULL, or the object
      *           to be copied is invalid
      */
     // TODO: Is there a depth that needs to specified here for the copy
     virtual IWme* AddWmeObjectCopy(IWMObject*       wmObject,
                                    const char*      attr,
                                    const IWMObject* value,
                                    Error*           err = 0) = 0;

     /**
      * @brief Adds a Wme whose value is an already existing object
      *
      * This method adds a Wme whose value is an already existing object.
      * No copying of the already existing object is necessary and at most
      * one Wme is created in this method. If the parent WMObject is invalid
      * (NULL or no longer existing in the agent's working memory), the 
      * attribute string is NULL, or the specified value object is invalid
      * then no Wme will be created and a NULL IWme pointer will be returned.
      *
      * @param wmObject a pointer to the WMObject that is the parent of this
      *         Wme
      * @param attr a string representing the attribute for the new Wme
      * @param value a pointer to the already existing object to make a link to
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the newly created Wme or NULL if the specified
      *           WMObject is invalid, the attribute string is NULL or the
      *           linked object is invalid.
      */
     virtual IWme* AddWmeObjectLink(IWMObject* wmObject,
                                    const char* attr,
                                    IWMObject* value,
                                    Error* err = 0) = 0;

     /**
      * @brief Adds a Wme whose value is a enw WMObject
      *
      * This method adds a new Wme whose value is a new object (with no
      * child Wme's). If the parent Wme is invalid (NULL or no longer existing
      * in the agent's working memory) or the attribute is NULL, then no new
      * Wme or WMObject will be created and a NULL IWme pointer will be
      * returned. Use the GetValue() method of the new Wme to obtain a 
      * pointer to the newly created WMObject.
      *
      * @param wmObject a pointer to the WMObject that is the parent of this 
      *         Wme
      * @param attr a string representing the attribute of the new Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the newly created Wme or NULL if the specified
      *           WMObject is invalid or the attribute string is NULL.
      */
     virtual IWme* AddWmeNewObject(IWMObject* wmObject,
                                   const char* attr,
                                   Error* err = 0) = 0;

     /**
      * @brief Replaces an existing Wme with one that has a different value
      *
      * This method replaces an existing Wme with a new one that has a 
      * different value. The new Wme has the same ID (belongs to the 
      * same WMObject) and attribute, but a new value. 
      * If the pointer to the old Wme is invalid (NULL or
      * no longer exists in the agent's working memory) then the Wme is 
      * not replaced and a NULL IWme pointer is returned.
      *
      * @param oldwme a pointer to the Wme to be replaced
      * @param value the value for the new Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the new Wme or NULL if the Wme to be replaced
      *          is invalid or the new value pointer is NULL
      */
     virtual IWme* ReplaceWme(IWme* oldwme,
                              const ISymbol* newvalue,
                              Error* err = 0) = 0;
     
     /**
      * @brief Replaces an existing Wme with one that has the specified integer
      *         value
      *
      * This method replaces an existing Wme with a new one that has the
      * specified integer value. The new Wme has the same ID (belongs to the 
      * same WMObject) and attribute, but a new value. If the pointer to the 
      * old Wme is invalid (NULL or longer exists in the agent's working 
      * memory) then the Wme is not replaced and a NULL IWme pointer is 
      * returned.
      *
      * @param oldwme a pointer to the Wme to be replaced
      * @param value the integer value of the new Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the new Wme or NULL if the Wme to be replaced
      *          is invalid
      */
     virtual IWme* ReplaceIntWme(IWme* oldwme,
                                 int newvalue,
                                 Error* err = 0) = 0;

     /**
      * @brief Replaces an existing Wme with one that has the specified
      *         double value
      * 
      * This method replaces an existing Wme with a new one that has the 
      * specified double value. The new Wme has the same ID (belongs to the 
      * same WMObject) and attribute, but a new value. If the pointer to the 
      * old Wme is invalid (NULL or no longer exists in the agent's working 
      * memory) then the Wme is not replaced and a NULL IWme pointer is 
      * returned.
      * 
      * @param oldWme a pointer to the Wme to be replaced
      * @param value the double value of the new Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the new Wme or NULL if the Wme to be replaced
      *          is invalid
      */
     virtual IWme* ReplaceDoubleWme(IWme* oldwme,
                                    double newvalue,
                                    Error* err = 0) = 0;
     
     /**
      * @brief Replaces an existing Wme with one that has the specified
      *         string value
      * 
      * This method replaces an existing Wme with a new one that has the 
      * specified string value. The new Wme has the same ID (belongs to the
      * same WMObject) and attribute, but a new value. If the pointer to the 
      * old Wme is invalid (NULL or no longer exists in the agent's working
      * memory) or the new string value is NULL, then the Wme is not replaced 
      * and a NULL IWme pointer is returned. 
      *
      * @param oldwme a pointer to the Wme to be replaced
      * @param value the string value of the new Wme
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the new Wme or NULL if the Wme to be replaced
      *          is invalid, or the specified string value is NULL
      */
     virtual IWme* ReplaceStringWme(IWme* oldwme,
                                    const char* newvalue,
                                    Error* err = 0) = 0;
    
     /**
      * @brief Replaces an existing Wme with one that contains a copy
      *         of the specified WMObject
      *
      * This method replaces an existing Wme with a new one that contains a 
      * copy of the specified WMObject. The new Wme has the same ID (belongs
      * to the same WMObject) and attribute as the original Wme, but a new
      * value (a copy of the specified WMObject). The copy of the specified
      * WMObject will have a different ID but will contain all the same
      * Wmes as the original object. A deep copy is performed so that an
      * WMObjects referenced by the specified WMObject will also be copied.
      * The copy reflects the state of the original WMObjects at the time
      * this method is invoked. Remember that WMObjects can also be modified
      * by the Soar agent.
      *
      * @param oldwme a pointer to the Wme to be replaced
      * @param value a pointer to the WMObject to perform a deep copy of
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the new Wme or NULL if the Wme to be replaced
      *          or the Wme to be copied is invalid
      */
     virtual IWme* ReplaceWmeObjectCopy(IWme* oldwme,
                                        const IWMObject* newvalue,
                                        Error* err = 0) = 0;
     // TODO: Should the first parameter be const since this action will
     //        change the return value of HasBeenRemoved().

     /**
      * @brief Replaces an existing Wme with one that contains a link to
      *         an already existing WMObject
      *
      * This method replaces an existing Wme with a new one that contains a 
      * link to the specified WMObject. The Wme has the same ID (belongs to 
      * the same WMObject) and attribute as the old wme, but a new value 
      * (a pointer to the specified WMObject). 
      *
      * @param oldwme a pointer to the Wme to be replaced
      * @param newvalue a pointer to the WMObject to add a link to 
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the new Wme or NULL if the Wme to be replaced
      *          or the Wme to be linked to is invalid
      */
     virtual IWme* ReplaceWmeObjectLink(IWme* oldwme,
                                        const IWMObject* newvalue,
                                        Error* err = 0) = 0;
     
     /** 
      * @brief Replaces an existing Wme with one that contains a new WMObject
      *
      * This method replaces an existing Wme with a new one that points to
      * a new WMObject. The new Wme has the same ID (belongs to the same
      * WMObject) and attribute as the old wme, but a new value ( a pointer
      * to a new WMObject). The new WMObject has no child Wmes. To obtain
      * a pointer to the new WMObject use the GetValue() method of the 
      * new IWme pointer.
      *
      * @param oldwme a pointer to the Wme to be replaced
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return A pointer to the new Wme or NULL if the Wme to be replaced
      *          is invalid
      */
     virtual IWme* ReplaceWmeNewObject(IWme* oldwme,
                                       Error* err = 0) = 0;
     

     /**
      * @brief Removes a Wme from the agent's working memory
      *
      * This method removes the specified Wme from the agent's working memory.
      * No error is returned if the specified Wme is no longer in the
      * agent's working memory or if the passed in IWme* is NULL.
      *
      * Removing a Wme that references a WMObject, may cause that WMObject
      * to be invalidated if the Wme is the last reference to it.
	  *
	  * Calling RemoveWme() implicitly decrements the reference count of the wme
	  * being removed, so a caller should not call Release() on the wme after calling
	  * RemoveWme().
      *
      * @param wme a pointer to the Wme to be removed
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
     virtual void RemoveWme(IWme* wme,
                            Error* err = 0) = 0;

     /**
      * @brief Removes an Object and all its Wmes from the agent's working
      *         memory
      *
      * This method removes the specified WMObject from the agent's working
      * memory along with all its child Wmes. No error is returned if the the
      * specified WMObject is no longer in the agent's working memory or if 
      * the WMObject pointer is NULL.
      * 
      * @param object a pointer to the object to be removed from the agent's 
      *         working memory
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
     virtual void RemoveObject(IWMObject* object,
                               Error* err = 0) = 0;

     /** 
      * @brief A convenience method for removing all the wmes owned by
      *         the specified WMObject
      *
      * This method removes all the Wmes associated with the given WMObject.
      * It is equivalent to iterating over all the Wmes of an object and
      * calling RemoveWme() on each of them. No error is returned if the 
      * specified WMObject pointer is NULL or no longer exists in the agent's 
      * working memory.
      *
      * @param object a pointer to the object to remove all the Wmes from
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
     virtual void RemoveObjectWmes(IWMObject* object,
                                   Error* err = 0) = 0;

     /**
      * @brief Returns a pointer to a SymbolFactory which can be used 
      *         to create Symbols for various purposes
      *
      * This method return a pointer to a Symbol factory that can be used
      * to create INT, DOUBLE and STRING symbols. Release should be called
      * to free up the resources of the SymbolFactory when the SymbolFactory
      * is no longer needed.
      * 
      * @param err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return a pointer to an ISymbolFactory interface
      */
     virtual ISymbolFactory* GetSymbolFactory(Error* err = 0) = 0;
      
	  /**
       * @brief Listen for changes to working memory.
       *
	   * @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	   * @param listener	The handler to call when event is fired
       */
	  virtual void AddWorkingMemoryListener(egSKIWorkingMemoryEventId            eventId, 
											IWorkingMemoryListener* listener, 
											Error*                  err = 0) = 0 ;

	  /**
       * @brief Remove an existing listener
       *
	   * @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	   * @param listener	The handler to call when event is fired
       */
	  virtual void RemoveWorkingMemoryListener(egSKIWorkingMemoryEventId            eventId, 
											   IWorkingMemoryListener* listener, 
											   Error*                  err = 0) = 0 ;
   };
}


#endif
