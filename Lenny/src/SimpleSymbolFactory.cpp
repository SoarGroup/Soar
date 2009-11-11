#include "SimpleSymbolFactory.h"
#include <utility>

using namespace EpmemNS;
using namespace std;

SimpleSymbolFactory::SimpleSymbolFactory()
	: counter(1), uidmap(), intmap(), floatmap(), strmap(), idmap()
	{
		std::pair<char, long> s1('S', 1);
		idmap[s1] = 0;
		uidmap[0] = NewIdentifierSymbol(0, 'S', 1);
	}

IntegerSymbol* SimpleSymbolFactory::GetIntegerSymbol( long val ) {
	IntegerSymbol *s;
	if (intmap.find(val) == intmap.end()) {
		intmap[val] = counter;
		s = NewIntegerSymbol(counter, val);
		uidmap[counter++] = s;
	} else {
		s = static_cast<IntegerSymbol*>(uidmap[intmap[val]]);
	}
	return s;
}

FloatSymbol* SimpleSymbolFactory::GetFloatSymbol( double val ) {
	FloatSymbol *s;
	if (floatmap.find(val) == floatmap.end()) {
		floatmap[val] = counter;
		s = NewFloatSymbol(counter, val);
		uidmap[counter++] = s;
	} else {
		s = static_cast<FloatSymbol*>(uidmap[floatmap[val]]);
	}
	return s;
}

StringSymbol* SimpleSymbolFactory::GetStringSymbol( const char* val ) {
	StringSymbol *s;
	if (strmap.find(val) == strmap.end()) {
		strmap[val] = counter;
		s = NewStringSymbol(counter, val);
		uidmap[counter++] = s;
	} else {
		s = static_cast<StringSymbol*>(uidmap[strmap[val]]);
	}
	return s;
}

IdentifierSymbol* SimpleSymbolFactory::GetIdentifierSymbol( char letter, long number ) {
	pair<char, long> x(letter, number);
	IdentifierSymbol* s;
	if (idmap.find(x) == idmap.end()) {
		idmap[x] = counter;
		s = NewIdentifierSymbol(counter, letter, number);
		uidmap[counter++] = s;
	} else {
		s = static_cast<IdentifierSymbol*>(uidmap[idmap[x]]);
	}
	return s;
}

Symbol* SimpleSymbolFactory::GetSymbolByUID( SymbolUID uid ) {
	if (uidmap.find(uid) == uidmap.end()) {
		return NULL;
	} else {
		return uidmap[uid];
	}
}