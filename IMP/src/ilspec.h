#ifndef IL_SPEC_H
#define IL_SPEC_H

#include <string>

#define MAX_IMP_LINE_LENGTH 4096

class InputLinkObject;

//The ordering here is important
enum eParseStage
{
	READING_PRE_BEGIN,
	READING_ERROR,
	READING_BEGIN_STAGE, 
	READING_CONTROL_STRUCTURE = READING_BEGIN_STAGE,		
	READING_IDENTIFIER,
	READING_ATTRIBUTE,
	READING_VALUE_TYPE,
	READING_START_VALUE,
	READING_UPDATE,
	READING_CREATE_ON,
	READING_DELETE_ON,
	READING_FINAL_STAGE = READING_DELETE_ON
};

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


public:

	InputLinkSpec();
	~InputLinkSpec();

	bool ImportIL(std::string& filename);
	bool ImportDM(std::string& filename);
	void ReadControlStructure();
};







#endif //IL_SPEC_H
