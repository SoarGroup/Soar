/////////////////////////////////////////////////////////////////
// FloatElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has a floating point value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_FLOAT_ELEMENT_H
#define SML_FLOAT_ELEMENT_H

#include "sml_ClientWMElement.h"

#include <string>

namespace sml {

class WorkingMemory ;
class Identifier ;

class FloatElement : public WMElement
{
	// Allow working memory to create these objects directly (user must use agent class to do this)
	friend WorkingMemory ;

protected:
	// The value for this wme is a floating point (double)
	double	m_Value ;

	// We need to convert to a string form at times
	std::string m_StringForm ;

public:
	// Returns the type of the value stored here (e.g. "string" or "int" etc.)
	virtual char const* GetValueType()	;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() ;

protected:
	FloatElement(Agent* pAgent, Identifier* pID, char const* pAttributeName, double value) ;
	virtual ~FloatElement(void);

	void SetValue(double value)
	{
		m_Value = value ;
	}

};

}	// namespace

#endif // SML_FLOAT_ELEMENT_H
