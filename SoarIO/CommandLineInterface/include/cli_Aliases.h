#ifndef CLI_ALIASES_H
#define CLI_ALIASES_H

#include <fstream>
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#include <map>
#pragma warning (default : 4702)
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

		bool Translate(std::vector<std::string>& argv);

		AliasMap::const_iterator GetAliasMapBegin();
		AliasMap::const_iterator GetAliasMapEnd();
		std::string List();

	private:
		
		AliasMap m_AliasMap;
	};

} // namespace cli

#endif // CLI_ALIASES_H
