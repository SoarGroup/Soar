/////////////////////////////////////////////////////////////////
// explain-backtraces command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "misc.h"
#include "agent.h"
#include "print.h"
#include "explain.h"
#include "utilities.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseExplainBacktraces(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "condition",	OPTARG_REQUIRED},
		{'f', "full",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	int condition = 0;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'f':
				condition = -1;
				break;

			case 'c':
				if ( !from_string( condition, m_OptionArgument ) ) return SetError(CLIError::kIntegerExpected);
				if (condition <= 0) return SetError(CLIError::kIntegerMustBePositive);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// never more than one arg
	if (m_NonOptionArguments > 1) return SetError(CLIError::kTooManyArgs);

	// we need a production if full or condition given
	if (condition) if (m_NonOptionArguments < 1) {
		SetErrorDetail("Production name required for that option.");
		return SetError(CLIError::kTooFewArgs);
	}

	// we have a production
	if (m_NonOptionArguments == 1) return DoExplainBacktraces(&argv[m_Argument - m_NonOptionArguments], condition);
	
	// query
	return DoExplainBacktraces();
}

void ExplainListChunks(AgentSML* pAgent)
{
	agent* pSoarAgent = pAgent->GetSoarAgent();

	explain_chunk_str *chunk;

	chunk = pSoarAgent->explain_chunk_list;

	if (!chunk) {
		print(pSoarAgent, "No chunks/justifications built yet!\n");
	} else {
		print(pSoarAgent, "List of all explained chunks/justifications:\n");
		while (chunk != NULL) {
			print(pSoarAgent, "Have explanation for %s\n", chunk->name);
			chunk = chunk->next_chunk;
		}
	}
}

bool ExplainChunks(AgentSML* pAgent, const char* pProduction, int mode)
{
	agent* pSoarAgent = pAgent->GetSoarAgent();

	// mode == -1 full
	// mode == 0 name
	// mode > 0 condition

	get_lexeme_from_string(pSoarAgent, const_cast<char*>(pProduction));

	if (pSoarAgent->lexeme.type != SYM_CONSTANT_LEXEME) {
		return false; // invalid production
	}

	switch (mode) {
		case -1: // full
			{
				explain_chunk_str* chunk = find_chunk(pSoarAgent, pSoarAgent->explain_chunk_list, pSoarAgent->lexeme.string);
				if (chunk) explain_trace_chunk(pSoarAgent, chunk);
			}
			break;
		case 0:
			{
				explain_chunk_str* chunk = find_chunk(pSoarAgent, pSoarAgent->explain_chunk_list, pSoarAgent->lexeme.string);
				if (!chunk) return false;

				/* First print out the production in "normal" form */
				print(pSoarAgent, "(sp %s\n  ", chunk->name);
				print_condition_list(pSoarAgent, chunk->conds, 2, FALSE);
				print(pSoarAgent, "\n-->\n   ");
				print_action_list(pSoarAgent, chunk->actions, 3, FALSE);
				print(pSoarAgent, ")\n\n");

				/* Then list each condition and the associated "ground" WME */
				int i = 0;
				condition* ground = chunk->all_grounds;

				for (condition* cond = chunk->conds; cond != NIL; cond = cond->next) {
					i++;
					print(pSoarAgent, " %2d : ", i);
					print_condition(pSoarAgent, cond);

					while (get_printer_output_column(pSoarAgent) < COLUMNS_PER_LINE - 40)
						print(pSoarAgent, " ");

					print(pSoarAgent, " Ground :");
					print_condition(pSoarAgent, ground);
					print(pSoarAgent, "\n");
					ground = ground->next;
				}
			}
			break;
		default:
			{
				explain_chunk_str* chunk = find_chunk(pSoarAgent, pSoarAgent->explain_chunk_list, pSoarAgent->lexeme.string);
				if (!chunk) return false;

				condition* ground = find_ground(pSoarAgent, chunk, mode);
				if (!ground) return false;

				explain_trace(pSoarAgent, pSoarAgent->lexeme.string, chunk->backtrace, ground);
			}
			break;
	}
	return true;
}

bool CommandLineInterface::DoExplainBacktraces(const std::string* pProduction, const int condition) {
	// quick sanity check
	if (condition < -1) return SetError(CLIError::kInvalidConditionNumber);

	if (!pProduction) {
		// no production means query, ignore other args
		ExplainListChunks(m_pAgentSML);
		return true;
	}

	ExplainChunks(m_pAgentSML, pProduction->c_str(), condition);
	return true;
}

