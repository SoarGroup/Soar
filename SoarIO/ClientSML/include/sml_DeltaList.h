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

#include <vector>

namespace sml {

class WMElement ;

class Delta
{
public:
	virtual bool isAdd()	= 0 ;
	virtual bool isRemove() = 0 ;
	virtual ~Delta() { } ;
	virtual WMElement* GetWME() = 0 ;
} ;

class AddDelta : public Delta
{
protected:
	// We do NOT own this object (because it's been added to the working memory tree)
	WMElement*	m_Element ;

public:
	AddDelta(WMElement* pWME) { m_Element = pWME ; }

	// Don't do anything to the WME when we are deleted
	// because we don't own it.
	virtual ~AddDelta() { } ;

public:
	virtual bool isAdd()	{ return true ; }
	virtual bool isRemove()	{ return false ; }
	virtual WMElement* GetWME() { return m_Element ; }
} ;

class RemoveDelta : public Delta
{
protected:
	// We DO own this object (because it's been removed from the working memory tree)
	WMElement*	m_Element ;

public:
	RemoveDelta(WMElement* pWME) { m_Element = pWME ; }

	// Delete the wm because we do own this one.
	virtual ~RemoveDelta() ;

public:
	virtual bool isAdd()	{ return false ; }
	virtual bool isRemove()	{ return true ; }
	virtual WMElement* GetWME() { return m_Element ; }
} ;

class DeltaList
{
protected:
	std::vector<Delta*>		m_DeltaList ;

public:
	DeltaList() { }

	~DeltaList()
	{
		Clear() ;
	}

	void Clear()
	{
		for (int i = 0 ; i < (int)m_DeltaList.size() ; i++)
		{
			Delta* pDelta = m_DeltaList[i] ;
			delete pDelta ;
		}

		m_DeltaList.clear() ;
	}

	void RemoveWME(WMElement* pWME)
	{
// BADBAD: We should check if pWME is in the add list right now.  If so, we just delete it from the add list and don't add it to the remove.
// In that case, we need to delete pWME (since we're taking ownership here).

		m_DeltaList.push_back(new RemoveDelta(pWME)) ;
	}

	void AddWME(WMElement* pWME)
	{
		m_DeltaList.push_back(new AddDelta(pWME)) ;
	}

	int GetSize()			{ return (int)m_DeltaList.size() ; }
	Delta* GetDelta(int i)	{ return m_DeltaList[i] ; }
};

}//closes namespace

#endif //SML_DELTA_LIST_H