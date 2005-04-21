#include "ilspec.h"
#include "ilobject.h"
#include <fstream>
#include <iostream>

/******************************************************************************
 * InputLinkSpec Class Function Definitions
 *
 *
 ******************************************************************************
 */

/* Default Constructor
 *
 * Creates an InputLinkSpec object.  Not necessary yet. 
 */
InputLinkSpec::InputLinkSpec()
{
	//necessary yet?
}

/* Deconstructaur
 *
 * Cleans up an InputLinkSpec object.  Not necessary yet.  
 */
InputLinkSpec::~InputLinkSpec()
{
	//necessary yet?
}


/* ImportDM
 *
 * This function creates an input link specification from the datamap
 * contained within "filename"
 * Returns true on success, false on failure.
 */
bool InputLinkSpec::ImportDM(string filename)
{
	fstream file;
	file.open(filename.c_str());

	if(!file.is_open())
	{
		cout<<"Error: unable to open file "<<filename<<endl;
		return false;
	}

	file.close();
	return true;
}

void InputLinkSpec::ReadControlStructure()
{

}

/* ImportIL
 *
 * This function creates an input link specification from the IL file passed.
 * Returns true on success, false on failure.
 */
bool InputLinkSpec::ImportIL(string filename)
{
	fstream file;
	file.open(filename.c_str());

	if(!file.is_open())
	{
		cout<<"Error: unable to open file "<<filename<<endl;
		return false;
	}

	int lineNumber = 1;

	//this should create InputLinkObjects to hold each line of data read
	while(!file.eof())
	{
		//Probably don't want to get a whole line at once
		/*char line[MAX_LINE_LENGTH];

		file.getline(line, MAX_LINE_LENGTH+1);

		//ensure that the line wasn't too long
		if(file.gcount() == MAX_LINE_LENGTH+1)
		{
		cout<<"Error: line "<<lineNumber<<" too long"<<endl;
		return false;
		}*/
//dev's notes

		char buf[MAX_LINE_LENGTH];

		//this should probably use Devvan's enum thinger.
		eParseStage parseStage = READING_BEGIN_STAGE;
		switch(parseStage)
		{
		case READING_CONTROL_STRUCTURE:
			break;
		case READING_IDENTIFIER:
			break;
		case READING_ATTRIBUTE:
			break;
		case READING_VALUE_TYPE:
			break;
		case READING_START_VALUE:
			break;
		case READING_UPDATE:
			break;
		case READING_CREATE_ON:
			break;
		case READING_DELETE_ON:
			break;
		case READING_COMPLETED;
		}


		//check if this line is a -for loop  (ignore for now)

		//should first get parent id name
		file.get(buf, ' ');

		cout<<buf<<endl;

		//then get attribute name

		//then get value type 

		//then optional and conditional args.

		++lineNumber;
	}

	file.close();
	return true;
}

