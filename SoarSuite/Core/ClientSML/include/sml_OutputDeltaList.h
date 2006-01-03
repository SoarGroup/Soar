/////////////////////////////////////////////////////////////////
// OutputDeltaList class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class records the list of changes that have
// occured to the output-link since the client
// last asked for them.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_OUTPUT_DELTA_LIST_H
#define SML_OUTPUT_DELTA_LIST_H

#include <vector>

namespace sml {

class WMElement ;

class WMDelta
{
public:
	enum ChangeType { kAdded = 1, kRemoved } ;

protected:
	ChangeType	m_ChangeType ;

	// If this is an element that has been removed then
	// we actually own this pointer.  Otherwise we don't.  Be careful.
	WMElement*	m_pWME ;

public:
	WMDelta(ChangeType change, WMElement* pWME)
	{
		m_ChangeType = change ;
		m_pWME		 = pWME ;
	}

	~WMDelta() ;

	ChangeType getChangeType() { return m_ChangeType ; }
	WMElement* getWME()		   { return m_pWME ; }
} ;

class OutputDeltaList
{
protected:
	std::vector<WMDelta*>		m_DeltaList ;

public:
	OutputDeltaList() { }

	~OutputDeltaList()
	{
		Clear(true) ;
	}

	void Clear(bool deleteContents)
	{
		if (deleteContents)
		{
			int size = (int)m_DeltaList.size() ;
			for (int i = 0 ; i < size ; i++)
				delete m_DeltaList[i] ;
		}

		m_DeltaList.clear() ;
	}

	void RemoveWME(WMElement* pWME)
	{
		WMDelta* pDelta = new WMDelta(WMDelta::kRemoved, pWME) ;
		m_DeltaList.push_back(pDelta) ;
	}

	void AddWME(WMElement* pWME)
	{
		WMDelta* pDelta = new WMDelta(WMDelta::kAdded, pWME) ;
		m_DeltaList.push_back(pDelta) ;
	}

	int GetSize()				{ return (int)m_DeltaList.size() ; }
	WMDelta* GetDeltaWME(int i)	{ return m_DeltaList[i] ; }
};

}//closes namespace

#endif //SML_OUTPUT_DELTA_LIST_H
