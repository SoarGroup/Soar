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

void ExplainListChunks(agent* thisAgent)
{
	explain_chunk_str *chunk;

	chunk = thisAgent->explain_chunk_list;

	if (!chunk) {
		print(thisAgent, "No chunks/justifications built yet!\n");
	} else {
		print(thisAgent, "List of all explained chunks/justifications:\n");
		while (chunk != NULL) {
			print(thisAgent, "Have explanation for %s\n", chunk->name);
			chunk = chunk->next_chunk;
		}
	}
}

bool ExplainChunks(agent* thisAgent, const char* pProduction, int mode)
{
	// mode == -1 full
	// mode == 0 name
	// mode > 0 condition

	get_lexeme_from_string(thisAgent, const_cast<char*>(pProduction));

	if (thisAgent->lexeme.type != SYM_CONSTANT_LEXEME) {
		return false; // invalid production
	}

	switch (mode) {
		case -1: // full
			{
				explain_chunk_str* chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list, thisAgent->lexeme.string);
				if (chunk) explain_trace_chunk(thisAgent, chunk);
			}
			break;
		case 0:
			{
				explain_chunk_str* chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list, thisAgent->lexeme.string);
				if (!chunk) return false;

				/* First print out the production in "normal" form */
				print(thisAgent, "(sp %s\n  ", chunk->name);
				print_condition_list(thisAgent, chunk->conds, 2, FALSE);
				print(thisAgent, "\n-->\n   ");
				print_action_list(thisAgent, chunk->actions, 3, FALSE);
				print(thisAgent, ")\n\n");

				/* Then list each condition and the associated "ground" WME */
				int i = 0;
				condition* ground = chunk->all_grounds;

				for (condition* cond = chunk->conds; cond != NIL; cond = cond->next) {
					i++;
					print(thisAgent, " %2d : ", i);
					print_condition(thisAgent, cond);

					while (get_printer_output_column(thisAgent) < COLUMNS_PER_LINE - 40)
						print(thisAgent, " ");

					print(thisAgent, " Ground :");
					print_condition(thisAgent, ground);
					print(thisAgent, "\n");
					ground = ground->next;
				}
			}
			break;
		default:
			{
				explain_chunk_str* chunk = find_chunk(thisAgent, thisAgent->explain_chunk_list, thisAgent->lexeme.string);
				if (!chunk) return false;

				condition* ground = find_ground(thisAgent, chunk, mode);
				if (!ground) return false;

				explain_trace(thisAgent, thisAgent->lexeme.string, chunk->backtrace, ground);
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
		ExplainListChunks(m_pAgentSoar);
		return true;
	}

	ExplainChunks(m_pAgentSoar, pProduction->c_str(), condition);
	return true;
}

