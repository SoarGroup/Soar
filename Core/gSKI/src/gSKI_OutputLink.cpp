#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_outputlink.cpp
*********************************************************************
* created:	   7/22/2002   13:40
*
* purpose: 
*********************************************************************/

#include "IgSKI_OutputProcessor.h"

#include "gSKI_OutputLink.h"
#include "gSKI_Error.h"
#include "gSKI_Agent.h"
#include "gSKI_Symbol.h"
#include "gSKI_WMObject.h"
#include "gSKI_OutputWme.h"
#include "MegaAssert.h"
#include "IgSKI_Iterator.h"

#include "gSKI_OutputWMObject.h"

#include <iostream>
#include <algorithm>

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_OutputLink);


namespace gSKI
{

   /*
     ===============================

     ===============================
   */
 
  OutputLink::OutputLink(Agent* agent):
    m_agent(agent),
    m_memory(agent),
    m_autoupdate(true)
   {
	  // DJP: The id for this callback seems to be important but I haven't figured out why yet.
	  // However, if I change it from "output-link" to anything else I don't get a callback any more.
	   // KJC:  "output-link" in Soar is a specific symconstant
      soar_add_callback( m_agent->GetSoarAgent(),
			 static_cast<void*>(m_agent->GetSoarAgent()),
			 OUTPUT_PHASE_CALLBACK,
			 OutputPhaseCallback,
			 static_cast<void*>(this),
			 0,
			 "output-link");
   }

   /*
     ===============================

     ===============================
   */

   OutputLink::~OutputLink() 
   {
      // Removing the static callback from the soar kernel
      soar_remove_callback( m_agent->GetSoarAgent(),
			    static_cast<void*>(m_agent->GetSoarAgent()),
			    OUTPUT_PHASE_CALLBACK,
				"output-link");
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::AddOutputProcessor(const char* attributePath,
				       IOutputProcessor* processor,
				       Error* error)
   {
      ClearError(error);
      
      if ( processor == 0 || attributePath == 0 ) return;

      std::string path(attributePath);

      m_processormap.insert(std::pair<std::string, IOutputProcessor*>
                            (path, processor));
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::RemoveOutputProcessor(const char* attributePath,
                                          IOutputProcessor* processor,
					  Error* error)
   {
      ClearError(error);
      
      if ( processor == 0 || attributePath == 0 ) return;

      std::string path(attributePath);
  
      for ( tProcessorIt it = m_processormap.find(std::string(path));
            it != m_processormap.upper_bound(path);
            /* don't increment an erasure loop normally */) {
        // Searching through the processor map and removing all instances
        // of the processor for this path
        IOutputProcessor* tempprocessor = it->second;
        if ( tempprocessor == processor ) {
          m_processormap.erase(it++);
        } else {
          ++it;
        }
      }

   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::GetRootObject(IWMObject** rootObject, Error* error)
   {
      ClearError(error);
      
      OutputWMObject* obj;
      m_memory.GetOutputRootObject(&obj);
      *rootObject = obj;
   }

   /*
     ===============================

     ===============================
   */

   // Inlined into header based on profiling data
   //IWorkingMemory* OutputLink::GetOutputMemory(Error* error)
   //{
   //   ClearError(error);
   //
   //   return &m_memory;
   //}

   /*
     ===============================

     ===============================
   */

   void OutputLink::OutputPhaseCallback( soar_callback_agent agent,
					 soar_callback_data callbackdata,
                                         soar_call_data calldata )
   {
      // Sorry but this has to be an old style cast to
      // keep egcs 1.1.2 from segfaulting
      // TODO: Find out why this cast won't work
      OutputLink* olink = (OutputLink*)(callbackdata);
      output_call_info* oinfo = static_cast<output_call_info*>(calldata);
      int callbacktype = oinfo->mode;
	  egSKIWorkingMemoryChange change = gSKI_ADDED_OUTPUT_COMMAND ;

      //std::cout << "Normal output link update cycle!" << std::endl;

	  // These update calls do two things:
	  // First, they create and delete gSKI working memory objects
	  // so the gSKI object map matches the current set of wmes on the output link.
	  // These are stored in m_wmobjectmap and m_wmemap in output working memory.
	  // Second, they notify output processors about the changes.

      switch (callbacktype) {
      case ADDED_OUTPUT_COMMAND:
         olink->InitialUpdate(oinfo->outputs);
		 change = gSKI_ADDED_OUTPUT_COMMAND ;
         break;
      case MODIFIED_OUTPUT_COMMAND:
         olink->Update(oinfo->outputs);
		 change = gSKI_MODIFIED_OUTPUT_COMMAND ;
         break;
      case REMOVED_OUTPUT_COMMAND:
         olink->FinalUpdate(oinfo->outputs);
		 change = gSKI_REMOVED_OUTPUT_COMMAND ;
         break;
      default:
         MegaAssert(false, "The static output callback is of unknown type!");
         break;
      }

	  // DJP: Notify any listeners about this event.
	  // I'm adding this as an alternative route to get notifications about output link changes
	  // that the existing output processor semantics doesn't support (e.g. getting all top level additions
	  // or getting notifications of when a modification has occured to an existing structure on the output link).
	  olink->m_memory.NotifyWorkingMemoryListeners(gSKIEVENT_OUTPUT_PHASE_CALLBACK, change, oinfo->outputs) ;
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::InitialUpdate(io_wme* wmelist)
   {
      // Perform necessary functions
      //std::cout << "\nInitial output link update!\n";

     ProcessIOWmes(wmelist);
   }
   /*
     ===============================

     ===============================
   */

   void OutputLink::Update(io_wme* wmelist)
   {
      // Perform necessary functions
      //std::cout << "\nNormal output link update!\n";

     ProcessIOWmes(wmelist);
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::FinalUpdate(io_wme* wmelist)
   {
      // Perform necessary functions
      //std::cout << "\nFinal output link update!\n";
     //
     // TODO: Insure that we don't want the call to ProcessIOWmes
     //        done here.
     //ProcessIOWmes(wmelist);
     //OutputWMObject* x = m_memory.GetOutputRootObject();

     //x->Release();
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::ProcessIOWmes(io_wme* wmelist)
   {
      // Counting the number of io_wmes
       /*
         int count = 0;
         for (io_wme* cur = wmelist; cur != 0; cur = cur->next) {
         ++count;
        
         std::cout << "\n------------------\n" 
         << "Number = " << count << std::endl
         << "ID     = " 
         << gSymbol::ConvertSymbolToString(cur->id) 
         << std::endl
         << "Attr   = " 
         << gSymbol::ConvertSymbolToString(cur->attr) 
         << std::endl
         << "Value  = " 
         << gSymbol::ConvertSymbolToString(cur->value) 
         << std::endl
         << "------------------\n";
         }
         std::cout << "\nRecieved " << count << " Wmes on output link!\n";
       */

      // Updating the working memory object with the io wme list
	  // This call walks the current list of wmes and creates and destroys
	  // gSKI proxy objects to match this list.
      m_memory.UpdateWithIOWmes(wmelist);

      // Searching for new matching patterns for IOutputProcessors
      
      if ( m_autoupdate ) {
         InvokeOutputProcessors();
      }
      
      
   }

   /*
     ===============================

     ===============================
   */
   void OutputLink::InvokeOutputProcessors(Error * err) 
   {
      ClearError(err);

      // TODO: Make this more efficient for repeated paths
      for ( tProcessorIt it = m_processormap.begin();
            it != m_processormap.end();
            ++it) {
        
         // Finding the objects that match the criterion
         OutputWMObject* rootObj;
         m_memory.GetOutputRootObject(&rootObj);
         tIWMObjectIterator* matchingObjects = m_memory.FindObjects(rootObj,
                                                                    it->first);
         
         for ( ;
               matchingObjects->IsValid();
               matchingObjects->Next() ) {
            // Getting the object from the iterator and invoking the processor
            IWMObject* curobj = matchingObjects->GetVal();
            it->second->ProcessOutput(&m_memory, curobj);
            curobj->Release();
         }
         
         rootObj->Release();
         matchingObjects->Release();
         
      }
   }
   
}

