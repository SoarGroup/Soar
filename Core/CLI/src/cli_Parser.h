#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include <map>
#include <string>
#include <vector>
#include <set>

#include "cli_Aliases.h"
#include "tokenizer.h"
#include "assert.h"

namespace cli
{
    class ParserCommand
    {
    public:
        virtual ~ParserCommand() {};
        virtual const char* GetString() const = 0;
        virtual const char* GetSyntax() const = 0;
        virtual bool Parse(std::vector<std::string>& argv) = 0;
    };

    class Parser : public soar::tokenizer_callback
    {
    public:
        virtual ~Parser() 
        {
            for (std::map<std::string, ParserCommand*>::iterator iter = cmds.begin(); iter != cmds.end(); ++iter)
                delete iter->second;
            cmds.clear();
        }

        void AddCommand(ParserCommand* p)
        {
            cmds[p->GetString()] = p;
        }

        virtual bool handle_command(std::vector< std::string >& argv)
        {
            error.clear();
            
            if (argv.empty()) 
                return true;

            aliases.Expand(argv);

            ParserCommand* command = PartialMatch(argv);
            if (!command) 
                return false;
            
            return command->Parse(argv);
        }

        ParserCommand* PartialMatch(const std::vector<std::string>& argv)
        {
            // TODO: use the ordered map to find the command

            std::vector< std::pair< std::string, ParserCommand* > > possibilities;

            for(unsigned index = 0; index < argv[0].size(); ++index) 
            {
                if (index == 0) 
                {
                    // Bootstrap the vector of possibilities
                    std::map<std::string, ParserCommand*>::const_iterator citer = cmds.begin();

                    while (citer != cmds.end()) 
                    {
                        if (citer->first[index] == argv[0][index]) 
                            possibilities.push_back(*citer);
                        ++citer;
                    }
                } 
                else 
                {
                    // Update the vector of possiblities
                    std::vector< std::pair< std::string, ParserCommand* > >::iterator liter = possibilities.begin();
                    while (liter != possibilities.end()) 
                    {
                        if (liter->first[index] != argv[0][index]) 
                        {
                            // Remove this possibility from the vector
                            liter = possibilities.erase(liter);
                        } 
                        else 
                        {
                            // check for exact match
                            if (argv[0] == liter->first) 
                                return liter->second;
                            ++liter;
                        }
                    }
                }

                if (!possibilities.size()) 
                {                
                    error = "No such command: " + argv[0];
                    return 0;
                }
            }

            if (possibilities.size() != 1) 
            {
                error = "Ambiguous command, possibilities: ";
                for (std::vector< std::pair< std::string, ParserCommand* > >::const_iterator liter = possibilities.begin(); liter != possibilities.end(); ++liter)
                    error.append("'" + liter->first + "' ");
                return 0;
            }
            return possibilities.front().second;
        }

        const std::string& GetError() const 
        { 
            return error; 
        }

        cli::Aliases& GetAliases()
        {
            return aliases;
        }

    private:
        std::map<std::string, ParserCommand*> cmds;
        cli::Aliases aliases;
        std::string error;
    };
}

#endif // CLI_PARSER_H
