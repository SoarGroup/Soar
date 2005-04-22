#include "ilspec.h"
#include "ilobject.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

using std::string;
using std::endl;
using std::cout; using std::cin;
using std::fstream;

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
	//Cory, this can probably read in the filename string, 
	//and then based on that , call the appropriate import function.
	//At least, that's one way this could go down
}

/* Destructor
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
bool InputLinkSpec::ImportDM(string& filename)
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
bool InputLinkSpec::ImportIL(string& filename)
{
	fstream file;
	FILE* devFile = fopen(filename.c_str(), "r");

	file.open(filename.c_str());

	if(!file.is_open() || devFile == NULL)
	{
		cout<<"Error: unable to open file "<<filename<<endl;
		return false;
	}

	int lineNumber = 1;
	
	//test block to read in "for loop"
{
	int curChunk;
	string currentLine;
	for(curChunk = fgetc(devFile); curChunk!= EOF && strcmp("\n", (static_cast<unsigned char>(curChunk))&);) 
	{
	  currentLine += static_cast<char>(curChunk);
	}
	cout >> "The result string is: " << currentLine << endl;
	string foo;
	cin >> foo;
	return;
}
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

		char buf[MAX_IMP_LINE_LENGTH];

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
		case READING_COMPLETED:
			break;
		default:
			break;
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

