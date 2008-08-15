/////////////////////////////////////////////////////////////////
// gp command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, Bob Marinier
// Date  : 2008
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include <iostream>

using namespace cli;

bool CommandLineInterface::ParseGP(std::vector<std::string>& argv) {
	// One argument (in brackets)
	if (argv.size() < 2) {
		return SetError(CLIError::kTooFewArgs);
	}
	if (argv.size() > 2) {
		SetErrorDetail("Expected one argument (the production) enclosed in braces.");
		return SetError(CLIError::kTooManyArgs);
	}

	return DoGP(argv[1]);
}

bool CommandLineInterface::DoGP(const std::string& productionString) {

	// productionString comments are trimmed off at this point.

	// set up collection of collections of strings segments:
	std::list< std::list< std::string > > topLevel;

	bool pipe = false;
	bool inValues = false;
	std::string::size_type pos = 0;			// result of current search
	std::string::size_type searchpos = 0;	// result of previous search
	const char* targets = "\\|[] \n\r\t";
	std::list< std::string > currentValueCollection;
	std::string currentValueToken;

	for ( pos = productionString.find_first_of(targets, searchpos); pos != std::string::npos; pos = productionString.find_first_of(targets, searchpos) ) {
		switch ( productionString[ pos ] ) {
			case '\\': // skip backslashes

				// consume it and the escaped char
				currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 2 );
				searchpos = pos + 2;
				break;

			case '|': // note pipe
				pipe = !pipe;

				// consume it
				currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 1 );
				searchpos = pos + 1;
				break;

			case '[': // start of value list
				if ( !pipe ) 
				{
					// we've started a values list, save the previous segment
					currentValueCollection.push_back( currentValueToken );
					topLevel.push_back( currentValueCollection );

					inValues = true;
					currentValueCollection.clear();
					currentValueToken.clear();
				} 
				else
				{
					// in a pipe, append it
					currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 1 );
				}

				// consume it
				searchpos = pos + 1;
				break;

			case ']': // end of values list
				if ( !pipe )
				{
					// make sure there is not a square bracket immediately following this one
					if ( productionString.size() < pos + 2 )
					{
						// productionString ends on ], bad
						// TODO: error code
						return SetError( CLIError::kNotImplemented );
					}
					if ( productionString[ pos + 1 ] == '[' )
					{
						// space required between value lists
						// TODO: error code
						return SetError( CLIError::kNotImplemented );;
					}

					// end of values list
					topLevel.push_back( currentValueCollection );

					inValues = false;
					currentValueCollection.clear();
					currentValueToken.clear();

				}
				else
				{
					// in a pipe, append it
					currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 1 );
				}

				// consume it
				searchpos = pos + 1;
				break;

			case ' ':	// whitespace
			case '\n':
			case '\r':
			case '\t':
				if ( inValues )
				{
					if ( pipe )
					{
						currentValueToken += productionString[ pos ];
					} 
					else 
					{
						currentValueToken += productionString.substr( searchpos, pos - searchpos );

						if ( currentValueToken.size() )
						{
							// tokenize
							currentValueCollection.push_back( currentValueToken );
						}
					}
				}
				else
				{
					// not in values list, append it
					currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 1 );
				}
				
				// consume it
				searchpos = pos + 1;
				break;
				
			default:
				assert( false );
				break;
		}
	}

	if (pipe || inValues) {
		// TODO: error code
		return SetError( CLIError::kNotImplemented );;
	}

	// final output
	std::cout << "****DoGP collections:" << std::endl;
	for ( std::list< std::list< std::string > >::iterator topIter = topLevel.begin(); topIter != topLevel.end(); ++topIter )
	{
		for ( std::list< std::string >::iterator valueIter = topIter->begin(); valueIter != topIter->end(); ++valueIter )
		{
			std::cout << "%" << *valueIter << "% ";
		}
		std::cout << "~~~" << std::endl;
	}

	return true;
}

