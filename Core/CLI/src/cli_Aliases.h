#ifndef CLI_ALIASES_H
#define CLI_ALIASES_H

#include <string>
#include <map>
#include <vector>
#include <iterator>
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
                
                t.evaluate("? help");
                t.evaluate("a alias");
                t.evaluate("aw add-wme");
                t.evaluate("chdir cd");
                t.evaluate("ctf command-to-file");
                t.evaluate("d run -d");
                t.evaluate("dir ls");
                t.evaluate("e run -e");
                t.evaluate("ex excise");
                t.evaluate("fc firing-counts");
                t.evaluate("gds_print gds-print");
                t.evaluate("h help");
                t.evaluate("inds indifferent-selection");
                t.evaluate("init soar init");
                t.evaluate("interrupt stop-soar");
                t.evaluate("is soar init");
                t.evaluate("man help");
                t.evaluate("p print");
                t.evaluate("pc print --chunks");
                t.evaluate("pr preferences");
                t.evaluate("ps print --stack");
                t.evaluate("pw pwatch");
                t.evaluate("quit exit");
                t.evaluate("r run");
                t.evaluate("rn rete-net");
                t.evaluate("rw remove-wme");
                t.evaluate("s run 1");
                t.evaluate("set-default-depth soar print-depth");
                t.evaluate("ss stop-soar");
                t.evaluate("st stats");
                t.evaluate("step run -d");
                t.evaluate("stop stop-soar");
                t.evaluate("topd pwd");
                t.evaluate("un unalias");
                t.evaluate("varprint print -v -d 100");
                t.evaluate("w watch");
                t.evaluate("wmes print -depth 0 -internal");
                t.evaluate("wmes print -i");

                // Backward compatibility aliases
                t.evaluate("init-soar soar init");
                t.evaluate("chunk-name-format chunk naming-style");
                t.evaluate("default-wme-depth output print-depth");
                t.evaluate("echo-commands output echo-commands");
                t.evaluate("gp-max soar max-gp");
                t.evaluate("internal-symbols debug internal-symbols");
                t.evaluate("max-chunks chunk max-chunks");
                t.evaluate("max-dc-time soar max-dc-time");
                t.evaluate("max-elaborations soar max-elaborations");
                t.evaluate("max-goal-depth soar max-goal-depth");
                t.evaluate("max-memory-usage soar max-memory-usage");
                t.evaluate("max-nil-output-cycles soar max-nil-output-cycles");
                t.evaluate("port debug port");
                t.evaluate("set-stop-phase soar stop-phase");
                t.evaluate("soarnews soar");
                t.evaluate("verbose output verbose");
                t.evaluate("waitsnc soar wait-snc");
                t.evaluate("warnings output warnings");
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
                {
                    return;
                }
                
                std::vector<std::string>::const_iterator i = argv.begin();
                if (argv.size() == 1)
                {
                    aliases.erase(*i);
                }
                else
                {
                    std::vector<std::string>& cmd = aliases[*(i++)];
                    cmd.clear();
                    std::copy(i, argv.end(), std::back_inserter(cmd));
                }
            }
            
            bool Expand(std::vector<std::string>& argv)
            {
                if (argv.empty())
                {
                    return false;
                }
                
                std::map< std::string, std::vector< std::string > >::iterator iter = aliases.find(argv.front());
                if (iter == aliases.end())
                {
                    return false;
                }
                
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

