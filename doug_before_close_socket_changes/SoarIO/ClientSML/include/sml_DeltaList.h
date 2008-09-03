/////////////////////////////////////////////////////////////////
// DeltaList class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class records the list of changes that have
// occured to working memory since it was last sent
// to the kernel (the "delta").
//
/////////////////////////////////////////////////////////////////

#ifndef SML_DELTA_LIST_H
#define SML_DELTA_LIST_H

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <vector>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

namespace sml {

class WMElement ;
class TagWme ;

class DeltaList
{
protected:
	std::vector<TagWme*>		m_DeltaList ;

public:
	DeltaList() { }

	~DeltaList()
	{
		Clear(true) ;
	}

	// We make deleting the contents optional as
	// we may just be moving the tags out of here
	// and into a message for sending, in which case
	// we won't delete them.
	void Clear(bool deleteContents) ;

	void RemoveWME(long timeTag) ;

	void AddWME(WMElement* pWME) ;

	void UpdateWME(long timeTagToRemove, WMElement* pWME)
	{
		// This is equivalent to a remove of the old value followed by an add of the new
		// We could choose to use a single tag for this later on.
		RemoveWME(timeTagToRemove) ;
		AddWME(pWME) ;
	}

	int GetSize()			{ return (int)m_DeltaList.size() ; }
	TagWme* GetDelta(int i)	{ return m_DeltaList[i] ; }
};

}//closes namespace

#endif //SML_DELTA_LIST_H
