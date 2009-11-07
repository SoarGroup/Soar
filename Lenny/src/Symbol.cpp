#include <string.h>
#include "Symbol.h"

using namespace std;

long Symbol::GetUID()
{
	return UID;
}

const char *Symbol::GetString()
{
	return str;
}

bool Symbol::IsConst()
{
	return cnst;
}

//

void Symbol::InitSymbol( long newUID, SymbolType newType, const char *newStr, bool newCnst )
{
	UID = newUID;
	cnst = newCnst;
	str = strdup(newStr);
	type = newType;
}

SymbolType Symbol::GetType()
{
	return type;
}