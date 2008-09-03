/////////////////////////////////////////////////////////////////
// Implementation for all iterator classes
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// An iterator is used to walk through a list of elements.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_ITERATOR_H
#define SML_CLIENT_ITERATOR_H

#include "sml_ClientIIterator.h"
#include "sml_ClientRelease.h"
#include "sml_ClientSML.h"
#include "sml_Connection.h"
#include "sml_ClientWME.h"

namespace sml
{

template<typename T>
class PointerIterator: public IIterator<T> {
public:
    
    /** Typedef of the template parameter */
    typedef T tReturnType;

public:
	PointerIterator(char const* pID, ClientSML* pClientSML)
	{
		SetId(pID) ;
		SetClientSML(pClientSML) ;
	}

    virtual ~PointerIterator() {}

	virtual void           Next(gSKI::Error* err = 0)
	{
		AnalyzeXML response ;

		GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IIterator_Pointer_Next, GetId()) ;
	}

	virtual bool           IsValid(gSKI::Error* err = 0) const
	{
		AnalyzeXML response ;

		if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IIterator_Pointer_IsValid, GetId()))
		{
			return response.GetResultBool(false) ;
		}

		return false ;
	}

//    virtual unsigned long  GetNumElements(Error* err = 0) const = 0;
    
	virtual tReturnType GetVal(gSKI::Error* err = 0)
	{
		AnalyzeXML response ;
		IWme* pWme ;

		if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IIterator_Pointer_GetVal, GetId()))
		{
			// BUGBUG: We need to create an object of the concrete type for "tReturnType" here.
			// For now, we're only using template<IWme*> so this will work, but we will need to solve
			// this if we want to use other iterators.
			pWme = new WME(response.GetResultString(), GetClientSML()) ;
		}

		return pWme ;
	}

	virtual void Release(gSKI::Error* err = 0)
	{
		Release::ReleaseObject(this, err) ;

		delete this ;
	}

};

/* Forward declarations for particular instantiations of the iterator */
class IProductionMatch;
class IConditionSet;
class IInstanceInfo;
class IProduction;
class ICondition;
class IMatchSet;
class IRhsAction;
class IMatch;
class IProductionMatch;
class IWme;
class IWMObject;
class IInstanceInfo;
class ISymbol;
class IAction;
class IAgent;
class IAgentThreadGroup;
class IActionElement;
class IRhsFunctionAction;
class ITestSet;
class ITest;
class IMultiAttribute;
/** Typedefs for iterator instantiations */

//typedef IIterator<IRhsFunctionAction*>        tIRhsFunctionActionIterator;
//typedef IIterator<IAgentThreadGroup*>         tIAgentThreadGroupIterator;
//typedef IIterator<IProductionMatch *>         tIProductionMatchIterator;
//typedef IIterator<IActionElement*>            tIActionElementIterator;
//typedef IIterator<egSKIPreferenceType>        tPreferenceTypeIterator;
//typedef IIterator<IInstanceInfo*>	          tIInstanceInfoIterator;
//typedef IIterator<IConditionSet *>            tIConditionSetIterator;
//typedef IIterator<IProduction *>              tIProductionIterator;
//typedef IIterator<ICondition *>               tIConditionIterator;
//typedef IIterator<IRhsAction *>               tIRhsActionIterator;
//typedef IIterator<IWMObject *>                tIWMObjectIterator;
//typedef IIterator<IMatchSet *>                tIMatchSetIterator;
//typedef IIterator<ITestSet *>                 tITestSetIterator;
//typedef IIterator<ISymbol*>                   tISymbolIterator;
//typedef IIterator<IMatch *>                   tIMatchIterator;
//typedef IIterator<IAgent*>                    tIAgentIterator;
//typedef IIterator<ITest *>                    tITestIterator;
typedef PointerIterator<IWme *>                 tWmeIterator;
//typedef IIterator<IMultiAttribute *>          tIMultiAttributeIterator;

}//closes namespace

#endif //SML_CLIENT_IITERATOR