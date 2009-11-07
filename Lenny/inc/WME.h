
#ifndef WME_H_
#define WME_H_

#include <string>

#include "Symbol.h"
#include "IdentifierSymbol.h"

class WME
{
	public:

		WME( IdentifierSymbol* newId, Symbol* newAttr, Symbol* newVal, long newUid );

		IdentifierSymbol* GetId();
		Symbol* GetAttr();
		Symbol* GetVal();
		long GetUID();
	
		std::string GetString();
	
	private:
		IdentifierSymbol* id;
		Symbol* attr;
		Symbol* val;
		long uid;

		std::string str;
};

#endif
