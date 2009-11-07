#include <string.h>
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

void Symbol::InitSymbol( long newUID, const char* newStr )
{
	UID = newUID;
	str = strdup( newStr );
}
