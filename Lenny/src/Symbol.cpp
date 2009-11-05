
#include "Symbol.h"

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

Symbol::Symbol( long newUID, const char *newStr, bool newCnst ): UID( newUID ), cnst( newCnst )
{
	stdstr.assign( newStr );
	str = stdstr.c_str();
}
