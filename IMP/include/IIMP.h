#ifndef IMP_INTERFACE_H
#define IMP_INTERFACE_H

#include "sml_ClientAgent.h"
#include "CodeGeneratorUtilities.h"
#include <string>
#include <vector>
#include <map>


/*
	This class should be thought of as an interface (with a little state).
	An environment using IMP-generated code should implement this interface.
	
*/
class IIMP
{
public:
	void CreateInputLink(sml::Agent& agent) = 0;
	void CleanUp(sml::Agent& agent) = 0;
	void Update(sml::Agent& agent) = 0;
private:
	std::vector<sml::StringElement*> m_stringWMEs;
	std::vector<sml::IntElement*>	 m_intWMEs;
	std::vector<sml::FloatElement*>	 m_floatWMEs;
	std::map<std::string, sml::Identifier*, stringsLess> m_idWMEs;
};

#endif