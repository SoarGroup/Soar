#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/********************************************************************
* @file gski_symbolfactory.h 
*********************************************************************
* created:	   7/11/2002   14:54
*
* purpose: 
********************************************************************/

#include "gSKI_OutputWMObject.h"

#include "IgSKI_Iterator.h"

#include "gSKI_OutputWorkingMemory.h"
#include "gSKI_OutputWme.h"
#include "gSKI_Symbol.h"
#include "gSKI_Error.h"

#include "MegaAssert.h"

#include "symtab.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//
//DEF_EXPOSE(gSKI_OutputWMObject);

namespace gSKI 
{

   /*
     ===============================

     ===============================
   */

   OutputWMObject::OutputWMObject(OutputWorkingMemory* manager,Symbol* sym):
      m_sym(sym),
      m_gsym(0),
      m_manager(manager)
   {
      MegaAssert( m_manager != 0, "Manager for OutputWMObject cannot be null!");
      MegaAssert( sym != 0 , "Symbol for OutputWMObject cannot be null!");
      MegaAssert( sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE,
                  "Symbol associated with a OutputWMObject must be an Identifier!");

      // gSymbol will add the reference to the soar symbol but since
      // I also hold onto the symbol I'll add one too
      m_gsym = new gSymbol(manager->GetSoarAgent(), m_sym, this, false);
      m_manager->registerObjectSymbol(m_gsym);

      symbol_add_ref(m_sym);
   }

   /*
     ===============================

     ===============================
   */

   OutputWMObject::~OutputWMObject() 
   {
     symbol_remove_ref(m_manager->GetSoarAgent(),
                       m_sym);
     m_gsym->Release();
   }

   /*
     ===============================

     ===============================
   */
   const ISymbol* OutputWMObject::GetId(Error* err) const
   {
      ClearError(err);

      return m_gsym;
   }

   /*
     ===============================

     ===============================
   */

   tIWMObjectIterator* OutputWMObject::GetObjectsReferencing(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWMObjectIterator* OutputWMObject::GetObjectsReferencedBy(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWmeIterator* OutputWMObject::GetWmesReferencing(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   tIWmeIterator* OutputWMObject::GetWMEs(const char* attributeName,
                                          egSKISymbolType valueType,
                                          Error* err) const
   {
      ClearError(err);

      std::vector<IWme*> matchingWmes;

      // Iterating over the referenced objects
      for ( std::vector<OutputWme*>::const_iterator it = m_vwmes.begin();
            it != m_vwmes.end();
            ++it ) {

         OutputWme* wme = *it;
         if ((attributeName == 0 || 
              wme->AttributeEquals( attributeName ) )
              && (valueType & wme->GetValue()->GetType() ) ) {
            wme->AddRef();
            matchingWmes.push_back(wme);
         }
      
      }
    
      return new tWmeIter(matchingWmes);
   }

   /*
     ===============================

     ===============================
   */

   egSKIWMObjectType OutputWMObject::GetObjectType(Error* err) const
   {
      ClearError(err);

      return gSKI_SIMPLE_OBJECT;
   }

   /*
     ===============================

     ===============================
   */

   IState* OutputWMObject::ToState(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   bool OutputWMObject::HasBeenRemoved(Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return false;
   }

   /*
     ===============================

     ===============================
   */

   bool OutputWMObject::IsEqual(IWMObject* object, Error* err) const
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return false;
   }

   /*
     ===============================
    
     ===============================
   */
   void OutputWMObject::ReInitialize() 
   {
      m_vwmes.clear();
      m_parentmap.clear();
      m_childmap.clear();
   }

   /*
     ===============================
    
     ===============================
   */
   void OutputWMObject::AddReferencedWme(OutputWme* wme)
   {
      m_vwmes.push_back(wme);
   }

   /*
     ===============================
    
     ===============================
   */
   void OutputWMObject::AddReferencedObject(OutputWMObject* obj, OutputWme* wme)
   {
      m_childmap.insert(std::pair<OutputWme*,OutputWMObject*>(wme, obj));
   }

   /*
     ===============================
    
     ===============================
   */
   void OutputWMObject::AddReferencingObject(OutputWMObject* obj, OutputWme* wme)
   {
      m_parentmap.insert(std::pair<OutputWme*,OutputWMObject*>(wme, obj));
   }

   /*
     ===============================
    
     ===============================
   */
   tIWMObjectIterator* 
   OutputWMObject::GetObjectsReferencedByAttribute(const std::string& attr) const
   {
      std::vector<IWMObject*> matchingObjects;

      // Iterating over the referenced objects
      for ( std::map<OutputWme*,OutputWMObject*>::const_iterator it = m_childmap.begin();
            it != m_childmap.end();
            ++it ) {

         OutputWme* wme = it->first;
         if ( wme->AttributeEquals( attr )) {
            MegaAssert(it->second, "A wme should never point to a null WMO!");
            it->second->AddRef();
            matchingObjects.push_back(it->second);
         }
      
      }
    
      return new tWMObjectIter(matchingObjects);
   }

}
