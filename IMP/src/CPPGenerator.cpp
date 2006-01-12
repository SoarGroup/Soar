#include "CPPGenerator.h"
#include "CodeGenerationConstants.h"
#include "InputLinkObject.h"

#include <iostream>
#include <cassert>

using std::string;
using std::cout; using std::endl;
using std::ostream;

extern string IntToString(int i);

CPPGenerator::CPPGenerator(string& fileName, ilObjVector_t& inObjects, typedObjectsMap_t& typed) :
			 CodeGenerator(fileName, inObjects, typed)
{

	GenerateHeaderInformation();
	GenerateCode();
}

//TODO comment
ostream& CPPGenerator::GenerateDeclareVariable(eElementType type, string& outVarName)
{	
	switch(type)
	{
		case ELEMENT_INT: //TODO replace this literal string
		  outVarName = "int" + IntToString(numIntElements++);
			file << k_pIntElement << k_space << outVarName << k_space;
			break;
		case ELEMENT_FLOAT:  //TODO replace this literal string
			outVarName = "float" + IntToString(numFloatElements++);
			file << k_pFloatElement << k_space << outVarName << k_space;
			break;
		case ELEMENT_STRING:
			outVarName = "string" + IntToString(numStringElements++);
			file << k_pStringElement << k_space << outVarName << k_space;
			break;
		case ELEMENT_ID://TODO
			break;
		default://TODO
			break;
	}
	return file;
}

void CPPGenerator::GenerateStoreWME(string& element, eElementType type)
{
	//cout << "GenerateStoreWME: received element with name: " << element << endl;
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
			//file << "m_IDWMEs";
			break;
		default://TODO
			break;
	}
	
	file << k_dot << k_push << k_openParen << element << k_closeParen 
	<< k_semi << endl;

}

//void printCreateWME(const string& wmeFunctionName, const string& parentName, )

void CPPGenerator::GenerateHeaderInformation()
{
	PrintSingleHeader(k_SML_Agent);
	file << endl << k_using << k_SML << k_scope << k_Identifier << k_semi << endl;
}

void CPPGenerator::GenerateCreateILFunctionTyped(int depth)
{
	cout << "CPPGenerator::GenerateCreateILFunctionTyped..." << endl;
	file << endl;
	PrintTabs(depth);

	//Generate the Create<type> function for each type
	for(typeMapItr_t typeItr = typedObjects.begin(); typeItr != typedObjects.end(); ++typeItr)
	{
		string className = typeItr->first;
		className[0] = toupper(className[0]);
		//Generate the function header, opening brace
		file << k_void << k_Create <<  className << k_openParen << k_AgentRef 
				<< k_AgentInstance << k_closeParen << endl;
		PrintTabs(depth) << k_openBrace << endl;

		//Generate the creation of all of the WMEs associated with this type
		PrintTabs(depth);
		ilObjVector_t& objects = typeItr->second;

		DoGenerateCreateInput(objects, depth);
		PrintZeroArgFunction(k_Commit, depth + 1, k_AgentInstanceDot);
		file << endl;

		//add closing brace
		PrintTabs(depth) << k_closeBrace << endl << endl;
	}
}

void CPPGenerator::DoGenerateCreateInput(ilObjVector_t& objects, int depth)
{
	//for now, just create the code for anything that has a start type and value
	for(ilObjVector_t::iterator objItr = objects.begin(); objItr != objects.end(); ++objItr)
	{
		string parent;
		eElementType type = objItr->GetCurrentType();
		string newVarName;

		switch(type)
		{
			case ELEMENT_TYPE_TBD:
				PrintTabs(depth + 1) << "//---------------------------------------" << endl;
				PrintTabs(depth + 1);
				file << "//Placeholder for a wme whose type needs to be determined at runtime...."	<< endl;
				PrintTabs(depth + 1) << "//The types for this wme are: ";
				objItr->PrintTypes(file);
				file << endl;
				PrintTabs(depth + 1) << "//Object's attribute name: " << objItr->GetAttributeName();
				file << ", parent: " << objItr->GetParent() << endl;
				//PrintTabs(depth + 1) << "Object's " << objItr->
				PrintTabs(depth + 1) << "//---------------------------------------" << endl;
				break;
			case ELEMENT_INT:
				PrintTabs(depth + 1);
				GenerateDeclareVariable(type, newVarName) << k_assign << k_space;
				PrintThreeArgFunction(k_CreateIntWME, objItr->GetParent(), objItr->GetAttributeName(), objItr->GetStartValue());
				file << endl;
				PrintTabs(depth +1);
				GenerateStoreWME(newVarName, type);
				break;
			case ELEMENT_FLOAT:
				PrintTabs(depth + 1);
				GenerateDeclareVariable(type, newVarName) << k_assign << k_space;
				PrintThreeArgFunction(k_CreateFloatWME, objItr->GetParent(), objItr->GetAttributeName(), objItr->GetStartValue());
				file << endl;
				PrintTabs(depth + 1);
				GenerateStoreWME(newVarName, type);
				break;
			case ELEMENT_STRING:
				PrintTabs(depth + 1);
				GenerateDeclareVariable(type, newVarName) << k_assign << k_space;
				PrintThreeArgFunction(k_CreateStringWME, objItr->GetParent(), objItr->GetAttributeName(), objItr->GetStartValue());
				file << endl;
				PrintTabs(depth + 1);
				GenerateStoreWME(newVarName, type);
				break;
			case ELEMENT_ID:
				PrintTabs(depth + 1);
				parent = objItr->GetParent();

				//This token denotes that the input link should be used as the parent object
				if(!stricmp(parent.c_str(), k_ILRootToken.c_str()))
				{
					parent = k_AgentInstance + k_dot + k_GetInputLink + k_openParen + k_closeParen;
				}

				//write out the Identifier declaration in front of the function
				file << k_pIdentifier << k_space << objItr->GetStartValue() << k_space << k_assign << k_space;
				PrintTwoArgFunction(k_CreateIdWME, parent, objItr->GetAttributeName());
				file << endl;
				break;
			default:
				assert(false);
				break;
		}//switch

		objItr->SetGeneratedName(newVarName);

	}//for - still objects to process
}

void CPPGenerator::GenerateCreateILFunction(int depth)
{
	cout << "CPPGenerator::CreateILFunction..." << endl;
	file << endl;
	PrintTabs(depth);

	file << k_void << k_IMP << k_scope << k_CreateInputLink << k_openParen;

	//feed in the arguments that the Create function takes

	file << k_AgentRef << k_AgentInstance << k_closeParen << endl;

	PrintTabs(depth) << k_openBrace << endl;

	DoGenerateCreateInput(ilObjects, depth);	
	PrintZeroArgFunction(k_Commit, depth + 1, k_AgentInstanceDot);
	file << endl;

	PrintTabs(depth) << k_closeBrace << endl;
}

void CPPGenerator::GenerateCleanupFunctionTyped(int indentDepth)
{//TODO do this
}
	
void CPPGenerator::GenerateUpdateILFunction(int indentDepth)
{
	cout << "GenerateUpdateILFunction...." << endl;

	//the resultant generated function will need to be called every cycle,
	//as that is the most-frequently any object will be updated

	file << endl;
	PrintTabs(indentDepth);

	//write out the function header and open brace
	file << k_void << k_IMP << k_scope << k_UpdateInputLink << k_openParen;
	file << k_AgentRef << k_AgentInstance << k_closeParen << endl;
	file << k_openBrace << endl;

cout << "\t\tsize of the objects list is:" << ilObjects.size() << endl;
	for(ilObjVector_t::iterator objItr = ilObjects.begin(); objItr != ilObjects.end(); ++objItr)
	{
		string updateString = objItr->GetUpdateValue();
		if(updateString == "")
			continue;//nothing specified about when to update this.  assume never.
		
		//TODO //FIXME in the future, when update frequencies are taken into account,
		// there will need to be else cases (or some other method) to handle the generation
		// of code that updates at those respective frequencies

		//write out the "if", condition, and the open brace
		PrintTabs(indentDepth + 1);
		//if the value stored in the WME is not equal to the simulation value
		//		(In the case of a string element, the std::string has an overload for
		//		!= which will allow comparison of std::string with char*)
		file << k_if << objItr->GetGeneratedName() << k_arrow << k_GetValue << k_openParen;
		file << k_closeParen << k_space << k_notEqual << k_space << updateString;
		file << k_closeParen << endl;

		PrintTabs(indentDepth + 1) << k_openBrace << endl;
		PrintTabs(indentDepth + 2);
		file << k_AgentInstance << k_dot << k_Update << k_openParen;
		file << objItr->GetGeneratedName();

		file << k_argSep << k_space << updateString << k_closeParen << k_semi << endl;

		//write out the closing brace
		PrintTabs(indentDepth + 1) << k_closeBrace << endl;
	}
	PrintTabs(indentDepth) << k_closeBrace << endl;
}

void CPPGenerator::GenerateUpdateILFunctionTyped(int indentDepth)
{
	//TODO  - actually do this
}

void CPPGenerator::GenerateCleanupFunction(int indentDepth)
{
	file << endl;
	PrintTabs(indentDepth);

	//write out the function header and open brace
	file << k_void << k_IMP << k_scope << k_CleanUp << k_openParen;
	file << k_AgentRef << k_AgentInstance << k_closeParen << endl;
	file << k_openBrace << endl;

	PrintTabs(indentDepth + 1) << k_AgentInstance << k_dot << k_GetKernel;
	file << k_openParen << k_closeParen << k_arrow << k_DestroyAgent;
	file << k_openParen << k_andpersand << k_AgentInstance << k_closeParen << k_semi << endl;
	PrintZeroArgFunction(k_Commit, indentDepth + 1, k_AgentInstanceDot);
	file << endl;

	//write out the closing brace
	PrintTabs(indentDepth);
	file << k_closeBrace << endl;
}


void CPPGenerator::GenerateCode()
{
cout << "CPPGenerator::GenerateCode....." << endl;

	int indent = 0;
	//The create function MUST be called before the Update function,
	//or else the individual ilobjects will not know their generated name,
	//and will not be able to insert that name into the generated update code
	GenerateCreateILFunction(indent);
	GenerateCreateILFunctionTyped(indent);
	//This MUST trail the generation of the input link creation code
	GenerateUpdateILFunction(indent);

	GenerateCleanupFunction(indent);
}


