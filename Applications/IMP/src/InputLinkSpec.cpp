#include "InputLinkSpec.h"
#include "InputLinkObject.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <sstream>

using std::string;
using std::endl;
using std::cout; using std::cin;
using std::fstream; using std::ostream;
using std::ostringstream;
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
InputLinkSpec::InputLinkSpec(ilObjVector_t& inObjects, typedObjectsMap_t& inTypedObjs) :
			 ilObjects(inObjects), typedObjects(inTypedObjs)
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

extern void Pause();
const int defaultLoopBegin = 0;
const int defaultLoopEnd = 1;
const int defaultIncrementAmount = 1;
//counts number of lines successfully imported from the file
int lineNumber = 0;

string IntToString(int i)
{
	ostringstream stream;
	stream << i;
	return stream.str();
}

string FloatToString(double d)
{
	ostringstream stream;
	stream << d;
	return stream.str();
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

	//file.close();
	return true;
}

//This is not quite cool.  The utility will remain the same, 
//but this may be more coherent as a member
void PrintStage(eParseStage stage, ostream& stream)
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
		case READING_CLASS_NAME:
			stream << "'Reading class name (open)'" << endl;
			break;
		case READING_PARENT_IDENTIFIER:
			stream << "'Reading parent identifier'" << endl;
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
		case READING_CLASS_CLOSE:
			stream << "'Reading class name (close)'" << endl;
			break;
		case READING_FINAL_STAGE:
			stream << "'Reading final stage'" << endl;
			break;
		default:
			break;
	}
}

//trims leading whitespace out of a string, returns a copy of the altered string
string ConsumeSpaces(const string& source, bool echo = false)
{
	string::size_type nonWhiteIndex = source.find_first_not_of(" \t\n");
	string temp(source);
	temp.erase(0,nonWhiteIndex);
	if(echo)
		cout << "ConsumeSpaces trimming:_" << source << "_ down to:_" << temp << "_" << endl;
	return temp;
}

//Returns a copy of the first whitespace-terminated word in source
string ReadOneWord(const string& source)
{
	string retVal;
	string::const_iterator sItr = source.begin();

	//the next word may be trailed by a whitespace
	//NOTE - author doesn't think EOF char(s) will ever be present in this string
	int pos = source.find_first_of(" \t\n");

	//must be the last string, just return it
	if(pos == source.npos)
		return source;

	for(int counter = 0; counter < pos; ++counter, ++sItr)
		retVal += *sItr;
  return retVal;
}

//Erase first whitespace-terminated word, including the first encountered whitespace.
void TrimOneWord(string& source)
{
	int pos = source.find_first_of(" \t\n");
	if(pos != source.npos)
		source.erase(0, pos + 1);
}

//returns a copy of one whitespace-terminated word from source string, 
//and removes it from source.  Optionally prints the read string
string ReadAndTrimOneWord(string& source, bool echo = false)
{
	string returnVal = ReadOneWord(source);
	TrimOneWord(source);
	if(echo)
		cout << "read >" << returnVal << "< and trimmed it out." << endl;
	return returnVal;
}

//returns true if the supplied token is found in source string,
//otherwise, false
bool TokenPresent(const string& token, const string& source)
{
	return source.find(token.c_str(), 0, token.size()) != string.npos;
}

//TODO would be nice to put these all in a map.....
//Returns the next parse stage, based on the token that's present
eParseStage GetNextStageByToken(string& line)
{
	string word = ReadOneWord(line);
	if(word == k_classOpenToken)
		return READING_CLASS_NAME;
	else if(word == k_classEndToken)
		return READING_CLASS_CLOSE;
	else if(word == k_forToken)
		return READING_CONTROL_STRUCTURE;
	else
		return UNKNOWN_STAGE;
}
//reads one line from file into a string.  The resultant string does not contain the
//newline that terminated it in the file
void WriteFileLineToString(fstream& source, string& dest, bool echo = false)
{
	char intermediate[MAX_IMP_LINE_LENGTH + 1];//enough space for maxline plus a null char
	//this will grab (maxline -1) chars, thinking we didn't allocate enough for the null,
	//but since we did, grab 1 more
	source.getline(intermediate, MAX_IMP_LINE_LENGTH+1);

	//ensure that the line wasn't too long
	if(source.gcount() >= MAX_IMP_LINE_LENGTH+1)
	{
		cout<<"Error: line "<<lineNumber<<" too long"<<endl;
		return;
	}

	dest = intermediate;
	if(echo)
		cout << "Entire file line:>" << dest << "<" << endl;
	lineNumber++;
}

/*void PeekAtNextLine(fstream& source, string& dest, bool echo = false)
{
	int startingPosition = source.tellg();
	WriteFileLineToString(source, dest, echo);	
	source.seekg(startingPosition);
	lineNumber--;//undoes the increment from WriteFileLineToString;
}*/

void InputLinkSpec::AddTypedObject(InputLinkObject& newObject)
{
	(typedObjects[newObject.GetSimulationClassName()]).push_back(newObject);
}


/* ImportIL
 *
 * This function creates an input link specification from the IL file passed.
 * Returns true on success, false on failure.
 */
bool InputLinkSpec::ImportIL(string& filename)
{
	string currentClassName = "";
	fstream file;
	file.open(filename.c_str());

	if(!file.is_open())
	{
		cout << "Error: unable to open file " << filename << ". pausing now before exit." << endl;
		Pause();
		return false;
	}

	//These 3 vars allow a control variable to be read in, and have
	//a pattern repeated the correct number of times, which may be
	//well after the control structure has been specified.  After the pattern
	//has been replicated and stored the appropriate number of times,
	//reset the counters
	int loopBegin = defaultLoopBegin;
	int loopEnd = defaultLoopEnd;
	int incrementAmount = defaultIncrementAmount;
	bool resetLoopCounters = false;
	bool classDescriptionJustFinished = false;
	bool unsatisfiedControlStructure = false;
	//the name of the currently active 'for' loop, "" if there is none
	string controlVariableName;
	ilObjVector_t pendingClassSpec;
	
	//Read all descriptions of input link objects
	while(!file.eof())
	{
		eParseStage parseStage = READING_BEGIN_STAGE;
		eParseStage lastCompletedState = READING_PRE_BEGIN;

		//This container holds the WME descriptions for a class
		//A 'for' loop that preceeds a class structure will cause the entire
		//class description, this container, to be repeated

		string line;
		WriteFileLineToString(file, line);
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
    //Will either be a control structure, an identifier name, or a class name
		if(TokenPresent(k_forToken, line))
			parseStage = READING_CONTROL_STRUCTURE;

		else if(TokenPresent(k_classOpenToken, line))
			parseStage = READING_CLASS_NAME;

		else if(TokenPresent(k_classEndToken, line))
			parseStage = READING_CLASS_CLOSE;

		else
			parseStage = READING_PARENT_IDENTIFIER;

		InputLinkObject ilObj;//this object will have its values filled in as they are read in
		//Begin parsing based on stage
		while(parseStage != READING_FINAL_STAGE)
		{
			PrintStage(parseStage, cout);
			bool readingFirstType = true, moreTypesLeft = true;
			string curWord = "";
			string controlStartVal, controlEndVal;

			switch(parseStage)
			{
				case READING_CONTROL_STRUCTURE:

					//clear off the token
					TrimOneWord(line);

					//read control variable name
					controlVariableName = ReadAndTrimOneWord(line);

					if(controlVariableName == "")
					{
						parseStage = READING_ERROR;
						cout << "Missing control variable name...." << endl;
						break;
					}
					controlStartVal = ReadAndTrimOneWord(line);
					//check the read
					if(controlStartVal == "")
					{
						parseStage = READING_ERROR;
						cout << "Missing control structure start value..." << endl;
						break;
					}
					//can't check the format, as atoi will return zero on failure
					//or if the string is actually value zero

					controlEndVal = ReadAndTrimOneWord(line);
					//check the read
					if(controlEndVal == "")
					{
						parseStage = READING_ERROR;
						cout << "Missing control structure end value..." << controlEndVal << endl;
						break;
					}

					//TODO read in the increment value, so that the control loop
					//can count both forward and backward, and by an arbitrary amount

					//set ACTUAL loop start and stop delimiters
					loopBegin = atoi(controlStartVal.c_str());
					loopEnd = atoi(controlEndVal.c_str());

					unsatisfiedControlStructure = true;
					resetLoopCounters = false;
					//TODO (if doing nested control loops, add an entry to the control queue)

					//since we just consumed the first line, read in the next one
					//for the identifier information
					WriteFileLineToString(file, line);
					lastCompletedState = READING_CONTROL_STRUCTURE;

					parseStage = GetNextStageByToken(line);
					if(parseStage == UNKNOWN_STAGE)//there may not be a token there
						parseStage = READING_PARENT_IDENTIFIER;
					//the 'else' case for this 'if' is actually handled by the line preceeding 'if'
					break;
				case READING_CLASS_NAME:
					//trim off the token
					TrimOneWord(line);

					//read the actual name now
					curWord = ReadAndTrimOneWord(line);
					if(curWord == "")
					{
						cout << "Didn't find class name after token..." << endl;
						parseStage = READING_ERROR;
						break;
					}

					currentClassName = curWord;
					WriteFileLineToString(file, line);
					lastCompletedState = READING_CLASS_NAME;
					parseStage = READING_PARENT_IDENTIFIER;
					break;
				case READING_PARENT_IDENTIFIER:
					curWord = ReadAndTrimOneWord(line);
					if(curWord == "")
					{
						cout << "***didn't find parent identifier..." << endl;
						parseStage = READING_ERROR;
						break;
					}
					ilObj.SetParentId(curWord);
					lastCompletedState = READING_PARENT_IDENTIFIER;
					parseStage = READING_ATTRIBUTE;
					break;
				case READING_ATTRIBUTE:
					curWord = ReadAndTrimOneWord(line);
					if(curWord == "")
					{
						cout << "***didn't find attribute for this wme pattern..." << endl;
						parseStage = READING_ERROR;
						break;
					}
					//replace the leading attrib token with a double quote
					curWord.replace(0, 1, "\"");
					curWord.push_back('\"');//add the trailing double quote, since this is a literal
					ilObj.SetAttribName(curWord);
					lastCompletedState = READING_ATTRIBUTE;
					parseStage = READING_VALUE_TYPE;
					break;
				case READING_VALUE_TYPE:
					curWord = ReadAndTrimOneWord(line);

					//if the first token hasn't been read, and there isn't one here, that's a problem.
					//This token is harder to strip of than the others because it may be touching the word
					//that follows it. NOTE that if the token is missing, and is actually used later on
					//the same line of the WME specification, weird results will occur
					if(!TokenPresent(k_typesOpenToken, curWord))
					{
						cout << "didn't find open token in:_" << curWord << "_" << endl;
						parseStage = READING_ERROR;
						break;
					}

					//trim off the '<' token
					curWord.replace(0, 1, "");

					//read all of the types
					do//TODO this may work better as a 'for' loop
					{
						//if it IS the first type listed, we already have a string to parse
						if(!readingFirstType)
							curWord = ReadAndTrimOneWord(line);
						else
						{
							//cout << "\tReading first type for this wme, which is >" << curWord << "<." << endl;
							readingFirstType = false;
						}

						if(!TokenPresent(k_typesCloseToken, curWord))
						{
							//if the token isn't present in this word, and the next word isn't
							//the closing token, then we really do have more types left
							string tentativeRead = ReadOneWord(line);
							if(tentativeRead != ">")
								moreTypesLeft = true;
							else
								moreTypesLeft = false;
						}
						else
						{ //strip any whitespace leading the '>'
							curWord = ConsumeSpaces(curWord);
							moreTypesLeft = false;
							//trim off the '>' token
							curWord.erase(curWord.size() - 1);
						}
						//cout << "Read value type as " << curWord << endl;
						ilObj.AddElementType(curWord);

						//if the type is an ID, it should be listed alone
						//These represent all the ways an ID could appear alone.
						//The closing brace could either be attached or separated by whitespace
						// < ID>  < ID > <ID> <ID >						
						if(!stricmp(curWord.c_str(), k_idString.c_str()))
						{
							parseStage = READING_IDENTIFIER_UNIQUE_NAME;
							if(moreTypesLeft)
							{
								parseStage = READING_ERROR;
								cout << "ID type should have been listed alone..." << endl;
							}
							break;
						}
						else//will execute more times than necessary, but is not incorrect
							parseStage = READING_START_VALUE;
					}
					while(moreTypesLeft);

					//set the type here.  If multiple types. mark it TBD
					if(ilObj.GetNumTypes() > 1)
					{
						ilObj.SetType(k_TBD);
					}
					else
						ilObj.SetType();//sets the type to whichever was specified

					lastCompletedState = READING_VALUE_TYPE;
					break;
				//This case is really just a special case of READING_START_VALUE	
				case READING_IDENTIFIER_UNIQUE_NAME:
					//get the actual ID unique name now
					curWord = ReadAndTrimOneWord(line);
					if(curWord == "")
					{
						parseStage = READING_ERROR;
						cout << "Unique identifier for ID type was not present...." << endl;
						break;
					}

					ilObj.SetStartValue(curWord);

					lastCompletedState = READING_IDENTIFIER_UNIQUE_NAME;
					parseStage = READING_FINAL_STAGE;//IDs never get updated during a simulation run
					break;
				case READING_START_VALUE:
					//The start value of a WME is optional, so peek and see if it's there
					if(!TokenPresent(k_startToken, line))
					{
						//no start value.  Perhaps there is an optional update value
						//no update of last stage completed, because nothing was done here
						parseStage = READING_UPDATE_VALUE;
						break;
					}
					else//token really was there.  Clear it
						TrimOneWord(line);

					//now that the token is out of the way, get the actual start value
					curWord = ReadAndTrimOneWord(line);

					//NOTE there are cases where this will be giving an int/float wme 
					//a literal string value (which will end up as zero). This
					//happens when the value is the name of a control loop variable.  
					//this value is set correctly later on
					ilObj.SetStartValue(curWord);
					lastCompletedState = READING_START_VALUE;
					parseStage = READING_UPDATE_VALUE;
					break;
				case READING_UPDATE_VALUE:
					//this is optional.  if there is no -update token, bug out.

					//peek and see if the token is there
					if(!TokenPresent(k_updateToken, line))
					{
						//don't update the last completed stage, because we did nothing here...
						parseStage = READING_UPDATE_FREQUENCY;
						break;
					}

					//token is there, so clear it off
					TrimOneWord(line);

					//read the update value
					curWord = ReadAndTrimOneWord(line);
					if(curWord != "")
					{
						ilObj.SetUpdateValue(curWord);
						lastCompletedState = READING_UPDATE_VALUE;
						parseStage = READING_UPDATE_FREQUENCY;
					}
					else
					{	//error - token without a value following
						cout << "Error, update token w/o a value following it" << endl;
						parseStage = READING_ERROR;
						break;
					}
					lastCompletedState = READING_UPDATE_VALUE;
					break;
				case READING_UPDATE_FREQUENCY:
					//peek and see if the token is there
					if(!TokenPresent(k_frequencyToken, line))
					{	//no token present. ignore any garbage that may remain on this line
						//did no work here, so don't set last parse stage
						parseStage = READING_FINAL_STAGE;
						break;
					}
					//clear off token
					TrimOneWord(line);

					//read off the frequency
					curWord = ReadAndTrimOneWord(line);
					if(curWord == "")
					{	//error - token without a value following
						parseStage = READING_ERROR;
						break;
					}

					//set the object's frequency
					ilObj.SetUpdateFrequency(curWord);

					//if this is a conditional frequency, read off the condition string
					if(curWord == k_conditionString)
					{
						curWord = ReadAndTrimOneWord(line);
						if(curWord == "")
						{
							//token without a value following
							parseStage = READING_ERROR;
							break;
						}

						ilObj.SetUpdateCondition(curWord);
						lastCompletedState = READING_UPDATE_FREQUENCY;
						parseStage = READING_FINAL_STAGE;
					}

					break;
				case READING_CREATE_ON:
					lastCompletedState = READING_CREATE_ON;
					//TODO actually do this
					parseStage = READING_DELETE_ON;
					break;
				case READING_DELETE_ON:
					lastCompletedState = READING_DELETE_ON;
					break;
				case READING_ERROR:
					cout << "Error encountered during parse. Last completed stage was ";
					PrintStage(lastCompletedState, cout);
					cout << endl;
					Pause();
					exit(-1);
					break;
				case READING_CLASS_CLOSE:
					//peek and look for token (this may be redundant)
					curWord = ReadOneWord(line);
					if(curWord == k_classEndToken)
						TrimOneWord(line);
					else
					{
						//what does this mean?  what kind of garbage text did we read in?
						if(curWord != "")
						{
							cout << "Got some unexpected text while looking for the 'end' token." << endl;
							cout << "\t" << curWord << endl;
							cout << "Execution will continue. Press any non-whitespace char + enter now." << endl;
							Pause();
						}
						lastCompletedState = READING_CLASS_CLOSE;
						parseStage = READING_FINAL_STAGE;
						break;
					}
					classDescriptionJustFinished = true;
					curWord = ReadOneWord(line);
					//if there is a word here, it should be the same class name as the opening one
					if(stricmp(curWord.c_str(), currentClassName.c_str()))
					{
						parseStage = READING_ERROR;
						cout << "Closing class name should match up with opening name." << endl;
						cout << "\tOpening was: " << currentClassName << " and closing was: " <<
							curWord << endl;
						break;
					}
					else // ending the use of this class name
						currentClassName = "";

					TrimOneWord(line);
					parseStage = READING_FINAL_STAGE;
					lastCompletedState = READING_CLASS_CLOSE;
					break;
				case READING_FINAL_STAGE:
					break;
				default:
					cout << "What? ended up in the default case, with value: " << parseStage << endl;
					assert(false);
					break;
			}//end switch
		}// end control loop

		//sometimes this will be a blank string, but that's fine
		ilObj.SetSimulationClassName(currentClassName);
		//cout << "Before storage, sim classname is:>" << ilObj.GetSimulationClassName() << "<" << endl;

		//now we've read a WME pattern. If it should be duplicated, do that now.
		//However, if we're reading an entire class pattern that needs to be repeated,
		//store this pattern as part of the overall class pattern, which will be repeated
		//in its entirety, when the class has been closed

		//=============================
		//= ZERO or ONE copies needed =
		//=============================
		if(controlVariableName == "")
		{
			if(ilObj.GetSimulationClassName() != "") /*TYPED case*/
			{
				//cout << "\t\tAdded object to typedObjectsVector because it had type: " << ilObj.GetSimulationClassName() << endl;
				AddTypedObject(ilObj);
			}
			else /*UNTYPED case*/
			{
				//cout << "Adding untyped object with attribute:>" << ilObj.GetAttributeName() << "<***" << endl;
				if(lastCompletedState == READING_CLASS_CLOSE)
				{}/*doing nothing here takes care of the case where the ilObject that we have is
				essentially empty*/  //TODO set some kind of flag to take care of this.  This is just sloppy
				else
					ilObjects.push_back(ilObj);
			}
		}
		//=============================
		//=== MULTIPLE copies needed ==
		//=============================		
		if(unsatisfiedControlStructure)
		{
			//TYPED case where a pattern was just read
			if( ilObj.GetSimulationClassName() != "") 
			{
				//We CAN'T have enough information to duplicate this object, because 
				//in that case, the simulation class type would be ""
				//Since don't have enough information to duplicate yet,
				//store the piece we just read into an intermediate container
				pendingClassSpec.push_back(ilObj);	
			}

			//we have enough information duplicate the wme patterns now
			else if(classDescriptionJustFinished)
			{
				resetLoopCounters = true;
				classDescriptionJustFinished = false;

				//make a copy of this class pattern
				//ilObjVector_t actualClassPattern = typedObjects[ilObj.GetAttributeName()];
				assert(!pendingClassSpec.empty());

				for(int counter = loopBegin; counter < loopEnd; counter += incrementAmount)
				{
					string counterAsString = IntToString(counter);
					ilObjVector_t actualClassObjects;
					for(ilObjItr objItr = pendingClassSpec.begin(); objItr != pendingClassSpec.end(); /*++objItr*/)
					{
						InputLinkObject actualObject = *objItr;//makes a copy 
						assert(controlVariableName != "");
						if(controlVariableName != "")
						{
							//replace instances of control variable name with value
							actualObject.ReplaceInternalTokens(controlVariableName, counterAsString);
						}

						/* copy the now-altered patterns into the actual objects container*/
						actualClassObjects.push_back(actualObject);

						++objItr;/* ??? For some reason, incrementing here works better than incrementing in the
						'for loop' increment section, which causes an extra iteration*/
					}//for every object that is a part of the class

					ilObjVector_t& thisTypesObjects = typedObjects[actualClassObjects.front().GetSimulationClassName()];

					for(ilObjItr tempItr = actualClassObjects.begin(); tempItr != actualClassObjects.end(); ++tempItr)
					{
						thisTypesObjects.push_back(*tempItr);
					}
					typedObjects[actualClassObjects.front().GetSimulationClassName()] = thisTypesObjects;

				}// for every iteration specified in the input file

				pendingClassSpec.clear();
			}//if we have enough information to duplicate object

			else/*untyped case*/
			{
				resetLoopCounters = true;
				//cout << "\t\tMULTIPLE COPIES and UNTYPED" << endl;

				//cout << "Adding MULTIPLE of untyped object with attribute:" << actualNewObject.GetAttributeName() << endl;
				for(int counter = loopBegin; counter < loopEnd; counter += incrementAmount)
				{
					//the parts of a WME pattern that might have a field equal to the control
					//variable name are the optional "update"/"start" fields
					string counterAsString = IntToString(counter);

					InputLinkObject actualObject = ilObj;//make a copy
					actualObject.ReplaceInternalTokens(controlVariableName, counterAsString);
					ilObjects.push_back(actualObject);
				}//for each copy of the untyped WME pattern that's needed
			}//else we're doing the untyped WME case
		}//Multiple copies case

		if(resetLoopCounters)
		{
			loopBegin = defaultLoopBegin;
			loopEnd = defaultLoopEnd;
			controlVariableName = "";
			unsatisfiedControlStructure = false;
			resetLoopCounters = false;
		}

	}//while there are still lines to read

	cout << "Read " << lineNumber << " lines." << endl;
	//========  DEBUG Printing stuff =====================================
	//cout << "Number of input link entries " << ilObjects.size() << endl;
	//for(ilObjItr objItr = ilObjects.begin(); objItr != ilObjects.end(); ++objItr)
	//	cout << *objItr;

	file.close();

	return true;
}

