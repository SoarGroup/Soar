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
	ELEMENT_ID,
	ELEMENT_TYPE_TBD,
	//ELEMENT_LAST_TYPE = ELEMENT_TYPE_TBD
};

const std::string k_intString("int");
const std::string k_stringString("str");
const std::string k_floatString("flt");
const std::string k_idString("id");
const std::string k_onChangeString("onchange");
const std::string k_conditionString("cond");
const std::string k_cycleString("cycle");
const std::string k_TBD("To Be Determined at runtime");

eElementType		stringToType(const std::string& source);
const std::string	typeToString(eElementType type);

enum eUpdateFrequency
{
	UPDATE_NEVER,
	UPDATE_ON_CHANGE,			//TODO isn't this really a case of ON_CONDITION?
	UPDATE_ON_CONDITION, 
	UPDATE_EVERY_CYCLE
};

//A union is actually cleaner for this, but there was some nonsense that resulted
//Only one of these members will "exist" at a time, which fits better with the union concept
struct WMEValue
{
	//this is a string so that a variable name can be stored here temporarily, to be replaced
	//with the string representation of the integer value later
	std::string i;
	//int i;
	//this is a string so that a variable name can be stored here temporarily, to be replaced
	//with the string representation of the floating point value later	
	std::string f;
	//double f;
	std::string s;
	std::string id;
};

typedef std::vector<eElementType> typesContainter;
typedef typesContainter::iterator typesIterator;

class InputLinkObject
{

public:
	InputLinkObject();
	//InputLinkObject(std::string& inParent, std::string& inName, std::vector<eElementType>& inTypes, std::string& inValue);
	~InputLinkObject();

	void		addElementType(std::string& inType){m_elementTypes.push_back(stringToType(inType));}
	void		setParentId(std::string& inParent){m_parentId = inParent;}
	void		setAttribName(std::string& inName){m_attribName = inName;}
	void		setStartValue(std::string& inValue);
	void		setUpdateValue(std::string& inValue);
	void		setType(std::string& inValue);
	void		setType(const std::string& inValue);
	void		setType();
	int			getNumTypes() const {return m_elementTypes.size();}
	std::string	getFrequencyAsString() const;
	std::string	getUpdateValue() const {return m_updateValue;}
	std::string	getValue() const;

	void setUpdateFrequency(std::string& inValue);
	void setUpdateCondition(std::string& inValue);

	friend std::ostream& operator << (std::ostream& stream, InputLinkObject& obj);
	bool		hasBeenInspected() const {return m_beenInspected;}
	void		markAsInspected() {m_beenInspected = true;}


private:
	//std::ostream& printType(std::ostream&, eElementType);
	//std::ostream& printValue(std::ostream&);
	std::string				m_parentId;
	std::string				m_attribName;
	typesContainter			m_elementTypes;
	eElementType			m_curType;
	WMEValue				m_value;
	std::string				m_updateValue;
	eUpdateFrequency		m_updateFrequency;
	std::string				m_updateCondition;
	bool					m_beenInspected;
};




#endif //IL_OBJECT_H