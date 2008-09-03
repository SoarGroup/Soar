/* File : sml_ClientInterface.i */

%include std_string.i

%rename(SetTagNameConst) sml::ElementXML::SetTagName(char const* tagName);
%rename(AddAttributeConst) sml::ElementXML::AddAttribute(char const* attributeName, char* attributeValue);
%rename(AddAttributeConstConst) sml::ElementXML::AddAttribute(char const* attributeName, char const* attributeValue);
%rename(SetCharacterDataConst) sml::ElementXML::SetCharacterData(char const* characterData);
%rename(SetBinaryCharacterDataConst) sml::ElementXML::SetBinaryCharacterData(char const* characterData, int length);

// These static function create a new Kernel object which should be destroyed later
// We need to let SWIG know this
%newobject sml::Kernel::CreateKernelInCurrentThread;
%newobject sml::Kernel::CreateKernelInNewThread;
%newobject sml::Kernel::CreateRemoteConnection;

// This function also creates a new object, but we need to tell SWIG how to delete it
%typemap(newfree) char* GenerateXMLString {
    sml::ClientXML::DeleteString($1);
}
%newobject sml::ClientXML::GenerateXMLString(bool) const ;

%typemap(newfree) char* GenerateXMLString {
    sml::ElementXML::DeleteString($1);
}
%newobject sml::ElementXML::GenerateXMLString(bool) const ;

// This function also creates a new object, but we need to tell SWIG how to delete it
%typemap(newfree) char* GenerateXMLString {
    sml::AnalyzeXML::DeleteString($1);
}
%newobject sml::AnalyzeXML::GenerateXMLString(bool) const ;

// This parsing method returns a new ElementXML object
%newobject sml::ElementXML::ParseXMLFromString;

// don't wrap the code for registering callbacks because we need to provide some custom code to make it work
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

%{
#include "sml_Names.h"
#include "sml_ElementXML.h"
#include "sml_AnalyzeXML.h"
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
%}

%include "sml_Names.h"
%include "sml_ElementXML.h"
%include "sml_AnalyzeXML.h"
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

