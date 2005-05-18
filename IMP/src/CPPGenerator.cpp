#include "CodeGenerator.h"
#include "CodeGenerationConstants.h"
#include "CodeGeneratorUtilities.h"
#include "ilobject.h"

#include <iostream>
#include <cassert>

using std::string;
using std::cout; using std::endl;
using std::ostream;

extern string intToString(int i);

CPPGenerator::CPPGenerator(string& fileName, ilObjVector_t& inObjects) :
			 CodeGenerator(fileName, inObjects)
{

	generateHeaderInformation();
	generateCode();
}

//TODO comment
ostream& CPPGenerator::generateDeclareVariable(eElementType type, string& outVarName)
{	
	switch(type)
	{
		case ELEMENT_INT: //TODO replace this literal string
		  outVarName = "int" + intToString(numIntElements++);
			file << k_pIntElement << k_space << outVarName << k_space;
			break;
		case ELEMENT_FLOAT:  //TODO replace this literal string
			outVarName = "float" + intToString(numFloatElements++);
			file << k_pFloatElement << k_space << outVarName << k_space;
			break;
		case ELEMENT_STRING:
			outVarName = "string" + intToString(numStringElements++);
			file << k_pStringElement << k_space << outVarName << k_space;
			break;
		case ELEMENT_ID://TODO
			break;
		default://TODO
			break;
	}
	return file;
}

void CPPGenerator::generateStoreWME(string& element, eElementType type)
{
	cout << "GenerateStoreWME: received element with name: " << element << endl;
	switch(type)
	{
		case ELEMENT_INT: //TODO replace this literal string
			file << "m_intWMEs";
			break;
		case ELEMENT_FLOAT:  //TODO replace this literal string
			file << "m_floatWMEs";
			break;
		case ELEMENT_STRING:
			file << "m_stringWMEs";
			break;
		case ELEMENT_ID://TODO
			break;
		default://TODO
			break;
	}
	
	file << k_dot << k_push << k_openParen << element << k_closeParen 
	<< k_semi << endl;

}

//void printCreateWME(const string& wmeFunctionName, const string& parentName, )

void CPPGenerator::generateHeaderInformation()
{
	printSingleHeader(k_SML_Agent);
	file << endl << k_using << k_SML << k_scope << k_Identifier << k_semi << endl;

}

void CPPGenerator::generateCreateILFunction(int depth)
{
	cout << "CPPGenerator::CreateILFunction..." << endl;
	file << endl;
	printTabs(depth);

	file << k_void << k_IMP << k_scope << k_CreateInputLink << k_openParen;

	//feed in the arguments that the Create function takes

	file << k_AgentRef << k_AgentInstance << k_closeParen << endl;

	printTabs(depth) << k_openBrace << endl;

	//for now, just create the code for anything that has a start type and value
	for(ilObjVector_t::iterator objItr = ilObjects.begin(); objItr != ilObjects.end(); ++objItr)
	{
		string parent;
		eElementType type = objItr->getCurrentType();
		string newVarName;

		switch(type)
		{
			case ELEMENT_TYPE_TBD:
				printTabs(depth + 1);
				file << "//Placeholder for a wme whose type needs to be determined at runtime...."	<< endl;
				break;
			case ELEMENT_INT:
				printTabs(depth + 1);
				generateDeclareVariable(type, newVarName) << k_assign << k_space;
				printThreeArgFunction(k_CreateIntWME, objItr->getParent(), objItr->getAttributeName(), objItr->getStartValue());
				file << endl;
				for(int counter = 0; counter <= depth; ++counter) file << "\t";				
				generateStoreWME(newVarName, type);
				break;	
			case ELEMENT_FLOAT:
				printTabs(depth + 1);
				printThreeArgFunction(k_CreateFloatWME, objItr->getParent(), objItr->getAttributeName(), objItr->getStartValue());
				file << endl;
				break;
			case ELEMENT_STRING:
				printTabs(depth + 1);
				printThreeArgFunction(k_CreateStringWME, objItr->getParent(), objItr->getAttributeName(), objItr->getStartValue());
				file << endl;
				break;
			case ELEMENT_ID:
				printTabs(depth + 1);

				parent = objItr->getParent();

				//This token denotes that the input link should be used as the parent object					
				if(!stricmp(parent.c_str(), k_ILRootToken.c_str()))
				{
				  parent = k_AgentInstance + k_dot + k_GetInputLink + k_openParen + k_closeParen;
				}
				
				//write out the Identifier declaration in front of the function
				file << k_Identifier << k_space << objItr->getStartValue() << k_space << k_assign << k_space;
				printTwoArgFunction(k_CreateIdWME, parent, objItr->getAttributeName());
				file << endl;
				break;
			default:
				assert(false);
				break;
		}//switch
		
		objItr->setGeneratedName(newVarName);		
cout << "Setting an object's generated name to: "	<< newVarName << endl;
cout << "\tand confirming: " << objItr->getGeneratedName() << endl;

	}//for - still objects to process

	printTabs(depth) << k_closeBrace << endl;	
}

void CPPGenerator::generateUpdateILFunction(int indentDepth)
{
	cout << "generageUpdateILFunction...." << endl;

	//the resultant generated function will need to be called every cycle,
	//as that is the most-frequently any object will be updated

	file << endl;
	printTabs(indentDepth);

	//write out the function header and open brace
	file << k_void << k_IMP << k_scope << k_UpdateInputLink << k_openParen;
	file << k_AgentRef << k_AgentInstance << k_closeParen << endl;
	file << k_openBrace << endl;

	for(ilObjVector_t::iterator objItr = ilObjects.begin(); objItr != ilObjects.end(); ++objItr)
	{ //FIME //TODO this condition alone isn't enough to determine if something gets printed
		string updateString = objItr->getUpdateValue();
		if(updateString == "")
			continue;

		//write out the "if", condition, and the open brace
		printTabs(indentDepth + 1);
		file << k_if << updateString << k_closeParen << endl;

		printTabs(indentDepth + 1)  << k_openBrace << endl;
		printTabs(indentDepth + 2);
		file << k_AgentInstance << k_dot << k_Update << k_openParen;
		file << objItr->getGeneratedName();
		file << k_argSep << k_space << updateString << k_closeParen << k_semi << endl;

		//write out the closing brace
		printTabs(indentDepth + 1) << k_closeBrace << endl;
	}
	printTabs(indentDepth) << k_closeBrace << endl;	
}

void CPPGenerator::generateCleanupFunction(int indentDepth)
{//TODO

	file << endl;
	printTabs(indentDepth);

	//write out the function header and open brace
	file << k_void << k_IMP << k_scope << k_CleanUp << k_openParen;
	file << k_AgentRef << k_AgentInstance << k_closeParen << endl;
	file << k_openBrace << endl;

	printTabs(indentDepth + 1) << k_AgentInstance << k_dot << k_GetKernel;
	file << k_openParen << k_closeParen << k_arrow << k_DestroyAgent;
	file << k_openParen << k_AgentInstance << k_closeParen << k_semi << endl;	

	//write out the closing brace
	printTabs(indentDepth);
	file << k_closeBrace << endl;
}


void CPPGenerator::generateCode()
{
cout << "CPPGenerator::generateCode....." << endl;

	int indent = 0;
	//The create function MUST be called before the Update fucntion,
	//or else the individual ilobjects will not know their generated name,
	//and will not be able to insert that name into the generated update code
	generateCreateILFunction(indent);	
	
	//This MUST trail the generation of the input link creation code
	generateUpdateILFunction(indent);
	
	generateCleanupFunction(indent);	
}

ostream& CPPGenerator::printTabs(int indentDepth)
{
	for(int counter = 0; counter < indentDepth; ++ counter) file << "\t";
	return file;
}
