/* File : sml_ClientInterface.i */
// Settings common to all target languages

//
// SWIG support for std::string (https://www.swig.org/Doc4.1/Library.html#Library_std_string)
//
%include std_string.i

// handle windows calling convention, __declspec(dllimport), correctly
// see https://www.swig.org/Doc4.1/Windows.html#Windows_interface_file
%include <windows.i>

// Language-independent exception handler to wrap ALL functions with
// See https://www.swig.org/Doc4.2/SWIGDocumentation.html#Customization_nn7
// As more exceptions are added to the codebase, we should add translations here
%include exception.i
%exception {
    try {
        $action
    }
    catch(const std::invalid_argument& e) {
        SWIG_exception(SWIG_ValueError, e.what());
    }
    catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
    catch(...) {
        SWIG_exception(SWIG_RuntimeError, "Unknown exception");
    }
}

//
// These functions need to be renamed because they only differ by a const type, which isn't enough to distinguish them
//
%rename(SetTagNameConst) soarxml::ElementXML::SetTagName(char const* tagName);
%rename(AddAttributeConst) soarxml::ElementXML::AddAttribute(char const* attributeName, char* attributeValue);
%rename(AddAttributeConstConst) soarxml::ElementXML::AddAttribute(char const* attributeName, char const* attributeValue);
%rename(SetCharacterDataConst) soarxml::ElementXML::SetCharacterData(char const* characterData);
%rename(SetBinaryCharacterDataConst) soarxml::ElementXML::SetBinaryCharacterData(char const* characterData, int length);

// These static functions create a new Kernel object that should be destroyed later.

%newobject sml::Kernel::CreateKernelInCurrentThread;
%newobject sml::Kernel::CreateKernelInNewThread;
%newobject sml::Kernel::CreateRemoteConnection;

//
// These static functions generate a new char* object that should be destroyed later
// We also need to tell SWIG how to delete the object (hence the typemaps)
//
%newobject sml::ClientXML::GenerateXMLString(bool) const ;
%newobject soarxml::ElementXML::GenerateXMLString(bool) const ;
%newobject sml::AnalyzeXML::GenerateXMLString(bool) const ;
%newobject sml::ClientAnalyzedXML::GenerateXMLString(bool) const ;

%newobject sml::ClientXML::GenerateXMLString(bool, bool) const ;
%newobject soarxml::ElementXML::GenerateXMLString(bool, bool) const ;
%newobject sml::AnalyzeXML::GenerateXMLString(bool, bool) const ;
%newobject sml::ClientAnalyzedXML::GenerateXMLString(bool, bool) const ;

%typemap(newfree) char* sml::ClientXML::GenerateXMLString {
    sml::ClientXML::DeleteString($1);
}
%typemap(newfree) char* GenerateXMLString {
    soarxml::ElementXML::DeleteString($1);
}
%typemap(newfree) char* sml::AnalyzeXML::GenerateXMLString {
    sml::AnalyzeXML::DeleteString($1);
}
%typemap(newfree) char* sml::ClientAnalyzedXML::GenerateXMLString {
    sml::ClientAnalyzedXML::DeleteString($1);
}

//
// This parsing method returns a new ElementXML object that should be destroyed later
//
%newobject soarxml::ElementXML::ParseXMLFromFile;
%newobject soarxml::ElementXML::ParseXMLFromString;

//
// This deletes the passed object so it is a destructor of sorts
//
%delobject soarxml::ElementXML::AddChild;

//
// Don't wrap the code for registering callbacks because we need to provide some custom code to make it work
//
%ignore sml::Agent::RegisterForProductionEvent(smlProductionEventId, ProductionEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForRunEvent(smlRunEventId, RunEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForPrintEvent(smlPrintEventId, PrintEventHandler, void*, bool ignoreOwnEchos = true,  bool addToBack = true);
%ignore sml::Agent::RegisterForXMLEvent(smlXMLEventId, XMLEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForOutputNotification(OutputNotificationHandler, void*, bool addToBack = true);
%ignore sml::Agent::AddOutputHandler(char const*, OutputEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForSystemEvent(smlSystemEventId, SystemEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForAgentEvent(smlAgentEventId, AgentEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForUpdateEvent(smlUpdateEventId, UpdateEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForStringEvent(smlStringEventId, StringEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::AddRhsFunction(char const*, RhsEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForClientMessageEvent(char const*, RhsEventHandler, void*, bool addToBack = true);

%ignore sml::Agent::UnregisterForRunEvent(int);
%ignore sml::Agent::UnregisterForProductionEvent(int);
%ignore sml::Agent::UnregisterForPrintEvent(int);
%ignore sml::Agent::UnregisterForXMLEvent(int);
%ignore sml::Agent::UnregisterForOutputNotification(int);
%ignore sml::Agent::RemoveOutputHandler(int);
%ignore sml::Kernel::UnregisterForSystemEvent(int);
%ignore sml::Kernel::UnregisterForUpdateEvent(int);
%ignore sml::Kernel::UnregisterForStringEvent(int);
%ignore sml::Kernel::UnregisterForAgentEvent(int);
%ignore sml::Kernel::RemoveRhsFunction(int);
%ignore sml::Kernel::UnregisterForClientMessageEvent(int);

// meant for C++ (non-SWIG) clients only
%ignore sml::Kernel::AddRhsFunction(char const*, RhsEventHandlerCpp, bool addToBack = true);
%ignore sml::Kernel::RegisterForClientMessageEvent(char const*, RhsEventHandlerCpp, void*, bool addToBack = true);

// Internal classes not useful to the user
%ignore sml::EventHandlerPlusData;
%ignore sml::RhsEventHandlerPlusData;
%ignore sml::AgentEventHandlerPlusData;
%ignore sml::StringEventHandlerPlusData;
%ignore sml::UpdateEventHandlerPlusData;
%ignore sml::SystemEventHandlerPlusData;
%ignore sml::XMLEventHandlerPlusData;
%ignore sml::OutputNotificationHandlerPlusData;
%ignore sml::RunEventHandlerPlusData;
%ignore sml::ProductionEventHandlerPlusData;
%ignore sml::PrintEventHandlerPlusData;
%ignore sml::OutputEventHandlerPlusData;

//
// Tell SWIG to include these files in the generated wrapper code
//
%{
#include "portability.h"
#include "sml_Names.h"
#include "ElementXML.h"
#include "sml_AnalyzeXML.h"
#include "sml_Events.h"
#include "sml_ClientErrors.h"
#include "sml_ClientEvents.h"
#include "sml_ClientWMElement.h"
#include "sml_ClientIntElement.h"
#include "sml_ClientFloatElement.h"
#include "sml_ClientStringElement.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientKernel.h"
#include "sml_ClientAgent.h"
#include "sml_ClientXML.h"
#include "sml_ClientTraceXML.h"
#include "sml_ClientAnalyzedXML.h"
#include "soar_instance.h"
%}
// name this () operator from soar_instance.h so it can be wrapped
%rename(compare) cmp_str::operator();

//
// Override EXPORT macro
//
#define EXPORT

//
// Tell SWIG to wrap these files
//
%include "sml_Names.h"
%include "ElementXML.h"
%include "sml_AnalyzeXML.h"
%include "sml_Events.h"
%include "sml_ClientErrors.h"
%include "sml_ClientEvents.h"
%include "sml_ClientWMElement.h"
%include "sml_ClientIntElement.h"
%include "sml_ClientFloatElement.h"
%include "sml_ClientStringElement.h"
%include "sml_ClientIdentifier.h"
%include "sml_ClientKernel.h"
%include "sml_ClientAgent.h"
%include "sml_ClientXML.h"
%include "sml_ClientTraceXML.h"
%include "sml_ClientAnalyzedXML.h"
%include "soar_instance.h"


%{
// Check for memory leaks
#if defined(_DEBUG) && defined(_WIN32)
#define _CRTDBG_MAP_ALLOC
#include "sml_Utils.h"
#include <stdlib.h>
#include <crtdbg.h>

bool __stdcall DllMain( void * hModule,
                       unsigned long  ul_reason_for_call,
                       void * lpReserved
					 )
{
	//_crtBreakAlloc = 142;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	unused(hModule) ;
	unused(ul_reason_for_call) ;
	unused(lpReserved) ;

    return 1;
}
#endif
%}
