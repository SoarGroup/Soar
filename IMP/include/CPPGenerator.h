#include "CodeGenerator.h"

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
	void GenerateCreateILFunctionTyped(int indentDepth = 0);
	void GenerateUpdateILFunction(int indentDepth = 0);
	void GenerateUpdateILFunctionTyped(int indentDepth = 0);
	void GenerateCleanupFunction(int indentDepth = 0);
	void GenerateCleanupFunctionTyped(int indentDepth = 0);
};
