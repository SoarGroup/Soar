#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_Wme.cpp          
*********************************************************************
* created:	   7/22/2002   16:06
*
* purpose: 
*********************************************************************/

#include "gSKI_Wme.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_Wme);

namespace gSKI
{
  /*
    ===============================

    ===============================
  */

   Wme::Wme(IWorkingMemory* manager, wme* wme):
      m_manager(manager)
   {
      MegaAssert( manager != 0, "Trying to create a Wme with a null manager!");
      MegaAssert( wme != 0, "Trying to create a Wme with a null soar wme!");
      m_id = wme->id;
      m_attribute = wme->attr;
      m_value = wme->value;
      m_timetag = wme->timetag;
   }
   
  /*
    ===============================

    ===============================
  */

  Wme::~Wme() {}

  /*
    ===============================

    ===============================
  */

  IWMObject* Wme::GetOwningObject(Error* err) const
  {
    ClearError(err);
    
    if ( m_owningobject != 0 ) {
       return m_owningobject;
    } else {
       // TODO: Look for it in working memory
       return 0;
    }
  }

  /*
    ===============================

    ===============================
  */

  const ISymbol* Wme::GetAttribute(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  const ISymbol* Wme::GetValue(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  long Wme::GetTimeTag(Error* err) const
  {
    ClearError(err);

    return m_timetag;
  }

  /*
    ===============================

    ===============================
  */

  egSKISupportType Wme::GetSupportType(Error * err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return gSKI_I_SUPPORT;
  }

  /*
    ===============================

    ===============================
  */

  tIProductionIterator* Wme::GetSupportProductions(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  bool Wme::HasBeenRemoved(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return false;
  }

  /*
    ===============================

    ===============================
  */

  bool Wme::IsEqual(IWme* wme, Error* err) const
  {
     ClearError(err);
     
     MegaAssert(false, "NOT IMPLEMENTED YET!");
     
     return false;
  }

  /*
    ===============================
    
    ===============================
  */
  
  bool Wme::Release(Error* err)
  {
    ClearError(err);
    
    MegaAssert(false, "NOT IMPLEMENTED YET!");
	return false ;
  }

  /*
    ===============================

    ===============================
  */

  bool Wme::IsClientOwned(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return false;
  }

  /*
    ===============================

    ===============================
  */

  void Wme::SetOwningObject(WMObject* obj)
  {
     m_owningobject = obj;
  }

  /*
    ===============================

    ===============================
  */
  bool Wme::AttributeEquals( const std::string& attr )
  {
    if ( m_attribute == 0 ) return false;
    if ( m_attribute->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE ) 
      return false;
    
    return !strcmp(attr.c_str(), m_attribute->sc.name);
    
  }

}
