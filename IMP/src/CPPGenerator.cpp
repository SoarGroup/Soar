#include "CodeGenerator.h"
#include "CodeGenerationConstants.h"
#include "CodeGeneratorUtilities.h"
#include "ilobject.h"

#include <iostream>
#include <cassert>

using std::string;
using std::cout; using std::endl;

CPPGenerator::CPPGenerator(string& fileName, ilObjVector_t& inObjects) :
			 CodeGenerator(fileName, inObjects)
{

	generateHeaderInformation();
	generateCode();
}



//void printCreateWME(const string& wmeFunctionName, const string& parentName, )

void CPPGenerator::generateHeaderInformation()
{

	printSingleHeader(k_SML_Agent);

}

void CPPGenerator::generateCreateILFunction(int depth)
{
	cout << "CPPGenerator::CreateILFunction..." << endl;
	file << endl;
	for(int counter = 0; counter < depth; ++ counter) file << "\t";
	
	file << k_CreateInputLink << k_openParen << k_closeParen << endl;

	for(int counter = 0; counter < depth; ++ counter) file << "\t";
	
	file << k_openBrace << endl;
	
	//for now, just create the code for anything that has a start type and value
	for(ilObjVector_t::iterator objItr = ilObjects.begin(); objItr != ilObjects.end(); ++objItr)
	{

		switch(objItr->getCurrentType())
		{
			case ELEMENT_TYPE_TBD:
				for(int counter = 0; counter <= depth; ++ counter) file << "\t";
				file << "//Place holder for a wme whose type needs to be determined at runtime...."	<< endl;
				break;
			case ELEMENT_INT:
				for(int counter = 0; counter <= depth; ++ counter) file << "\t";
				printThreeArgFunction(k_CreateIntWME, objItr->getParent(), objItr->getAttributeName(), objItr->getValue());
				file << endl;
				break;	
			case ELEMENT_FLOAT:
				for(int counter = 0; counter <= depth; ++ counter) file << "\t";
				printThreeArgFunction(k_CreateFloatWME, objItr->getParent(), objItr->getAttributeName(), objItr->getValue());
				file << endl;
				break;
			case ELEMENT_STRING:
				for(int counter = 0; counter <= depth; ++ counter) file << "\t";
				printThreeArgFunction(k_CreateStringWME, objItr->getParent(), objItr->getAttributeName(), objItr->getValue());
				file << endl;
				break;
			case ELEMENT_ID:
				for(int counter = 0; counter <= depth; ++ counter) file << "\t";
				printTwoArgFunction(k_CreateIdWME, objItr->getParent(), objItr->getAttributeName());
				file << endl;
				break;
			default:
				assert(false);
				break;
		}
	}//for - still objects to process

for(int counter = 0; counter < depth; ++ counter) file << "\t";
file << k_closeBrace << endl;
	
}

void CPPGenerator::generateUpdateILFunction()
{

}
void CPPGenerator::generateCleanupFunction(){}
void CPPGenerator::generateCode()
{
cout << "CPPGenerator::generateCode....." << endl;

	generateCreateILFunction(1);
	generateUpdateILFunction();
	generateCleanupFunction();




	
}