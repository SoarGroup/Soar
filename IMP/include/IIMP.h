#ifndef IMP_INTERFACE_H
#define IMP_INTERFACE_H

#include "sml_ClientAgent.h"
#include <string>
#include <vector>
#include <map>


struct stringsLess
{
	bool operator() (const std::string first, const std::string second) const
	{
		return first < second;
	}
};

class IIMP
{
public:
	void CreateInputLink(sml::Agent& agent);
	void CleanUp(sml::Agent& agent);
	void Update(sml::Agent& agent);
private:
	std::vector<sml::StringElement*> m_stringWMEs;
	std::vector<sml::IntElement*>	 m_intWMEs;
	std::vector<sml::FloatElement*>	 m_floatWMEs;
	std::map<std::string, sml::Identifier*, stringsLess> m_idWMEs;
};

#endif