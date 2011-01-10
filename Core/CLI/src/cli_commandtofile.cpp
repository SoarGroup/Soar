#include <portability.h>

#include "cli_CommandLineInterface.h"

#include <fstream>

#include <assert.h>

#include "cli_Commands.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoCommandToFile(const eLogMode mode, const std::string& filename, std::vector< std::string >& argv)
{
    std::string oldResult(m_Result.str());
    m_Result.str("");

    // Fire off command
    bool ret = m_Parser.handle_command(argv);

    if (!m_Result.str().empty())
        m_Result << std::endl;

	std::string res = m_Result.str();
    m_Result.str("");
	m_Result << oldResult;

    if (!DoCLog(mode, &filename, 0, true))
        return false;

    if (!DoCLog(LOG_ADD, 0, &res, true))
        return false;

    if (!DoCLog(LOG_CLOSE, 0, 0, true))
        return false;

	return ret;
}
