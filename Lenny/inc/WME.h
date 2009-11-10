
#ifndef WME_H_
#define WME_H_

#include <string>

#include "Symbol.h"
#include "IdentifierSymbol.h"

namespace EpmemNS {

typedef long WMEUID;

class WME
{
	public:

		WME( IdentifierSymbol* newId, Symbol* newAttr, Symbol* newVal, WMEUID newUid );

		IdentifierSymbol* GetId();
		Symbol* GetAttr();
		Symbol* GetVal();
		WMEUID GetUID();
	
		std::string GetString();
	
	private:
		IdentifierSymbol* id;
		Symbol* attr;
		Symbol* val;
		WMEUID uid;

		std::string str;
};
}

#endif
