/* File : sml_ClientInterface.i */
%module sml

%{
#include "../ClientSML/include/sml_ClientWMElement.h"
#include "../ClientSML/include/sml_ClientIntElement.h"
#include "../ClientSML/include/sml_ClientFloatElement.h"
#include "../ClientSML/include/sml_ClientStringElement.h"
#include "../ClientSML/include/sml_ClientIdentifier.h"
#include "../ClientSML/include/sml_ClientAgent.h"
#include "../ClientSML/include/sml_ClientKernel.h"
#include "../ElementXML/include/ElementXMLInterface.h"
%}

%include "../ClientSML/include/sml_ClientWMElement.h"
%include "../ClientSML/include/sml_ClientIntElement.h"
%include "../ClientSML/include/sml_ClientFloatElement.h"
%include "../ClientSML/include/sml_ClientStringElement.h"
%include "../ClientSML/include/sml_ClientIdentifier.h"
%include "../ClientSML/include/sml_ClientAgent.h"
%include "../ClientSML/include/sml_ClientKernel.h"
%include "../ElementXML/include/ElementXMLInterface.h"




