/////////////////////////////////////////////////////////////////
// allocate command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2010
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "agent.h"
#include "cli_CLIError.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAllocate(std::vector<std::string>& argv) 
{
	static const std::string empty;

	if (argv.size() == 1)
		return DoAllocate(empty, 0);

	if (argv.size() > 3) 
		return SetError(CLIError::kTooManyArgs);

	if (argv.size() < 3) 
		return SetError(CLIError::kTooFewArgs);

	int blocks = 0;
	if (!from_string(blocks, argv[2]))
		return SetError(CLIError::kIntegerExpected);

	if (blocks < 1)
		return SetError(CLIError::kIntegerMustBePositive);

	return DoAllocate(argv[1], blocks);
}

bool CommandLineInterface::DoAllocate(const std::string& pool, int blocks) 
{
	if (pool.empty())
	{
		GetMemoryPoolStatistics();
		return true;
	}

	for (memory_pool* p = m_pAgentSoar->memory_pools_in_use; p != NIL; p = p->next) 
	{
		if (strncmp(pool.c_str(), p->name, pool.length()) == 0)
		{
			for (int i = 0; i < blocks; ++i)
				add_block_to_memory_pool(m_pAgentSoar, p);

			m_Result << p->name << " blocks increased by " << blocks;
			return true;
		}
	}
	return false;
}



