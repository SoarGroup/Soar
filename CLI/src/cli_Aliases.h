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

            t.evaluate("alias a");
            t.evaluate("add-wme aw");
            t.evaluate("cd chdir");
            t.evaluate("command-to-file ctf");
            t.evaluate("default-wme-depth set-default-depth");
            t.evaluate("excise ex");
            t.evaluate("explain-backtraces eb");
            t.evaluate("firing-counts fc");
            t.evaluate("gds-print gds_print");
            t.evaluate("help h");
            t.evaluate("help man");
            t.evaluate("help ?");
            t.evaluate("indifferent-selection inds");
            t.evaluate("init-soar init");
            t.evaluate("init-soar is");
            t.evaluate("learn l");
            t.evaluate("ls dir");
            t.evaluate("preferences pr");
            t.evaluate("preferences support");
            t.evaluate("print p");
            t.evaluate("print --chunks pc");
            t.evaluate("print -i wmes");
            t.evaluate("print -v -d 100 varprint");
            t.evaluate("pwd topd");
            t.evaluate("pwatch pw");
            t.evaluate("run -d step");
            t.evaluate("run -d d");
            t.evaluate("run -e e");
            t.evaluate("remove-wme rw");
            t.evaluate("rete-net rn");
            t.evaluate("soarnews sn");
            t.evaluate("stats st");
            t.evaluate("stop-soar stop");
            t.evaluate("stop-soar ss");
            t.evaluate("stop-soar interrupt");
            t.evaluate("unalias un");
            t.evaluate("watch w");
        }

        virtual ~Aliases() {}

        virtual bool handle_command(std::vector< std::string >& argv)
        {
            SetAlias(argv);
            return true;
        }
        
        void SetAlias(std::vector< std::string >& argv)
        {
            if (argv.empty())
                return;

            std::string command( argv.back() );
            argv.pop_back();

            if (argv.empty())
                aliases.erase(command);
            else
                aliases[command] = argv;
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

