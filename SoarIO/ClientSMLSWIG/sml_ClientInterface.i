/* File : sml_ClientInterface.i */

%rename(SetTagNameConst) sml::ElementXML::SetTagName(char const* tagName);
%rename(AddAttributeConst) sml::ElementXML::AddAttribute(char const* attributeName, char* attributeValue);
%rename(AddAttributeConstConst) sml::ElementXML::AddAttribute(char const* attributeName, char const* attributeValue);
%rename(SetCharacterDataConst) sml::ElementXML::SetCharacterData(char const* characterData);
%rename(SetBinaryCharacterDataConst) sml::ElementXML::SetBinaryCharacterData(char const* characterData, int length);

%newobject sml::Kernel::CreateEmbeddedConnection(char const*, bool, bool, int);
%newobject sml::Kernel::CreateEmbeddedConnection(char const*, bool, bool);
%newobject sml::Kernel::CreateEmbeddedConnection(char const*, bool);
%newobject sml::Kernel::CreateRemoteConnection(bool, char const*, int);
%newobject sml::Kernel::CreateRemoteConnection(bool, char const*);

%ignore sml::Agent::RegisterForAgentEvent(smlEventId, AgentEventHandler, void*);
%ignore sml::Agent::UnregisterForAgentEvent(smlEventId, AgentEventHandler, void*);
%ignore sml::Agent::RegisterForProductionEvent(smlEventId, ProductionEventHandler, void*);
%ignore sml::Agent::UnregisterForProductionEvent(smlEventId, ProductionEventHandler, void*);
%ignore sml::Agent::RegisterForRunEvent(smlEventId, RunEventHandler, void*);
%ignore sml::Agent::UnregisterForRunEvent(smlEventId, RunEventHandler, void*);
%ignore sml::Agent::RegisterForPrintEvent(smlEventId, PrintEventHandler, void*);
%ignore sml::Agent::UnregisterForPrintEvent(smlEventId, PrintEventHandler, void*);
%ignore sml::Kernel::RegisterForSystemEvent(smlEventId, SystemEventHandler, void*);
%ignore sml::Kernel::UnregisterForSystemEvent(smlEventId, SystemEventHandler, void*);


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


