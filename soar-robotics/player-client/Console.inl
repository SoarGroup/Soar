#ifndef CONSOLE_INL
#define CONSOLE_INL

#include "Console.h"

#include <iostream>
#include <string>

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
        else if ( command == "quit" )
        {
            break;
        }

        if ( result.length() )
        {
            std::cout << result << std::endl;
        }
    }

    return 0;
}


#endif // CONSOLE_INL

