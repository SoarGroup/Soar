/* File : sml_ClientInterface.i */
%module sml

%include "typemaps.i"
%apply char *OUTPUT { char* pResult }; // make sure SWIG knows this pattern indicates an output pointer

%{
#include "../ClientSML/include/sml_ClientWMElement.h"
#include "../ClientSML/include/sml_ClientIntElement.h"
#include "../ClientSML/include/sml_ClientFloatElement.h"
#include "../ClientSML/include/sml_ClientStringElement.h"
#include "../ClientSML/include/sml_ClientIdentifier.h"
#include "../ClientSML/include/sml_ClientAgent.h"
#include "../ClientSML/include/sml_ClientKernel.h"
%}

%include "../ClientSML/include/sml_ClientWMElement.h"
%include "../ClientSML/include/sml_ClientIntElement.h"
%include "../ClientSML/include/sml_ClientFloatElement.h"
%include "../ClientSML/include/sml_ClientStringElement.h"
%include "../ClientSML/include/sml_ClientIdentifier.h"
%include "../ClientSML/include/sml_ClientAgent.h"
%include "../ClientSML/include/sml_ClientKernel.h"






