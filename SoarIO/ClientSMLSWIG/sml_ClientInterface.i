/* File : sml_ClientInterface.i */

%rename(SetTagNameConst) sml::ElementXML::SetTagName(char const* tagName);
%rename(AddAttributeConst) sml::ElementXML::AddAttribute(char const* attributeName, char* attributeValue);
%rename(AddAttributeConstConst) sml::ElementXML::AddAttribute(char const* attributeName, char const* attributeValue);
%rename(SetCharacterDataConst) sml::ElementXML::SetCharacterData(char const* characterData);
%rename(SetBinaryCharacterDataConst) sml::ElementXML::SetBinaryCharacterData(char const* characterData, int length);

// These static function create a new Kernel object which should be destroyed later
// We need to let SWIG know this
%newobject sml::Kernel::CreateEmbeddedConnectionClientThread(char const*, bool, int);
%newobject sml::Kernel::CreateEmbeddedConnectionClientThread(char const*, bool);
%newobject sml::Kernel::CreateEmbeddedConnectionClientThread(char const*);
%newobject sml::Kernel::CreateEmbeddedConnectionSoarThread(char const*, int);
%newobject sml::Kernel::CreateEmbeddedConnectionSoarThread(char const*);
%newobject sml::Kernel::CreateRemoteConnection(bool, char const*, int);
%newobject sml::Kernel::CreateRemoteConnection(bool, char const*);

// don't wrap the code for registering callbacks because we need to provide some custom code to make it work
%ignore sml::Agent::RegisterForAgentEvent(smlAgentEventId, AgentEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForProductionEvent(smlProductionEventId, ProductionEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForRunEvent(smlRunEventId, RunEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForPrintEvent(smlPrintEventId, PrintEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForSystemEvent(smlSystemEventId, SystemEventHandler, void*, bool addToBack = true);

%{
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
%}

%include "../ConnectionSML/include/sml_ElementXML.h"
%include "../ConnectionSML/include/sml_AnalyzeXML.h"
%include "../ClientSML/include/sml_ClientErrors.h"
%include "../ClientSML/include/sml_ClientEvents.h"
%include "../ClientSML/include/sml_ClientWMElement.h"
%include "../ClientSML/include/sml_ClientIntElement.h"
%include "../ClientSML/include/sml_ClientFloatElement.h"
%include "../ClientSML/include/sml_ClientStringElement.h"
%include "../ClientSML/include/sml_ClientIdentifier.h"
%include "../ClientSML/include/sml_ClientKernel.h"
%include "../ClientSML/include/sml_ClientAgent.h"


