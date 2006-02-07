#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_InputWorkingMemory.cpp 
*********************************************************************
* created:	   7/22/2002   16:28
*
* purpose: 
*********************************************************************/

#include "gSKI_InputWorkingMemory.h"
#include "gSKI_InputWMObject.h"
#include "gSKI_InputWme.h"
#include "gSKI_Wme.h"
#include "gSKI_Symbol.h"
#include "gSKI_Error.h"
#include "gSKI_Agent.h"

#include <set>

#include "MegaAssert.h"

#include "symtab.h"
#include "io.h"
#include "wmem.h"
#include "agent.h"
#include "gdatastructs.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_InputWorkingMemory);

namespace gSKI
{
   /*
     ===============================
     
     ===============================
   */
   
   InputWorkingMemory::InputWorkingMemory(Agent* agent):
      m_agent(agent),
      m_rootInputObject(0)
   {}

   /*
     ===============================
     
     ===============================
   */
   
   InputWorkingMemory::~InputWorkingMemory() 
   {
      // Releasing my reference to the root object
      if ( m_rootInputObject != 0 )
      {
         m_rootInputObject->Release();
      }

      // Releasing all the stored objects
      ReleaseAllWmes();
      ReleaseAllWMObjects();
      ReleaseAllSymbols();
   }
   
   /*
     ===============================
     
     ===============================
   */
   
   IAgent* InputWorkingMemory::GetAgent(Error * err) const
   {
      ClearError(err);
      
      return m_agent;
   }
   
   /*
     ===============================
     
     ===============================
   */  
   IWme* InputWorkingMemory::AddWme(IWMObject* wmobject,
				    const ISymbol* attrSymbol,
				    const ISymbol* valSymbol)
   {
      if ( wmobject == 0 || attrSymbol == 0 || valSymbol == 0 ) 
      {
         MegaAssert(false,"Null pointer passed to AddWme function!");
	      return 0;
      }
    
      InputWMObject* iobj = GetOrCreateObjectFromInterface(wmobject);
      gSymbol* attr = gSymbol::ConvertSymbol(attrSymbol);
      gSymbol* val = gSymbol::ConvertSymbol(valSymbol);
      InputWme* wme = new InputWme(this, iobj, attr, val);
      
      iobj->AddReferencedWme(wme);
      
      return wme;
   }

   /*
     ===============================
     
     ===============================
   */
   
   IWme* InputWorkingMemory::AddWme(const ISymbol* idSymbol, 
                                    const ISymbol* attrSymbol,
                                    const ISymbol* valSymbol,
                                    Error* err)
   {
      ClearError(err);
      
      if ( idSymbol == 0 || attrSymbol == 0 || valSymbol == 0 ) 
      {
	      SetErrorExtended(err, 
			      gSKIERR_INVALID_PTR, 
			      "Null pointer passed to AddWme function!" );
	      return 0;
      }
      
      InputWMObject* iobj = GetOrCreateObjectFromSymbol(idSymbol);
      
      return AddWme(iobj, attrSymbol, valSymbol);
   }
   
   /*
     ===============================
     
     ===============================
   */
   
   IWme* InputWorkingMemory::AddWmeInt(IWMObject* wmObject,
                                       const char* attr,
                                       int intValue,
                                       Error* err)
   {
      ClearError(err);

      if ( wmObject == 0 || attr == 0 ) 
      {
	      SetErrorExtended(err,
			      gSKIERR_INVALID_PTR,
			      "Null pointer passed to AddWmeInt function!");
	      return 0;
      }

      ISymbol* attribute = new gSymbol(m_agent->GetSoarAgent(), attr);
      ISymbol* value = new gSymbol(m_agent->GetSoarAgent(), intValue);

      IWme* pWme = AddWme(wmObject, attribute, value);
      attribute->Release();
      value->Release();
      return pWme;
   }

   /*
     ===============================

     ===============================
   */

   IWme* InputWorkingMemory::AddWmeDouble(IWMObject* wmObject,
                                          const char* attr,
                                          double dValue,
                                          Error* err)
   {
      ClearError(err);

      if ( wmObject == 0 || attr == 0 ) 
      {
	      SetErrorExtended(err,
			      gSKIERR_INVALID_PTR,
			      "Null pointer passed to AddWmeDouble function!");
	      return 0;
      }      
     
      ISymbol* attribute = new gSymbol(m_agent->GetSoarAgent(), attr);
      ISymbol* value = new gSymbol(m_agent->GetSoarAgent(), dValue);

      IWme* pWme = AddWme(wmObject, attribute, value);
      attribute->Release();
      value->Release();
      return pWme;
   }

   /*
     ===============================
     
     ===============================
   */
   
   IWme* InputWorkingMemory::AddWmeString(IWMObject* wmObject,
                                          const char* attr,
                                          const char* val,
                                          Error* err)
   {
      ClearError(err);

      if ( wmObject == 0 || attr == 0 || val == 0 ) 
      {
	      SetErrorExtended(err,
			      gSKIERR_INVALID_PTR,
			      "Null pointer passed to AddWmeString function!");
	      return 0;
      }      
      
      ISymbol* attribute = new gSymbol(m_agent->GetSoarAgent(), attr);
      ISymbol* value = new gSymbol(m_agent->GetSoarAgent(), val);
      
      IWme* pWme = AddWme(wmObject, attribute, value);
      attribute->Release();
      value->Release();
      return pWme;
   }

   /*
     ===============================
     
     ===============================
   */
   
   IWme* InputWorkingMemory::AddWmeObjectCopy(IWMObject*       wmObject,
                                              const char*      attr,
                                              const IWMObject* val,
                                              Error*           err)
   {

      ClearError(err);

      if ( wmObject == 0 || attr == 0 || val == 0 ) 
      {
	      SetErrorExtended(err,
			      gSKIERR_INVALID_PTR,
			      "Null pointer passed to AddWmeObjectCopy function!");
	      return 0;
      }
 
      ISymbol* attribute = new gSymbol(m_agent->GetSoarAgent(), attr);
      InputWMObject* value = CopyObjectFromInterface(val);

      IWme* pWme = AddWme(wmObject, attribute, value->GetId());
      attribute->Release();
      return pWme;
   }

   /*
     ===============================
     
     ===============================
   */

   IWme* InputWorkingMemory::AddWmeObjectLink(IWMObject* wmObject,
                                              const char* attr,
                                              IWMObject* val,
                                              Error* err)
   {
      ClearError(err);

      if ( wmObject == 0 || attr == 0 || val == 0 ) 
      {
	      SetErrorExtended(err,
			      gSKIERR_INVALID_PTR,
			      "Null pointer passed to AddWmeObjectCopy function!");
	      return 0;
      }

      ISymbol* attribute = new gSymbol(m_agent->GetSoarAgent(), attr);
      InputWMObject* value = GetOrCreateObjectFromInterface(val);      

      IWme* pWme = AddWme(wmObject, attribute, value->GetId());
      attribute->Release();
      return pWme;
   }
   
   /*
     ===============================
     
     ===============================
   */
   
   IWme* InputWorkingMemory::AddWmeNewObject(IWMObject* wmObject,
                                             const char* attr,
                                             Error* err)
   {
      ClearError(err);

      if ( wmObject == 0 || attr == 0 ) 
      {
	         SetErrorExtended(err,
                          gSKIERR_INVALID_PTR,
                          "Null pointer passed to AddWmeNewObject function!");
         return 0;
      }
      
      ISymbol* attribute = new gSymbol(m_agent->GetSoarAgent(), attr);

      InputWMObject* value = new InputWMObject(this, *attr);
      m_wmobjectmap.insert(tWMObjMap::value_type( value->GetSoarSymbol()->common.hash_id, value ));

      IWme* pWme = AddWme(wmObject, attribute, value->GetId());
      attribute->Release(); // The WME adds a reference on construction
      return pWme;
   }

   /*
     ===============================
     
     ===============================
   */

   IWme* InputWorkingMemory::ReplaceWme(IWme* oldwme,
                                        const ISymbol* newvalue,
                                        Error* err)
   {
      ClearError(err);

      if ( oldwme == 0 || newvalue == 0 ) {
         SetErrorExtended(err,
                          gSKIERR_INVALID_PTR,
                          "Null pointer passed to ReplaceWme function!");
         return 0;
      }

      // Getting a hold of the old WMObject and attribute symbol
      IWMObject* wmobj = oldwme->GetOwningObject();
      const ISymbol* attr = oldwme->GetAttribute();

      RemoveWme(oldwme);

      // Now adding the new wme
      return AddWme(wmobj, attr, newvalue);
   }

   /*
     ===============================
     
     ===============================
   */

   IWme* InputWorkingMemory::ReplaceIntWme(IWme* oldwme,               
                                           int newvalue,
                                           Error* err)
   {
      return replaceWme(oldwme, newvalue, err);
   }

   /*
     ===============================
     
     ===============================
   */

   IWme* InputWorkingMemory::ReplaceDoubleWme(IWme* oldwme,
                                              double newvalue,
                                              Error* err)
   {
      return replaceWme(oldwme, newvalue, err);
   }

   /*
     ===============================
     
     ===============================
   */
   
   IWme* InputWorkingMemory::ReplaceStringWme(IWme* oldwme,
                                              const char* newvalue,
                                              Error* err)
   {
      return replaceWme(oldwme, newvalue, err);
   }

   /*
     ===============================
     
     ===============================
   */
   
   IWme* InputWorkingMemory::ReplaceWmeObjectCopy(IWme* oldwme,
                                                  const IWMObject* newvalue,
                                                  Error* err)
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   IWme* InputWorkingMemory::ReplaceWmeObjectLink(IWme* oldwme,
                                                  const IWMObject* newvalue,
                                                  Error* err)
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================
     
     ===============================
   */

   IWme* InputWorkingMemory::ReplaceWmeNewObject(IWme* oldwme,
                                                 Error* err)
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================
     
     ===============================
   */

   void InputWorkingMemory::RemoveWme(IWme* wme,
                                      Error* err)
   {
      ClearError(err);
      if ( wme == 0 ) {
         SetErrorExtended(err,
                          gSKIERR_INVALID_PTR,
                          "Null pointer passed to RemoveWme function!");
         return;
      }

      InputWme* inwme = GetOrCreateWmeFromInterface(wme);

      inwme->Remove();
   }

   /*
     ===============================
     
     ===============================
   */

   void InputWorkingMemory::RemoveObject(IWMObject* object,
                                         Error* err)
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");
   }

   /*
     ===============================
     
     ===============================
   */

   void InputWorkingMemory::RemoveObjectWmes(IWMObject* object,
                                             Error* err)
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");
   }

   /*
     ===============================
     
     ===============================
   */

   ISymbolFactory* InputWorkingMemory::GetSymbolFactory(Error* err)
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */
   
   void InputWorkingMemory::GetObjectById(const char* idstring,
                                                IWMObject** object,
                                                Error* err) const
   {
      ClearError(err);

	  // Make sure we got enough of an id string to do the lookup
	  if (!idstring || strlen(idstring) < 2)
	  {
		 SetErrorExtended(err, 
			      gSKIERR_SYMBOL_NOT_OBJECT, 
			      "Invalid id string passed to GetObjectById function!" );
	      return ;
	  }

	  // Get the first letter in the identifier
	  char letter = idstring[0] ;

	  // Get the numeric part of the identifier (if this isn't a number we'll get back 0)
	  unsigned long value = atol(&idstring[1]) ;

	  // See if there's a symbol matching this in the kernel.
	  Symbol* id = find_identifier(GetSoarAgent(), letter, value) ;

	  if (!id)
	  {
		 SetErrorExtended(err, 
			      gSKIERR_SYMBOL_NOT_OBJECT, 
			      "Failed to find a matching identifier in GetObjectById function!" );
	      return ;
	  }

	  // See if this object already exists
	  InputWMObject* pWMObject = NULL ;
	  tWMObjMap::const_iterator iter = m_wmobjectmap.find(id->common.hash_id) ;

	  // If the object exists we return it.  Otherwise, we'll return NULL.
	  if (iter != m_wmobjectmap.end())
		  pWMObject = iter->second ;

	  // We need to add a reference to this object because the caller
	  // should release it when they're finished with it.
	  if (pWMObject)
		  pWMObject->AddRef() ;

	  *object = pWMObject ;
   }

   /*
     ===============================
     
     ===============================
   */

   tIWMObjectIterator* InputWorkingMemory::GetAllObjects(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================
     
     ===============================
   */

   tIWmeIterator* InputWorkingMemory::GetAllWmes(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }
  
   /*
     ===============================
     
     ===============================
   */

   tIWMObjectIterator* InputWorkingMemory::FindObjectsByCriteria(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================
     
     ===============================
   */

   tIWmeIterator* InputWorkingMemory::FindWmesByCriteria(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   IWMStaticView* InputWorkingMemory::CreateSubView(const IWMObject* rootobject,
                                                    Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   wme* InputWorkingMemory::GetInputWme(Symbol* id, 
                                        Symbol* attribute, 
                                        Symbol* value)
   {
      // Making sure that all the passed in pointers are non null
      if ( id == 0 || attribute == 0 || value == 0 ) 
      {
         return 0;
      }

      // Looking through the input_wmes linked list
      for (wme* curwme=id->id.input_wmes; curwme!=0; curwme=curwme->next ) 
      {
         if ( curwme->attr == attribute && 
              curwme->value == value ) 
         {
               // For safety sake
               MegaAssert( curwme->id == id, 
                  "InputWme's id doesn't match specified id!!!");
               return curwme;
         }
      }
      return 0;
   }

   wme* InputWorkingMemory::GetSlotWme(Symbol* id,
                                       Symbol* attribute,
                                       Symbol* value)
   {
      // Making sure that all the passed in pointers are non null
      if ( id == 0 || attribute == 0 || value == 0 ) {
         return 0;
      }

      // Id has to be an identifier
      if ( id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE ) return 0;

      // Looking through the slots for the wme with the 
      // appropriate id, attribute and value
      for (slot* curslot=id->id.slots; curslot != 0; curslot=curslot->next ) {
         // Looking through the wme's in this slot
         for (wme* curwme=curslot->wmes; curwme != 0; curwme=curwme->next ) {
            if ( curwme->attr == attribute &&
                 curwme->value == value ) {
               // For safety sake check also that the id's match
               MegaAssert ( curwme->id == id,
                            "SlotWme's id doesn't match specified id!" );
               return curwme;
            }
         }
      }
      
      // No matching Wme was found
      return 0;
   }   

   /*
     ===============================

     ===============================
   */
  // TODO: Remove this completely in the future
  /*
   void InputWorkingMemory::ReInitializeWMObjects()
   {
      for ( tWMObjectIt it = m_wmobjectmap.begin(); 
            it != m_wmobjectmap.end(); 
            ++it ) {
         it->second->ReInitialize();
      }
   }
  */
      
   /*
    ===============================

    ===============================
   */
   tIWMObjectIterator* InputWorkingMemory::FindObjects( InputWMObject* obj, 
                                                        const std::string& path)
   {
      return obj->GetObjectsReferencedByAttribute(path);
   }

   /*
    ===============================
    ===============================
   */
   InputWMObject* InputWorkingMemory::GetOrCreateObjectFromSymbol(const ISymbol* idsym)
   {
//      MegaAssert( false, "Not implemented yet!");

// BADBAD: For now, just create a new WM Object.  I think we should really
// be doing a lookup in the m_wmobjectmap for the wm.

	  // This code follows the logic of "GetRootInputObject"

	  // This is just a cast to a gSymbol, which is a wrapper around a kernel Symbol*.
	  gSymbol* gID = gSymbol::ConvertSymbol(idsym);

	  // Get the underlying kernel object
	  Symbol* id = gID->GetSoarSymbol() ;

	  // Create the WMObject
	  InputWMObject* pWMObject = new InputWMObject(this, id) ;

	 // This map goes from hash id for the symbol to a particular object
	 // so registering here allows us to look the object up by symbol name in the future.
	 m_wmobjectmap.insert(tWMObjMap::value_type( id->common.hash_id, pWMObject ));

	 return pWMObject;
   }

   /*
     ===============================
     ===============================
   */
   InputWme* InputWorkingMemory::GetOrCreateWmeFromInterface(IWme* wme) 
   {
      // First try to dynamic cast the object
      //InputWme* iwme = (InputWme*)(wme);
      InputWme* iwme = dynamic_cast<InputWme*>(wme);
      
      // If the dynamic cast didn't work then make a new input wme
      // if possible
      if ( iwme == 0 ) {
	      MegaAssert(false, "Not implemented yet!");
      }

      return iwme;
   }

   /*
     ===============================
     ===============================
   */
   InputWMObject* InputWorkingMemory::GetOrCreateObjectFromInterface(IWMObject* obj)
   {
      // First try to dynamic cast the object
      InputWMObject* iobj = (InputWMObject*)(obj);

      // If the dynamic cast didn't work then do a little more searching
      // using the symbol
      if ( iobj == 0 ) 
      {
         iobj = GetOrCreateObjectFromSymbol(obj->GetId());
      }

      // Checking that this InputWMObject belongs to this agent/manager
      if ( iobj != 0 ) 
      {
         if ( iobj->GetManager() != this ) 
         {
            MegaAssert( false, "This InputWMObject belongs to another agent!");
            return 0;
         }
      }

      return iobj;
   }

   /*
     ===============================
     ===============================
   */
   InputWMObject* InputWorkingMemory::CopyObjectFromInterface(const IWMObject* obj)
   {
      MegaAssert( false, "Not implemented yet!");

      return 0;
   }

   /*
     ===============================
     ===============================
   */
   void InputWorkingMemory::GetRootInputObject(InputWMObject** rootObject, Error* error)
   {
      ClearError(error);

	  // DJP: Adding a check that the gSKI input link object still
	  // corresponds to the kernel's input link object.  This should always be true
	  // unless the kernel experiences a memory leak on an init-soar.  I'm not really sure it's wise
	  // to patch things up in that case, but we'll try it and see.
	  if (m_rootInputObject)
	  {
		  Symbol* isym = GetSoarAgent()->io_header_input ;
		  if (m_rootInputObject->GetSoarSymbol() != isym)
		  {
				// The input link symbols don't match between gSKI and the kernel.
				fprintf(stderr, "gSKI and the kernel disagree now about the input link symbol--signs of a serious problem\n") ;

				// Delete the current gSKI object and create a new one based on the new input link id.
				// This could still be a problem if anyone is retaining a link to the old m_rootInputObject but in fairness that's rare.
				m_rootInputObject->Release() ;
				m_rootInputObject = 0 ;
		  }
	  }

      if ( m_rootInputObject == 0 ) {
         // Create the working memory object from the raw soar symbol
         MegaAssert( m_agent != 0, "Raw agent pointer can not be null!");

         Symbol* isym = GetSoarAgent()->io_header_input;
         MegaAssert( isym != 0, "Raw root input symbol can not be null!");

         m_rootInputObject = new InputWMObject(this, isym);

         m_wmobjectmap.insert(tWMObjMap::value_type( isym->common.hash_id, m_rootInputObject ));

         // Adding a reference before returning a pointer to the object
         // (This reference is added for the m_rootInputObject member)
         if ( m_rootInputObject != 0 ) m_rootInputObject->AddRef();

      }

      // Adding a reference before returning a pointer to the object
      // (This reference is added for the returned pointer)
      if ( m_rootInputObject != 0 ) m_rootInputObject->AddRef();

      *rootObject = m_rootInputObject;
   }
  
   /*
     ===============================
     ===============================
   */
   agent* InputWorkingMemory::GetSoarAgent() const
   {
      return m_agent->GetSoarAgent();
   }

   /*
     ===============================
     ===============================
   */
  void InputWorkingMemory::Update()
  {
    // Iterating over the InputWMObjects and updating them recursively
    // by starting with the root input object
    std::set<InputWMObject*> processedObjects;

    // If the root input object hasn't been obtained yet
    // then obtain it 
    if ( m_rootInputObject == 0 ) {
      GetRootInputObject(&m_rootInputObject);
      // Since the GetRootInputObject is meant to be used externally
      // release the extra reference that it adds to the root object
      m_rootInputObject->Release();
    }

    m_rootInputObject->Update(processedObjects);
    
  }

   /*
     ===============================
     ===============================
   */
   void InputWorkingMemory::Reinitialize()
   {
      if ( m_rootInputObject != 0 ) {
         m_rootInputObject->Release();
         m_rootInputObject = 0;
      }
      ReleaseAllWmes();
      ReleaseAllWMObjects();
      ReleaseAllSymbols();
   }

   void InputWorkingMemory::registerObjectSymbol(gSymbol* pSym)
   {
      MegaAssert(pSym->GetType() == gSKI_OBJECT, "Tried to register non-object symbol");

      std::pair<tSymMapItr, bool> r = m_symMap.insert(pSym);
      if(r.second)
      {
         pSym->AddRef();
      }
   }
   
   void InputWorkingMemory::ReleaseAllWmes() 
   {
      // Iterating over all input wmes and releasing them
      for ( tWmeMapItr it = m_wmemap.begin(); it != m_wmemap.end(); ++it ) 
      {
         it->second->Release();
      }

      // Now clearing out all the entries in the map
      m_wmemap.clear();
   }
   
   void InputWorkingMemory::ReleaseAllWMObjects()
   {
      for ( tWMObjMapItr it = m_wmobjectmap.begin(); it != m_wmobjectmap.end(); ++it ) {

         //unsigned long id = it->first;
         InputWMObject* myobj = it->second;

         // Deleting all the input producers associated with the 
         // WMO
         myobj->DeleteInputProducers();

	      myobj->Release();
      }

      m_wmobjectmap.clear();
   }

   void InputWorkingMemory::ReleaseAllSymbols() 
   {
      for ( tSymMapItr it = m_symMap.begin();
            it != m_symMap.end();
            ++it ) {
         (*it)->Release();
      }
      m_symMap.clear();
   }

    /**
    * @brief Listen for changes to wmes attached to the output link.
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param listener	The handler to call when event is fired
    */
   void InputWorkingMemory::AddWorkingMemoryListener(egSKIWorkingMemoryEventId eventId, 
							     IWorkingMemoryListener* listener, 
								 Error*               err)
   {
      ClearError(err);

	  // Nothing to listen for on the input side yet
   }

    /**
    * @brief Remove an existing listener
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param listener	The handler to call when event is fired
    */
   void InputWorkingMemory::RemoveWorkingMemoryListener(egSKIWorkingMemoryEventId eventId, 
							     IWorkingMemoryListener* listener, 
								 Error*               err)
   {
      ClearError(err);

	  // Nothing to listen for on the input side yet
   }

}
