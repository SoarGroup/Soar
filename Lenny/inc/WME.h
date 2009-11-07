#ifndef _WME_H_
#define _WME_H_

#include "Symbol.h"
#include "IdentifierSymbol.h"

class WME {
public:
	WME(IdentifierSymbol *id, Symbol *attr, Symbol *val, long uid)
	: id(id), attr(attr), val(val), uid(uid)
	{}

	IdentifierSymbol *GetId()   { return id; }
	Symbol *GetAttr()           { return attr; }
	Symbol *GetVal()            { return val; }
	
	long GetUID() { return uid; }

	string GetString();
	
private:
	IdentifierSymbol *id;
	Symbol *attr;
	Symbol *val;
	long uid;
};

#endif