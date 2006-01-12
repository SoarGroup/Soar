#include "CodeGenerator.h"

/*
	This class generates the C++ flavor of the SML client interface code.
	See superclass header for descriptions of functions
*/
class CPPGenerator : public CodeGenerator
{
public:
	CPPGenerator(std::string& fileName, ilObjVector_t& ilObjects, typedObjectsMap_t& typed);
private:

protected:
	void DoGenerateCreateInput(ilObjVector_t& objects, int depth = 0);
	void GenerateHeaderInformation();
	void GenerateCode();
	void GenerateStoreWME(std::string& element, eElementType type);
	std::ostream& GenerateDeclareVariable(eElementType type, std::string& outVarName);//TODO work on hierarchy (move up)
	void GenerateCreateILFunction(int indentDepth = 0);
	void GenerateCreateILFunctionTyped(int indentDepth = 0);
	void GenerateUpdateILFunction(int indentDepth = 0);
	void GenerateUpdateILFunctionTyped(int indentDepth = 0);
	void GenerateCleanupFunction(int indentDepth = 0);
	void GenerateCleanupFunctionTyped(int indentDepth = 0);
};
