#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

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

#include "sml_DeltaList.h"
#include "sml_ClientWMElement.h"
#include "sml_ClientIdentifier.h"
#include "sml_Connection.h"
#include "sml_TagWme.h"

using namespace sml ;

void DeltaList::RemoveWME(long timeTag)
{
// BADBAD: We should scan the existing list of tags and if we are adding this value
// just delete that tag and don't add anything to the delta list.
// (This will happen if we change a value twice within a commit cycle).
// We probably shouldn't do this if the object being removed is an identifier
// as we might leave pending adds that are children of the object.
// (Then again, that might be ok as presumably those adds would fail when we
//  got to the kernel, possibly saving a bunch of time in the matcher).

	// Create the wme tag
	TagWme* pTag = new TagWme() ;

	// For removes, we just use the time tag
	pTag->SetTimeTag(timeTag) ;
	pTag->SetActionRemove() ;

	m_DeltaList.push_back(pTag) ;	
}

void DeltaList::AddWME(WMElement* pWME)
{
	// Create the wme tag
	TagWme* pTag = new TagWme() ;

	// For adds we send everything
	pTag->SetIdentifier(pWME->GetIdentifier()->GetIdentifierSymbol()) ;
	pTag->SetAttribute(pWME->GetAttribute()) ;
	pTag->SetValue(pWME->GetValueAsString(), pWME->GetValueType()) ;
	pTag->SetTimeTag(pWME->GetTimeTag()) ;
	pTag->SetActionAdd() ;

	m_DeltaList.push_back(pTag) ;	
}

// We make deleting the contents optional as
// we may just be moving the tags out of here
// and into a message for sending, in which case
// we won't delete them.
void DeltaList::Clear(bool deleteContents)
{
	if (deleteContents)
	{
		for (int i = 0 ; i < (int)m_DeltaList.size() ; i++)
		{
			TagWme* pDelta = m_DeltaList[i] ;
			delete pDelta ;
		}
	}

	m_DeltaList.clear() ;
}
