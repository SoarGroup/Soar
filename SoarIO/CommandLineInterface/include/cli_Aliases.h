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
		
		bool IsAlias(const std::string& command);
		bool NewAlias(const std::vector<std::string>& substitution, const std::string& commandToSubstitute);
		bool RemoveAlias(const std::string& command);
		std::string List();
		bool Translate(std::vector<std::string>& argv);
		
	private:
		
		AliasMap aliasMap;
	};

} // namespace cli

#endif // CLI_ALIASES_H
