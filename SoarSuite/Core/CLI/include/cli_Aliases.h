/////////////////////////////////////////////////////////////////
// Aliases class file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
// This class handles aliases for the command line interface.
//
/////////////////////////////////////////////////////////////////

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
		void NewAlias(const std::vector<std::string>& substitution, const std::string& commandToSubstitute);
		bool RemoveAlias(const std::string& command);

		bool Translate(std::vector<std::string>& argv);

		AliasMap::const_iterator GetAliasMapBegin();
		AliasMap::const_iterator GetAliasMapEnd();
		std::string List(const std::string* pCommand = 0);

	private:
		
		AliasMap m_AliasMap;
	};

} // namespace cli

#endif // CLI_ALIASES_H
