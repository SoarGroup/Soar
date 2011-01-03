#include <portability.h>

#include "cli_CommandLineInterface.h"
#include "sml_Utils.h"
#include "cli_Aliases.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoAlias(std::vector< std::string >* argv)
{
    if (!argv || argv->size() == 1)
    {
        std::map< std::string, std::vector< std::string > >::const_iterator iter = m_Parser.GetAliases().Begin();
        if (iter == m_Parser.GetAliases().End())
        {
            m_Result << "No aliases found.";
            return true;
        }

        while (iter != m_Parser.GetAliases().End())
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

    m_Parser.GetAliases().SetAlias(*argv);
    return true;
}

