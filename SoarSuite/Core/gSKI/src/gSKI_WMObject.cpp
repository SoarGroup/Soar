#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_symbolfactory.h 
*********************************************************************
* created:	   7/11/2002   14:54
*
* purpose: 
********************************************************************/

#include "gSKI_WMObject.h"

#include "IgSKI_Iterator.h"

#include "gSKI_WorkingMemory.h"
#include "gSKI_Wme.h"
#include "gSKI_Symbol.h"
#include "gSKI_Error.h"

#include "MegaAssert.h"

#include "symtab.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//
//DEF_EXPOSE(gSKI_WMObject);

namespace gSKI 
{

   /*
     ===============================

     ===============================
   */

   WMObject::WMObject(WorkingMemory* manager,Symbol* sym):
      m_gsym(0),
      m_manager(manager)
   {
      MegaAssert( m_manager != 0, "Manager for WMObject cannot be null!");
      MegaAssert( sym != 0 , "Symbol for WMObject cannot be null!");
      MegaAssert( sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE,
                  "Symbol associated with a WMObject must be an Identifier!");

      // The gSymbol will add the reference
      m_gsym = new gSymbol(manager->GetSoarAgent(), sym, this, false);
   }

   /*
     ===============================

     ===============================
   */

   WMObject::~WMObject() 
   {
     m_gsym->Release();
   }

   /*
     ===============================

     ===============================
   */
   const ISymbol* WMObject::GetId(Error* err) const
   {
      ClearError(err);

      return m_gsym;
   }

   /*
     ===============================

     ===============================
   */

   tIWMObjectIterator* WMObject::GetObjectsReferencing(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWMObjectIterator* WMObject::GetObjectsReferencedBy(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWmeIterator* WMObject::GetWmesReferencing(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWmeIterator* WMObject::GetWMEs(const char* attributeName,
                                    egSKISymbolType valueType,
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

   egSKIWMObjectType WMObject::GetObjectType(Error* err) const
   {
      ClearError(err);

      return gSKI_SIMPLE_OBJECT;
   }

   /*
     ===============================

     ===============================
   */

   IState* WMObject::ToState(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   bool WMObject::HasBeenRemoved(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return false;
   }

   /*
     ===============================

     ===============================
   */

   bool WMObject::IsEqual(IWMObject* object, Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return false;
   }

   /*
     ===============================
    
     ===============================
   */
  
   bool WMObject::Release(Error* err)
   {
      ClearError(err);
    
      MegaAssert(false, "NOT IMPLEMENTED YET!");
	  return false ;
   }

   /*
     ===============================

     ===============================
   */

   bool WMObject::IsClientOwned(Error* err) const
   {
      ClearError(err);

      return false;
   }

   /*
     ===============================
    
     ===============================
   */
   void WMObject::ReInitialize() 
   {
      m_vwmes.clear();
      m_parentmap.clear();
      m_childmap.clear();
   }

   /*
     ===============================
    
     ===============================
   */
   void WMObject::AddReferencedWme(Wme* wme)
   {
      m_vwmes.push_back(wme);
   }

   /*
     ===============================
    
     ===============================
   */
   void WMObject::AddReferencedObject(WMObject* obj, Wme* wme)
   {
      m_childmap.insert(std::pair<Wme*,WMObject*>(wme, obj));
   }

   /*
     ===============================
    
     ===============================
   */
   void WMObject::AddReferencingObject(WMObject* obj, Wme* wme)
   {
      m_parentmap.insert(std::pair<Wme*,WMObject*>(wme, obj));
   }

   /*
     ===============================
    
     ===============================
   */
   tIWMObjectIterator* 
   WMObject::GetObjectsReferencedByAttribute(const std::string& attr) const
   {
      std::vector<IWMObject*> matchingObjects;

      // Iterating over the referenced objects
      for ( std::map<Wme*,WMObject*>::const_iterator it = m_childmap.begin();
            it != m_childmap.end();
            ++it ) {

         Wme* wme = it->first;
         if ( wme->AttributeEquals( attr )) {
            matchingObjects.push_back(it->second);
         }
      
      }
    
      return new tWMObjectIter(matchingObjects);
   }

}
