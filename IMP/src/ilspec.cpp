#include "ilspec.h"
#include "ilobject.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>

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
//counts number of lines successfully imported from the file
int lineNumber = 0;

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


//This is not quite cool.  The utility will remain the same, 
//but this may need to be a member
void printStage(eParseStage stage, ostream& stream)
{
	stream << "<+> ";
	switch(stage)
	{
		case READING_PRE_BEGIN:
			stream << "'Before Reading'" << endl;
			break;
		case READING_CONTROL_STRUCTURE: //this is also READING_BEGIN_STAGE
			stream << "'Reading control structure'" << endl;
			break;
		case READING_PARENT_IDENTIFIER:
			stream << "'Reading identifier'" << endl;
			break;
		case READING_ATTRIBUTE:
			stream << "'Reading attribute'" << endl;
			break;
		case READING_VALUE_TYPE:
			stream << "'Reading value type'" << endl;
			break;
		case READING_IDENTIFIER_UNIQUE_NAME:
			stream << "'Reading unique name for id'" << endl;
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
			stream << "***Error in parse***" << endl;
			break;
		case READING_FINAL_STAGE:
			stream << "'Reading final stage'" << endl;
		default:
			break;
	}
}

//TODO comment
string readOneWord(const string& source)
{
	string retVal;
	string::const_iterator sItr = source.begin();
	
	//the next word may be trailed by a space, a newline, or EOF
	int pos = source.find_first_of(" \n");

	//TODO...handle the EOF case
	if(pos == source.npos)
		return source;

	for(int counter = 0; counter < pos; ++counter, ++sItr)
		retVal += *sItr;
  return retVal;
}

//Erase all characters up to and including the first encountered space.
//TODO make this trim ALL leading spaces
void trimOneWord(string& source)
{
	int pos = source.find_first_of(" ");
	if(pos != source.npos)
		source.erase(0, pos + 1);	
}
//TODO comment
string readAndTrimOneWord(string& source, bool echo = false)
{
	string returnVal = readOneWord(source);
	trimOneWord(source);
	if(echo)
		cout << "read >" << returnVal << "< and trimmed it out." << endl;
	return returnVal;
}

void writeFileLineToString(fstream& source, string& dest, bool echo = false)
{
	char intermediate[MAX_IMP_LINE_LENGTH + 1];//TODO check these bounds
	source.getline(intermediate, MAX_IMP_LINE_LENGTH + 1);//what is this + 1 for? looks like it might overflow
	
	//ensure that the line wasn't too long //TODO confirm that this is correct
	if(source.gcount() >= MAX_IMP_LINE_LENGTH+1)
	{
		cout<<"Error: line "<<lineNumber<<" too long"<<endl;
		return;
	}
	
	dest = intermediate;
	if(echo)
		cout << "Entire file line:>" << dest << "<" << endl;
	lineNumber++;
	//cout << "just read line " << lineNumber << endl;
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


	//this should create InputLinkObjects to hold each line of data read
	while(!file.eof())
	{
		eParseStage parseStage = READING_BEGIN_STAGE;
		eParseStage lastCompletedState = READING_PRE_BEGIN;
		int loopIteration = defaultLoopBegin;
		int loopEnd = defaultLoopEnd;

		string line;
		writeFileLineToString(file, line, true);

		//peek at the first word, but don't remove it from the stream
		string firstWord = readOneWord(line);

    //The captured word will either be a control structure or an identifier name
		if(firstWord == k_forToken)
			parseStage = READING_CONTROL_STRUCTURE;		
		else
			parseStage = READING_PARENT_IDENTIFIER;

		InputLinkObject ilObj;
		//Begin parsing based on stage
		while(parseStage != READING_FINAL_STAGE)
		//for(; loopIteration < loopEnd; ++loopIteration)
		{
			printStage(parseStage, cout);
			bool readingFirstType = true, moreTypesLeft = true, hasStartValue = false;
			string controlName, curWord = "";
			int controlStartVal, controlEndVal = 0;

			switch(parseStage)
			{
				case READING_CONTROL_STRUCTURE:
					//clear off the token
					trimOneWord(line);

					//read control variable name
					controlName = readAndTrimOneWord(line);
					controlStartVal = atoi(readAndTrimOneWord(line).c_str());
					controlEndVal = atoi(line.c_str());
					cout << "After all the control nonsense, the values are: " << controlName << " "
						<< controlStartVal << " " << controlEndVal << endl;

					//set ACTUAL loop start and stop delimiters			
					loopIteration = controlStartVal;
					loopEnd = controlEndVal;

					//TODO (if doing nested control loops, add an entry to the control queue)

					//since we just consumed the first line, read in the next one 
					//for the identifier information
					writeFileLineToString(file, line, true);
					lastCompletedState = READING_CONTROL_STRUCTURE;
					parseStage = READING_PARENT_IDENTIFIER;
					break;
				case READING_PARENT_IDENTIFIER:

					curWord = readAndTrimOneWord(line);
					ilObj.setParentId(curWord);
					lastCompletedState = READING_PARENT_IDENTIFIER;
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
						else
							cout << "\tReading first type for this wme, which is >" << curWord << "<." << endl;

						readingFirstType = false;
						int pos = curWord.find(k_typesCloseToken.c_str());
						if(pos == curWord.npos)
							moreTypesLeft = true;
						else
						{ //TODO make sure any whitespace leading the '>' is taken care of...
							moreTypesLeft = false;
							//trim off the '>' token
							curWord.erase(pos);
						}
						cout << "Read value type as " << curWord << endl;
						ilObj.addElementType(curWord);

						//TODO tough error checking to do here---v
						//if the type is an ID, it should be listed alone
						if(!stricmp(curWord.c_str(), k_idString.c_str()))
						{
							//lastCompletedState = READING_VALUE_TYPE;
							parseStage = READING_IDENTIFIER_UNIQUE_NAME;
							break;
						}
						else//will execute more times than necessary, but is not incorrect
							parseStage = READING_START_VALUE;
					}
					while(moreTypesLeft);

					lastCompletedState = READING_VALUE_TYPE;
					break;
				//This case is really just a special case of READING_START_VALUE	
				case READING_IDENTIFIER_UNIQUE_NAME:

					//get the actual ID unique name now
					curWord = readAndTrimOneWord(line);//TODO error checking

					ilObj.setStartValue(curWord);
					lastCompletedState = READING_IDENTIFIER_UNIQUE_NAME;
					parseStage = READING_FINAL_STAGE;//IDs never get updated
					break;
				case READING_START_VALUE:
					//The start value of a WME is optional, so peek and see if it's there
					curWord = readOneWord(line);	
					if(curWord != k_startToken)
					{
						//no start value.  Perhaps there is an optional update value
						parseStage = READING_UPDATE;
						break;
					}
					else//token really was there.  Clear it
						trimOneWord(line);
						
					//now that the token is out of the way, get the actual start value
					curWord = readAndTrimOneWord(line, true);

					//NOTE there are cases where this will be giving an int/float wme 
					//a literal string value (which will end up as zero). This
					//happens when the value is the name of a control loop variable.  
					//this value is set correctly later on
					ilObj.setStartValue(curWord);
					lastCompletedState = READING_START_VALUE;
					parseStage = READING_UPDATE;
					break;
				case READING_UPDATE:
					//this is optional.  if there is no -update token, bug out.
					//TODO make a note that this is not a literal string, but rather the
					//name of the variable whose value will be used for the update
					
					//peek and see if the token is there
					curWord	= readOneWord(line);
					if(curWord == k_updateToken)
					{	//token is there, so clear it off
						trimOneWord(line);
					}
					else
					{	//nothing else should be on this line. If there is, ignore it
						parseStage = READING_FINAL_STAGE;
						break;
					}
					//the -update token will be followed by 1 or 2 strings if done correctly
					
					//read off the frequency
					curWord = readAndTrimOneWord(line, true);
					if(curWord == "")
					{
						cout << "***Was expecting a frequency for the update, but found none..." << endl;
						parseStage = READING_ERROR;
						break;
					}
					else //FIXME TODO pick up work here too
						ilObj.setUpdateFrequency(curWord);
					
					
					if(curWord == "\n" || curWord == "" )
					{
						ilObj.setUpdateValue(line);
						parseStage = READING_CREATE_ON;
					}
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
				case READING_FINAL_STAGE:
					break;
				default:
					cout << "What? ended up in the default case, with value: " << parseStage << endl;
					assert(false);
					break;
			}//end switch
		}// end control loop


		//now we've read a WME pattern. If it should be duplicated, do that now
		for(int counter = loopIteration; counter < loopEnd; ++counter)
		{
			//the parts of a WME pattern that might have a field equal to the control
			//variable name are the optional "update"/"start fields
			//
		}

		ilObjects.push_back(ilObj);


	}//while there are still lines to read

	cout << "Read " << lineNumber << " lines." << endl;
	cout << "Number of input link entries " << ilObjects.size() << endl;
	for(ilObjItr objItr = ilObjects.begin(); objItr != ilObjects.end(); ++objItr)
	{
		objItr->print(cout);
	}
	file.close();
	return true;
}

