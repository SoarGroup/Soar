/* File : sml_ClientAgent.i */
%module sml

%{
#include "../ClientSML/include/sml_ClientAgent.h"

namespace sml {
	class Identifier;
	class StringElement;
}

using namespace sml;

%}

%include "../ClientSML/include/sml_ClientAgent.h"