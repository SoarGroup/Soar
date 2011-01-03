/////////////////////////////////////////////////////////////////
// unalias command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_Aliases.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoUnalias(std::vector<std::string>& argv) 
{
    m_Parser.GetAliases().SetAlias(argv);
    return true;
}
