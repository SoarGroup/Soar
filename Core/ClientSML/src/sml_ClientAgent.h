/////////////////////////////////////////////////////////////////
// Agent class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used by a client app (e.g. an environment) to represent
// a Soar agent and to send commands and I/O to and from that agent.
//
/////////////////////////////////////////////////////////////////
#ifndef SML_AGENT_H
#define SML_AGENT_H

#include "sml_ClientWorkingMemory.h"
#include "sml_ClientErrors.h"
#include "sml_ListMap.h"
#include "sml_ClientEvents.h"
#include "Export.h"

#include <string>
#include <map>
#include <list>
#include <utility>  // For std::pair

namespace soarxml
{
    class ElementXML ;
}

namespace sml
{

    class Kernel ;
    class Connection ;
    class AnalyzeXML ;
    class ClientXML ;
    class ClientAnalyzedXML ;
    struct DebuggerProcessInformation;

    class RunEventHandlerPlusData : public EventHandlerPlusData
    {
        public:
            RunEventHandler m_Handler ;

            RunEventHandlerPlusData() {}

            RunEventHandlerPlusData(int eventID, RunEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
            {
                m_Handler = handler ;
            }
    } ;

    class ProductionEventHandlerPlusData : public EventHandlerPlusData
    {
        public:
            ProductionEventHandler m_Handler ;

            ProductionEventHandlerPlusData() {}

            ProductionEventHandlerPlusData(int eventID, ProductionEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
            {
                m_Handler = handler ;
            }
    } ;

    class PrintEventHandlerPlusData : public EventHandlerPlusData
    {
        public:
            PrintEventHandler m_Handler ;
            bool m_IgnoreOwnEchos ;

            PrintEventHandlerPlusData()
            {
                m_IgnoreOwnEchos = true;
            }

            PrintEventHandlerPlusData(int eventID, PrintEventHandler handler, void* userData, bool ignoreOwnEchos, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
            {
                m_Handler = handler ;
                m_IgnoreOwnEchos = ignoreOwnEchos ;
            }
    } ;

    class XMLEventHandlerPlusData : public EventHandlerPlusData
    {
        public:
            XMLEventHandler m_Handler ;

            XMLEventHandlerPlusData() {}

            XMLEventHandlerPlusData(int eventID, XMLEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
            {
                m_Handler = handler ;
            }
    } ;

    class OutputEventHandlerPlusData : public EventHandlerPlusData
    {
        public:
            OutputEventHandler  m_Handler ;
            std::string         m_AttributeName ;

            OutputEventHandlerPlusData() {}

            OutputEventHandlerPlusData(int eventID, char const* pAttributeName, OutputEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
            {
                m_Handler = handler ;

                if (pAttributeName)
                {
                    m_AttributeName = pAttributeName ;
                }
            }
    } ;

    class OutputNotificationHandlerPlusData : public EventHandlerPlusData
    {
        public:
            OutputNotificationHandler   m_Handler ;

            OutputNotificationHandlerPlusData() {}

            OutputNotificationHandlerPlusData(int eventID, OutputNotificationHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
            {
                m_Handler = handler ;
            }
    } ;

    class EXPORT Agent : public ClientErrors
    {
            // By using a lot of friends we can keep methods from being exposed
            // to the client and yet allow access between our internal classes.
            friend class Kernel ;
            friend class WorkingMemory ;
            friend class ObjectMap<Agent*> ;    // So can delete agent
            friend class WMElement ;
            friend class StringElement ;
            friend class IntElement ;
            friend class FloatElement ;
            friend class Identifier ;

        protected:
            // The mapping from event number to a list of handlers to call when that event fires
            typedef sml::ListMap<smlRunEventId, RunEventHandlerPlusData>                RunEventMap ;
            typedef sml::ListMap<smlProductionEventId, ProductionEventHandlerPlusData>  ProductionEventMap ;
            typedef sml::ListMap<smlPrintEventId, PrintEventHandlerPlusData>            PrintEventMap ;
            typedef sml::ListMap<smlXMLEventId, XMLEventHandlerPlusData>                XMLEventMap ;
            typedef sml::ListMap<std::string, OutputEventHandlerPlusData>               OutputEventMap ;
            typedef sml::ListMap<smlWorkingMemoryEventId, OutputNotificationHandlerPlusData>    OutputNotificationMap ;

        protected:
            // We maintain a local copy of working memory so we can just send changes
            WorkingMemory   m_WorkingMemory ;

            // The kernel that owns this agent
            Kernel*         m_Kernel ;

            // The name of this agent
            std::string     m_Name ;

            // Map from event id to handler function(s)
            RunEventMap             m_RunEventMap ;
            ProductionEventMap      m_ProductionEventMap ;
            PrintEventMap           m_PrintEventMap ;
            XMLEventMap             m_XMLEventMap ;
            OutputEventMap          m_OutputEventMap ;
            OutputNotificationMap   m_OutputNotificationMap ;

            // These are little utility classes we define in the .cpp file to help with searching the event maps
            class TestRunCallbackFull ;
            class TestRunCallback ;
            class TestProductionCallbackFull ;
            class TestProductionCallback ;
            class TestPrintCallbackFull ;
            class TestPrintCallback ;
            class TestXMLCallbackFull ;
            class TestXMLCallback ;
            class TestOutputCallbackFull ;
            class TestOutputCallback ;
            class TestOutputNotificationCallbackFull ;
            class TestOutputNotificationCallback ;

            // Used to generate unique IDs for callbacks
            int m_CallbackIDCounter ;

            // Internally we register a print callback and store its id here.
            int m_XMLCallback ;

            // When true, if a wme is updated to the same value as before we "blink" the wme by removing
            // the old wme and adding a new one, causing rules to rematch in Soar.
            bool m_BlinkIfNoChange ;

        protected:
            Agent(Kernel* pKernel, char const* pAgentName);

            // This is protected so the client doesn't try to delete it.
            // Client should just delete the kernel object (or call Kernel::DestroyAgent() if just want to destroy this agent).
            virtual ~Agent();

            Connection* GetConnection() const ;
            WorkingMemory* GetWM()
            {
                return &m_WorkingMemory ;
            }

            /*************************************************************
            * @brief This function is called when output is received
            *        from the Soar kernel.
            *
            * @param pIncoming  The output command (list of wmes added/removed from output link)
            * @param pResponse  The reply (no real need to fill anything in here currently)
            *************************************************************/
            void ReceivedOutput(AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;

            /*************************************************************
            * @brief This function is called when an event is received
            *        from the Soar kernel.
            *
            * @param pIncoming  The event command
            * @param pResponse  The reply (no real need to fill anything in here currently)
            *************************************************************/
            void ReceivedEvent(AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
            void ReceivedRunEvent(smlRunEventId id, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
            void ReceivedProductionEvent(smlProductionEventId id, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
            void ReceivedPrintEvent(smlPrintEventId id, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
            void ReceivedXMLEvent(smlXMLEventId id, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;

            /** NOTE: Slightly different sig as this is called without analyzing the incoming msg so it's a bit faster */
            void ReceivedXMLTraceEvent(smlXMLEventId id, soarxml::ElementXML* pIncomingMsg, soarxml::ElementXML* pResponse) ;

            /*************************************************************
            * @brief This function is called after output has been received
            *        and processed from the kernel.
            *************************************************************/
            void FireOutputNotification() ;

            /*************************************************************
            * @brief Call any registered handlers to notify them when
            *        a new working memory element is added to the top
            *        level of the output link.
            *************************************************************/
            void ReceivedOutputEvent(WMElement* pWmeAdded) ;
            bool IsRegisteredForOutputEvent() ;

            /*************************************************************
            * @brief Returns true if changes to i/o links should be
            *        committed (sent to kernelSML) immediately when they
            *        occur, so the client doesn't need to remember to call commit.
            *************************************************************/
            bool IsAutoCommitEnabled() ;

        public:
            /*************************************************************
            * @brief Returns this agent's name.
            *************************************************************/
            char const* GetAgentName() const
            {
                return m_Name.c_str() ;
            }

            /*************************************************************
            * @brief Returns a pointer to the kernel object that owns this Agent.
            *************************************************************/
            Kernel*     GetKernel() const
            {
                return m_Kernel ;
            }

            /*************************************************************
            * @brief Load a set of productions from a file.
            *
            * The file must currently be on a filesystem that the kernel can
            * access (i.e. can't send to a remote PC unless that PC can load
            * this file).
            *
            * @param echoResults  If true the results are also echoed through the smlEVENT_ECHO event, so they can appear in a debugger (or other listener)
            * @returns True if finds file to load successfully.
            *************************************************************/
            bool LoadProductions(char const* pFilename, bool echoResults = true) ;

            /*************************************************************
            * @brief Returns the id object for the input link.
            *        The agent retains ownership of this object.
            *************************************************************/
            Identifier* GetInputLink() ;

            /*************************************************************
            * @brief An alternative that matches the older SGIO interface.
            *        The agent retains ownership of this object.
            *************************************************************/
            Identifier* GetILink()
            {
                return GetInputLink() ;
            }

            /*************************************************************
            * @brief Returns the id object for the output link.
            *        The agent retains ownership of this object.
            *        Note this will be null until the first time an agent
            *        puts something on the output link.
            *************************************************************/
            Identifier* GetOutputLink() ;

            /*************************************************************
            * @brief Builds a new WME that has a string value and schedules
            *        it for addition to Soar's input link.
            *
            *        The agent retains ownership of this object.
            *
            *        The returned object is valid until the caller
            *        deletes the parent identifier.
            *
            *        If "auto commit" is turned off in ClientKernel,
            *        the WME is not added to Soar's input link until the
            *        client calls "Commit".
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *************************************************************/
            StringElement* CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue);

            /*************************************************************
            * @brief Same as CreateStringWME but for a new WME that has
            *        an int as its value.
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *************************************************************/
            IntElement* CreateIntWME(Identifier* parent, char const* pAttribute, long long value) ;

            /*************************************************************
            * @brief Same as CreateStringWME but for a new WME that has
            *        a floating point value.
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *************************************************************/
            FloatElement* CreateFloatWME(Identifier* parent, char const* pAttribute, double value) ;

            /*************************************************************
            * @brief Same as CreateStringWME but for a new WME that has
            *        an identifier as its value.
            *        The identifier value is generated here and will be
            *        different from the value Soar assigns in the kernel.
            *        The kernel will keep a map for translating back and forth.
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *************************************************************/
            Identifier*     CreateIdWME(Identifier* parent, char const* pAttribute) ;

            /*************************************************************
            * @brief Creates a new WME that has an identifier as its value.
            *        The value in this case is the same as an existing identifier.
            *        This allows us to create a graph rather than a tree.
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *************************************************************/
            Identifier*     CreateSharedIdWME(Identifier* parent, char const* pAttribute, Identifier* pSharedValue) ;

            /*************************************************************
            * @brief Update the value of an existing WME.
            *        If "auto commit" is turned off in ClientKernel,
            *        the value is not actually sent to the kernel
            *        until "Commit" is called.
            *
            *        If "BlinkIfNoChange" is false then updating a wme to the
            *        same value it already had will be ignored.
            *        This value is true by default, so updating a wme to the same
            *        value causes the wme to be deleted and a new identical one to be added
            *        which will trigger rules to rematch.
            *        You can turn this flag on and off around a set of calls to update if you wish.
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *************************************************************/
            void    Update(StringElement* pWME, char const* pValue) ;
            void    Update(IntElement* pWME, long long value) ;
            void    Update(FloatElement* pWME, double value) ;

            /*************************************************************
            * @brief This flag controls whether updating a wme to the same
            *        value that it already has causes it to "blink" or not.
            *        Blinking means the wme is removed and an identical wme is added,
            *        causing rules that test this wme to be rematched and to fire again.
            *************************************************************/
            void SetBlinkIfNoChange(bool state)
            {
                m_BlinkIfNoChange = state ;
            }
            bool IsBlinkIfNoChange()
            {
                return m_BlinkIfNoChange ;
            }

            /*************************************************************
            * @brief Schedules a WME from deletion from the input link and removes
            *        it from the client's model of working memory.
            *
            *        If this is an identifier then all of its children will be
            *        deleted too (assuming it's the only parent -- i.e. part of a tree not a full graph).
            *
            *        The caller should not access this WME after calling
            *        DestroyWME() or any of its children if this is an identifier.
            *        If "auto commit" is turned off in ClientKernel,
            *        the WME is not removed from the input link until
            *        the client calls "Commit"
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *************************************************************/
            bool    DestroyWME(WMElement* pWME) ;

            /*************************************************************
            * @brief Reinitialize this Soar agent.
            *        This will also cause the output link structures stored
            *        here to be erased and the current input link to be sent over
            *        to the Soar agent for the start of its next run.
            *************************************************************/
            char const* InitSoar() ;

            /*************************************************************
            * @brief Register an "Output event handler".
            *        This is one way to be notified when output occurs on the output link.
            *        You register for a specific attribute name (e.g. "move") and when that attribute is added to the
            *        output link the handler you have registered for that name is called.
            * @param pAttributeName The attribute which will trigger this callback ("move" in the example).
            * @param handler        A function that will be called when the event happens
            * @param pUserData      Arbitrary data that will be passed back to the handler function when the event happens.
            * @param addToBack      If true add this handler is called after existing handlers.  If false, called before existing handlers.
            *
            * @returns A unique ID for this callback (used to unregister the callback later)
            *************************************************************/
            int AddOutputHandler(char const* pAttributeName, OutputEventHandler handler, void* pUserData, bool addToBack = true) ;

            /*************************************************************
            * @brief Unregister for a particular output event
            *************************************************************/
            bool RemoveOutputHandler(int callbackID) ;

            /*************************************************************
            * @brief Register for a "RunEvent".
            *        Multiple handlers can be registered for the same event.
            * @param smlEventId     The event we're interested in (see the list below for valid values)
            * @param handler        A function that will be called when the event happens
            * @param pUserData      Arbitrary data that will be passed back to the handler function when the event happens.
            * @param addToBack      If true add this handler is called after existing handlers.  If false, called before existing handlers.
            *
            * Current set is:
            * smlEVENT_BEFORE_SMALLEST_STEP,
            * smlEVENT_AFTER_SMALLEST_STEP,
            * smlEVENT_BEFORE_ELABORATION_CYCLE,
            * smlEVENT_AFTER_ELABORATION_CYCLE,
            * smlEVENT_BEFORE_PHASE_EXECUTED,
            * smlEVENT_AFTER_PHASE_EXECUTED,
            * smlEVENT_BEFORE_DECISION_CYCLE,
            * smlEVENT_AFTER_DECISION_CYCLE,
            * smlEVENT_AFTER_INTERRUPT,
            * smlEVENT_AFTER_HALTED,
            * smlEVENT_BEFORE_RUN_STARTS,
            * smlEVENT_AFTER_RUN_ENDS,
            * smlEVENT_BEFORE_RUNNING,
            * smlEVENT_AFTER_RUNNING
            *
            * @returns A unique ID for this callback (used to unregister the callback later)
            *************************************************************/
            int RegisterForRunEvent(smlRunEventId id, RunEventHandler handler, void* pUserData, bool addToBack = true) ;

            /*************************************************************
            * @brief Unregister for a particular event
            *************************************************************/
            bool    UnregisterForRunEvent(int callbackID) ;

            /*************************************************************
            * @brief Register to be notified when output has been received from the agent.
            *        This event is a bit special, because we ensure that the client side data structures
            *        have been updated before this event is triggered.  So you can examine the current contents
            *        of the output link (GetOutputLink() etc.) and it will be up to date.
            *************************************************************/
            int RegisterForOutputNotification(OutputNotificationHandler handler, void* pUserData, bool addToBack = true) ;
            bool UnregisterForOutputNotification(int callbackID) ;

            /*************************************************************
            * @brief Register for a "ProductionEvent".
            *        Multiple handlers can be registered for the same event.
            * @param smlEventId     The event we're interested in (see the list below for valid values)
            * @param handler        A function that will be called when the event happens
            * @param pUserData      Arbitrary data that will be passed back to the handler function when the event happens.
            * @param addToBack      If true add this handler is called after existing handlers.  If false, called before existing handlers.
            *
            * Current set is:
            * Production Manager
            * smlEVENT_AFTER_PRODUCTION_ADDED,
            * smlEVENT_BEFORE_PRODUCTION_REMOVED,
            * smlEVENT_AFTER_PRODUCTION_FIRED,
            * smlEVENT_BEFORE_PRODUCTION_RETRACTED,
            *
            * @returns A unique ID for this callback (used to unregister the callback later)
            *************************************************************/
            int RegisterForProductionEvent(smlProductionEventId id, ProductionEventHandler handler, void* pUserData, bool addToBack = true) ;

            /*************************************************************
            * @brief Unregister for a particular event
            *************************************************************/
            bool    UnregisterForProductionEvent(int callbackID) ;

            /*************************************************************
            * @brief Register for an "PrintEvent".
            *        Multiple handlers can be registered for the same event.
            * @param smlEventId     The event we're interested in (see the list below for valid values)
            * @param handler        A function that will be called when the event happens
            * @param pUserData      Arbitrary data that will be passed back to the handler function when the event happens.
            * @param ignoreOwnEchos If true and registering for echo event, commands issued through this connection won't echo.  If false, echos all commands.  Ignored for non-echo events.
            * @param addToBack      If true add this handler is called after existing handlers.  If false, called before existing handlers.
            *
            * Current set is:
            * smlEVENT_PRINT
            *
            * @returns A unique ID for this callback (used to unregister the callback later)
            *************************************************************/
            int RegisterForPrintEvent(smlPrintEventId id, PrintEventHandler handler, void* pUserData, bool ignoreOwnEchos = true, bool addToBack = true) ;

            /*************************************************************
            * @brief Unregister for a particular event
            *************************************************************/
            bool UnregisterForPrintEvent(int callbackID) ;

            /*************************************************************
            * @brief Register for an "XMLEvent".
            *        Multiple handlers can be registered for the same event.
            * @param smlEventId     The event we're interested in (see the list below for valid values)
            * @param handler        A function that will be called when the event happens
            * @param pUserData      Arbitrary data that will be passed back to the handler function when the event happens.
            * @param addToBack      If true add this handler is called after existing handlers.  If false, called before existing handlers.
            *
            * Current set is:
            * smlEVENT_PRINT
            *
            * @returns A unique ID for this callback (used to unregister the callback later)
            *************************************************************/
            int RegisterForXMLEvent(smlXMLEventId id, XMLEventHandler handler, void* pUserData, bool addToBack = true) ;

            /*************************************************************
            * @brief Unregister for a particular event
            *************************************************************/
            bool UnregisterForXMLEvent(int callbackID) ;

            /*==============================================================================
            ===
            === There are a number of different ways to read information from
            === the output link.  Choose whichever method seems easiest to you.
            ===
            === Method 1:
            ===         a) Call "Run(n)".
            ===         b) Call "Commands", "GetCommand" and "GetParamValue"
            ===            to get top level WMEs that have been added since the last cycle.
            ===
            === Method 2:
            ===         a) Call "Run(n)".
            ===         b) Call "GetOutputLink" and "GetNumberChildren", "GetChild"
            ===            to walk the tree and examine its current state.
            ===         c) You can use "IsJustAdded" and "AreChildrenModified"
            ===            to see what WMEs just changed.
            ===         d) This method does not require change tracking, you can disable
            ===            it by calling SetOutputLinkChangeTracking(false).
            ===
            === Method 3:
            ===         a) Call "Run(n)".
            ===         b) Call "GetNumberOutputLinkChanges" and "GetOutputLinkChange"
            ===            and "IsOutputLinkChangeAdd" to get the list of
            ===            all WMEs added and removed since the last decision cycle.
            ===
            === Method 4:
            ===         a) Call "AddOutputHandler" to register functions that are called
            ===            when a specific attributes are added to the output link.
            ===         b) Call "Run(n)".  If the target attribute is added the output function
            ===            is called.
            ===
            === Method 1 is the closest to the original SGIO and should be sufficient
            === in almost all cases.  However, Methods 2 & 3 provide complete
            === access to the output-link, while Method 1 only allows access to
            === top level wmes with this format: (I1 ^output-link I3) (I3 ^move M3) (M3 ^position 10).
            === i.e. All commands are added as identifiers at the top level.
            === Method 4 is closest to the gSKI model for I/O and is more event driven but
            === like Method 1 assumes that commands are added by name to the top level and that
            === the list of commands is known in advance (usually not a problem).
            ===
            ==============================================================================*/

            /*************************************************************
            * @brief Enable or disable output-link change tracking. Do
            *        NOT use if using Commands, GetCommand,
            *        GetOutputLinkChange, AddOutputHandler.
            *************************************************************/
            void SetOutputLinkChangeTracking(bool setting)
            {
                GetWM()->SetOutputLinkChangeTracking(setting);
            }

            /*************************************************************
            * @brief Get number of changes to output link since last cycle.
            *************************************************************/
            int     GetNumberOutputLinkChanges() ;

            /*************************************************************
            * @brief Get the n-th wme added or deleted to output link since
            *        last cycle.
            *************************************************************/
            WMElement*  GetOutputLinkChange(int index) ;

            /*************************************************************
            * @brief Returns true if the n-th wme change to the output-link
            *        since the last cycle was a wme being added.
            *        (false => it was a wme being deleted)
            *************************************************************/
            bool        IsOutputLinkChangeAdd(int index) ;

            /*************************************************************
            * @brief Deprecated: Clears the current list of changes
            *        to the output-link.
            # @deprecated This is now called automatically after each
            *        decision cycle.
            *************************************************************/
            void    ClearOutputLinkChanges() ;

            /*************************************************************
            * @brief Get the number of "commands".  A command in this context
            *        is an identifier wme that have been added to the top level of
            *        the output-link since the last decision cycle.
            *
            *        NOTE: This function may involve searching a list so it's
            *        best to not call it repeatedly.
            *************************************************************/
            int     GetNumberCommands() ;

            /*************************************************************
            * @brief Returns true if there are "commands" available.
            *        A command in this context is an identifier wme that
            *        has been added to the top level of the output-link
            *        since the last decision cycle.
            *
            *        NOTE: This function may involve searching a list so it's
            *        best to not call it repeatedly.
            *************************************************************/
            bool    Commands()
            {
                return GetNumberCommands() > 0 ;
            }

            /*************************************************************
            * @brief Get the n-th "command".  A command in this context
            *        is an identifier wme that have been added to the top level of
            *        the output-link since the last decision cycle.
            *
            *        Returns NULL if index is out of range.
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *
            * @param index  The 0-based index for which command to get.
            *************************************************************/
            Identifier* GetCommand(int index) ;

            /*************************************************************
            * @brief Send the most recent list of changes to working memory
            *        over to the kernel.
            *************************************************************/
            bool Commit() ;

            /*************************************************************
            * @brief Returns true if this agent has uncommitted changes.
            *************************************************************/
            bool IsCommitRequired() ;

            /*************************************************************
            * @brief   Run one Soar agent for the specified number of decisions
            *
            * @returns The result of executing the run command.
            *          The output from during the run is sent to a different callback.
            *************************************************************/
            char const* RunSelf(int numberSteps, smlRunStepSize stepSize = sml_DECISION) ;
            char const* RunSelfForever() ;

            /*************************************************************
            * @brief   Run Soar until either output is generated or
            *          the maximum number of decisions is reached.
            *
            * We don't generally want Soar to just run until it generates
            * output without any limit as an error in the AI logic might cause
            * it to never return control to the environment, so there is a maximum
            * decision count (currently 15) and if the agent fails to produce output
            * before then this command returns.  (This value can be changed with the
            * max-nil-output-cycles command).
            *************************************************************/
            char const* RunSelfTilOutput() ;

            /*************************************************************
            * @brief Returns true if this agent was part of the last set
            *        of agents that was run.
            *************************************************************/
            bool WasAgentOnRunList() ;

            /*************************************************************
            * @brief Returns whether the last run for this agent was
            *        interrupted (by a stop call) or completed normally.
            *************************************************************/
            smlRunResult GetResultOfLastRun() ;

            /*************************************************************
            * @brief Interrupt the currently running Soar agent.
            *
            * Call this after calling "Run" in order to stop a Soar agent.
            * The usual way to do this is to register for an event (e.g. AFTER_DECISION_CYCLE)
            * and in that event handler decide if the user wishes to stop soar.
            * If so, call to this method inside that handler (this ensures you're calling on the same
            * thread that Soar is running on so you don't get blocked).
            *
            * The request to Stop may not be honored immediately.
            * Soar will stop at the next point it is considered safe to do so.
            *
            *************************************************************/
            char const* StopSelf() ;

            /*************************************************************
            * @brief Resend the complete input link to the kernel
            *        and remove our output link structures.
            *        We do this when the user issues an "init-soar" event.
            *        There should be no reason for the client to call this method directly.
            *************************************************************/
            void Refresh() ;

            /*************************************************************
            * @brief Returns the phase that the agent will execute when next
            *        asked to run.
            *************************************************************/
            smlPhase GetCurrentPhase() ;

            /*************************************************************
            * @brief Returns the current decision cycle counter.
            *************************************************************/
            int GetDecisionCycleCounter() ;

            /*************************************************************
            * @brief Returns the current run state of the agent.
            *        Mostly of use to determine if agent halted in last run.
            *************************************************************/
            smlRunState GetRunState() ;

            /*************************************************************
            * @brief Process a command line command
            *
            * @param pCommandLine Command line string to process.
            * @param echoResults  If true the results are also echoed through the smlEVENT_ECHO event, so they can appear in a debugger (or other listener)
            * @param noFilter     If true this command line by-passes any external filters that have been registered (this is not common) and is executed immediately.
            * @returns The string form of output from the command.
            *************************************************************/
            char const* ExecuteCommandLine(char const* pCommandLine, bool echoResults = false, bool noFilter = false) ;

            /*************************************************************
            * @brief Execute a command line command and return the result
            *        as an XML object.
            *
            * @param pCommandLine Command line string to process.
            * @param pAgentName   Agent name to apply the command line to.
            * @param pResponse    The XML response will be returned within this object.
            *                     The caller should allocate this and pass it in.
            * @returns True if the command succeeds.
            *************************************************************/
            bool ExecuteCommandLineXML(char const* pCommandLine, ClientAnalyzedXML* pResponse) ;

#ifndef NO_SVS
            void        SendSVSInput(const std::string& txt);
            std::string GetSVSOutput();
            std::string SVSQuery(const std::string& q);
#endif

            /*************************************************************
            * @brief Get last command line result
            *
            * (This is the last result for any command sent to the kernel,
            *  not just for this agent).
            *
            * @returns True if the last command line call succeeded.
            *************************************************************/
            bool GetLastCommandLineResult();

            /*************************************************************
            * @brief Returns true if this string is the name of a production
            *        that is currently loaded in the agent.
            *************************************************************/
            bool IsProductionLoaded(char const* pProductionName) ;

            /*************************************************************
            * @brief This method is used to update this client's representation
            *        of the input link to match what is currently on the agent's
            *        input link.
            *        Calling this method recreates the entire input link tree on the
            *        client side, invalidating any existing pointers.
            *
            *        NOTE: This is the reverse of how a client normally uses the input link
            *        but can be useful for tools that wish to debug or monitor changes in the input link.
            *
            *        NOTE: If two clients try to modify the input link at once we don't
            *        make any guarantees about what will or won't work.
            *************************************************************/
            bool SynchronizeInputLink() ;

            /*************************************************************
            * @brief This method is used to update this client's representation
            *        of the output link to match what is currently on the agent's
            *        output link.
            *        Calling this method recreates the entire output link tree on the
            *        client side, invalidating any existing pointers.
            *
            *        NOTE: Calling this method shouldn't generally be necessary as the output link
            *        structures in the client are usually automatically kept in synch with the agent.
            *        It's here in case a client connects to an existing kernel and agent
            *        and wants to get up to date on the current state of the output link.
            *************************************************************/
            bool SynchronizeOutputLink() ;

            /*************************************************************
            * @brief This method spawns a debugger to connect to this agent.
            *        Java must be in the path. If jarpath is NULL, the
            *        function will search for SoarJavaDebugger.jar first in
            *        the current directory, then in $SOAR_HOME. Returns
            *        false if the jar is not found or process spawning fails.
            *************************************************************/
            bool SpawnDebugger(int port = -1, const char* jarpath = 0);

            /*************************************************************
            * @brief Kills the previously spawned debugger.
            *************************************************************/
            bool KillDebugger();

            /*************************************************************
            * @brief Convert a client-side identifier string to kernel-side.
            *************************************************************/
            char const* ConvertIdentifier(char const* pClientIdentifier);

        protected:
            // for {Spawn, Kill}Debugger()
            DebuggerProcessInformation* m_pDPI;

    };

}//closes namespace

#endif //SML_AGENT_H
