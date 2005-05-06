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
using std::ios;

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
		case READING_UPDATE_VALUE:
			stream << "'Reading update value'" << endl;
			break;
		case READING_UPDATE_FREQUENCY:
			stream << "'Reading update frequency'" << endl;
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

bool tokenPresent(const string& token, const string& source)
{
	return source.find(token.c_str(), 0, token.size()) != string.npos;
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
		bool quitCondition = false;

		string line;
		writeFileLineToString(file, line, true);
		//If we have a blank line. Check if that's because we're at EOF, and also check if the
		//next read is EOF
		if(line == "")
		{
			if(file.eof())
				break;
			int test = file.peek();
			if(file.eof())
			{
				file.clear();
				cout << "end of file was reached..." << endl;
				break;
			}
			//otherwise, there is more of the file to read, and we are probably just 
			//looking at a blank line - get another one.
			continue;
		}

		//peek at the first word, but don't remove it from the stream


    //The captured word will either be a control structure or an identifier name
		if(tokenPresent(k_forToken, line))
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
					//TODO error checking
					curWord = readAndTrimOneWord(line, true);
					ilObj.setParentId(curWord);
					lastCompletedState = READING_PARENT_IDENTIFIER;
					parseStage = READING_ATTRIBUTE;
					break;
				case READING_ATTRIBUTE:
					curWord = readAndTrimOneWord(line, true); // TODO error checking
					//trim off the attrib token
					curWord.replace(0, 1, "");
					ilObj.setAttribName(curWord);
					lastCompletedState = READING_ATTRIBUTE;
					parseStage = READING_VALUE_TYPE;
					break;
				case READING_VALUE_TYPE:
					curWord = readAndTrimOneWord(line);
					//if the first token hasn't been read, and there isn't one here, that's a problem
					//this token is harder to strip of than the others because it may be touching the word
					//that follows it. NOTE that if the token is missing, and is actually used later on
					//the same line of the WME specification, weird results will occur
					if(!tokenPresent(k_typesOpenToken, curWord))
					{
						cout << "didn't find open token in:_" << curWord << "_" << endl;
						parseStage = READING_ERROR;
						break;
					}

					//trim off the '<' token
					curWord.replace(0,1, "");

					do//TODO this may work better as a 'for' loop
					{
						//if it IS the first type listed, we already have a string to parse
						if(!readingFirstType)
							curWord = readAndTrimOneWord(line, true);
						else
						{
							cout << "\tReading first type for this wme, which is >" << curWord << "<." << endl;
							readingFirstType = false;
						}
/*cout << "\t\tTrim time.  String is:_" << curWord << endl <<
 "_.  size is: " << curWord.size();
cout << " and curword(size -1 ) is:_" << curWord[curWord.size() -1] << "_" << endl;*/

						if(!tokenPresent(k_typesCloseToken, curWord))
							moreTypesLeft = true;

						else
						{ //TODO make sure any whitespace leading the '>' is taken care of...
							moreTypesLeft = false;
							//trim off the '>' token
							curWord.erase(curWord.size() - 1);
						}
						cout << "Read value type as " << curWord << endl;
						ilObj.addElementType(curWord);

						//TODO tough error checking to do here---v
						//if the type is an ID, it should be listed alone
						if(!stricmp(curWord.c_str(), k_idString.c_str()))
						{
							parseStage = READING_IDENTIFIER_UNIQUE_NAME;
							break;
						}
						else//will execute more times than necessary, but is not incorrect
							parseStage = READING_START_VALUE;
					}
					while(moreTypesLeft);

					//set the type here.  If multiple types. mark it TBD
					if(ilObj.getNumTypes() > 1)
					{
						ilObj.setType(k_TBD);
					}
					else //TODO would be much cleaner to do this internally in 
						ilObj.setType(typeToString(ilObj.getFirstType()));

					lastCompletedState = READING_VALUE_TYPE;
					break;
				//This case is really just a special case of READING_START_VALUE	
				case READING_IDENTIFIER_UNIQUE_NAME:

					//get the actual ID unique name now
					curWord = readAndTrimOneWord(line, true);//TODO error checking

					ilObj.setStartValue(curWord);
					lastCompletedState = READING_IDENTIFIER_UNIQUE_NAME;
					parseStage = READING_FINAL_STAGE;//IDs never get updated
					break;
				case READING_START_VALUE:
					//The start value of a WME is optional, so peek and see if it's there
					if(!tokenPresent(k_startToken, line))
					{
						//no start value.  Perhaps there is an optional update value
						//no update of last stage completed, because nothing was done here
						parseStage = READING_UPDATE_VALUE;
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
					parseStage = READING_UPDATE_VALUE;
					break;
				case READING_UPDATE_VALUE:
					//this is optional.  if there is no -update token, bug out.

					//peek and see if the token is there
					if(!tokenPresent(k_updateToken, line))
					{
						//don't update the last completed stage, because we did nothing here...
						parseStage = READING_UPDATE_FREQUENCY;
						break;
					}

					//token is there, so clear it off
					trimOneWord(line);

					//read the update value
					curWord = readAndTrimOneWord(line);
					if(curWord != "")
					{
						ilObj.setUpdateValue(curWord);
						lastCompletedState = READING_UPDATE_VALUE;
						parseStage = READING_UPDATE_FREQUENCY;
					}
					else
					{	//error - token without a value following
						parseStage = READING_ERROR;
						break;
					}
					lastCompletedState = READING_UPDATE_VALUE;
					break;
				case READING_UPDATE_FREQUENCY:
					//peek and see if the token is there
					if(!tokenPresent(k_frequencyToken, line))
					{	//no token present. ignore any garbage that may remain on this line
						//did no work here, so don't set last parse stage
						parseStage = READING_FINAL_STAGE;
						break;
					}

					//read off the frequency
					curWord = readOneWord(line);
					if(curWord == "")
					{	//error - token without a value following
						parseStage = READING_ERROR;
						break;
					}
					
					//read of the value for the frequency
					ilObj.setUpdateFrequency(curWord);	

					//if this is a conditional frequency, read off the condition string
					if(curWord == k_conditionString)
					{
					  curWord = readAndTrimOneWord(line);
						if(curWord == "")
						{
							//token without a value following
							parseStage = READING_ERROR;
							break;
						}

						ilObj.setUpdateCondition(curWord);
						lastCompletedState = READING_UPDATE_FREQUENCY;
						parseStage = READING_FINAL_STAGE;
					}

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

