#ifndef _SIMPLE_SYMBOL_FACTORY_H_
#define _SIMPLE_SYMBOL_FACTORY_H_

#include "SymbolFactory.h"
#include <map>

using std::map;

class SimpleSymbolFactory : public SymbolFactory {
public:
	SimpleSymbolFactory()
	: counter(1), intmap(), floatmap(), strmap(), idmap()
	{
		pair<char, long> s1('S', 1);
		idmap[s1] = 0;
	}

	IntegerSymbol* GetIntegerSymbol( long val ) {
		if (intmap.find(val) == intmap.end()) {
			intmap[val] = counter++;
		}
		return NewIntegerSymbol(intmap[val], val);
	}

	FloatSymbol* GetFloatSymbol( double val ) {
		if (floatmap.find(val) == floatmap.end()) {
			floatmap[val] = counter++;
		}
		return NewFloatSymbol(floatmap[val], val);
	}

	StringSymbol* GetStringSymbol( const char* val ) {
		if (strmap.find(val) == strmap.end()) {
			strmap[val] = counter++;
		}
		return NewStringSymbol(strmap[val], val);
	}
	
	IdentifierSymbol* GetIdentifierSymbol( char letter, long number ) {
		pair<char, long> x(letter, number);
		if (idmap.find(x) == idmap.end()) {
			idmap[x] = counter++;
		}
		return NewIdentifierSymbol(idmap[x], letter, number);
	}
	
private:
	long counter;
	map<long, long> intmap;
	map<double, long> floatmap;
	map<string, long> strmap;
	map<pair<char, long>, long> idmap;
};

#endif