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

#include <iostream>
#include <sstream>

using namespace cli;

typedef std::list< std::string > valueCollection;
struct iterTriple
{
    valueCollection::iterator current;
    valueCollection::iterator begin;
    valueCollection::iterator end;
};

bool CommandLineInterface::DoGP(const std::string& productionString) 
{
    // set up collection of collections of strings segments:
    std::list< valueCollection > topLevel;

    bool pipe = false;
    bool inValues = false;
    std::string::size_type pos = 0;            // result of current search
    std::string::size_type searchpos = 0;    // result of previous search
    const char* targets = "\\|[] \n\r\t#";
    valueCollection currentValueCollection;
    std::string currentValueToken;
    size_t total = 0;

    for ( pos = productionString.find_first_of(targets, searchpos); pos != std::string::npos; pos = productionString.find_first_of(targets, searchpos) ) {
        switch ( productionString[ pos ] ) {
            case '\\': // skip backslashes

                // if the escaped char is a space, consume the space and drop the backslash
                if(productionString[ pos+1 ] == ' ')
                {
                    currentValueToken += productionString.substr( searchpos, (pos - searchpos) );
                    currentValueToken += productionString.substr( pos + 1, 1 ); // we could just append a space here, but this form lets us easily add other characters we might want to append like this
                }
                else  // otherwise consume it and the escaped char
                {
                    currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 2 );
                }
                searchpos = pos + 2;
                break;

            case '|': // note pipe
                pipe = !pipe;

                // consume it
                currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 1 );
                searchpos = pos + 1;

                if(!pipe && inValues)
                {

                    if ( currentValueToken.size() ) // I don't think this is necessary since it should always contain at least the pipes
                    {
                        // tokenize
                        currentValueCollection.push_back( currentValueToken );
                        currentValueToken.clear();
                    }
                }
                break;

            case '[': // start of value list
                if ( !pipe ) 
                {
                    // make sure there is not a square bracket immediately following this one
                    if ( productionString.size() < pos + 2 )
                        return SetError( "gp production ends with [" );
                    if ( productionString[ pos + 1 ] == ']' )
                        return SetError( "gp can't have empty value collections" );

                    // we've started a values list, finish and save the previous segment
                    currentValueToken += productionString.substr( searchpos, pos - searchpos );
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
                        return SetError( "gp production ends with ]" );
                    if ( productionString[ pos + 1 ] == '[' )
                        return SetError( "gp production requires space between value lists" );

                    // end of values list
                    currentValueToken += productionString.substr( searchpos, pos - searchpos );
                    if ( currentValueToken.size() )
                    {
                        // tokenize
                        currentValueCollection.push_back( currentValueToken );
                        currentValueToken.clear();
                    }
                    if ( currentValueCollection.size() )
                    {
                        if (total == 0) {
                            total += currentValueCollection.size();
                        } else {
                            total *= currentValueCollection.size();
                        }
                        //std::cout << "total: " << total << std::endl;
                        topLevel.push_back( currentValueCollection );
                    }

                    inValues = false;
                    currentValueCollection.clear();
                }
                else
                {
                    // in a pipe, append it
                    currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 1 );
                }

                // consume it
                searchpos = pos + 1;
                break;

            case ' ':    // whitespace
            case '\n':
            case '\r':
            case '\t':
                if ( inValues )
                {
                    if ( pipe )
                    {
                        currentValueToken += productionString.substr( searchpos, pos - searchpos + 1 );
                    } 
                    else 
                    {
                        currentValueToken += productionString.substr( searchpos, pos - searchpos );

                        if ( currentValueToken.size() )
                        {
                            // tokenize
                            currentValueCollection.push_back( currentValueToken );
                            currentValueToken.clear();
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

            case '#':
                if (!pipe)
                {
                    // if not in a pipe string, ignore to end of line.
                    searchpos = productionString.find_first_of("\n", pos + 1);
                }
                else
                {
                    // in a pipe, append it
                    currentValueToken += productionString.substr( searchpos, (pos - searchpos) + 1 );
                }
                break;
                
            default:
                assert( false );
                break;
        }
    }

    // grab end of production
    currentValueToken += productionString.substr( searchpos, std::string::npos );
    if ( currentValueToken.size() )
    {
        // tokenize
        currentValueCollection.push_back( currentValueToken );
        currentValueToken.clear();
    }
    topLevel.push_back( currentValueCollection );

    if (pipe || inValues) {
        std::ostringstream message;
        message << "gp syntax error, unmatched ";
        if (pipe) {
            message << "pipe";
        }
        if (pipe && inValues) {
            message << ", ";
        }
        if (inValues) {
            message << "values bracket";
        }
        message << ".";
        return SetError( message.str() );;
    }

    if (m_GPMax != 0) {
        if (total > m_GPMax) {
            std::ostringstream message;
            message << "gp-max exceeded, current production produces " << total << " productions.";
            return SetError( message.str() );
        }
    }

    // final output
    //std::cout << "****DoGP collections:" << std::endl;
    //for ( std::list< valueCollection >::iterator topIter = topLevel.begin(), endIter = topLevel.end(); topIter != endIter; ++topIter )
    //{
    //    std::cout << "~~~" << std::endl;
    //    for ( valueCollection::iterator valueIter = topIter->begin(); valueIter != topIter->end(); ++valueIter )
    //    {
    //        std::cout << "%" << *valueIter << "% ";
    //    }
    //    std::cout << std::endl;
    //}

    // create a collection of iterators
    // two for each value collection in the topLevel collection
    // one that points to the current value, and one that represents the end of the value collection
    std::list< iterTriple > valueIters;
    for ( std::list< valueCollection >::iterator topIter = topLevel.begin(), endIter = topLevel.end(); topIter != endIter; ++topIter )
    {
        iterTriple it;
        it.begin = topIter->begin(); it.current = topIter->begin(); it.end = topIter->end();
        valueIters.push_back( it );
    }

    bool finished = false;
    uint64_t prodnum = 1;
    while(!finished)
    {
        // generate production corresponding to current values
        std::string generatedProduction = "";
        for ( std::list< iterTriple >::iterator valueItersIter = valueIters.begin(), endIter = valueIters.end(); valueItersIter != endIter; ++valueItersIter )
        {
            generatedProduction += *(valueItersIter->current);
            // if this is the first part of the production, 
            // find the end of the production name and add a number to differentiate it
            if(valueItersIter == valueIters.begin())
            {
                pos = generatedProduction.find_first_of("\n \t\r");
                std::stringstream numsstr; numsstr << "*" << prodnum;
                generatedProduction.insert(pos, numsstr.str());
                ++prodnum;
            }
        }

        //std::cout << std::endl << "++++++" << std::endl << generatedProduction <<  std::endl << "++++++" << std::endl;
        if(!DoSP(generatedProduction))
        {
            return false;
        }

        // update value iterators
        finished = true;
        for ( std::list< iterTriple >::iterator valueItersIter = valueIters.begin(), endIter = valueIters.end(); valueItersIter != endIter; ++valueItersIter )
        {
            // increment the value iterator
            ++(valueItersIter->current);
            // If it is at the end, set it back to the beginning.  
            // Otherwise we're at a new value, so break out and generate a new production.
            if(valueItersIter->current == valueItersIter->end)
            {
                valueItersIter->current = valueItersIter->begin;
            }
            else
            {
                finished = false;
                break;
            }
        }
    }
    
    return true;
}

