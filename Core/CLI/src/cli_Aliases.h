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
                t.evaluate("aw wm add");
                t.evaluate("chdir cd");
                t.evaluate("ctf output command-to-file");
                t.evaluate("d run -d");
                t.evaluate("dir ls");
                t.evaluate("e run -e");
                t.evaluate("ex production excise");
                t.evaluate("fc production firing-counts");
                t.evaluate("gds_print print --gds");
                t.evaluate("inds decide indifferent-selection");
                t.evaluate("init soar init");
                t.evaluate("interrupt soar stop");
                t.evaluate("is soar init");
                t.evaluate("man help");
                t.evaluate("p print");
                t.evaluate("pc print --chunks");
                t.evaluate("ps print --stack");
                t.evaluate("pw production watch");
                t.evaluate("quit exit");
                t.evaluate("r run");
                t.evaluate("rn load rete-network");
                t.evaluate("rw wm remove");
                t.evaluate("s run 1");
                t.evaluate("set-default-depth output print-depth");
                t.evaluate("ss soar stop");
                t.evaluate("st stats");
                t.evaluate("step run -d");
                t.evaluate("stop soar stop");
                t.evaluate("topd pwd");
                t.evaluate("un alias -r");
                t.evaluate("varprint print -v -d 100");
                t.evaluate("w trace");
                t.evaluate("wmes print -depth 0 -internal");

                // Backward compatibility aliases
                t.evaluate("unalias alias -r");
                t.evaluate("indifferent-selection decide indifferent-selection");
                t.evaluate("numeric-indifferent-mode decide numeric-indifferent-mode");
                t.evaluate("predict decide predict");
                t.evaluate("select decide select");
                t.evaluate("srand decide srand");

                t.evaluate("replay-input load percepts");
                t.evaluate("rete-net load rete-network");
                t.evaluate("load-library load library");
                t.evaluate("source load file");
                t.evaluate("capture-input save percepts");

                t.evaluate("excise production excise");
                t.evaluate("firing-counts production firing-counts");
                t.evaluate("matches production matches");
                t.evaluate("multi-attributes production optimize-attribute");
                t.evaluate("pbreak production break");
                t.evaluate("production-find production find");
                t.evaluate("pwatch production watch");

                t.evaluate("add-wme wm add");
                t.evaluate("wma wm activation");
                t.evaluate("remove-wme wm remove");
                t.evaluate("watch-wmes wm watch");

                t.evaluate("allocate memory allocate");
                t.evaluate("memories memory usage");

                t.evaluate("internal-symbols debug internal-symbols");
                t.evaluate("port debug port");

                t.evaluate("init-soar soar init");
                t.evaluate("stop-soar soar stop");
                t.evaluate("gp-max soar max-gp");
                t.evaluate("max-dc-time soar max-dc-time");
                t.evaluate("max-elaborations soar max-elaborations");
                t.evaluate("max-goal-depth soar max-goal-depth");
                t.evaluate("max-memory-usage soar max-memory-usage");
                t.evaluate("max-nil-output-cycles soar max-nil-output-cycles");
                t.evaluate("set-stop-phase soar stop-phase");
                t.evaluate("soarnews soar");
                t.evaluate("version soar version");
                t.evaluate("waitsnc soar wait-snc");

                t.evaluate("chunk-name-format chunk naming-style");
                t.evaluate("max-chunks chunk max-chunks");

                t.evaluate("clog output output log");
                t.evaluate("command-to-file output command-to-file");
                t.evaluate("default-wme-depth output print-depth");
                t.evaluate("echo-commands output echo-commands");
                t.evaluate("verbose output verbose");
                t.evaluate("warnings output warnings");

                t.evaluate("c explain chunk");
                t.evaluate("i explain instantiation");
                t.evaluate("wt explain wm-trace");
                t.evaluate("et explain explanation-trace");

                t.evaluate("watch trace");

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

