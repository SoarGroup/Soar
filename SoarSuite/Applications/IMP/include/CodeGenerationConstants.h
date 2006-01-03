#ifndef IMP_CODE_GENERATION_CONSTANTS
#define IMP_CODE_GENERATION_CONSTANTS

#include <string>

const std::string k_SML_Agent					= "sml_ClientAgent";

const std::string k_Create						= "Create";
const std::string k_CreateStringWME		= "CreateStringWME";
const std::string k_CreateIntWME			= "CreateIntWME";
const std::string k_CreateFloatWME		= "CreateFloatWME";
const std::string k_CreateIdWME				= "CreateIdWME";
const std::string k_GetValue					= "GetValue";
const std::string k_Identifier				= "Identifier";
const std::string k_pIdentifier				= "Identifier*";
const std::string k_IntElement				= "IntElement";
const std::string k_pIntElement				= "IntElement*";
const std::string k_FloatElement			= "FloatElement";
const std::string k_pFloatElement			= "FloatElement*";
const std::string k_StringElement			= "StringElement";
const std::string k_pStringElement		= "StringElement*";
const std::string k_CreateSharedIdWME = "CreateSharedIdWME";
const std::string k_Update						=	"Update";
const std::string k_Destroy						= "Destroy";
const std::string k_DestroyWME				= "DestroyWME";
const std::string k_DestroyAgent			= "DestroyAgent";
const std::string k_GetInputLink			= "GetInputLink";
const std::string k_Commit						= "Commit";
const std::string k_GetKernel					= "GetKernel";
const std::string k_GetAgent					= "GetAgent";
const std::string k_Agent							= "Agent ";
const std::string k_AgentRef					= "Agent& ";
const std::string k_AgentInstance			= "agent";
const std::string k_AgentInstanceDot	= "agent.";
const std::string k_IMP								= "IMP";
const std::string k_SML								= "sml";

const std::string k_CreateInputLink 	= "CreateInputLink";
const std::string k_UpdateInputLink		= "UpdateInputLink";
const std::string k_CleanUp						= "CleanUp";

const std::string k_void				= "void ";
const std::string k_import			= "import ";
const std::string k_using				= "using ";
const std::string k_namespace		= "namespace ";
const std::string k_push				= "push_back";
const std::string k_if					= "if(";
const std::string k_scope				= "::";
const std::string k_hExtension	= ".h";
const std::string k_include			= "#include ";
const std::string k_semi				=	";";
const std::string k_openParen		= "(";
const std::string k_closeParen	= ")";
const std::string	k_openBrace		= "{";
const std::string k_closeBrace	= "}";
const std::string k_openAngleBrace	= "<";
const std::string k_clostAngleBrace = ">";
const std::string k_notEqual		= "!=";
const std::string k_argSep			= ",";
const std::string k_dQuote			= "\"";
const std::string k_space				= " ";
const std::string k_dot					= ".";
const std::string k_arrow				= "->";
const std::string k_andpersand	= "&";
const std::string k_assign			= "=";

const std::string k_ILRootToken = "ILRoot";

#endif IMP_CODE_GENERATION_CONSTANTS