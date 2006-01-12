#include "InputLinkObject.h"
#include <iostream>
#include <cassert>

using std::string;
using std::vector;
using std::cout; using std::endl;
using std::ostream;

extern void Pause();
/******************************************************************************
* ILObject Class Function Definitions
*
*
******************************************************************************
*/

InputLinkObject::InputLinkObject()
{
	m_updateFrequency	= UPDATE_NEVER;
	//m_beenInspected		= false;
}

/* Destructor
 *
 */
InputLinkObject::~InputLinkObject()
{
	m_elementTypes.clear();
}

string InputLinkObject::GetStartValue() const
{
	switch(m_curType)
	{
		case ELEMENT_STRING:
			return m_value.s;
			break;
		case ELEMENT_FLOAT:
			return m_value.f;
			break;
		case ELEMENT_ID:
			return m_value.id;
			break;
		case ELEMENT_INT:
			return m_value.i;
			break;
		case ELEMENT_TYPE_TBD:
			return k_TBD;
			break;
		default:
			assert(false);
			return "Wow...how did this happen?";
			break;
	}
}

void InputLinkObject::PrintTypes(ostream& stream)
{
	for(typesIterator_t tItr = m_elementTypes.begin(); tItr != m_elementTypes.end(); ++tItr)
	{
		stream << TypeToString(*tItr) << " ";
	}
}

ostream& operator << (ostream& stream, InputLinkObject& obj)
{
	stream << "Insertion operator printing an input link object..." << endl;
	stream << "\tParent: \t" << obj.m_parentId << endl;
	stream << "\tAttribute: \t" << obj.m_attribName << endl;
	for(typesIterator_t tItr = obj.m_elementTypes.begin(); tItr != obj.m_elementTypes.end(); ++tItr)
	{
		stream << "\tType: \t\t";
		//stream << obj.printType(stream, *tItr) << endl;
		switch(*tItr)//TODO consider using the print function, and storing these literals
		{
			case ELEMENT_FLOAT:
				stream << "float element";
				break;
			case ELEMENT_INT:
				stream << "int element";
				break;
			case ELEMENT_ID:
				stream << "identifier";
				break;
			case ELEMENT_STRING:
				stream << "string element";
				break;
			case ELEMENT_TYPE_TBD:
				stream << k_TBD;
				break;
			default:
				stream << "BAAAAD things have happened";
				assert(false);
				break;
		}
		stream << endl;
	}
	stream << "\tValue: \t\t";

	stream << obj.GetStartValue() << endl;
	stream << "\tFrequency: \t" << obj.GetFrequencyAsString() << endl;

	stream << endl;
	return stream;
}


void InputLinkObject::SetType(string& inValue)
{
	m_curType = StringToType(inValue);
}


void InputLinkObject::SetType(const string& inValue)
{
	m_curType = StringToType(inValue);
}


void InputLinkObject::SetType()
{
	assert(!m_elementTypes.empty());
	m_curType = m_elementTypes[0];
}


//All values are stored as string representation
void InputLinkObject::SetStartValue(string& inValue)
{
	//sanity check
	assert(!m_elementTypes.empty());

	switch(m_elementTypes[0])
	{
		case ELEMENT_STRING:
			m_value.s = inValue;
			break;
		case ELEMENT_INT:
			m_value.i = inValue; //atoi(inValue.c_str());
			break;
		case ELEMENT_FLOAT:	
			m_value.f = inValue; //atof(inValue.c_str());
			break;
		case ELEMENT_ID:
			m_value.id = inValue;
			break;
		default:
			assert(!"got unexpected enum type....");
			break;
	}
}

void InputLinkObject::SetUpdateValue(string& inValue)
{
	switch(m_curType)
	{
		case ELEMENT_INT:
		case ELEMENT_FLOAT:
		case ELEMENT_STRING:
			m_updateValue = inValue;
			break;
		case ELEMENT_ID:
			//TODO think about the case where a wme points to one id, and later to another.
			//Isn't this conceivable?  and in that case, the ID could need to be updated
			assert(!"ID type should never receive an update value");
			break;
		default:
			break;			
	}
}

void InputLinkObject::SetUpdateFrequency(string& inValue)
{
	if(inValue == k_onChangeString)
		m_updateFrequency = UPDATE_ON_CHANGE;
	else if(inValue == k_conditionString)
		m_updateFrequency = UPDATE_ON_CONDITION;
	else if(inValue == k_cycleString)
		m_updateFrequency = UPDATE_EVERY_CYCLE;
	else
		cout << "Got some unexpected freqency:>" << inValue << "<" << endl;
}

string InputLinkObject::GetFrequencyAsString() const
{
	switch(m_updateFrequency)
	{
		case UPDATE_EVERY_CYCLE:
			return "every cycle";
			break;
		case UPDATE_NEVER:
			return "never";
			break;
		case UPDATE_ON_CHANGE:
			return "on change";
			break;
		case UPDATE_ON_CONDITION:
			return "on condition";
			break;
		default:
			assert("object acquired some goofy, unexpected frequency" == 0);
			return "foo";//won't execute
			break;
	}
}
void InputLinkObject::ReplaceInternalTokens(const string& token, string& valueAsString)
{
	//cout << "ReplaceInternalTokens called...." << endl;
	//cout << "\ttoken to replace is: " << token << endl;
	//cout << "\tvalue to replace it with is " << valueAsString << endl;
	assert(token != "");//this has GOT to be trouble brewing...
	string::size_type pos =	m_updateValue.find(token);
	if(pos != string.npos)
	{
		//cout << "About to replace an update value's control variable reference with its string value..." << endl;
		//cout << "\tvariable: " << token << endl;
		//cout << "\tvariable's value: " << valueAsString << endl;
		//cout << "\tunchanged update value is: " << m_updateValue << endl;
		m_updateValue.replace(pos, token.size(), valueAsString);
		//cout << "\tand now the new value is: " << m_updateValue << endl;
	}//if the update value needs to be replaced
	//else
		//cout << "\tDidn't find token " << token << " in update value>" << m_updateValue << "<" << endl;

	//Look for the counter variable's name as a substring of the start value
	pos = GetStartValue().find(token);
	if(pos != string.npos)
	{
		string modifiedValue = GetStartValue();
		modifiedValue.replace(pos, token.size(), valueAsString);
		SetStartValue(modifiedValue);
	}//if the star start value needs to be replaced
	//else
		//cout << "\tDidn't find token " << token << " in start value>" << GetStartValue() << "<" << endl;
}

void InputLinkObject::SetUpdateCondition(string& inValue){	m_updateCondition = inValue;} 

//does a case-insensitive string comparison, mapping strings to enums
eElementType StringToType(const string& source)
{
	if(!stricmp(source.c_str(), k_intString.c_str()))
		return ELEMENT_INT;
	else if(!stricmp(source.c_str(), k_stringString.c_str()))
		return ELEMENT_STRING;
	else if(!stricmp(source.c_str(), k_floatString.c_str()))
		return ELEMENT_FLOAT;
	else if(!stricmp(source.c_str(), k_idString.c_str()))
		return ELEMENT_ID;
	else if(!stricmp(source.c_str(), k_TBD.c_str()))
		return ELEMENT_TYPE_TBD;
	else
	{
		cout << "stringToType: bad type found: " << source << endl;
		Pause();
		exit(-1);
	}
}

const string TypeToString(eElementType type)
{
	switch(type)
	{
	case ELEMENT_FLOAT:
		return k_floatString;
		break;
	case ELEMENT_INT:
		return k_intString;
		break;
	case ELEMENT_ID:
		return k_idString;
		break;
	case ELEMENT_STRING:
		return k_stringString;
		break;
	default:
		assert(false);
		return "BAAAAD things have happened";//never executes in debug
		break;
	}
}
