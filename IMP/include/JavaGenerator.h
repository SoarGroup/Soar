#ifndef JAVAGENERATOR_H
#define JAVAGENERATOR_H

/*
	This class defines the Java flavor of the ClientSML interface code
*/

class JavaGenerator : public CodeGenerator
{
public:
	JavaGenerator(std::string& fileName);
private:
protected:
	//TODO this doesn't inherit everything that it should (but if the correct fxns are made
	//pure virtual, a definition will be forced, or else this won't link)
	void GenerateHeaderInformation(){}//TODO define
	void GenerateCode(){} //TODO define
	void GenerateCreateILFunction(int indentDepth = 0){} //TODO define
	void GenerateUpdateILFunction(int indentDepth = 0){} //TODO define
	void GenerateCleanupFunction(int indentDepth = 0){} //TODO define
};

#endif //JAVAGENERATOR_H