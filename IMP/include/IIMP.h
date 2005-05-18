#ifndef IMP_INTERFACE_H
#define IMP_INTERFACE_H

#include "sml_ClientAgent.h"

class IIMP
{
public:
	void CreateInputLink(sml::Agent& agent);
	void CleanUp(sml::Agent& agent);
	void Update(sml::Agent& agent);
private:
	std::vector<sml::StringElement*>	m_stringWMEs;
	std::vector<sml::IntElement*>		m_intWMEs;
	std::vector<sml::FloatElement*>	m_floatWMEs;
};

#endif