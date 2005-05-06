#ifndef IL_SPEC_H
#define IL_SPEC_H

#include <string>
#include <vector>

#define MAX_IMP_LINE_LENGTH 4096

class InputLinkObject;

//TODO mark the ones that are optional
//The ordering here is important
enum eParseStage
{
	READING_PRE_BEGIN,
	READING_ERROR,
	READING_BEGIN_STAGE,
	READING_CONTROL_STRUCTURE = READING_BEGIN_STAGE,
	READING_PARENT_IDENTIFIER,
	READING_IDENTIFIER_UNIQUE_NAME,
	READING_ATTRIBUTE,
	READING_VALUE_TYPE,
	READING_START_VALUE,
	READING_UPDATE_VALUE,
	READING_UPDATE_FREQUENCY,
	READING_CREATE_ON,
	READING_DELETE_ON,
	READING_FINAL_STAGE //= READING_DELETE_ON
};

const std::string k_forToken				= "-for";
const std::string k_typesOpenToken	= "<";
const std::string k_typesCloseToken	= ">";
const std::string k_startToken			= "-start";
const std::string k_updateToken			= "-update";
const std::string k_frequencyToken	= "-freq";

typedef std::vector<InputLinkObject> ilObjVector_t;
typedef ilObjVector_t::iterator ilObjItr;

/************************************************************************
 * InputLinkSpec
 * 
 * This class contains the necessary data and methods for encapsulating
 * an input link specification as given to the IMP program.
 *************************************************************************
 */
class InputLinkSpec
{
private:
	ilObjVector_t ilObjects;
	void ReadControlStructure();
public:

	InputLinkSpec();
	~InputLinkSpec();
	//The import functions parse out pieces of the input 
	//link and store them in objects so that the actual input link
	//code can be generated
	bool ImportIL(std::string& filename);
	bool ImportDM(std::string& filename);

};




#endif //IL_SPEC_H
