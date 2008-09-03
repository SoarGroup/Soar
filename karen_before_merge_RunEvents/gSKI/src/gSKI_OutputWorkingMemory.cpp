#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_OutputWorkingMemory.cpp 
*********************************************************************
* created:	   7/22/2002   16:28
*
* purpose: 
*********************************************************************/

#include "gSKI_OutputWorkingMemory.h"
#include "gSKI_OutputWMObject.h"
#include "gSKI_OutputWme.h"
#include "gSKI_Error.h"
#include "gSKI_Agent.h"
#include "gSKI_Symbol.h"

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
//DEF_EXPOSE(gSKI_OutputWorkingMemory);

namespace gSKI
{

   OutputWorkingMemory::OutputWorkingMemory(Agent* agent):
      m_agent(agent),
      m_rootOutputObject(0)
   {
   }

   OutputWorkingMemory::~OutputWorkingMemory() 
   {
      ReleaseAllWmes();
      ReleaseAllWMObjects();
      ReleaseAllSymbols();
      if(m_rootOutputObject)
      {
         m_rootOutputObject->Release();
      }
   }

   IAgent* OutputWorkingMemory::GetAgent(Error * err) const
   {
      ClearError(err);
      return m_agent;
   }

   IWme* OutputWorkingMemory::AddWme(IWMObject* wmobject, 
                                       const ISymbol* attrSymbol,
                                       const ISymbol* valSymbol)
   {
      if ( wmobject == 0 || attrSymbol == 0 || valSymbol == 0 )
      {
         MegaAssert(false,"Null pointer passed to AddWme function!");
         return 0;
      }
      
      OutputWMObject* iobj = GetOrCreateObjectFromInterface(wmobject);
      gSymbol* attr = gSymbol::ConvertSymbol(attrSymbol);
      gSymbol* val = gSymbol::ConvertSymbol(valSymbol);

      OutputWme* wme = new OutputWme(this, iobj, attr, val);
      m_wmemap.insert(tWmeMap::value_type(wme->GetTimeTag(), wme));
      iobj->AddReferencedWme(wme);

	  // DJP -- it appears that the ref-counting between input wmes and output wmes differs in some manner.
	  // I've not figured out exactly how, but without adding an additional reference to this wme we will
	  // get a crash if we (a) store the returned wme, (b) have the agent delete the wme during a run and then
	  // (c) issue an init-soar to reinitialize at which point we try to release our copy of this wme and find it
	  // already gone.  We don't have this problem on the input link so I believe the issue is that we're somehow
	  // off by one on the ref-counting over here.
	  wme->AddRef() ;

      return wme;
   }

   IWme* OutputWorkingMemory::AddWme(const ISymbol* idSymbol, 
                                       const ISymbol* attrSymbol,
                                       const ISymbol* valSymbol,
                                       Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::AddWmeInt(IWMObject* wmObject,
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

   IWme* OutputWorkingMemory::AddWmeDouble(IWMObject* wmObject,
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

   IWme* OutputWorkingMemory::AddWmeString(IWMObject* wmObject,
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

   IWme* OutputWorkingMemory::AddWmeObjectCopy(IWMObject*       wmObject,
                                          const char*      attr,
                                          const IWMObject* value,
                                          Error*           err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::AddWmeObjectLink(IWMObject* wmObject,
                                          const char* attr,
                                               IWMObject* value,
                                          Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::AddWmeNewObject(IWMObject* wmObject,
                                          const char* attr,
                                          Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::ReplaceWme(IWme* oldwme,
                                          const ISymbol* newvalue,
                                          Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::ReplaceIntWme(IWme* oldwme,
                                             int newvalue,
                                             Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::ReplaceDoubleWme(IWme* oldwme,
                                                double newvalue,
                                                Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::ReplaceStringWme(IWme* oldwme,
                                                const char* newvalue,
                                                Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::ReplaceWmeObjectCopy(IWme* oldwme,
                                                   const IWMObject* newvalue,
                                                   Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::ReplaceWmeObjectLink(IWme* oldwme,
                                                   const IWMObject* newvalue,
                                                   Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWme* OutputWorkingMemory::ReplaceWmeNewObject(IWme* oldwme,
                                                   Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   void OutputWorkingMemory::RemoveWme(IWme* wme,
                                       Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
   }

   void OutputWorkingMemory::RemoveObject(IWMObject* object,
                                    Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
   }

   void OutputWorkingMemory::RemoveObjectWmes(IWMObject* object,
                                          Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
   }

   ISymbolFactory* OutputWorkingMemory::GetSymbolFactory(Error* err)
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   void OutputWorkingMemory::GetObjectById(const char* idstring,
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
	  OutputWMObject* pWMObject = NULL ;
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

   tIWMObjectIterator* OutputWorkingMemory::GetAllObjects(Error* err) const
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   tIWmeIterator* OutputWorkingMemory::GetAllWmes(Error* err) const
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0 ;
   }
   
   tIWMObjectIterator* OutputWorkingMemory::FindObjectsByCriteria(Error* err) const
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   tIWmeIterator* OutputWorkingMemory::FindWmesByCriteria(Error* err) const
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   IWMStaticView* OutputWorkingMemory::CreateSubView(const IWMObject* rootobject,
                                 Error* err) const
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

	/**
    * @brief Notify any listeners about a change to working memory.
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param change		The type of change that just occured (e.g. addition, modification, removal)
	* @param io_wmelist	The new list of wmes on the output link
    */
   void OutputWorkingMemory::NotifyWorkingMemoryListeners(egSKIWorkingMemoryEventId eventId, egSKIWorkingMemoryChange change, io_wme* io_wmelist)
   {
   	  // Notify any listeners about this event
	  if (m_workingMemoryListeners.GetNumListeners(eventId) != 0)
	  {
		  // After the update calls we have a list of the current set of i/o objects
		  // in m_wmobjectmap and m_wmemap of OutputWorkingMemory.
		  // We want to send that list to the listeners.
		  // It seems we ought to just wrap an iterator around the m_wmemap but I don't see any examples
		  // of doing that and all my attempts so far to do it have failed, so instead we'll create a vector
		  // and return an iterator over it.

		  // NOTE: It's important that the order of the elements in this vector be such that parent WMEs
		  // are created before children.  I.E. We need to have (I3 ^plane P1) before (P1 ^speed 10).
		  // (Without this, the client side will have a hard time re-creating the output link).
		  // This is currently guaranteed to occur because m_wmemap is a map based on timestamps, so the iteration
		  // through the map will be in timestamp order, which in turn ensures the relationship we need.
		  std::vector<IWme*> wmes ;

		  for (tWmeMapItr iter = m_wmemap.begin() ; iter != m_wmemap.end() ; iter++)
		  {
			  wmes.push_back(iter->second) ;
		  }

		  // Creates a gSKI iterator which will not release the objects in its list when it is deleted.
		  // This means that the listener should addRef() any objects they choose to keep from within the list.
		  tWmeIter wmelist(wmes) ;

		  // Fire the event
          WorkingMemoryNotifier wmn(m_agent, change, &wmelist);
	      m_workingMemoryListeners.Notify(eventId, wmn);
	  }
   }

    /**
    * @brief Listen for changes to wmes attached to the output link.
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param listener	The handler to call when event is fired
    */
   void OutputWorkingMemory::AddWorkingMemoryListener(egSKIWorkingMemoryEventId eventId, 
							     IWorkingMemoryListener* listener, 
								 Error*               err)
   {
      ClearError(err);
      m_workingMemoryListeners.AddListener(eventId, listener);

   }

    /**
    * @brief Remove an existing listener
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param listener	The handler to call when event is fired
    */
   void OutputWorkingMemory::RemoveWorkingMemoryListener(egSKIWorkingMemoryEventId eventId, 
							     IWorkingMemoryListener* listener, 
								 Error*               err)
   {
      ClearError(err);
      m_workingMemoryListeners.RemoveListener(eventId, listener);
   }

   void OutputWorkingMemory::UpdateWithIOWmes(io_wme* wmelist)
   {
      // Create WMObjects and Wmes for all the wmes that don't already exist

      // First remove all the connections from last time (these may no 
      // longer be valid
      ReInitializeWMObjects();

      // These sets are used to hold the ids of Wmes and Objects that have
      // been processed (from the io wmelist)
      std::set<unsigned long> objectids;
      std::set<unsigned long> wmeids;

      for (io_wme* curiowme=wmelist; curiowme!=0; curiowme=curiowme->next)
      {
         // Getting the real raw input wme from the soar kernel
         wme* rawwme = GetOutputWme(curiowme->id, curiowme->attr, curiowme->value);

         // If it wasn't found as an input wme then look in the slots
         if ( rawwme == 0 ) 
         {
            rawwme = GetSlotWme( curiowme->id, curiowme->attr, curiowme->value);
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
         OutputWMObject* idObject = GetOrCreateWMObject(curiowme->id);
         // OutputWMObject* attrObject = GetOrCreateWMObject(curiowme->attr);
         OutputWMObject* valObject = GetOrCreateWMObject(curiowme->value);
         MegaAssert(idObject != 0, 
                    "The ID of an io wme must map to a OutputWMObject!");

         // Getting or making the necessary Wme object and setting its
         // parent object
         OutputWme* wme = GetOrCreateWme(rawwme, valObject);
         MegaAssert( wme != 0,
                     "IO wmes must be mappable to OutputWme objects!");

         wme->SetOwningObject(idObject);
         idObject->AddReferencedWme(wme);

         // Setting up the referencing and referenced links for the object
         if ( valObject != 0 ) 
         {
            idObject->AddReferencedObject(valObject, wme);
            valObject->AddReferencingObject(idObject, wme);
            // Inserting the value wm object into the set of currently
            // used object ids
            objectids.insert(curiowme->value->common.hash_id);
         }
      }

      // Cleaning up old Wmes and WMObjects that no longer exist
      for ( tWMObjMapItr it = m_wmobjectmap.begin(); it != m_wmobjectmap.end(); 
            /* erasure loop don't increment */ ) 
      {
         unsigned long objectid = it->first;
         // If the id doesn't exist in the set of object ids then 
         // this element should be removed from the map and the 
         // corresponding WMObject deleted
         if ( objectids.find(objectid) == objectids.end() ) 
         {
            OutputWMObject* obj = it->second;
            m_wmobjectmap.erase(it++);
            obj->Release();
         }
         else 
         {
            ++it;
         }
      }

      for ( tWmeMapItr it = m_wmemap.begin(); it != m_wmemap.end(); 
            /* erasure loop, don't increment */ ) 
      {
         unsigned long wmeid = it->first;
         // If this wme id doesn't exist in the wme map then
         // this wme should be removed from the map and the
         // corresponding WMObject deleted
         if ( wmeids.find(wmeid) == wmeids.end() ) 
         {
            OutputWme* wme = it->second;
            m_wmemap.erase(it++);
            wme->Release();
         }
         else 
         {
            ++it;
         }
      }

   }

   wme* OutputWorkingMemory::GetOutputWme(Symbol* id, 
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
         if ( curwme->attr == attribute && curwme->value == value ) 
         {
            // For safety sake
            MegaAssert( curwme->id == id, 
                        "OutputWme's id doesn't match specified id!!!");
            return curwme;
         }
      }
      return 0;
   }

   wme* OutputWorkingMemory::GetSlotWme(Symbol* id,
                                  Symbol* attribute,
                                  Symbol* value)
   {
      // Making sure that all the passed in pointers are non null
      if ( id == 0 || attribute == 0 || value == 0 ) 
      {
         return 0;
      }

      // Id has to be an identifier
      if ( id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE ) return 0;

      // Looking through the slots for the wme with the 
      // appropriate id, attribute and value
      for (slot* curslot=id->id.slots; curslot != 0; curslot=curslot->next ) 
      {
         // Looking through the wme's in this slot
         for (wme* curwme=curslot->wmes; curwme != 0; curwme=curwme->next ) 
         {
            if ( curwme->attr == attribute && curwme->value == value ) 
            {
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
   
   OutputWMObject* OutputWorkingMemory::GetOrCreateWMObject(Symbol* idSymbol)
   {
      // Return a null pointer if one is passed
      if ( idSymbol == 0 ) return 0;

      // Check to make sure that symbol is the correct type
      if ( idSymbol->common.symbol_type != IDENTIFIER_SYMBOL_TYPE ) return 0;

      OutputWMObject* obj;
      unsigned long oid = idSymbol->common.hash_id;
      tWMObjMapItr objectIt = m_wmobjectmap.find(oid);
      if ( objectIt == m_wmobjectmap.end() ) 
      {
	      // Object hasn't been previously created then make it
	      obj = new OutputWMObject(this, idSymbol);
	      m_wmobjectmap.insert(std::pair<unsigned long, OutputWMObject*>(oid, obj));
      } 
      else 
      {
	      obj = objectIt->second;
      }
      return obj;
   }

   OutputWme* OutputWorkingMemory::GetOrCreateWme(wme* rawWme, 
                                                   IWMObject* valobj) 
   {
      // Returning a null pointer if one is passed in
      if ( rawWme == 0 ) return 0;

      OutputWme* wme = 0;
      unsigned long wid = rawWme->timetag;
      
      tWmeMapItr wmeIt = m_wmemap.find(wid);
      if ( wmeIt == m_wmemap.end() ) 
      {
         // Wme hasn't been previously created then make it
         wme = new OutputWme( this, rawWme, valobj );
         m_wmemap.insert(tWmeMap::value_type(wid, wme));
      } 
      else 
      {
         wme = wmeIt->second;
      }

      return wme;
   }

   void OutputWorkingMemory::ReInitializeWMObjects()
   {
      for ( tWMObjMapItr it = m_wmobjectmap.begin(); it != m_wmobjectmap.end(); ++it ) 
      {
            it->second->ReInitialize();
      }
   }
      
   tIWMObjectIterator* OutputWorkingMemory::FindObjects( OutputWMObject* obj, 
                                                   const std::string& path)
   {
      return obj->GetObjectsReferencedByAttribute(path);
   }

   void OutputWorkingMemory::GetOutputRootObject(OutputWMObject** rootObject)
   {
      if ( m_rootOutputObject == 0 ) 
      {
         // Get the raw symbol from the agent structure and 
         // create a OutputWMObject for it
         m_rootOutputObject = 
            GetOrCreateWMObject(m_agent->GetSoarAgent()->io_header_output);

         MegaAssert( m_rootOutputObject != 0, 
            "There is no root output link object!");

         // add a reference because m_rootOutputObject is in the object
         // list as well.
         if(m_rootOutputObject)
         {
            m_rootOutputObject->AddRef();
         }
      }

      // Another reference is added for the returned pointer value
      if ( m_rootOutputObject != 0 ) m_rootOutputObject->AddRef();

      *rootObject = m_rootOutputObject;
   }

   agent* OutputWorkingMemory::GetSoarAgent() const
   {
      return m_agent->GetSoarAgent();
   }

   void OutputWorkingMemory::Reinitialize()
   {
      if ( m_rootOutputObject != 0 ) 
      {
         m_rootOutputObject->Release();
         m_rootOutputObject = 0;
      }

      ReleaseAllWmes();
      ReleaseAllWMObjects();
      ReleaseAllSymbols();
   }

   void OutputWorkingMemory::registerObjectSymbol(gSymbol* pSym)
   {
      MegaAssert(pSym->GetType() == gSKI_OBJECT, "Tried to register non-object symbol");

      std::pair<tSymSetItr, bool> r = m_symSet.insert(pSym);
      if(r.second)
      {
         pSym->AddRef();
      }
   }

   void OutputWorkingMemory::ReleaseAllSymbols() {
      for(tSymSetItr it = m_symSet.begin(); it != m_symSet.end(); ++it)
      {
         gSymbol* pSym = *it;
         pSym->Release();
      }
      m_symSet.clear();
   }

   void OutputWorkingMemory::ReleaseAllWmes()
   {
      for ( tWmeMapItr it = m_wmemap.begin(); it != m_wmemap.end(); ++it ) 
      {
	      it->second->Release();
      }
      m_wmemap.clear();
   }
   
   /*
     ===============================
     ===============================
   */
   void OutputWorkingMemory::ReleaseAllWMObjects()
   {
      for ( tWMObjMapItr it = m_wmobjectmap.begin(); it != m_wmobjectmap.end();
	         ++it ) 
      {
	      it->second->Release();
      }
      m_wmobjectmap.clear();
   }

   /*
     ===============================
     ===============================
   */
   OutputWMObject* OutputWorkingMemory::GetOrCreateObjectFromInterface(IWMObject* obj)
   {
      // First try to dynamic cast the object
      OutputWMObject* iobj = (OutputWMObject*)(obj);

      // If the dynamic cast didn't work then do a little more searching
      // using the symbol
      if ( iobj == 0 ) 
      {
         //iobj = GetOrCreateObjectFromSymbol(obj->GetId());
      }

      // Checking that this OutputWMObect belongs to this agent/manager
      if ( iobj != 0 ) 
      {
	      if ( iobj->GetManager() != this ) 
         {
	         MegaAssert( false, "This OutputWMObject belongs to another agent!");
	         return 0;
	      }
      }
      
      return iobj;
   }
}
