#ifndef IL_SPEC_H
#define IL_SPEC_H

#include <string>

using namespace std;

class InputLinkObject;

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

	enum parseStage {
		READING_CONTROL_STRUCTURE,
		READING_IDENTIFIER,
		READING_ATTRIBUTE,
		READING_VALUE_TYPE,
		READING_START_VALUE,
		READING_UPDATE,
		READING_CREATE_ON,
		READING_DELETE_ON,
		READING_COMPLETED,
	};

public:

	InputLinkSpec();
	~InputLinkSpec();

	bool ImportIL(string filename);
	bool ImportDM(string filename);

};







#endif //IL_SPEC_H
