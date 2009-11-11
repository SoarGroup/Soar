#ifndef _SIMPLE_SYMBOL_FACTORY_H_
#define _SIMPLE_SYMBOL_FACTORY_H_

#include "SymbolFactory.h"
#include <map>
#include <utility>

class SimpleSymbolFactory : public EpmemNS::SymbolFactory {
public:
	SimpleSymbolFactory();

	EpmemNS::IntegerSymbol* GetIntegerSymbol( long val );
	EpmemNS::FloatSymbol* GetFloatSymbol( double val );
	EpmemNS::StringSymbol* GetStringSymbol( const char* val );
	EpmemNS::IdentifierSymbol* GetIdentifierSymbol( char letter, long number );
	
	EpmemNS::Symbol* GetSymbolByUID( EpmemNS::SymbolUID uid );
	
private:
	EpmemNS::SymbolUID counter;

	std::map<EpmemNS::SymbolUID, EpmemNS::Symbol*> uidmap;
	std::map<long, EpmemNS::SymbolUID> intmap;
	std::map<double, EpmemNS::SymbolUID> floatmap;
	std::map<std::string, EpmemNS::SymbolUID> strmap;
	std::map<std::pair<char, long>, EpmemNS::SymbolUID> idmap;
};

#endif