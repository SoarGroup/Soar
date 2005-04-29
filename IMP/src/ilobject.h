#ifndef IL_OBJECT_H
#define IL_OBJECT_H

#include <vector>
#include <string>
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

eElementType stringToType(std::string& source);

enum eUpdateFrequency
{
  UPDATE_ON_CHANGE,
  UPDATE_ON_CONDITION, 
  UPDATE_EVERY_CYCLE
};

class InputLinkObject
{

public:
	//InputLinkObject(std::string& inParent, std::string& inName, std::vector<eElementType>& inTypes, std::string& inValue);
	~InputLinkObject();
	void addElementType(eElementType inType){m_elementTypes.push_back(inType);}
	void setParentId(std::string& inParent){m_parentId = inParent;}
	void setAttribName(std::string& inName){m_attribName = inName;}
	void setStartValue(std::string& inValue){m_startingValue = inValue;}
private:
	std::string m_parentId;
	std::string m_attribName;
	std::vector <eElementType> m_elementTypes;
	std::string m_startingValue;
	//std::string updateValue;
	
};




#endif //IL_OBJECT_H