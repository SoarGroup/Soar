#ifndef IMP_CODE_GENERATOR
#define IMP_CODE_GENERATOR

#include "CodeGenerationConstants.h"
#include "ilspec.h"
#include "ilobject.h"

#include <fstream>
#include <iostream>
#include <string>

extern void Pause();

class CodeGenerator
{
public:

	CodeGenerator(std::string& fileName, ilObjVector_t& inObjects, typedObjectsMap_t& inTyped) : 
			ilObjects(inObjects), typedObjects(inTyped)
	{
		file.open(fileName.c_str(), std::ios::out);

		if(!file.is_open())
		{
			std::cout << "Error: unable to open file " << fileName << std::endl;
			Pause();
			exit(-1);
		}
		numStringElements = 0;
		numIntElements = 0;
		numFloatElements = 0;
	}

private:

protected:

	virtual std::ostream& PrintTabs(int indentDepth);
	
	//This function will be responsible for writing out the
	//"#include"s or imports to bring the necessary symbols into scope
	virtual void GenerateHeaderInformation() = 0;

	//parent function that calls all of the specific code generation functions
	virtual void GenerateCode() = 0;

	//does the dirty work of the CreateILFunction. Separate so that it can be reused in
	//other contexts 	//TODO make this pure virtual
	virtual void DoGenerateCreateInput(ilObjVector_t& objects, int depth = 0){}

	virtual void GenerateCreateILFunction(int indentDepth = 0) = 0;

	virtual void GenerateUpdateILFunction(int indentDepth = 0) = 0;

	virtual void GenerateCleanupFunction(int indentDepth = 0) = 0;

	void PrintSingleImport(const std::string& name);

	void PrintSingleHeader(const std::string& fileName);
	
	virtual void PrintZeroArgFunction(const std::string&functName, int numTabs = 0, const std::string& object = "");

	virtual void PrintSingleArgFunction(const std::string& functName, const std::string& arg1, int numTabs = 0, const std::string& object = "");

	virtual void PrintTwoArgFunction(const std::string& functName, const std::string& arg1, const std::string& arg2, int numTabs = 0, const std::string& object = "");

	virtual void PrintThreeArgFunction(const std::string& functName, const std::string& arg1, const std::string& arg2, const std::string& arg3, int numTabs = 0, const std::string& object = "");

	//Filename to write out the generated code to
	std::fstream file;

	int numIntElements;
	int numStringElements;
	int numFloatElements;

	ilObjVector_t& ilObjects;
	typedObjectsMap_t& typedObjects;
};


class CPPGenerator : public CodeGenerator
{
public:
	CPPGenerator(std::string& fileName, ilObjVector_t& ilObjects, typedObjectsMap_t& typed);
private:

protected:
	void DoGenerateCreateInput(ilObjVector_t& objects, int depth = 0);
	void GenerateHeaderInformation();
	void GenerateCode();
	void GenerateStoreWME(std::string& element, eElementType type);//TODO  work on hierarchy for this
	//void generateDeclareVariable(eElementType type);//TODO work on hierarchy (move up)

	/************************************************************************/
	/* Create a variable declaration with the given type.  The internally created name
	   is "passed" back in the reference arg
	/************************************************************************/
	std::ostream& GenerateDeclareVariable(eElementType type, std::string& outVarName);//TODO work on hierarchy (move up)
	void GenerateCreateILFunction(int indentDepth = 0);
	void GenerateCreateILFunctionTyped(int indentDepth = 0);//TODO move this up
	void GenerateUpdateILFunction(int indentDepth = 0);
	void GenerateUpdateILFunctionTyped(int indentDepth = 0);//TODO move this up
	void GenerateCleanupFunction(int indentDepth = 0);
	void GenerateCleanupFunctionTyped(int indentDepth = 0);//TODO move this up
};

class JavaGenerator : public CodeGenerator
{
public:
	JavaGenerator(std::string& fileName);
private:
protected:
//TODO this doesn't inherit everything that it should (but if the correct fxns are made
//pure virtual, it will force a definition
	void GenerateHeaderInformation(){}//TODO define
	void GenerateCode(){} //TODO define
	void GenerateCreateILFunction(int indentDepth = 0){} //TODO define
	void GenerateUpdateILFunction(int indentDepth = 0){} //TODO define
	void GenerateCleanupFunction(int indentDepth = 0){} //TODO define
};

#endif IMP_CODE_GENERATOR