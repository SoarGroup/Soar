#ifndef ILINK_ABSTRACTION_SPEC_MODULE
#define ILINK_ABSTRACTION_SPEC_MODULE

#include <string>

using namespace std;

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
	bool ImportIL(string filename);
	bool ImportDM(string filename);


};







#endif ILINK_ABSTRACTION_SPEC_MODULE
