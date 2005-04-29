#include "ilobject.h"
#include <iostream>

using std::string;
using std::vector;
using std::cout;

/******************************************************************************
* ILObject Class Function Definitions
*
*
******************************************************************************
*/

/*InputLinkObject::InputLinkObject(string inParent, string inName, vector<eElementType>& inTypes, string inValue)
		: m_parentId(inParent), m_attribName(inName), m_elementTypes(inTypes), m_startingValue(inValue)
{
	
}*/

/* Destructor
 *
 */
InputLinkObject::~InputLinkObject()
{
	m_elementTypes.clear();
}

eElementType stringToType(string& source)
{
	if(!stricmp(source.c_str(), k_intString.c_str()))
		return ELEMENT_INT;
	else if(!stricmp(source.c_str(), k_stringString.c_str()))
		return ELEMENT_STRING;
	else if(!stricmp(source.c_str(), k_floatString.c_str()))
		return ELEMENT_FLOAT;
	else if(!stricmp(source.c_str(), k_idString.c_str()))
		return ELEMENT_ID;
	else
	{
		cout << "bad type: " << source << " found." << std::endl;
		string foo;
		std::cin >> foo;
		exit(-1);
	}
}