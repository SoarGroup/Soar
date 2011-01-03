#ifndef CLI_OPTIONS_H
#define CLI_OPTIONS_H

namespace cli
{
    enum eOptionArgument 
    {
        OPTARG_NONE,
        OPTARG_REQUIRED,
        OPTARG_OPTIONAL,
    };

    struct OptionsData 
    {
        int shortOpt;
        const char* longOpt;
        eOptionArgument argument;
    };

    class Options
    {
    public:
        Options()
        {
            m_Argument = 0;
            m_NonOptionArguments = 0;    
        }

        bool ProcessOptions(std::vector<std::string>& argv, OptionsData* options)
        {
            // default to indifference
            m_Option = 0;
            m_OptionArgument.clear();
            error.clear();

            // increment current argument and check bounds
            while (static_cast<unsigned>(++m_Argument) < argv.size()) 
            {
                // args less than 2 characters cannot mean anything to us
                if (argv[m_Argument].size() < 2) 
                {
                    ++m_NonOptionArguments;
                    continue;
                }

                if (argv[m_Argument][0] == '-') 
                {
                    if (argv[m_Argument][1] == '-') 
                    {
                        // possible long m_Option
                        if (argv[m_Argument].size() > 2) 
                        {
                            // long m_Option, save it
                            std::string longOption = argv[m_Argument].substr(2);

                            // check for partial match
                            std::vector<OptionsData> possibilities;
                            std::vector<OptionsData>::iterator liter;
                            std::set< std::string > addedLongOptions;

                            for(unsigned index = 0; index < longOption.size(); ++index) 
                            {
                                if (index == 0) 
                                {
                                    // Bootstrap the vector of possibilities
                                    for (int i = 0; options[i].shortOpt != 0; ++i) 
                                    {
                                        if (options[i].longOpt[index] == longOption[index]) 
                                        {
                                            // don't add duplicates (bug 976)
                                            if ( addedLongOptions.insert( options[i].longOpt ).second )
                                                possibilities.push_back(options[i]);
                                        }
                                    }
                                } 
                                else 
                                {
                                    // Update the vector of possiblities

                                    // A more efficient search here would be nice.
                                    liter = possibilities.begin();
                                    while (liter != possibilities.end()) 
                                    {
                                        if ((*liter).longOpt[index] != longOption[index]) 
                                        {
                                            // Remove this possibility from the vector
                                            liter = possibilities.erase(liter);
                                        } 
                                        else 
                                        {
                                            // check for exact match
                                            if (longOption == (*liter).longOpt) 
                                            {
                                                // exact match, we're done
                                                m_Option = liter->shortOpt;
                                                MoveBack(argv, m_Argument, m_NonOptionArguments);
                                                if (!HandleOptionArgument(argv, liter->longOpt, liter->argument)) 
                                                    return false;
                                                return true;
                                            }
                                            ++liter;
                                        }
                                    }
                                }

                                if (!possibilities.size()) 
                                {
                                    error = "No such option: " + longOption;
                                    return false;
                                }
                            }

                            if (possibilities.size() != 1) 
                            {
                                error = "Ambiguous option, possibilities: ";
                                for (liter = possibilities.begin(); liter != possibilities.end(); ++liter)
                                {
                                    error.append("'--");
                                    error.append((*liter).longOpt);
                                    error.append("' ");
                                }
                                return false;
                            }

                            // We have a partial match
                            m_Option = (*(possibilities.begin())).shortOpt;
                            MoveBack(argv, m_Argument, m_NonOptionArguments);
                            if (!HandleOptionArgument(argv, (*(possibilities.begin())).longOpt, (*(possibilities.begin())).argument)) return false;
                            return true;
                        }

                        // end of options special flag '--'
                        // FIXME: remove -- argument?
                        m_Option = -1; // done
                        return true; // no error
                    }

                    // short m_Option(s)
                    for (int i = 0; options[i].shortOpt != 0; ++i) 
                    {
                        if (argv[m_Argument][1] == options[i].shortOpt) 
                        {
                            if (argv[m_Argument].size() > 2) 
                            {
                                std::vector<std::string>::iterator target = argv.begin();
                                target += m_Argument;

                                std::string original = *target;
                                *target = (*target).substr(0,2);
                                ++target;

                                argv.insert(target, "-" + original.substr(2));
                            }
                            m_Option = options[i].shortOpt;
                            MoveBack(argv, m_Argument, m_NonOptionArguments);
                            if (!HandleOptionArgument(argv, options[i].longOpt, options[i].argument)) 
                                return false;
                            return true;
                        }
                    }
                    error = std::string("No such option: ") + argv.at( m_Argument ).at( 1 );
                    return false;
                }
                ++m_NonOptionArguments;
            }

            // out of arguments
            m_Option = -1;    // done
            return true;    // no error
        }

        int GetOption() const 
        { 
            return m_Option; 
        }

        int GetArgument() const 
        { 
            return m_Argument; 
        }

        const std::string& GetOptionArgument() const 
        { 
            return m_OptionArgument;
        }

        int GetNonOptionArguments() const 
        { 
            return m_NonOptionArguments; 
        }

        bool CheckNumNonOptArgs(int min, int max)
        {
            if (m_NonOptionArguments < min)
            {
                error = "Too few arguments.";
                return false;
            }

            if (m_NonOptionArguments > max)
            {
                error = "Too many arguments.";
                return false;
            }
            return true;
        }

        const std::string& GetError() const 
        { 
            return error; 
        }

    private:
        int m_Argument;
        int m_Option;
        std::string m_OptionArgument;
        int m_NonOptionArguments;
        std::string error;

        void MoveBack(std::vector<std::string>& argv, int what, int howFar)
        {
            assert(what >= howFar);
            assert(what > 0);
            assert(howFar >= 0);

            if (howFar == 0)
                return;

            std::vector<std::string>::iterator target = argv.begin();
            target += what;

            std::vector<std::string>::iterator dest = target - howFar;

            argv.insert(dest, *target);

            target = argv.begin();
            target += what + 1;

            argv.erase(target);
        }

        bool HandleOptionArgument(std::vector<std::string>& argv, const char* option, eOptionArgument arg)
        {
            switch (arg) 
            {
                case OPTARG_NONE:
                    break;
                case OPTARG_REQUIRED:
                    // required argument
                    if (static_cast<unsigned>(++m_Argument) >= argv.size()) 
                    {
                        error = "Option '" + std::string(option) + "' requires an argument.";
                        return false;
                    }
                    m_OptionArgument = argv[m_Argument];
                    MoveBack(argv, m_Argument, m_NonOptionArguments);
                    break;

                case OPTARG_OPTIONAL:
                default:
                    // optional argument
                    if (static_cast<unsigned>(++m_Argument) < argv.size()) 
                    {
                        if (argv[m_Argument].size()) 
                        {
                            if (argv[m_Argument][0] != '-') 
                            {
                                m_OptionArgument = argv[m_Argument];
                                MoveBack(argv, m_Argument, m_NonOptionArguments);
                            } 
                        }
                    }
                    if (!m_OptionArgument.size()) 
                        --m_Argument;
                    break;
            }

            return true;
        }
    };
}

#endif // CLI_OPTIONS_H
