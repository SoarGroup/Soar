/* File : sml_ClientWMElement.i */
%module sml

%{
#include "../ClientSML/include/sml_ClientWMElement.h"

namespace sml {
	class Identifier;
}

using namespace sml;

%}

%include "../ClientSML/include/sml_ClientWMElement.h"