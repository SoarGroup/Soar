#ifndef _SIMPLE_SYMBOL_FACTORY_H_
#define _SIMPLE_SYMBOL_FACTORY_H_

#include "SymbolFactory.h"
#include <map>

namespace EpmemNS {

using std::map;
using std::string;
using std::pair;

class SimpleSymbolFactory : public SymbolFactory {
public:
	SimpleSymbolFactory()
	: counter(1), uidmap(), intmap(), floatmap(), strmap(), idmap()
	{
		pair<char, long> s1('S', 1);
		idmap[s1] = 0;
		uidmap[0] = NewIdentifierSymbol(0, 'S', 1);
	}

	IntegerSymbol* GetIntegerSymbol( long val ) {
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

	FloatSymbol* GetFloatSymbol( double val ) {
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

	StringSymbol* GetStringSymbol( const char* val ) {
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
	
	IdentifierSymbol* GetIdentifierSymbol( char letter, long number ) {
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
	
	Symbol* GetSymbolByUID( SymbolUID uid ) {
		if (uidmap.find(uid) == uidmap.end()) {
			return NULL;
		} else {
			return uidmap[uid];
		}
	}
	
private:
	SymbolUID counter;

	map<SymbolUID, Symbol*> uidmap;
	map<long, SymbolUID> intmap;
	map<double, SymbolUID> floatmap;
	map<string, SymbolUID> strmap;
	map<pair<char, long>, SymbolUID> idmap;
};
}

#endif