#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_InputWme.cpp          
*********************************************************************
* created:	   7/22/2002   16:06
*
* purpose: 
*********************************************************************/

#include "gSKI_InputWme.h"

#include <vector>

#include "io.h"

#include "gSKI_InputWMObject.h"
#include "gSKI_InputWorkingMemory.h"
#include "IgSKI_Production.h"
#include "gSKI_Agent.h"
#include "gSKI_Symbol.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"

#ifdef DEBUG_UPDATE
#include "..\..\ConnectionSML\include\sock_Debug.h"	// For PrintDebugFormat
#endif

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"


//DEF_EXPOSE(gSKI_InputWme);

namespace gSKI
{
   InputWme::InputWme( InputWorkingMemory* manager, 
                       InputWMObject* iobj,
                       gSymbol* attribute,
                       gSymbol* value ):
     m_manager(manager),
     m_owningobject(iobj),
     m_attribute(attribute),
     m_value(value),
     m_rawwme(0),
     m_removeWme(false)
   {

     MegaAssert( m_manager != 0, "The manager of this input wme cannot be null!");
     MegaAssert( m_owningobject != 0, "The owning object of this wme cannot be null!");
     MegaAssert( attribute != 0, "The attribute of this wme cannot be null!");
     MegaAssert( value != 0, "The value of this wme cannot be null!");

     gSymbol::ConvertSymbol(m_attribute)->AddRef();
     gSymbol::ConvertSymbol(m_value)->AddRef();

     // No reference is added to the owning object to avoid a circular reference.

     // 09/26/05 KJC:  commenting this in constructor so that updates can happen
	 // ONLY from InputPhaseCallback
	 //Update();
   }

  InputWme::~InputWme() 
  {
      m_attribute->Release();
      m_value->Release();
  }

  IWMObject* InputWme::GetOwningObject(Error* err) const
  {
    ClearError(err);
    
    if ( m_owningobject != 0 ) {
       return m_owningobject;
    } else {
       // TODO: Look for it in working memory (should this ever happen)
       return 0;
    }
  }

  const ISymbol* InputWme::GetAttribute(Error* err) const
  {
    ClearError(err);
    return m_attribute;
  }

  const ISymbol* InputWme::GetValue(Error* err) const
  {
    ClearError(err);
    return m_value;
  }

  // TODO: This method should probably return an unsigned long
  long InputWme::GetTimeTag(Error* err) const
  {
	  // This method can only be validly called once the underlying kernel wme has been created
	  assert(m_rawwme) ;

    ClearError(err);
      return m_rawwme ? m_rawwme->timetag : 0;
    }

  egSKISupportType InputWme::GetSupportType(Error * err) const
  {
    ClearError(err);

    return gSKI_I_SUPPORT;
  }

  tIProductionIterator* InputWme::GetSupportProductions(Error* err) const
  {
    ClearError(err);

    // There are no supporting productions for input wmes so return an empty iterator
    std::vector<IProduction*> temp;
    return new tProductionIter(temp);
  }

  bool InputWme::HasBeenRemoved(Error* err) const
  {
    ClearError(err);

      return m_rawwme == 0;
  }

  bool InputWme::IsEqual(IWme* wme, Error* err) const
  {
     ClearError(err);
     MegaAssert(false, "NOT IMPLEMENTED YET!");
     return false;
  }

  void InputWme::SetOwningObject(InputWMObject* obj)
  {
      m_owningobject = obj;
    } 

  bool InputWme::AttributeEquals( const std::string& attr ) const
  {
    if ( m_attribute == 0 ) return false;
    if ( m_attribute->GetType() != gSKI_STRING ) 
      return false;
    
    return ( m_attribute->GetString() == attr);
    
  }

  void InputWme::Remove() 
  {
    m_removeWme = true;
	 // 09/26/05 KJC:  commenting this direct call so that updates will happen
	 // ONLY from InputPhaseCallback
    //Update();
  }

  void InputWme::Update(bool forceAdds, bool forceRemoves)
  {
     // Adding the WME directly to working memory if were in the input or output phase
     egSKIPhaseType curphase = m_manager->GetAgent()->GetCurrentPhase();
	 
	 bool doAdds    = ((curphase == gSKI_INPUT_PHASE && !forceRemoves )|| forceAdds) ;
	 bool doRemoves = ((curphase == gSKI_INPUT_PHASE && !forceAdds) || forceRemoves) ;

#ifdef DEBUG_UPDATE
	  std::string id = m_owningobject->GetId()->GetString() ;
	  std::string att = m_attribute->GetString() ;
	  std::string value = m_value->GetString() ;
	  std::string soarwme = m_rawwme == 0 ? "No soar wme." : "Has soar wme." ;
	  std::string remove = m_removeWme ? (doRemoves ? "Remove wme now." : "Marked for remove, but not doRemoves.") : "" ;
	  std::string add    = !m_rawwme ? (doAdds ? "Add wme." : "Marked to add, but not doAdd.") : "" ;

	  PrintDebugFormat("Updating %s ^%s %s.  Status is: %s %s %s ", id.c_str(), att.c_str(), value.c_str(), soarwme.c_str(), remove.c_str(), add.c_str()) ;
#endif

	 if ( doAdds || doRemoves ) 
	 {
		 // If there is no raw wme for this than make one 
		 // (unless it has already been removed)
		 if ( doAdds && m_rawwme == 0 && !m_removeWme ) 
		 {
			 agent* a = m_manager->GetSoarAgent();
			 Symbol* idsym = m_owningobject->GetSoarSymbol();
			 Symbol* attr = m_attribute->GetSoarSymbol();
			 Symbol* val = m_value->GetSoarSymbol();

#ifdef DEBUG_UPDATE
			 PrintDebugFormat("Adding %s ^%s %s", id.c_str(), att.c_str(), value.c_str()) ;
#endif

			 m_rawwme = add_input_wme( a, idsym, attr, val );

			 MegaAssert( m_rawwme != 0, "Trouble adding an input wme!");
		 }

		 // Removing any wme's scheduled for removal
		 if ( doRemoves && m_removeWme)
		 {
			 if (m_rawwme != 0 ) 
			 {
#ifdef DEBUG_UPDATE
			    PrintDebugFormat("Removing %s ^%s %s", id.c_str(), att.c_str(), value.c_str()) ;
#endif

				Bool retvalue =  remove_input_wme(m_manager->GetSoarAgent(), m_rawwme);

				MegaAssert( retvalue, "Trouble removing an input wme!");
				m_rawwme = 0;
			 }

			 // We need to handle the case where the gSKI wme was created and deleted before the kernel wme
			 // was ever created (Soar wasn't run).  So we'll release the gSKI object whether the kernel object
			 // exists or not.

			 // Detaching this object from the other input objects
			 MegaAssert( m_owningobject != 0, "Invalid owning object for InputWme." );
			 if(m_owningobject != 0 )
			 {
				 m_owningobject->RemoveReferencedWme(this);
			 }
			 SetOwningObject(0);

			 // After the wme is really removed, we release ourselves.  We do it this way
			 // because a client calling "RemoveWme()" has no way to know when the gSKI Wme object
			 // can be released, because the removal of the kernel wme won't occur until the next
			 // input phase after RemoveWme is called.  Thus "RemoveWme" now includes a reference decrement
			 // as part of its actions, providing the client a good way to do clean up.
			 this->Release() ;
		 }
     }
  }

}
