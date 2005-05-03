#include "ilobject.h"
#include <iostream>
#include <cassert>

using std::string;
using std::vector;
using std::cout;

extern void pause();
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

InputLinkObject::InputLinkObject()
{
	m_value.i		= NULL;
	m_value.id	= NULL;
	m_value.s		= NULL;
	m_value.f		= NULL;
}

/* Destructor
 *
 */
InputLinkObject::~InputLinkObject()
{
	m_elementTypes.clear();
	if(m_value.i	!= NULL) delete m_value.i;
	if(m_value.id != NULL) delete m_value.id;
	if(m_value.s	!= NULL) delete m_value.id;
	if(m_value.f	!= NULL) delete m_value.id;
}

void InputLinkObject::setType(string& inValue)
{
	m_curType = stringToType(inValue);
}

void InputLinkObject::setStartValue(string& inValue)
{
	//sanity checks
	assert(!m_elementTypes.empty());
	assert(m_value.i == NULL);
	assert(m_value.id == NULL);
	assert(m_value.f == NULL);
	assert(m_value.s == NULL);

	switch(m_elementTypes[0])
	{
		case ELEMENT_STRING:
			m_value.s = new string(inValue);
			break;
		case ELEMENT_INT:
			*m_value.i = atoi(inValue.c_str());
			break;
		case ELEMENT_FLOAT:
			*m_value.f = atof(inValue.c_str());
			break;
		case ELEMENT_ID:
			m_value.id = new string(inValue);
			break;
		default:
			assert(!"got unexpected enum type....");
			break;
	}
}

void InputLinkObject::setUpdateValue(std::string& inValue)
{
	switch(m_curType)
	{
		case ELEMENT_INT:
		case ELEMENT_FLOAT:
		case ELEMENT_STRING:
			m_updateValue = inValue;
			break;
		case ELEMENT_ID:
			//TODO think about the case where an wme points to one id, and later to another.
			//isn't this conceivable?  and in that case, the ID could need to be updated
			assert(!"ID type should never receive an update value");
			break;
		default:
			break;			
	}
}
//does a case-insensitive string comparison, mapping strings to enums
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
		cout << "bad type found in 'stringToType' >" << source << "< found." << std::endl;
		pause();
		exit(-1);
	}
}