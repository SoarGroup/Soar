/////////////////////////////////////////////////////////////////
// Implementation for all iterator classes
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// An iterator is used to walk through a list of elements.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientIterator.h"
#include "sml_Connection.h"

using namespace sml ;

// I tried putting these implementations here, but for some reason I get a linker
// error when we use the class and I don't see why.  I guess the signature is wrong here?
// Or maybe the compiler simply can't handle it because it only sees the header when
// the template is used.
// Anyway, for now I'm putting them in the header.

/*
template<typename T> void PointerIterator<T>::Next(gSKI::Error* err)
{
	AnalyzeXML response ;

	GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IIterator_Pointer_Next, GetId()) ;
}

template<typename T> bool PointerIterator<typename T>::IsValid(gSKI::Error* err) const
{
	AnalyzeXML response ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IIterator_Pointer_IsValid, GetId()))
	{
		return response.GetResultBool() ;
	}

	return false ;
}
*/