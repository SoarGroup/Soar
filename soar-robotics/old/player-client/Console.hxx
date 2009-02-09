#ifndef CONSOLE_HXX
#define CONSOLE_HXX

#include "Console.h"

#include <iostream>
#include <string>
#include <sstream>

int Console::run()
{
    std::cout << "soarplayerclient> ";
    for ( std::string command; std::getline( std::cin, command ); std::cout << "soarplayerclient> " )
    {
        std::string result;
        if ( command == "run" )
        {
            result = m_client.command_run();
        }
        else if ( command == "stop" )
        {
            result = m_client.command_stop();
        }
        else if ( command == "debug" )
        {
            // spawn debugger
            result = m_client.command_debug();
        }
        else if ( command == "step" )
        {
            // call step
            result = m_client.command_step();
        }
        else if ( command == "reset" )
        {
            // init-soar
            result = m_client.command_reset();
        }
        else if ( command == "reload" )
        {
            // reload productions
            result = m_client.command_reload();
        }
        else if ( command == "quit" )
        {
            break;
        }
        else 
        {
            std::stringstream help;
            help << "Available commands are:\n";
            help << "\tdebug quit reload reset run step stop";
            result = help.str();
        }

        if ( result.length() )
        {
            std::cout << result << std::endl;
        }
    }

    return 0;
}

#endif // CONSOLE_HXX

