#ifndef IL_OBJECT_H
#define IL_OBJECT_H

#include <vector>
#include <string>
#include <iostream>
/************************************************************************
* ILObject
* 
* This class contains the necessary data and methods for encapsulating
* a specific line entry as given to the IMP program by the user.  
*************************************************************************
*/

enum eElementType
{
	ELEMENT_STRING,
	ELEMENT_INT,
	ELEMENT_FLOAT,
	ELEMENT_ID
};

const std::string k_intString("int");
const std::string k_stringString("str");
const std::string k_floatString("flt");
const std::string k_idString("id");
const std::string k_onChangeString("onchange");
const std::string k_conditionString("cond");
const std::string k_cycleString("cycle");
const std::string k_TBD("To Be Determined at runtime");

eElementType	stringToType(const std::string& source);
const std::string		typeToString(eElementType type);

enum eUpdateFrequency
{
  UPDATE_ON_CHANGE,
  UPDATE_ON_CONDITION, 
  UPDATE_EVERY_CYCLE
};

struct WMEValue //a union is actually cleaner, but there was some nonsense that resulted
{
	int* i;
	double* f;
	std::string* s;
	std::string* id;
};

typedef std::vector<eElementType> typesContainter;
typedef typesContainter::iterator typesIterator;

class InputLinkObject
{

public:
	InputLinkObject();
	//InputLinkObject(std::string& inParent, std::string& inName, std::vector<eElementType>& inTypes, std::string& inValue);
	~InputLinkObject();
	//TODO prolly should make this take in a string as well
	void				addElementType(std::string& inType){m_elementTypes.push_back(stringToType(inType));}
	void				setParentId(std::string& inParent){m_parentId = inParent;}
	void				setAttribName(std::string& inName){m_attribName = inName;}
	void				setStartValue(std::string& inValue);
	void				setUpdateValue(std::string& inValue);
	void				setType(std::string& inValue);
	void				setType(const std::string& inValue);
	int					getNumTypes() const {return m_elementTypes.size();}
	eElementType	getFirstType() /*const*/ {return m_elementTypes[0];}

	void setUpdateFrequency(std::string& inValue);
	void setUpdateCondition(std::string& inValue);
	
	friend std::ostream& operator << (std::ostream& stream, InputLinkObject& obj);
	
private:
	//std::ostream& printType(std::ostream&, eElementType);
	//std::ostream& printValue(std::ostream&);
	std::string				m_parentId;
	std::string				m_attribName;
	typesContainter		m_elementTypes;
	eElementType			m_curType;
	WMEValue					m_value;
	std::string				m_updateValue;
	eUpdateFrequency	m_updateFrequency;
	std::string				m_updateCondition;
};




#endif //IL_OBJECT_H