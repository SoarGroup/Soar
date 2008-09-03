#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_inputlink.cpp
*********************************************************************
* created:	   7/22/2002   12:53
*
* purpose: 
*********************************************************************/

#include "gSKI_InputLink.h"
#include "gSKI_InputWMObject.h"
#include "gSKI_InputWorkingMemory.h"
#include "gSKI_Error.h"
#include "gSKI_Agent.h"
#include "MegaAssert.h"

#include "io.h"

#include <iostream>

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_InputLink);


namespace gSKI
{
   /*
     ===============================

     ===============================
   */

	//KJC:  This should really register thru add_input_function 
	//      to be more explicit.
  InputLink::InputLink(Agent* agent):
    m_agent(agent),
    m_memory(agent)
  {
     soar_add_callback( m_agent->GetSoarAgent(),
			static_cast<void*>(m_agent->GetSoarAgent()),
			INPUT_PHASE_CALLBACK,
			InputPhaseCallback,
			static_cast<void*>(this),
			0,
			"static_input_callback");
     
     // Getting the root input object
//      m_memory.GetRootInputObject(&m_rootObject);
     
  }

   /*
     ===============================

    ===============================
   */

   InputLink::~InputLink() 
   {
     // Removing the callback
     soar_remove_callback( m_agent->GetSoarAgent(),
			   static_cast<void*>(m_agent->GetSoarAgent()),
                           INPUT_PHASE_CALLBACK,
                           "static_input_callback" );

     // Releasing the root object on destruction
//      if (m_rootObject != 0) m_rootObject->Release();
   }

   /*
    ===============================
    
    ===============================
   */

   void InputLink::GetRootObject(IWMObject** rootObject, Error* err)
   {
      ClearError(err);

      // Adding a reference before passing a pointer
      // to this object back
//       if (m_rootObject != 0) m_rootObject->AddRef();

//       *rootObject = m_rootObject;
      InputWMObject* obj;
      m_memory.GetRootInputObject(&obj);
      *rootObject = obj;
   }

   /*
    ===============================

    ===============================
   */

   IWorkingMemory* InputLink::GetInputLinkMemory(Error* err)
   {
      ClearError(err);

      return &m_memory;
   }

   /*
    ===============================
    
    ===============================
   */

   void InputLink::AddInputProducer(IWMObject* object,
                                    IInputProducer* producer,
                                    Error* err)
   {
      ClearError(err);

      // Checking for valid pointers
      MegaAssert( object != 0, "Can't add input producer to null WMObject!");
      MegaAssert( producer != 0, "Can't add null input producer to WMObject!");

      if ( object == 0 || producer == 0 ) {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      // Getting a pointer to a InputWMObject that corresponds to this
      // interface
      InputWMObject* iobj = 
         m_memory.GetOrCreateObjectFromInterface(object);

      MegaAssert
         ( iobj != 0, 
           "Specified WMObject could not be converted to a InputWMObject!");

      iobj->AddInputProducer(producer);

   }

   /*
    ===============================
    
    ===============================
   */
  
   void InputLink::RemoveInputProducer(IWMObject* object,
                                       IInputProducer* producer,
                                       Error* err)
   {
      ClearError(err);

      // Checking for valid pointers
      MegaAssert( object != 0, "Can't remove producer from null object!");
      MegaAssert( producer != 0, "Can't remove null producer from object!");

      if ( object == 0 || producer == 0 ) {
         SetError( err, gSKIERR_INVALID_PTR);
         return;
      }

      // Getting a pointer to the InputWMObject associated with this
      // interface pointer
      InputWMObject* iobj =
         m_memory.GetOrCreateObjectFromInterface(object);

      MegaAssert
         ( iobj != 0,
           "Specified WMObject could not be converted to a InputWMObject!");
      
      iobj->RemoveInputProducer(producer);

   }

   /*
    ===============================
    
    ===============================
   */

   void InputLink::AddWmeReplacementPolicy(IWme* wme,
                                        IWmeReplacementPolicy* policy,
                                        Error* err)
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");
   }

   /*
    ===============================
    
    ===============================
   */

   void InputLink::RemoveWmeReplacementPolicy(IWme* wme,
                                          IWmeReplacementPolicy* policy,
                                          Error* err)
   {
      ClearError(err);

      MegaAssert(false, "NOT IMPLEMENTED YET!");    
   }

   /*
    ===============================
    
    ===============================
   */

  void InputLink::InputPhaseCallback( soar_callback_agent agent,
                                      soar_callback_data callbackdata,
                                      soar_call_data calldata )
  {
    InputLink* ilink = static_cast<InputLink*>(callbackdata);
    int callbacktype = (int)reinterpret_cast<long long>(calldata);

    switch(callbacktype) {
    case TOP_STATE_JUST_CREATED:
      ilink->InitialUpdate();
      break;
    case NORMAL_INPUT_CYCLE:
      ilink->Update();
      break;
    case TOP_STATE_JUST_REMOVED:
      ilink->FinalUpdate();
      break;
    default:
      MegaAssert(false, "The static input callback is of unknown type!");
      break;
    }
  }

  /*
    ===============================
    
    ===============================
  */
  
  void InputLink::InitialUpdate()
  {
    // Perform necessary update functions
    //std::cout << "\nInitial input link update!\n";
  }

  /*
    ===============================
    
    ===============================
  */
  
  void InputLink::Update()
  {
     //std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    // Perform necessary update functions
    //std::cout << "\nNormal input link update cycle!\n";
    m_memory.Update(false, false);
  }

  /*
    ===============================
    
    ===============================
  */
  
  void InputLink::FinalUpdate()
  {
	  Reinitialize();
    // Perform necessary update functions
    //std::cout << "\nFinal input link update cycle!\n";
  }
  
}
