/////////////////////////////////////////////////////////////////
// PrintListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*       smlEVENT_PRINT
*/
/////////////////////////////////////////////////////////////////

#ifndef PRINT_LISTENER_H
#define PRINT_LISTENER_H

#include "sml_EventManager.h"
#include "sml_AgentOutputFlusher.h"

#include <iostream>
#include <string>
#include <sstream>
#include <map>

namespace sml
{

    class KernelSML ;
    class Connection ;
    
    class PrintListener : public EventManager<smlPrintEventId>
    {
        protected:
            const static int kNumberPrintEvents = smlEVENT_LAST_PRINT_EVENT - smlEVENT_FIRST_PRINT_EVENT + 1 ;
            KernelSML*      m_pKernelSML ;
            std::stringstream m_BufferedPrintOutput[kNumberPrintEvents];
            AgentOutputFlusher* m_pAgentOutputFlusher[kNumberPrintEvents];
            
            // When false we don't forward print callback events to the listeners.  (Useful when we're backdooring into the kernel)
            bool            m_EnablePrintCallback ;
            
        public:
            PrintListener()
            {
                m_pKernelSML = 0 ;
            }
            
            virtual ~PrintListener()
            {
                Clear() ;
            }
            void Init(KernelSML* pKernelSML, AgentSML* pAgentSML) ;
            
            // Called when an event occurs in the kernel
            virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;
            
            // Returns true if this is the first connection listening for this event
            virtual bool AddListener(smlPrintEventId eventID, Connection* pConnection) ;
            
            // Returns true if at least one connection remains listening for this event
            virtual bool RemoveListener(smlPrintEventId eventID, Connection* pConnection) ;
            
            void OnEvent(smlPrintEventId eventID, AgentSML* pAgentSML, const char* msg) ;
            
            // Allows us to temporarily stop forwarding print callback output from the kernel to the SML listeners
            void EnablePrintCallback(bool enable)
            {
                m_EnablePrintCallback = enable ;
            }
            
            // Activate the print callback (flush output).  For echo events we want to specify which connection triggered the event.
            void FlushOutput(smlPrintEventId eventID)
            {
                FlushOutput(NULL, eventID) ;
            }
            void FlushOutput(Connection* pSourceConnection, smlPrintEventId eventID);
            
    } ;
    
}

#endif
