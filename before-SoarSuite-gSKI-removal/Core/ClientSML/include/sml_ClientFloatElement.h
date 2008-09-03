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
	friend class WorkingMemory ;

protected:
	// The value for this wme is a floating point (double)
	double	m_Value ;

	// We need to convert to a string form at times
	std::string m_StringForm ;

public:
	// Returns the type of the value stored here (e.g. "string" or "int" etc.)
	virtual char const* GetValueType() const	;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() const ;

	// Returns the current value
	double GetValue() const { return m_Value ; }
	
	virtual FloatElement* ConvertToFloatElement() { return this; }

protected:
	FloatElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, double value, long timeTag) ;
	virtual ~FloatElement(void);

	void SetValue(double value)
	{
		m_Value = value ;
	}

#ifdef SML_DIRECT
	virtual Direct_WME_Handle DirectAdd(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle wmobject, long timeTag) ;
#endif
};

}	// namespace

#endif // SML_FLOAT_ELEMENT_H
