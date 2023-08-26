#include "handlers.hpp"
#include "handlers.hpp"

#include "portability.h"

#include <vector>
#include <sstream>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"
#include "thread_Event.h"

#include "misc.h"
#include "TestHelpers.hpp"

void Handlers::MyBoolShutdownHandler(sml::smlSystemEventId, void* pUserData, sml::Kernel*)
{
    no_agent_assertTrue(pUserData);
    bool* pHandlerReceived = static_cast< bool* >(pUserData);
    *pHandlerReceived = true;
}

void Handlers::MyEventShutdownHandler(sml::smlSystemEventId, void* pUserData, sml::Kernel*)
{
    no_agent_assertTrue(pUserData);
    soar_thread::Event* pEvent = static_cast< soar_thread::Event* >(pUserData);
    pEvent->TriggerEvent();
}

void Handlers::MyShutdownTestShutdownHandler(sml::smlSystemEventId, void* pUserData, sml::Kernel* pKernel)
{
    no_agent_assertTrue(pUserData);
    no_agent_assertTrue(pKernel);

    pKernel->Shutdown();
    delete pKernel;

    soar_thread::Event* pEvent = static_cast< soar_thread::Event* >(pUserData);
    pEvent->TriggerEvent();
}

void Handlers::MyDeletionHandler(sml::smlAgentEventId, void* pUserData, sml::Agent*)
{
    no_agent_assertTrue(pUserData);
    bool* pHandlerReceived = static_cast< bool* >(pUserData);
    *pHandlerReceived = true;
}

void Handlers::MySystemEventHandler(sml::smlSystemEventId, void*, sml::Kernel*)
{
    // BUGBUG see comments above registration line
}

void Handlers::MyCreationHandler(sml::smlAgentEventId, void* pUserData, sml::Agent*)
{
    no_agent_assertTrue(pUserData);
    bool* pHandlerReceived = static_cast< bool* >(pUserData);
    *pHandlerReceived = true;
}

void Handlers::MyProductionHandler(sml::smlProductionEventId id, void* pUserData, sml::Agent*, char const*, char const*)
{
    no_agent_assertTrue(pUserData);
    int* pInt = static_cast< int* >(pUserData);

    // Increase the count
    *pInt += 1 ;

    no_agent_assertTrue(id == sml::smlEVENT_BEFORE_PRODUCTION_REMOVED);
}

const char *Handlers::MyClientMessageHandler(sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pMessage, int *bufSize, char *buf)
{
    std::stringstream res;
    res << "handler-message" << pMessage;

		if ( res.str().size() + 1 > *bufSize )
		{
			*bufSize = res.str().size() + 1;
			return NULL;
		}
		strcpy( buf, res.str().c_str() );

    no_agent_assertTrue(pUserData);
    bool* pHandlerReceived = static_cast< bool* >(pUserData);
    *pHandlerReceived = true;

		return buf;
}

// This is a very dumb filter--it adds "--depth 2" to all commands passed to it.
const char *Handlers::MyFilterHandler(sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pCommandLine, int *bufSize, char *buff)
{
    soarxml::ElementXML* pXML = soarxml::ElementXML::ParseXMLFromString(pCommandLine) ;
    no_agent_assertTrue(pXML);
    no_agent_assertTrue(pXML->GetAttribute(sml::sml_Names::kFilterCommand));

    std::stringstream commandLine;
    commandLine << pXML->GetAttribute(sml::sml_Names::kFilterCommand) << " --depth 2";

    // Replace the command attribute in the XML
    no_agent_assertTrue(pXML->AddAttribute(sml::sml_Names::kFilterCommand, commandLine.str().c_str()));

    // Convert the XML back to a string and put it into a std::string ready to return
    char* pXMLString = pXML->GenerateXMLString(true) ;
    no_agent_assertTrue(pXMLString);
    std::string res(pXMLString);

    pXML->DeleteString(pXMLString);
    delete pXML ;

		if ( res.size() + 1 > *bufSize )
		{
			*bufSize = res.size() + 1;
			return NULL;
		}
		strcpy( buff, res.c_str() );

    no_agent_assertTrue(pUserData);
    bool* pHandlerReceived = static_cast< bool* >(pUserData);
    *pHandlerReceived = true;

		return buff;
}

void Handlers::MyRunEventHandler(sml::smlRunEventId, void* pUserData, sml::Agent*, sml::smlPhase)
{
    no_agent_assertTrue(pUserData);
    int* pInt = static_cast< int* >(pUserData);

    // Increase the count
    *pInt = *pInt + 1 ;
}

void Handlers::MyUpdateEventHandler(sml::smlUpdateEventId, void* pUserData, sml::Kernel*, sml::smlRunFlags)
{
    no_agent_assertTrue(pUserData);
    int* pInt = static_cast< int* >(pUserData);

    // Increase the count
    *pInt = *pInt + 1 ;
}

void Handlers::MyOutputNotificationHandler(void* pUserData, sml::Agent*)
{
    no_agent_assertTrue(pUserData);
    int* pInt = static_cast< int* >(pUserData);

    // Increase the count
    *pInt = *pInt + 1 ;
}

void Handlers::MyRunSelfRemovingHandler(sml::smlRunEventId, void* pUserData, sml::Agent* pAgent, sml::smlPhase)
{
    no_agent_assertTrue(pUserData);
    int* myCallback = static_cast< int* >(pUserData);

    // This callback removes itself from the list of callbacks
    // as a test to see if we can do that inside a callback handler.
    no_agent_assertTrue(*myCallback != -1);
    no_agent_assertTrue(pAgent->UnregisterForRunEvent(*myCallback));

    *myCallback = -1 ;
}

std::string Handlers::MyStringEventHandler(sml::smlStringEventId id, void* pUserData, sml::Kernel*, char const*)
{
    no_agent_assertTrue(pUserData);
    bool* pHandlerReceived = static_cast< bool* >(pUserData);
    *pHandlerReceived = true;

    // new: string events need to return empty string on success
    return "";
}

// Register a second handler for the same event, to make sure that's ok.
void Handlers::MyDuplicateRunEventHandler(sml::smlRunEventId, void* pUserData, sml::Agent*, sml::smlPhase)
{
    no_agent_assertTrue(pUserData);
    int* pInt = static_cast< int* >(pUserData);
    no_agent_assertTrue(*pInt == 25);
}

void Handlers::DebugPrintEventHandler(sml::smlPrintEventId, void* pUserData, sml::Agent*, char const* pMessage)
{
    std::cout << pMessage;
    std::cout.flush();
}

void Handlers::MyPrintEventHandler(sml::smlPrintEventId, void* pUserData, sml::Agent*, char const* pMessage)
{
    // In this case the user data is a string we're building up
    no_agent_assertTrue(pUserData);
    std::stringstream* pTrace = static_cast< std::stringstream* >(pUserData);

    (*pTrace) << pMessage ;
}

void Handlers::MyXMLEventHandler(sml::smlXMLEventId, void* pUserData, sml::Agent*, sml::ClientXML* pXML)
{
    // pXML should be some structured trace output.
    // Let's examine it a bit.
    // We'll start by turning it back into XML so we can look at it in the debugger.
    char* pStr = pXML->GenerateXMLString(true) ;
    no_agent_assertTrue(pStr);

    // This will always succeed.  If this isn't really trace XML
    // the methods checking on tag names etc. will just fail
    sml::ClientTraceXML* pRootXML = pXML->ConvertToTraceXML() ;
    no_agent_assertTrue(pRootXML);

    // The root object is just a <trace> tag.  The substance is in the children
    // so we'll get the first child which should exist.
    sml::ClientTraceXML childXML ;
    no_agent_assertTrue(pRootXML->GetChild(&childXML, 0));
    sml::ClientTraceXML* pTraceXML = &childXML ;

    if (pTraceXML->IsTagState())
    {
        no_agent_assertTrue(pTraceXML->GetDecisionCycleCount());
        std::string count = pTraceXML->GetDecisionCycleCount() ;

        no_agent_assertTrue(pTraceXML->GetStateID());
        std::string stateID = pTraceXML->GetStateID() ;

        no_agent_assertTrue(pTraceXML->GetImpasseObject());
        std::string impasseObject = pTraceXML->GetImpasseObject() ;

        no_agent_assertTrue(pTraceXML->GetImpasseType());
        std::string impasseType = pTraceXML->GetImpasseType() ;
    }

    // Make a copy of the object we've been passed which should remain valid
    // after the event handler has completed.  We only keep the last message
    // in this test.  This is a stress test for our memory allocation logic.
    // We're not allowed to keep pXML that we're passed, but we can copy it and keep the copy.
    // (The copy is very efficient, the underlying object is ref-counted).
    sml::ClientXML** clientXMLStorage = static_cast< sml::ClientXML** >(pUserData);;
    if (*clientXMLStorage != NULL)
    {
        delete *clientXMLStorage ;
    }
    *clientXMLStorage = new sml::ClientXML(pXML) ;

    pXML->DeleteString(pStr) ;
}

void Handlers::MyInterruptHandler(sml::smlRunEventId, void* pUserData, sml::Agent* pAgent, sml::smlPhase)
{
    pAgent->GetKernel()->StopAllAgents() ;

    no_agent_assertTrue(pUserData);
    bool* pHandlerReceived = static_cast< bool* >(pUserData);
    *pHandlerReceived = true;
}

const char *Handlers::MyRhsFunctionHandler(sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pArgument, int *buffSize, char *buff )
{
    // This is optional because the callback needs to be around for the program to function properly
    // we only test for this specifically in one part of clientsmltest
    if (pUserData)
    {
        bool* pHandlerReceived = static_cast< bool* >(pUserData);
        *pHandlerReceived = true;
    }

    std::stringstream res;
    res << "my rhs result " << pArgument;

		if ( res.str().size() + 1 > *buffSize )
		{
			*buffSize = res.str().size() + 1;
			return NULL;
		}
		strcpy( buff, res.str().c_str() );

		return buff;
}


const sml::RhsEventHandlerCPP Handlers::GetRhsFunctionHandlerCPP(bool* receivedFlag)
{
    return [receivedFlag](
        sml::smlRhsEventId,
        sml::Agent*,
        char const* pFunctionName,
        char const* pArgument) -> std::string
    {
        *receivedFlag = true;
        std::stringstream res;
        res << "my CPP rhs result " << pArgument;
        return res.str();
    };
}

void Handlers::MyMemoryLeakUpdateHandlerDestroyChildren(sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags)
{
    MyMemoryLeakUpdateHandlerInternal(true, id, pUserData, pKernel, runFlags);
}

void Handlers::MyMemoryLeakUpdateHandler(sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags)
{
    MyMemoryLeakUpdateHandlerInternal(false, id, pUserData, pKernel, runFlags);
}

void Handlers::MyMemoryLeakUpdateHandlerInternal(bool destroyChildren, sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags)
{
    static int step(0);

    static sml::Identifier* pRootID1(0);
    static sml::Identifier* pRootID2(0);
    static sml::StringElement* pRootString(0);
    static sml::FloatElement* pRootFloat(0);
    static sml::IntElement* pRootInt(0);

    static sml::Identifier* pChildID1(0);
    static sml::Identifier* pChildID2(0);
    static sml::Identifier* pChildID3(0);
    static sml::Identifier* pChildID4(0);
    static sml::StringElement* pChildString(0);
    static sml::FloatElement* pChildFloat(0);
    static sml::IntElement* pChildInt(0);

    static sml::Identifier* pSharedID(0);

    no_agent_assertTrue(pUserData);
    sml::Agent* pAgent = static_cast< sml::Agent* >(pUserData);

    //std::cout << "step: " << step << std::endl;

    switch (step % 3)
    {
        case 0:
            pRootID1 = pAgent->GetInputLink()->CreateIdWME("a") ;
            pRootID2 = pAgent->GetInputLink()->CreateIdWME("b") ;
            pRootString = pAgent->GetInputLink()->CreateStringWME("g", "gvalue") ;
            pRootFloat = pAgent->GetInputLink()->CreateFloatWME("h", 1.0) ;
            pRootInt = pAgent->GetInputLink()->CreateIntWME("i", 1) ;

            pChildID1 = pRootID1->CreateIdWME("c") ;
            pChildID2 = pRootID1->CreateIdWME("d") ;
            pChildID3 = pRootID2->CreateIdWME("e") ;
            pChildID4 = pRootID2->CreateIdWME("f") ;
            pChildString = pRootID1->CreateStringWME("j", "jvalue") ;
            pChildFloat = pRootID1->CreateFloatWME("k", 2.0) ;
            pChildInt = pRootID1->CreateIntWME("l", 2) ;

            pSharedID = pRootID1->CreateSharedIdWME("m", pChildID1);

            no_agent_assertTrue(pAgent->Commit());
            break;

        case 1:
            if (destroyChildren)
            {
                // Destroying the children should be unnecessary but should not be illegal
                no_agent_assertTrue(pChildID1->DestroyWME());
                no_agent_assertTrue(pChildID2->DestroyWME());
                no_agent_assertTrue(pChildID3->DestroyWME());
                no_agent_assertTrue(pChildID4->DestroyWME());

                // These three child leaks are not detected by looking at GetIWMObjMapSize
                // TODO: figure out how to detect these
                no_agent_assertTrue(pChildString->DestroyWME());
                no_agent_assertTrue(pChildFloat->DestroyWME());
                no_agent_assertTrue(pChildInt->DestroyWME());
            }

            // Destroying the original apparently destroys this.
            no_agent_assertTrue(pSharedID->DestroyWME());

            no_agent_assertTrue(pRootID1->DestroyWME());
            no_agent_assertTrue(pRootID2->DestroyWME());
            no_agent_assertTrue(pRootString->DestroyWME());
            no_agent_assertTrue(pRootFloat->DestroyWME());
            no_agent_assertTrue(pRootInt->DestroyWME());

            no_agent_assertTrue(pAgent->Commit());

            pRootID1 = 0;
            pRootID2 = 0;
            pRootString = 0;
            pRootFloat = 0;
            pRootInt = 0;

            pChildID1 = 0;
            pChildID2 = 0;
            pChildID3 = 0;
            pChildID4 = 0;
            pChildString = 0;
            pChildFloat = 0;
            pChildInt = 0;

            pSharedID = 0;
            break;

        default:
            break;
    }

    ++step;
}

void Handlers::MyCallStopOnUpdateEventHandler(sml::smlUpdateEventId, void*, sml::Kernel* pKernel, sml::smlRunFlags)
{
    pKernel->StopAllAgents();
}

void Handlers::MyAgentCreationUpdateEventHandler(sml::smlUpdateEventId, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags)
{
    no_agent_assertTrue(pUserData);
    RunningAgentData* pData = static_cast< RunningAgentData* >(pUserData);

    pData->count += 1;
    //std::cout << std::endl << "Update: " << pData->count;

    if (pData->count == 2)
    {
        pData->pOnTheFly = pKernel->CreateAgent("onthefly");
        no_agent_assertTrue_msg(pKernel->GetLastErrorDescription(), !pKernel->HadError());
        no_agent_assertTrue(pData->pOnTheFly);
        //std::cout << std::endl << "Created onthefly agent";
    }
}

void Handlers::MyOrderingPrintHandler(sml::smlPrintEventId /*id*/, void* pUserData, sml::Agent* /*pAgent*/, char const* pMessage)
{
    no_agent_assertTrue(pMessage);

    no_agent_assertTrue(pUserData);
    int* pInt = static_cast< int* >(pUserData);
    std::stringstream value;
    value << "pInt == " << *pInt;
//  std::cout << "Print handler " << value.str() << std::endl;
    no_agent_assertTrue_msg(value.str().c_str(), *pInt == 0 || *pInt == 2);
    ++(*pInt);
}

void Handlers::MyOrderingRunHandler(sml::smlRunEventId /*id*/, void* pUserData, sml::Agent* /*pAgent*/, sml::smlPhase /*phase*/)
{
    no_agent_assertTrue(pUserData);
    int* pInt = static_cast< int* >(pUserData);
    std::stringstream value;
    value << "Run handler " << "pInt == " << *pInt;
//  std::cout << value.str() << std::endl;
    no_agent_assertTrue_msg(value.str().c_str(), *pInt == 1 || *pInt == 3);
    ++(*pInt);
}

const char *Handlers::MyRhsFunctionFailureHandler(sml::smlRhsEventId, void*, sml::Agent*, char const*, char const* pArgument, int *buffSize, char *buff)
{
    std::ostringstream message;
    message << "test-failure handler called for: " << pArgument;
    no_agent_assertTrue_msg(message.str().c_str(), false);

		buff[0] = 0;
		return buff;
}

const char *Handlers::MySuccessHandler(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pFunctionName, char const* pArgument, int *buffSize, char *buff)
{
    no_agent_assertTrue(pUserData);
    bool* pHandlerReceived = static_cast< bool* >(pUserData);
    *pHandlerReceived = true;

		buff[0] = 0;
    return buff;
}
