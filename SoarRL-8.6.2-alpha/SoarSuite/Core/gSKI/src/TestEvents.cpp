#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file TestEvents.cpp 
*********************************************************************
* created:	   6/13/2002   12:17
*
* purpose: 
*********************************************************************/


#ifndef NO_MEGA_UNIT_TESTS

#include "MegaUnitTest.h"

#include "EventManagementTemplates.h"
//#include "gSKI_Agent.h"
//
//DEF_EXPOSE(TestEvents);

using namespace gSKI;


/** */
class TestPrintListener: public IPrintListener {
public:

   TestPrintListener(): m_count(1) {}

   /** 
   * @brief Event callback function
   *
   * This method recieves callbacks when the print event occurs for an agent.
   *
   * @param eventId  Id of the event that occured (can only be gSKIEVENT_PRINT)
   * @param agentPtr Pointer to the agent that fired the print event
   * @param msg      Pointer to c-style string containing the print text
   */
   virtual void HandleEvent(egSKIPrintEventId eventId, IAgent* agentPtr, const char* msg)
   {
      VALIDATE(eventId == gSKIEVENT_PRINT);
      VALIDATE(agentPtr == 0);

      switch (m_count % 3)
      {
      case 0:
         VALIDATE(strcmp(msg, "Test3") == 0);
         break;
      case 1:
         VALIDATE(strcmp(msg, "Test1") == 0);
         break;
      case 2:
         VALIDATE(strcmp(msg, "Test2") == 0);
         break;
      }

      m_count++;
   }

private:

   unsigned int m_count;
};


/**  */
DEF_TEST_INSUITE(EventSystem, PrintListener)
{
   ListenerManager<IPrintListener, Agent::PrintNotifier> llist;

   TestPrintListener listener;
   TestPrintListener listener2;
   
   llist.AddListener(gSKIEVENT_PRINT, &listener);

   // Try with single callback
   VALIDATE(llist.GetNumListeners(gSKIEVENT_PRINT) == 1);

   {
      Agent::PrintNotifier pn1(0, "Test1");
      llist.Notify(gSKIEVENT_PRINT, pn1);
      Agent::PrintNotifier pn2(0, "Test2");
      llist.Notify(gSKIEVENT_PRINT, pn2);
      Agent::PrintNotifier pn3(0, "Test3");
      llist.Notify(gSKIEVENT_PRINT, pn3);
   }

   // Test with wrong pointer
   llist.RemoveListener(gSKIEVENT_PRINT, &listener2);
   VALIDATE(llist.GetNumListeners(gSKIEVENT_PRINT) == 1);
   VALIDATE(llist.GetNumListeners(gSKIEVENT_BEFORE_SHUTDOWN) == 0);

   llist.RemoveListener(gSKIEVENT_PRINT, &listener);
   VALIDATE(llist.GetNumListeners(gSKIEVENT_PRINT) == 0);

   llist.RemoveListener(gSKIEVENT_PRINT, &listener);
   VALIDATE(llist.GetNumListeners(gSKIEVENT_PRINT) == 0);

   llist.AddListener(gSKIEVENT_PRINT, &listener2);
   llist.AddListener(gSKIEVENT_PRINT, &listener);
   VALIDATE(llist.GetNumListeners(gSKIEVENT_PRINT) == 2);
   {
      Agent::PrintNotifier pn1(0, "Test1");
      llist.Notify(gSKIEVENT_PRINT, pn1);
      Agent::PrintNotifier pn2(0, "Test2");
      llist.Notify(gSKIEVENT_PRINT, pn2);
      Agent::PrintNotifier pn3(0, "Test3");
      llist.Notify(gSKIEVENT_PRINT, pn3);
   }
   
   ListenerManager<IPrintListener, Agent::PrintNotifier>::tListenerVectorIt it;
   for(it = llist.Begin(gSKIEVENT_PRINT); it != llist.End(gSKIEVENT_PRINT); ++it)
      VALIDATE((*it == &listener) || (*it == &listener2));

   llist.RemoveListener(gSKIEVENT_PRINT, &listener);
   VALIDATE(llist.GetNumListeners(gSKIEVENT_PRINT) == 1);

   llist.RemoveListener(gSKIEVENT_PRINT, &listener2);
   VALIDATE(llist.GetNumListeners(gSKIEVENT_PRINT) == 0);
}

#endif
