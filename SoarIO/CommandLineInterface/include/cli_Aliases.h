#ifndef CLI_ALIASES_H
#define CLI_ALIASES_H

#include <fstream>
#include <map>
#include <vector>

namespace cli {

	typedef std::map<std::string, std::vector<std::string> > AliasMap;
	typedef std::map<std::string, std::vector<std::string> >::iterator AliasMapIter;
	typedef std::map<std::string, std::vector<std::string> >::const_iterator AliasMapConstIter;

	class Aliases
	{
	public:

		Aliases();
		
		bool Translate(std::vector<std::string>& argv);
		
	private:
		
		void LoadAliases(std::ifstream& aliasesFile);
		AliasMap aliasMap;
	};

} // namespace cli

#endif // CLI_ALIASES_H
