/////////////////////////////////////////////////////////////////
// Abstract interface for all iterator classes
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// An iterator is used to walk through a list of elements.
// This abstract interface defines type-safe versions of
// these iterators.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_IITERATOR
#define SML_CLIENT_IITERATOR

#include "sml_ClientIRelease.h"

namespace sml
{

template<typename T>
class IIterator: public IRelease {
public:
    
    /** Typedef of the template parameter */
    typedef T tReturnType;

public:
    virtual ~IIterator() {}

	virtual void           Next(gSKI::Error* err = 0) = 0;

	virtual bool           IsValid(gSKI::Error* err = 0) const = 0;

//    virtual unsigned long  GetNumElements(Error* err = 0) const = 0;
    
	virtual tReturnType    GetVal(gSKI::Error* err = 0) = 0;
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
typedef IIterator<IRhsFunctionAction*>        tIRhsFunctionActionIterator;
typedef IIterator<IAgentThreadGroup*>         tIAgentThreadGroupIterator;
typedef IIterator<IProductionMatch *>         tIProductionMatchIterator;
typedef IIterator<IActionElement*>            tIActionElementIterator;
typedef IIterator<egSKIPreferenceType>        tPreferenceTypeIterator;
typedef IIterator<IInstanceInfo*>	          tIInstanceInfoIterator;
typedef IIterator<IConditionSet *>            tIConditionSetIterator;
typedef IIterator<IProduction *>              tIProductionIterator;
typedef IIterator<ICondition *>               tIConditionIterator;
typedef IIterator<IRhsAction *>               tIRhsActionIterator;
typedef IIterator<IWMObject *>                tIWMObjectIterator;
typedef IIterator<IMatchSet *>                tIMatchSetIterator;
typedef IIterator<ITestSet *>                 tITestSetIterator;
typedef IIterator<ISymbol*>                   tISymbolIterator;
typedef IIterator<IMatch *>                   tIMatchIterator;
typedef IIterator<IAgent*>                    tIAgentIterator;
typedef IIterator<ITest *>                    tITestIterator;
typedef IIterator<IWme *>                     tIWmeIterator;
typedef IIterator<IMultiAttribute *>          tIMultiAttributeIterator;

}//closes namespace

#endif //SML_CLIENT_IITERATOR