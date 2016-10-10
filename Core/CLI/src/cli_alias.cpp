#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Aliases.h"

#include "sml_AgentSML.h"
#include "sml_Names.h"
#include "sml_Utils.h"

#include "agent.h"
#include "output_manager.h"

using namespace cli;
using namespace sml;


bool CommandLineInterface::DoAlias(std::vector< std::string >* argv, bool doRemove)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (doRemove)
    {
        m_Parser.GetAliases().SetAlias(*argv);
        return true;
    }
    if (!argv || argv->size() == 1)
    {
        std::map< std::string, std::vector< std::string > >::const_iterator iter = m_Parser.GetAliases().Begin();
        if (iter == m_Parser.GetAliases().End())
        {
            m_Result << "No aliases found.";
            return true;
        }
        
        Output_Manager* outputManager = thisAgent->outputManager;

        if (!argv) PrintCLIMessage_Header("Soar Aliases", 43);
        outputManager->reset_column_indents();
        outputManager->set_column_indent(0, 28);
        outputManager->set_column_indent(1, 43);
        std::string lStr;
        while (iter != m_Parser.GetAliases().End())
        {
            if (!argv || argv->front() == iter->first)
            {
                std::string expansion;
                for (std::vector<std::string>::const_iterator j = iter->second.begin(); j != iter->second.end(); ++j)
                {
                    expansion += *j;
                    expansion += ' ';
                }
                expansion = expansion.substr(0, expansion.length() - 1);
                
                if (m_RawOutput)
                {
                    lStr.clear();
                    if (!argv)
                    {
                        outputManager->sprinta_sf(thisAgent, lStr, "%s %-%-= %s\n", iter->first.c_str(), expansion.c_str());
                    m_Result << lStr;
                    } else {
                        m_Result << iter->first << " = \"" << expansion << "\"\n";
                    }
                }
                else
                {
                    AppendArgTagFast(sml_Names::kParamAlias, sml_Names::kTypeString, iter->first);
                    AppendArgTagFast(sml_Names::kParamAliasedCommand, sml_Names::kTypeString, expansion);
                }
                
                if (argv)
                {
                    return true;
                }
            }
            ++iter;
        }
        return true;
    }
    
    m_Parser.GetAliases().SetAlias(*argv);
    return true;
}

