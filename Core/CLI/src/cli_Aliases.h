#ifndef CLI_ALIASES_H
#define CLI_ALIASES_H

#include <string>
#include <map>
#include <vector>
#include "tokenizer.h"

namespace cli 
{

    class Aliases : public soar::tokenizer_callback
    {
    public:
        Aliases()
        {
            // defaults
            soar::tokenizer t;
            t.set_handler(this);

            t.evaluate("a alias");
            t.evaluate("aw add-wme");
            t.evaluate("chdir cd");
            t.evaluate("ctf command-to-file");
            t.evaluate("set-default-depth default-wme-depth");
            t.evaluate("ex excise");
            t.evaluate("eb explain-backtraces");
            t.evaluate("fc firing-counts");
            t.evaluate("gds-print gds_print");
            t.evaluate("h help");
            t.evaluate("man help");
            t.evaluate("? help");
            t.evaluate("inds indifferent-selection");
            t.evaluate("init init-soar");
            t.evaluate("is init-soar");
            t.evaluate("l learn");
            t.evaluate("dir ls");
            t.evaluate("pr preferences");
            t.evaluate("p print");
            t.evaluate("pc print --chunks");
            t.evaluate("wmes print -i");
            t.evaluate("varprint print -v -d 100");
            t.evaluate("topd pwd");
            t.evaluate("pw pwatch");
            t.evaluate("step run -d");
            t.evaluate("d run -d");
            t.evaluate("e run -e");
            t.evaluate("rw remove-wme");
            t.evaluate("rn rete-net");
            t.evaluate("sn soarnews");
            t.evaluate("st stats");
            t.evaluate("stop stop-soar");
            t.evaluate("ss stop-soar");
            t.evaluate("interrupt stop-soar");
            t.evaluate("un unalias");
            t.evaluate("w watch");
        }

        virtual ~Aliases() {}

        virtual bool handle_command(std::vector< std::string >& argv)
        {
            SetAlias(argv);
            return true;
        }
        
        void SetAlias(const std::vector< std::string >& argv)
        {
            if (argv.empty())
                return;
            
            std::vector<std::string>::const_iterator i = argv.begin();
            if (argv.size() == 1)
            {
                aliases.erase(*i);
            }
            else
            {
            	std::vector<std::string> &cmd = aliases[*(i++)];
            	cmd.clear();
            	std::copy(i, argv.end(), std::back_inserter(cmd));
            }
        }

        bool Expand(std::vector<std::string>& argv)
        {
            if (argv.empty())
                return false;
            
            std::map< std::string, std::vector< std::string > >::iterator iter = aliases.find(argv.front());
            if (iter == aliases.end())
                return false;

            // overwrite first argument in argv
            std::vector< std::string >::iterator insertion = argv.begin();
            insertion->assign(iter->second[0]);

            // insert any remaining args after that one
            for (unsigned int i = 1; i < iter->second.size(); ++i)
            {
                ++insertion;
                insertion = argv.insert(insertion, iter->second[i]);
            }

            return true;
        }

        std::map< std::string, std::vector< std::string > >::const_iterator Begin()
        {
            return aliases.begin();
        }

        std::map< std::string, std::vector< std::string > >::const_iterator End()
        {
            return aliases.end();
        }

    private:
        std::map< std::string, std::vector< std::string > > aliases;
    };

} // namespace cli

#endif // CLI_ALIASES_H

