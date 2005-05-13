#ifndef IMP_CODE_GENERATOR
#define IMP_CODE_GENERATOR

#include "CodeGenerationConstants.h"
#include "ilspec.h"

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
		file.open(fileName.c_str(), std::ios::out);//TODO open with ios overwrite flags
		
		if(!file.is_open())
		{
			std::cout << "Error: unable to open file " << fileName << std::endl;
			pause();
			exit(-1);
		}
	}

private:

protected:
	//This function will be responsible for writing out the
	//"#include"s or imports to bring the necessary symbols into scope
	virtual void generateHeaderInformation() = 0;
	
	//this is just a placeholder function
	virtual void generateCode() = 0;
	
	virtual void generateCreateILFunction(int depth) = 0;
	
	virtual void generateUpdateILFunction() = 0;
	
	virtual void generateCleanupFunction() = 0;
	
	void printSingleImport(const std::string& name)
	{
		file << k_import << name << k_semi << std::endl;
	}

	void printSingleHeader(const std::string& fileName)
	{
		file << k_include << k_dQuote << fileName << k_hExtension << k_dQuote << std::endl;
	}

	void printSingleArgFunction(const std::string& functName, const std::string& arg1, int numTabs = 0)
	{
		for(int counter = 0; counter < numTabs; ++ counter) file << "\t";
		
		file << functName << k_openParen << arg1 << k_closeParen << k_semi;
	}

	void printTwoArgFunction(const std::string& functName, const std::string& arg1, const std::string& arg2, int numTabs = 0)
	{
		for(int counter = 0; counter < numTabs; ++ counter) file << "\t";
			
		file << functName << k_openParen << arg1 << k_argSep << k_space 
		<< arg2 << k_closeParen << k_semi;
	}

	void printThreeArgFunction(const std::string& functName, const std::string& arg1, const std::string& arg2, const std::string& arg3, int numTabs = 0)
	{
		for(int counter = 0; counter < numTabs; ++ counter) file << "\t";
		
		file << functName << k_openParen << arg1 << k_argSep << k_space
		<< arg2 << k_argSep << k_space << arg3 << k_closeParen << k_semi;
	}

	//Filename to write out the generated code to
	std::fstream file;
	
	ilObjVector_t& ilObjects;
};


class CPPGenerator : public CodeGenerator
{
public:
	CPPGenerator(std::string& fileName, ilObjVector_t& ilObjects);
private:
protected:
	void generateHeaderInformation();
	void generateCode();
	void generateCreateILFunction(int depth);
	void generateUpdateILFunction();
	void generateCleanupFunction();
};

class JavaGenerator : public CodeGenerator
{
public:
	JavaGenerator(std::string& fileName);
private:
protected:
	void generateHeaderInformation(){}//TODO define
	void generateCode(){} //TODO define
	void generateCreateILFunction(int depth){} //TODO define
	void generateUpdateILFunction(){} //TODO define
	void generateCleanupFunction(){} //TODO define
};

#endif IMP_CODE_GENERATOR