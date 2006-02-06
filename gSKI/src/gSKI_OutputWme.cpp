#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_OutputWme.cpp          
*********************************************************************
* created:	   7/22/2002   16:06
*
* purpose: 
*********************************************************************/

#include "gSKI_OutputWorkingMemory.h"
#include "gSKI_OutputWme.h"
#include "gSKI_Symbol.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_OutputWme);

namespace gSKI
{
   OutputWme::OutputWme(OutputWorkingMemory* manager, wme* wme, IWMObject* valobj):
     m_manager(manager),
     m_owningobject(0),
     m_gattr(0),
     m_gvalue(0),
     m_timetag(0)
   {
      MegaAssert( manager != 0, "Trying to create a OutputWme with a null manager!");
      MegaAssert( wme != 0, "Trying to create a OutputWme with a null soar wme!");

      // TODO: This may cause problems in cases where the attribute is an identifier symbol
      MegaAssert(wme->attr->sc.common_symbol_info.symbol_type != IDENTIFIER_SYMBOL_TYPE, 
                 "System does not yet support object attributes (specify a string)");

      if(wme->attr->sc.common_symbol_info.symbol_type != IDENTIFIER_SYMBOL_TYPE)
      {
         m_gattr = new gSymbol(m_manager->GetSoarAgent(), wme->attr, 0, false);
      }
      else
      {
         m_gattr = new gSymbol(m_manager->GetSoarAgent(), "error_attribute_cannot_be_object");
      }

      if(valobj)
      {
         ((OutputWMObject*)valobj)->AddRef();
      }
      m_gvalue = new gSymbol(m_manager->GetSoarAgent(), wme->value, valobj, true);
      if(valobj)
      {
         m_manager->registerObjectSymbol(m_gvalue);
      }

      m_timetag = wme->timetag;

      MegaAssert( m_gattr != 0, "Failed to create an ISymbol for the wme attribute!");
      MegaAssert( m_gvalue != 0, "Failed to create an ISymbol for the wme value!");
   }

   OutputWme::OutputWme(OutputWorkingMemory* manager, 
                        OutputWMObject* wmobj,
                        gSymbol* attr,
                        gSymbol* value):
      m_manager(manager),
      m_owningobject(0),
      m_gattr(attr),
      m_gvalue(value),
      m_timetag(0)
   {
      MegaAssert( manager != 0, "Trying to create a OutputWme with a null manager!");
      MegaAssert( wmobj != 0, "Trying to create a OutputWme with a null WMObject!");
      MegaAssert( attr != 0, "Trying to create a OutputWme with a null attribute!");
      MegaAssert( value != 0, "Trying to create a OutputWme with a null value!");

      // TODO: Adding the wme can only be done during certain points of the cycle
      // this is a quick fix so that Jens' won't be held up.
      
      m_gattr->AddRef();
      m_gvalue->AddRef();

      // Quickly adding the wme to working memory
      agent* a = m_manager->GetSoarAgent();
      Symbol* id = wmobj->GetSoarSymbol();
      Symbol* att = m_gattr->GetSoarSymbol();
      Symbol* val = m_gvalue->GetSoarSymbol();
      wme* rawwme = add_input_wme( a,
                                   id,
                                   att,
                                   val );
      m_timetag = rawwme->timetag;
      MegaAssert( rawwme != 0, "Trouble adding a new wme to the output link!");

   }
                        
   OutputWme::~OutputWme() 
   {
      if ( m_gattr ) 
      {
         m_gattr->Release();
      }
      if ( m_gvalue ) 
      {
         m_gvalue->Release();
      }
   }

   IWMObject* OutputWme::GetOwningObject(Error* err) const
   {
      ClearError(err);
       
      if ( m_owningobject != 0 ) 
      {
         return m_owningobject;
      } 
      else 
      {
         // TODO: Look for it in working memory
         return 0;
      }
   }

   const ISymbol* OutputWme::GetAttribute(Error* err) const
   {
      ClearError(err);
      return m_gattr;
   }

   const ISymbol* OutputWme::GetValue(Error* err) const
   {
      ClearError(err);
      return m_gvalue;
   }

   long OutputWme::GetTimeTag(Error* err) const
   {
      ClearError(err);
      return m_timetag;
   }

   egSKISupportType OutputWme::GetSupportType(Error * err) const
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return gSKI_I_SUPPORT;
   }

   tIProductionIterator* OutputWme::GetSupportProductions(Error* err) const
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return 0;
   }

   bool OutputWme::HasBeenRemoved(Error* err) const
   {
      ClearError(err);
      //MegaAssert(false, "NOT IMPLEMENTED YET!");
      return false;
   }

   bool OutputWme::IsEqual(IWme* wme, Error* err) const
   {
      ClearError(err);
      MegaAssert(false, "NOT IMPLEMENTED YET!");
      return false;
   }

   void OutputWme::SetOwningObject(OutputWMObject* obj)
   {
      m_owningobject = obj;
   }

   bool OutputWme::AttributeEquals( const std::string& attr )
   {
      if ( m_gattr == 0 ) 
      {
         return false;
      }
      if ( m_gattr->GetType() != gSKI_STRING ) 
      {
         return false;
      }
         
      return attr == m_gattr->GetString();
   }

}
