#ifndef CONSOLE_HXX
#define CONSOLE_HXX

#include "Console.h"

#include <iostream>
#include <string>
#include <sstream>

int Console::run()
{
    std::cout << "soarplayerclient> ";
    for ( std::string command_line; std::getline( std::cin, command_line ); std::cout << "soarplayerclient> " )
    {
    	
    	std::string command = command_line.substr( 0, command_line.find( " " ) );
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
        else if ( command == "output" )
        {
        	size_t split = command_line.find( " " );
        	if ( split == std::string::npos )
        	{
        		result = "output command required";
        	}
        	else
        	{
	        	result = m_client.command_output( command_line.substr( split + 1 ) );
	        }
        }
        else 
        {
            std::stringstream help;
            help << "Available commands are:\n";
            help << "\tdebug quit reload reset run step stop output";
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

