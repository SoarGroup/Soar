#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"
#include "cli_Aliases.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAlias(std::vector<std::string>& argv) 
{
    if (argv.size() == 1) 
        return DoAlias(); // list all

    argv.erase(argv.begin());

    return DoAlias(&argv);
}

bool CommandLineInterface::DoAlias(std::vector< std::string >* argv)
{
    if (!argv || argv->size() == 1)
    {
        std::map< std::string, std::vector< std::string > >::const_iterator iter = m_Aliases.Begin();
        if (iter == m_Aliases.End())
            return SetError(kAliasNotFound);

        while (iter != m_Aliases.End())
        {
            if (!argv || argv->front() == iter->first)
            {
                std::string expansion;
                for (std::vector<std::string>::const_iterator j = iter->second.begin(); j != iter->second.end(); ++j) {
                    expansion += *j;
                    expansion += ' ';
                }
                expansion = expansion.substr(0, expansion.length() - 1);

                if (m_RawOutput)
                    m_Result << iter->first << "=" << expansion << "\n";
                else
                {
                    AppendArgTagFast(sml_Names::kParamAlias, sml_Names::kTypeString, iter->first);
                    AppendArgTagFast(sml_Names::kParamAliasedCommand, sml_Names::kTypeString, expansion);
                }

                if (argv)
                    return true;
            }
            ++iter;
        }
        return true;
    }

    m_Aliases.SetAlias(*argv);
    return true;
}

