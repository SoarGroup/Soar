#include "ilspec.h"
#include "ilobject.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using std::string;
using std::endl;
using std::cout; using std::cin;
using std::fstream; using std::ostream;

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

extern void pause();
const int defaultLoopBegin = 0;
const int defaultLoopEnd = 1;

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



//This is not quite cool.  The utility will remain the same, 
//but this may need to be a member
void printStage(eParseStage stage, ostream& stream)
{
	switch(stage)
	{
		case READING_PRE_BEGIN:
			stream << "'Before Reading'" << endl;
			break;
		case READING_CONTROL_STRUCTURE:
			stream << "'Reading control structure'" << endl;
			break;
		case READING_IDENTIFIER:
			stream << "'Reading identifier'" << endl;
			break;
		case READING_ATTRIBUTE:
			stream << "'Reading attribute'" << endl;
			break;
		case READING_VALUE_TYPE:
			stream << "'Reading value type'" << endl;
			break;
		case READING_START_VALUE:
			stream << "'Reading start value'" << endl;
			break;
		case READING_UPDATE:
			stream << "'Reading update'" << endl;
			break;
		case READING_CREATE_ON:
			stream << "'Reading 'create on' condition'" << endl;
			break;
		case READING_DELETE_ON:
			stream << "'Reading \"delete on\" condition'" << endl;
			break;
		case READING_ERROR:
			stream << "'Error in parse'" << endl;
			break;
		default:
			break;
	}
}

//TODO comment
string readOneWord(const string& source)
{
	string retVal;
	string::const_iterator sItr = source.begin();
	int pos = source.find_first_of(" ");
	for(int counter = 0; counter < pos; ++counter, ++sItr)
		retVal += *sItr;
  return retVal;
}

//Erase all characters up to and including the first encountered space.
void trimOneWord(string& source)
{
	int pos = source.find_first_of(" ");
	source.erase(0, pos + 1);	
}
//TODO comment
string readAndTrimOneWord(string& source)
{
	string returnVal = readOneWord(source);
	trimOneWord(source);
	return returnVal;
}

void writeFileLineToString(fstream& source, string& dest)
{
	char intermediate[MAX_IMP_LINE_LENGTH + 1];
	source.getline(intermediate, MAX_IMP_LINE_LENGTH + 1);//what is this + 1 for? looks like it might overflow
	dest = intermediate;
}

/* ImportIL
 *
 * This function creates an input link specification from the IL file passed.
 * Returns true on success, false on failure.
 */
bool InputLinkSpec::ImportIL(string& filename)
{
	fstream file;
	file.open(filename.c_str());

	if(!file.is_open())
	{
		cout << "Error: unable to open file " << filename << ". pausing now before exit." << endl;
		pause();
		return false;
	}

	//counts number of lines successfully imported from the file
	int lineNumber = 0;
	
	//this should create InputLinkObjects to hold each line of data read
	while(!file.eof())
	{
		eParseStage parseStage = READING_BEGIN_STAGE;
		eParseStage lastCompletedState = READING_PRE_BEGIN;
		int loopBegin = 0;//FIXME TODO shouldn't these be defaults?
		int loopEnd = 1;

		string line;
		writeFileLineToString(file, line);

		string firstWord = readOneWord(line);	 
		//cout << "first word is >" << firstWord << "<" << endl;
		
		//ensure that the line wasn't too long
		if(file.gcount() >= MAX_IMP_LINE_LENGTH+1)
		{
			cout<<"Error: line "<<lineNumber<<" too long"<<endl;
			return false;
		}

		//numCharsRead = sscanf(curLine, "%s", curWord);
//TODO fix these comments
    //the first read should always be a string, so if this fails, the input was
    //improperly formated.  The captured word will either be a control structure
    //or an identifier name

		if(firstWord == k_forDelimiter)
		{
			int controlStartVal, controlEndVal = 0;

			cout << "'for' control structure recognized." << endl;

			//clear everything up to and including the first space
			trimOneWord(line);

			//read control variable name
			string controlName = readAndTrimOneWord(line);
			//cout << "control variable name is >" << controlName << "<" << endl;

			controlStartVal = atoi(readAndTrimOneWord(line).c_str());
			//cout << "control start val is >" << controlStartVal << "<" << endl;

			controlEndVal = atoi(line.c_str());
			//cout << "control end val is >" << controlEndVal << "<" << endl;
			
			cout << "After all the control nonsense, the values are " << controlName << " "
				<< controlStartVal << " " << controlEndVal << endl;
			
			//set ACTUAL loop start and stop delimiters			
			loopBegin = controlStartVal;
			loopEnd = controlEndVal;

			//TODO (if doing nested control loops, add an entry to the control queue)
			
			//since we just consumed the first line, read in the next one 
			//for the identifier information
			writeFileLineToString(file, line);
			lastCompletedState = READING_CONTROL_STRUCTURE;
			parseStage = READING_IDENTIFIER;			
		}
		else
			parseStage = READING_IDENTIFIER;

		string curWord;
		InputLinkObject ilObj;
// ALGORITHM **********************
//if there's a control structure
  //repeat the specified pattern, store each
//otherwise
	//read the specified pattern and store

		//Begin parsing based on stage
		//while(parseStage != READING_FINAL_STAGE)
		//{
		for(; loopBegin < loopEnd; ++loopBegin)
		{
//			printStage(parseStage, cout);
			bool readingFirstType = true, moreTypesLeft = true;

			switch(parseStage)
			{

				case READING_CONTROL_STRUCTURE:
					break;
				case READING_IDENTIFIER:

					//The least amount of entries for an il description are 4 strings
					//numCharsRead = sscanf(curLine, "%s %s %s %s, , , , );
					//TODO more error checking
					curWord = readAndTrimOneWord(line);
					cout << "ParentIdName is >" << curWord << "<" << endl;
					ilObj.setParentId(curWord);
					lastCompletedState = READING_IDENTIFIER;
					parseStage = READING_ATTRIBUTE;
					break;
				case READING_ATTRIBUTE:
					curWord = readAndTrimOneWord(line);
					//trim off the attrib token
					curWord.replace(0, 1, "");
					ilObj.setAttribName(curWord);
					lastCompletedState = READING_ATTRIBUTE;
					parseStage = READING_VALUE_TYPE;
					break;
				case READING_VALUE_TYPE:
					curWord = readAndTrimOneWord(line);
					//if the first token hasn't been read, and there isn't one here, that's a problem
					if(curWord.find_first_of(k_typesOpenToken.c_str(),0, curWord.size()) == curWord.npos)
					{
						cout << "didn't find open token:>" << curWord << "<" << endl;
						parseStage = READING_ERROR;
						break;
					}

					//trim off the '<' token
					curWord.replace(0,1, "");
					do//TODO this may work better as a 'for' loop
					{
						//if it IS the first type listed, we already have a string to parse
						if(!readingFirstType)
							curWord = readAndTrimOneWord(line);

						readingFirstType = false;
						int pos = curWord.find(k_typesCloseToken.c_str());
						if(pos == curWord.npos)
						{
cout << "\t\tdidn't find the end token on: " << curWord << endl;
							moreTypesLeft = true;
						}
						else
						{
cout << "\t\tfound the end token on: " << curWord << endl;
							moreTypesLeft = false;
							//trim off the '>' token
							curWord.erase(pos);
						}
						cout << "Read value type as " << curWord << endl;
						ilObj.addElementType(stringToType(curWord));							
					}
					while(moreTypesLeft);

					lastCompletedState = READING_VALUE_TYPE;
					parseStage = READING_START_VALUE;
					break;
				case READING_START_VALUE:
					
					lastCompletedState = READING_START_VALUE;
					parseStage = READING_UPDATE;
					break;
				case READING_UPDATE:
					lastCompletedState = READING_UPDATE;
					parseStage = READING_CREATE_ON;
					break;
				case READING_CREATE_ON:
					lastCompletedState = READING_CREATE_ON;
					parseStage = READING_DELETE_ON;
					break;
				case READING_DELETE_ON:
					lastCompletedState = READING_DELETE_ON;
					break;
				case READING_ERROR:
					cout << "Error encountered during parse. Last completed stage was ";
					printStage(lastCompletedState, cout);
					cout << endl;
					pause();
					exit(-1);
					break;
				default:
					break;
			}//end switch
		}// end control loop

//		loopBegin = defaultLoopBegin;
//		loopEnd = defaultLoopEnd;

		//should first get parent id name
		//then get attribute name
		//then get value type
		//then optional and conditional args.
		++lineNumber;
//     cout << "just read line " << lineNumber << endl;
	}//while there are still lines to read

	cout << "Read " << lineNumber << " lines." << endl;
	file.close();
	return true;
}

