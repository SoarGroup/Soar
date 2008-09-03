#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_WorkingMemory.cpp 
*********************************************************************
* created:	   7/22/2002   16:28
*
* purpose: 
*********************************************************************/

#include "gSKI_WorkingMemory.h"
#include "gSKI_WMObject.h"
#include "gSKI_Wme.h"
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
//DEF_EXPOSE(gSKI_WorkingMemory);

namespace gSKI
{
  /*
    ===============================

    ===============================
  */

   WorkingMemory::WorkingMemory(Agent* agent):
     m_agent(agent),
     m_rootOutputObject(0)
   {}

  /*
    ===============================

    ===============================
  */

  WorkingMemory::~WorkingMemory() {}

  /*
    ===============================

    ===============================
  */

  Agent* WorkingMemory::GetAgent(Error * err) const
  {
    ClearError(err);

    return m_agent;
  }

  /*
    ===============================

    ===============================
  */

  IWme* WorkingMemory::AddWme(const ISymbol* idSymbol, 
                              const ISymbol* attrSymbol,
                              const ISymbol* valSymbol,
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

  IWme* WorkingMemory::AddWmeInt(IWMObject* wmObject,
                                 const char* attr,
                                 int intValue,
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

  IWme* WorkingMemory::AddWmeDouble(IWMObject* wmObject,
                                    const char* attr,
                                    double dValue,
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

  IWme* WorkingMemory::AddWmeString(IWMObject* wmObject,
                                    const char* attr,
                                    const char* value,
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

  IWme* WorkingMemory::AddWmeObjectCopy(IWMObject*       wmObject,
                                        const char*      attr,
                                        const IWMObject* value,
                                        Error*           err)
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  IWme* WorkingMemory::AddWmeObjectLink(IWMObject* wmObject,
                                        const char* attr,
                                        IWMObject* value,
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

  IWme* WorkingMemory::AddWmeNewObject(IWMObject* wmObject,
                                       const char* attr,
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

  IWme* WorkingMemory::ReplaceWme(IWme* oldwme,
                                  const ISymbol* newvalue,
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

  IWme* WorkingMemory::ReplaceIntWme(IWme* oldwme,
                                     int newvalue,
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

  IWme* WorkingMemory::ReplaceDoubleWme(IWme* oldwme,
                                        double newvalue,
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

  IWme* WorkingMemory::ReplaceStringWme(IWme* oldwme,
                                        const char* newvalue,
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

  IWme* WorkingMemory::ReplaceWmeObjectCopy(IWme* oldwme,
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

  IWme* WorkingMemory::ReplaceWmeObjectLink(IWme* oldwme,
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

  IWme* WorkingMemory::ReplaceWmeNewObject(IWme* oldwme,
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

  void WorkingMemory::RemoveWme(IWme* wme,
                                Error* err)
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");
  }

  /*
    ===============================

    ===============================
  */

  void WorkingMemory::RemoveObject(IWMObject* object,
                                   Error* err)
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");
  }

  /*
    ===============================

    ===============================
  */

  void WorkingMemory::RemoveObjectWmes(IWMObject* object,
                                       Error* err)
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");
  }

  /*
    ===============================

    ===============================
  */

  ISymbolFactory* WorkingMemory::GetSymbolFactory(Error* err)
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  void WorkingMemory::GetObjectById(const char* idstring,
                                    IWMObject** object,
                                    Error* err) const
  {
     ClearError(err);

     MegaAssert(false, "NOT IMPLEMENTED YET!");

     object = 0;
  }

  /*
    ===============================

    ===============================
  */

  tIWMObjectIterator* WorkingMemory::GetAllObjects(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  tIWmeIterator* WorkingMemory::GetAllWmes(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }
  
  /*
    ===============================

    ===============================
  */

  tIWMObjectIterator* WorkingMemory::FindObjectsByCriteria(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  tIWmeIterator* WorkingMemory::FindWmesByCriteria(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */
  
  IWMStaticView* WorkingMemory::CreateSubView(const IWMObject* rootobject,
                               Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

   void WorkingMemory::UpdateWithIOWmes(io_wme* wmelist)
   {
      // Create WMObjects and Wmes for all the wmes that don't already exist

      // First remove all the connections from last time (these may no 
      // longer be valid
      ReInitializeWMObjects();

      // These sets are used to hold the ids of Wmes and Objects that have
      // been processed (from the io wmelist)
      std::set<unsigned long> objectids;
      std::set<unsigned long> wmeids;

      for (io_wme* curiowme=wmelist; curiowme!=0; curiowme=curiowme->next) {

	 // Getting the real raw input wme from the soar kernel
	 wme* rawwme = GetInputWme( curiowme->id, 
				    curiowme->attr, 
				    curiowme->value );
         
         // If it wasn't found as an input wme then look in the slots
         if ( rawwme == 0 ) {
            rawwme = GetSlotWme( curiowme->id,
                                 curiowme->attr,
                                 curiowme->value );
         }
	 MegaAssert( rawwme != 0, "IO wme not found in kernel!");

	 // Getting the unique id's for id symbol and the wme and
	 // inserting them in a set to keep track of processed 
	 // wmobjects and wmes
	 unsigned long oid = curiowme->id->common.hash_id;
	 unsigned long wid = rawwme->timetag;
	 objectids.insert(oid);
	 wmeids.insert(wid);

	 // TODO: Properly handle cases where the attribute is an
	 // identifier symbol. This may require modifications to the
	 // interface?

	 // Getting or making the various WMObjects that 
	 WMObject* idObject = GetOrCreateWMObject(curiowme->id);
	 // WMObject* attrObject = GetOrCreateWMObject(curiowme->attr);
	 WMObject* valObject = GetOrCreateWMObject(curiowme->value);
	 MegaAssert( idObject != 0, 
		     "The ID of an io wme must map to a WMObject!");

	 // Getting or making the necessary Wme object and setting its
	 // parent object
	 Wme* wme = GetOrCreateWme(rawwme);
	 MegaAssert( wme != 0,
		     "IO wmes must be mappable to Wme objects!");
	 
	 wme->SetOwningObject(idObject);
	 idObject->AddReferencedWme(wme);

	 // Setting up the referencing and referenced links for the object
	 if ( valObject != 0 ) {
	    idObject->AddReferencedObject(valObject, wme);
	    valObject->AddReferencingObject(idObject, wme);
	 }

      }

      // Cleaning up old Wmes and WMObjects that no longer exist
      for ( tWMObjectIt it = m_wmobjectmap.begin(); 
            it != m_wmobjectmap.end(); 
            ++it ) {
	unsigned long objectid = it->first;
	// If the id doesn't exist in the set of object ids then 
        // this element should be removed from the map and the 
        // corresponding WMObject deleted
	if ( objectids.find(objectid) == objectids.end() ) {
	  WMObject* obj = it->second;
	  m_wmobjectmap.erase(it);
	  delete obj;
	}
      }

      for ( tWmeIt it = m_wmemap.begin(); it != m_wmemap.end(); ++it ) {
	unsigned long wmeid = it->first;
        // If this wme id doesn't exist in the wme map then
        // this wme should be removed from the map and the
        // corresponding WMObject deleted
	if ( wmeids.find(wmeid) == wmeids.end() ) {
	  Wme* wme = it->second;
	  m_wmemap.erase(it);
	  delete wme;
	}
      }

   }

  /*
    ===============================

    ===============================
  */

   wme* WorkingMemory::GetInputWme(Symbol* id, 
				   Symbol* attribute, 
				   Symbol* value)
   {
      // Making sure that all the passed in pointers are non null
      if ( id == 0 || attribute == 0 || value == 0 ) {
	 return 0;
      }

      // Looking through the input_wmes linked list
      for (wme* curwme=id->id.input_wmes; curwme!=0; curwme=curwme->next ) {
	 if ( curwme->attr == attribute && 
	      curwme->value == value ) {
	    // For safety sake
	    MegaAssert( curwme->id == id, 
			"InputWme's id doesn't match specified id!!!");
	    return curwme;
	 }
      }
      return 0;
   }

   /*
     ===============================
     
     ===============================
   */
   
   wme* WorkingMemory::GetSlotWme(Symbol* id,
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

   WMObject* WorkingMemory::GetOrCreateWMObject(Symbol* idSymbol)
   {
      // Return a null pointer if one is passed
      if ( idSymbol == 0 ) return 0;

      // Check to make sure that symbol is the correct type
      if ( idSymbol->common.symbol_type != IDENTIFIER_SYMBOL_TYPE ) return 0;

      WMObject* obj;
      unsigned long oid = idSymbol->common.hash_id;
      tWMObjectIt objectIt = m_wmobjectmap.find(oid);
      if ( objectIt == m_wmobjectmap.end() ) {
	 // Object hasn't been previously created then make it
	 obj = new WMObject(this, idSymbol);
	 m_wmobjectmap.insert(std::pair<unsigned long, WMObject*>(oid, obj));
      } else {
	 obj = objectIt->second;
      }

      return obj;
   }

  /*
    ===============================

    ===============================
  */
      
  Wme* WorkingMemory::GetOrCreateWme(wme* rawWme) 
  {
     // Returning a null pointer if one is passed in
     if ( rawWme == 0 ) return 0;

     Wme* wme = 0;
     unsigned long wid = rawWme->timetag;
     
     tWmeIt wmeIt = m_wmemap.find(wid);
     if ( wmeIt == m_wmemap.end() ) {
       // Wme hasn't been previously created then make it
       wme = new Wme( this, rawWme );
       m_wmemap.insert(std::pair<unsigned long, Wme* >(wid, wme));
     } else {
       wme = wmeIt->second;
     }

     return wme;
  }

  /*
    ===============================

    ===============================
  */
  void WorkingMemory::ReInitializeWMObjects()
  {
     for ( tWMObjectIt it = m_wmobjectmap.begin(); 
	   it != m_wmobjectmap.end(); 
	   ++it ) {
	it->second->ReInitialize();
     }
  }
      
  /*
    ===============================

    ===============================
  */
  tIWMObjectIterator* WorkingMemory::FindObjects( WMObject* obj, 
                                                  const std::string& path)
  {
    return obj->GetObjectsReferencedByAttribute(path);
  }

  /*
    ===============================

    ===============================
  */
  void WorkingMemory::GetOutputRootObject(WMObject** rootObject)
  {
    if ( m_rootOutputObject == 0 ) {

      // Get the raw symbol from the agent structure and 
      // create a WMObject for it
      m_rootOutputObject = 
        GetOrCreateWMObject(m_agent->GetSoarAgent()->io_header_output);

      MegaAssert( m_rootOutputObject != 0, 
                  "There is no root output link object!");
    }
      
    *rootObject = m_rootOutputObject;
  }

  /*
    ===============================
    ===============================
  */
  agent* WorkingMemory::GetSoarAgent()
  {
    return m_agent->GetSoarAgent();
  }

  /*
    ===============================
    ===============================
  */
  void WorkingMemory::Reinitialize()
  {
     ReleaseAllWmes();
     ReleaseAllWMObjects();
  }

  /*
    ===============================
    ===============================
  */
   void WorkingMemory::ReleaseAllWmes()
   {
      for ( tWmeIt it = m_wmemap.begin(); it != m_wmemap.end(); ++it ) {
	 it->second->Release();
      }
      m_wmemap.clear();
   }
   
   /*
     ===============================
     ===============================
   */
   void WorkingMemory::ReleaseAllWMObjects()
   {
      for ( tWMObjectIt it = m_wmobjectmap.begin(); it != m_wmobjectmap.end();
	    ++it ) {
	 it->second->Release();
      }
      m_wmobjectmap.clear();
   }

       /**
    * @brief Listen for changes to wmes attached to the output link.
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param listener	The handler to call when event is fired
    */
   void WorkingMemory::AddWorkingMemoryListener(egSKIWorkingMemoryEventId eventId, 
							     IWorkingMemoryListener* listener, 
								 Error*               err)
   {
      ClearError(err);

	  // Nothing to listen for yet.
   }

    /**
    * @brief Remove an existing listener
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param listener	The handler to call when event is fired
    */
   void WorkingMemory::RemoveWorkingMemoryListener(egSKIWorkingMemoryEventId eventId, 
							     IWorkingMemoryListener* listener, 
								 Error*               err)
   {
      ClearError(err);

	  // Nothing to listen for yet.
   }
}
