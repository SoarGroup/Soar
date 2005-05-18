#ifndef IMP_CODE_GENERATOR
#define IMP_CODE_GENERATOR

#include "CodeGenerationConstants.h"
#include "ilspec.h"
#include "ilobject.h"

#include <fstream>
#include <iostream>
#include <string>

extern void pause();

class CodeGenerator
{
public:

	CodeGenerator(std::string& fileName, ilObjVector_t& inObjects) :
				 ilObjects(inObjects)
	{
		file.open(fileName.c_str(), std::ios::out);
		
		if(!file.is_open())
		{
			std::cout << "Error: unable to open file " << fileName << std::endl;
			pause();
			exit(-1);
		}
		numStringElements = 0;
		numIntElements = 0;
		numFloatElements = 0;
	}

private:

protected:
	//This function will be responsible for writing out the
	//"#include"s or imports to bring the necessary symbols into scope
	virtual void generateHeaderInformation() = 0;
	
	//this is just a placeholder function
	virtual void generateCode() = 0;
	
	virtual void generateCreateILFunction(int indentDepth = 0) = 0;
	
	virtual void generateUpdateILFunction(int indentDepth = 0) = 0;
	
	virtual void generateCleanupFunction(int indentDepth = 0) = 0;
	
	//virtual void 
	
	void printSingleImport(const std::string& name)
	{
		file << k_import << name << k_semi << std::endl;
	}

	void printSingleHeader(const std::string& fileName)
	{
		file << k_include << k_dQuote << fileName << k_hExtension << k_dQuote << std::endl;
	}

	void printSingleArgFunction(const std::string& functName, const std::string& arg1, int numTabs = 0, std::string object = "")
	{
		for(int counter = 0; counter < numTabs; ++ counter) file << "\t";
		
		file << object << functName << k_openParen << arg1 << k_closeParen << k_semi;
	}

	void printTwoArgFunction(const std::string& functName, const std::string& arg1, const std::string& arg2, int numTabs = 0, std::string object = "")
	{
		for(int counter = 0; counter < numTabs; ++ counter) file << "\t";
			
		file << object << functName << k_openParen << arg1 << k_argSep << k_space 
		<< arg2 << k_closeParen << k_semi;
	}

	void printThreeArgFunction(const std::string& functName, const std::string& arg1, const std::string& arg2, const std::string& arg3, int numTabs = 0, std::string object = "")
	{
		for(int counter = 0; counter < numTabs; ++ counter) file << "\t";

		file << object << functName << k_openParen << arg1 << k_argSep << k_space
		<< arg2 << k_argSep << k_space << arg3 << k_closeParen << k_semi;
	}

	//Filename to write out the generated code to
	std::fstream file;

	int numIntElements;
	int numStringElements;
	int numFloatElements;

	ilObjVector_t& ilObjects;
};


class CPPGenerator : public CodeGenerator
{
public:
	CPPGenerator(std::string& fileName, ilObjVector_t& ilObjects);
private:
	std::ostream& printTabs(int indentDepth);
protected:
	void generateHeaderInformation();
	void generateCode();
	void generateStoreWME(std::string& element, eElementType type);//TODO  work on hierarchy for this
	//void generateDeclareVariable(eElementType type);//TODO work on hierarchy (move up)

	/************************************************************************/
	/* Create a variable declaration with the given type.  The internally created name
	   is "passed" back in the reference arg
	/************************************************************************/
	std::ostream& generateDeclareVariable(eElementType type, std::string& outVarName);//TODO work on hierarchy (move up)
	void generateCreateILFunction(int indentDepth = 0);
	void generateUpdateILFunction(int indentDepth = 0);
	void generateCleanupFunction(int indentDepth = 0);
};

class JavaGenerator : public CodeGenerator
{
public:
	JavaGenerator(std::string& fileName);
private:
protected:
	void generateHeaderInformation(){}//TODO define
	void generateCode(){} //TODO define
	void generateCreateILFunction(int indentDepth = 0){} //TODO define
	void generateUpdateILFunction(int indentDepth = 0){} //TODO define
	void generateCleanupFunction(int indentDepth = 0){} //TODO define
};

#endif IMP_CODE_GENERATOR