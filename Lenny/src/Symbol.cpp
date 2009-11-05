
#include "Symbol.h"

using namespace std;

long Symbol::GetUID()
{
	return UID;
}

const char* Symbol::GetString()
{
	return str;
}

bool Symbol::IsConst()
{
	return cnst;
}

//

void Symbol::InitSymbol( long newUID, string& newStr, bool newCnst )
{
	UID = newUID;
	cnst = newCnst;

	stdstr.assign( newStr );
	str = stdstr.c_str();
}
