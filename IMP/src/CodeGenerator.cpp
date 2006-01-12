#include "CPPGenerator.h"

using std::string;
using std::ostream;

/*  PRIVATE FUNCTION DEFS  */

/*  PROTECTED FUNCTION DEFS  */
ostream& CodeGenerator::PrintTabs(int indentDepth)
{
	for(int counter = 0; counter < indentDepth; ++ counter) file << "\t";
	return file;
}

void CodeGenerator::PrintSingleHeader(const string& fileName)
{
	file << k_include << k_dQuote << fileName << k_hExtension << k_dQuote << std::endl;
}

void CodeGenerator::PrintSingleImport(const string& name)
{
	file << k_import << name << k_semi << std::endl;
}

void CodeGenerator::PrintZeroArgFunction(const string& functName, int numTabs, const string& object)
{
	PrintTabs(numTabs);
	file << object << functName << k_openParen << k_closeParen << k_semi;
}

void CodeGenerator::PrintSingleArgFunction(const string& functName, const string& arg1, int numTabs, const string& object)
{
	PrintTabs(numTabs);
	file << object << functName << k_openParen << arg1 << k_closeParen << k_semi;
}

void CodeGenerator::PrintTwoArgFunction(const std::string& functName, const std::string& arg1, const std::string& arg2, int numTabs, const std::string& object)
{
	PrintTabs(numTabs);
	file << object << functName << k_openParen << arg1 << k_argSep << k_space
		<< arg2 << k_closeParen << k_semi;
}

void CodeGenerator::PrintThreeArgFunction(const string& functName, const string& arg1, const string& arg2, const string& arg3, int numTabs, const string& object)
{
	PrintTabs(numTabs);
	file << object << functName << k_openParen << arg1 << k_argSep << k_space
		<< arg2 << k_argSep << k_space << arg3 << k_closeParen << k_semi;
}