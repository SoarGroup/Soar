#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
* PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
* FOR LICENSE AND COPYRIGHT INFORMATION. 
*************************************************************************/

/********************************************************************
* @file gski_donottouch.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_DoNotTouch.h"
#include "gSKI_Agent.h"
#include "gSKI_Kernel.h"

#include "agent.h"
#include "gdatastructs.h"
#include "production.h"
#include "rete.h"
#include "wmem.h"
#include "trace.h"
#include "print.h"
#include "parser.h"
#include "rhsfun.h"
#include "decide.h"
#include "explain.h"
#include "soar_rand.h"
#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes
#include "gski_event_system_functions.h" // support for triggering XML events

//#include "../../SoarIO/ConnectionSML/include/sock_Debug.h"

extern Bool print_sym (agent* thisAgent, void *item, FILE* f);

namespace gSKI 
{
	namespace EvilBackDoor 
	{
		typedef struct wme_filter_struct {
			Symbol *id;
			Symbol *attr;
			Symbol *value;
			bool adds;
			bool removes;
		} wme_filter;

		void TgDWorkArounds::SetSysparam (IAgent* agent, int param_number, long new_value) 
		{
			//agnt->sysparams[param_number] = new_value;
			Agent* internalAgent = (Agent*)(agent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");
			internalAgent->GetSoarAgent()->sysparams[param_number] = new_value;
			internalAgent->GetKernel()->FireSystemPropertyChangedEvent() ;
		}

		long TgDWorkArounds::GetSysparam(IAgent* agent, int param_number)
		{
			Agent* internalAgent = (Agent*)(agent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");
			return internalAgent->GetSoarAgent()->sysparams[param_number];
		}

		const long* TgDWorkArounds::GetSysparams(IAgent* agent)
		{
			Agent* internalAgent = (Agent*)(agent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to GetSysparams.");
			return internalAgent->GetSoarAgent()->sysparams;
		}

		rete_node* TgDWorkArounds::NameToProduction (IAgent* agent, char* string_to_test)
		{
			Agent* internalAgent = (Agent*)(agent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");

			::Symbol* sym;
			sym = find_sym_constant(internalAgent->GetSoarAgent(), string_to_test);

			if (sym && sym->sc.production)
				return sym->sc.production->p_node;
			else
				return 0;
		}


		void TgDWorkArounds::PrintPartialMatchInformation(IAgent* thisAgent, 
		struct rete_node_struct *p_node,
			wme_trace_type wtt)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");

			print_partial_match_information(internalAgent->GetSoarAgent(), p_node, wtt);
		}

		void TgDWorkArounds::PrintMatchSet(IAgent* thisAgent, wme_trace_type wtt, ms_trace_type  mst)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");

			print_match_set(internalAgent->GetSoarAgent(), wtt, mst);
		}

		void TgDWorkArounds::XMLPartialMatchInformation(IAgent* thisAgent, 
		struct rete_node_struct *p_node,
			wme_trace_type wtt)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to xmlPartialMatchInformation.");

			xml_partial_match_information(internalAgent->GetSoarAgent(), p_node, wtt);
		}

		void TgDWorkArounds::XMLMatchSet(IAgent* thisAgent, wme_trace_type wtt, ms_trace_type  mst)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to xmlMatchSet.");

			xml_match_set(internalAgent->GetSoarAgent(), wtt, mst);
		}


		void TgDWorkArounds::PrintStackTrace(IAgent* thisAgent, bool print_states, bool print_operators)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");
			Symbol *g;

			// We don't want to keep printing forever (in case we're in a state no change cascade).
			const int maxStates = 500 ;
			int stateCount = 0 ;

			for (g = internalAgent->GetSoarAgent()->top_goal; g != NIL; g = g->id.lower_goal) 
			{
				stateCount++ ;

				if (stateCount > maxStates)
					continue ;

				if (print_states)
				{
					print_stack_trace (internalAgent->GetSoarAgent(),g, g, FOR_STATES_TF, false);
					print (internalAgent->GetSoarAgent(), "\n");
				}
				if (print_operators && g->id.operator_slot->wmes) 
				{
					print_stack_trace (internalAgent->GetSoarAgent(), g->id.operator_slot->wmes->value,
						g, FOR_OPERATORS_TF, false);
					print (internalAgent->GetSoarAgent(), "\n");
				}
			}

			if (stateCount > maxStates)
			{
				print (internalAgent->GetSoarAgent(), "...Stack goes on for another %d states\n", stateCount - maxStates);
			}
		}

		void get_context_var_info ( agent* agnt, Symbol **dest_goal,
			Symbol **dest_attr_of_slot,
			Symbol **dest_current_value);

		Symbol *read_identifier_or_context_variable (agent* agnt) 
		{
			Symbol *id;
			Symbol *g, *attr, *value;

			if (agnt->lexeme.type==IDENTIFIER_LEXEME) {
				id = find_identifier (agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number);
				if (!id) {
					print (agnt, "There is no identifier %c%lu.\n", agnt->lexeme.id_letter,
						agnt->lexeme.id_number);
					print_location_of_most_recent_lexeme(agnt);
					return NIL;
				}
				return id;
			}
			if (agnt->lexeme.type==VARIABLE_LEXEME) 
			{
				get_context_var_info (agnt, &g, &attr, &value);
				if (!attr) {
					print (agnt, "Expected identifier (or context variable)\n");
					print_location_of_most_recent_lexeme(agnt);
					return NIL;
				}
				if (!value) {
					print (agnt, "There is no current %s.\n", agnt->lexeme.string);
					print_location_of_most_recent_lexeme(agnt);
					return NIL;
				}
				if (value->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) {
					print (agnt, "The current %s ", agnt->lexeme.string);
					print_with_symbols (agnt, "(%y) is not an identifier.\n", value);
					print_location_of_most_recent_lexeme(agnt);
					return NIL;
				}
				return value;
			}
			print (agnt, "Expected identifier (or context variable)\n");
			print_location_of_most_recent_lexeme(agnt);
			return NIL;
		}		

		void do_print_for_production (agent* agnt, 
			production *prod, 
			bool internal, 
			bool print_filename, 
			bool full_prod) 
		{
			if (!full_prod) 
			{
				print_with_symbols(agnt, "%y  ",prod->name);
			}
			if (print_filename) 
			{
				print_string(agnt, "# sourcefile : ");
				if (prod->filename) 
				{
					print_string(agnt, prod->filename);
				} 
				else 
				{
					print_string(agnt, " _unknown_ ");
				}
			}
			print(agnt, "\n");
			if (full_prod) 
			{
				print_production (agnt, prod, internal);
				print(agnt, "\n");
			}
		}

		char *stringToEscapedString (char *s, char first_and_last_char, char *dest) 
		{
			char *ch;

			ch = dest;
			*ch++ = first_and_last_char;
			while (*s) {
				if ((*s==first_and_last_char)||(*s=='\\')) *ch++ = '\\';
				*ch++ = *s++;
			}
			*ch++ = first_and_last_char;
			*ch = 0;
			return dest;
		}


		char *symbolToString (Symbol *sym, 
			Bool rereadable, 
			char *dest) 
		{
			Bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
			Bool is_rereadable;
			Bool has_angle_bracket;

			switch(sym->common.symbol_type) 
			{
			case VARIABLE_SYMBOL_TYPE:
				strcpy (dest, sym->var.name);
				return dest;

			case IDENTIFIER_SYMBOL_TYPE:
				sprintf (dest, "%c%lu", sym->id.name_letter, sym->id.name_number);
				return dest;

			case INT_CONSTANT_SYMBOL_TYPE:
				sprintf (dest, "%ld", sym->ic.value);
				return dest;

			case FLOAT_CONSTANT_SYMBOL_TYPE:
				sprintf (dest, "%#g", sym->fc.value);
				{ /* --- strip off trailing zeros --- */
					char *start_of_exponent;
					char *end_of_mantissa;
					start_of_exponent = dest;
					while ((*start_of_exponent != 0) && (*start_of_exponent != 'e'))
						start_of_exponent++;
					end_of_mantissa = start_of_exponent - 1;
					while (*end_of_mantissa == '0') end_of_mantissa--;
					end_of_mantissa++;
					while (*start_of_exponent) *end_of_mantissa++ = *start_of_exponent++;
					*end_of_mantissa = 0;
				}
				return dest;

			case SYM_CONSTANT_SYMBOL_TYPE:
				if (!rereadable) {
					strcpy (dest, sym->sc.name);
					return dest;
				}
				determine_possible_symbol_types_for_string (sym->sc.name,
					(int)strlen (sym->sc.name),
					&possible_id,
					&possible_var,
					&possible_sc,
					&possible_ic,
					&possible_fc,
					&is_rereadable);

				has_angle_bracket = sym->sc.name[0] == '<' ||
					sym->sc.name[strlen(sym->sc.name)-1] == '>';

				if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
					(!is_rereadable) ||
					has_angle_bracket) {
						/* BUGBUG if in context where id's could occur, should check
						possible_id flag here also */
						return stringToEscapedString (sym->sc.name, '|', dest);
					}
					strcpy (dest, sym->sc.name);
					return dest;

			default:
				{ 
					//                  char msg[128];
					//                  strcpy(msg,
					//                     "Internal Soar Error:  symbol_to_string called on bad symbol\n");
					//                  abort_with_fatal_error(thisAgent, msg);
				}
			}
			return NIL; /* unreachable, but without it, gcc -Wall warns here */
		}


		int compare_attr (const void * e1, const void * e2)
		{
			wme **p1, **p2;

			char s1[MAX_LEXEME_LENGTH*2+20], s2[MAX_LEXEME_LENGTH*2+20];

			p1 = (wme **) e1;
			p2 = (wme **) e2;

			symbolToString ((*p1)->attr, TRUE, s1);
			symbolToString ((*p2)->attr, TRUE, s2);

			return (strcmp (s1, s2));
		}


		/* This should probably be in the Soar kernel interface. */
#define NEATLY_PRINT_BUF_SIZE 10000
		void neatly_print_wme_augmentation_of_id (agent* thisAgent, wme *w, int indentation) {
			char buf[NEATLY_PRINT_BUF_SIZE], *ch;

			gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagWME);
			gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kWME_TimeTag, w->timetag);
			gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kWME_Attribute, symbol_to_string (thisAgent, w->attr, true, 0, 0));
			gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kWME_Value, symbol_to_string (thisAgent, w->value, true, 0, 0));
			gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kWME_ValueType, symbol_to_typeString(thisAgent, w->value));
			if (w->acceptable) gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kWMEPreference, "+");
			gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagWME);

			strcpy (buf, " ^");
			ch = buf;
			while (*ch) ch++;
			symbol_to_string (thisAgent, w->attr, TRUE, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf)); while (*ch) ch++;
			*(ch++) = ' ';
			symbol_to_string (thisAgent, w->value, TRUE, ch, NEATLY_PRINT_BUF_SIZE - (ch - buf)); while (*ch) ch++;
			if (w->acceptable) { strcpy (ch, " +"); while (*ch) ch++; }

			if (get_printer_output_column(thisAgent) + (ch - buf) >= 80) {
				print (thisAgent, "\n");
				print_spaces (thisAgent, indentation+6);
			}
			print_string (thisAgent, buf);
		}

		void print_augs_of_id (agent* agnt,
			Symbol *id, 
			int depth, 
			bool internal,
			int indent, 
			tc_number tc) 
		{
			slot *s;
			wme *w;

			wme **list;    /* array of WME pointers, AGR 652 */
			int num_attr;  /* number of attributes, AGR 652 */
			int attr;      /* attribute index, AGR 652 */

			/* AGR 652  The plan is to go through the list of WMEs and find out how
			many there are.  Then we malloc an array of that many pointers.
			Then we go through the list again and copy all the pointers to that array.
			Then we qsort the array and print it out.  94.12.13 */

			if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) return;
			if (id->id.tc_num==tc) return;
			id->id.tc_num = tc;

			/* --- first, count all direct augmentations of this id --- */
			num_attr = 0;
			for (w=id->id.impasse_wmes; w!=NIL; w=w->next) num_attr++;
			for (w=id->id.input_wmes; w!=NIL; w=w->next) num_attr++;
			for (s=id->id.slots; s!=NIL; s=s->next) {
				for (w=s->wmes; w!=NIL; w=w->next) num_attr++;
				for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) num_attr++;
			}

			/* --- next, construct the array of wme pointers and sort them --- */
			list = (wme**)allocate_memory(agnt, num_attr*sizeof(wme *), MISCELLANEOUS_MEM_USAGE);
			attr = 0;
			for (w=id->id.impasse_wmes; w!=NIL; w=w->next)
				list[attr++] = w;
			for (w=id->id.input_wmes; w!=NIL; w=w->next)
				list[attr++] = w;
			for (s=id->id.slots; s!=NIL; s=s->next) {
				for (w=s->wmes; w!=NIL; w=w->next)
					list[attr++] = w;
				for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next)
					list[attr++] = w;
			}
			qsort (list, num_attr, sizeof (wme *), compare_attr); 

			/* --- finally, print the sorted wmes and deallocate the array --- */
			if (internal) {
				for (attr=0; attr < num_attr; attr++) {
					w = list[attr];
					print_spaces (agnt, indent);
					print_wme (agnt, w);
				}
			} else {
				print_spaces (agnt, indent);
				print_with_symbols (agnt, "(%y", id);

				// XML format of an <id> followed by a series of <wmes> each of which shares the original ID.
				// <id id="s1"><wme tag="123" attr="foo" attrtype="string" val="123" valtype="string"></wme><wme attr="bar" ...></wme></id>
				gSKI_MakeAgentCallbackXML(agnt, kFunctionBeginTag, kWME_Id);
				gSKI_MakeAgentCallbackXML(agnt, kFunctionAddAttribute, kWME_Id, symbol_to_string (agnt, id, true, 0, 0));

				for (attr=0; attr < num_attr; attr++) {
					w = list[attr];
					neatly_print_wme_augmentation_of_id (agnt, w, indent);
				}
				
				gSKI_MakeAgentCallbackXML(agnt, kFunctionEndTag, kWME_Id);

				print (agnt, ")\n");
			}
			free_memory(agnt, list, MISCELLANEOUS_MEM_USAGE);
			/* AGR 652 end */

			/* --- if depth<=1, we're done --- */
			if (depth<=1) return;

			/* --- call this routine recursively --- */
			for (w=id->id.input_wmes; w!=NIL; w=w->next) {
				print_augs_of_id (agnt, w->attr, depth-1, internal, indent+2, tc);
				print_augs_of_id (agnt, w->value, depth-1, internal, indent+2, tc);
			}
			for (w=id->id.impasse_wmes; w!=NIL; w=w->next) {
				print_augs_of_id (agnt, w->attr, depth-1, internal, indent+2, tc);
				print_augs_of_id (agnt, w->value, depth-1, internal, indent+2, tc);
			}
			for (s=id->id.slots; s!=NIL; s=s->next) {
				for (w=s->wmes; w!=NIL; w=w->next) {
					print_augs_of_id (agnt, w->attr, depth-1, internal, indent+2, tc);
					print_augs_of_id (agnt, w->value, depth-1, internal, indent+2, tc);
				}
				for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next) {
					print_augs_of_id (agnt, w->attr, depth-1, internal, indent+2, tc);
					print_augs_of_id (agnt, w->value, depth-1, internal, indent+2, tc);
				}
			}
		}


		void do_print_for_identifier (agent* agnt, Symbol *id, int depth, bool internal) {
			tc_number tc;

			tc = get_new_tc_number(agnt);
			print_augs_of_id (agnt, id, depth, internal, 0, tc);
		}

		void do_print_for_production_name (agent* agnt,
			char *prod_name, 
			bool internal,
			bool print_filename, 
			bool full_prod) 
		{
			Symbol *sym;

			sym = find_sym_constant (agnt, agnt->lexeme.string);
			if (sym && sym->sc.production) {
				/* kjh CUSP(B11) begin */  /* also Soar-Bugs #161 */
				if (print_filename) {
					if (full_prod) 
						print_string(agnt, "# sourcefile : ");
					print (agnt, "%s\n", sym->sc.production->filename);
				}
				/* KJC added so get at least some output for any named productions */
				if ((full_prod) || (!print_filename)) {
					print_production (agnt, sym->sc.production, internal);
					print (agnt, "\n");
				} /* end CUSP B11 kjh */
			} else {
				print (agnt, "No production named %s\n", prod_name);
				print_location_of_most_recent_lexeme(agnt);
			}
		}

		void do_print_for_wme (agent* agnt, wme *w, int depth, bool internal) {
			tc_number tc;

			if (internal && (depth==0)) {
				print_wme (agnt, w);
				print (agnt, "\n");
			} else {
				tc = get_new_tc_number(agnt);
				print_augs_of_id (agnt, w->id, depth, internal, 0, tc);
			}
		}

		void get_lexeme_from_string (agent* agnt, char * the_lexeme)
		{
			int i;
			char * c;
			bool sym_constant_start_found = FALSE;
			bool sym_constant_end_found = FALSE;

			for (c = the_lexeme, i = 0; *c; c++, i++)
			{
				if (*c == '|')
				{
					if (!sym_constant_start_found)
					{
						i--;
						sym_constant_start_found = TRUE;
					}
					else
					{
						i--;
						sym_constant_end_found = TRUE;
					}
				}
				else
				{
					agnt->lexeme.string[i] = *c;
				} 
			}

			agnt->lexeme.string[i] = '\0'; /* Null terminate lexeme string */

			agnt->lexeme.length = i;

			if (sym_constant_end_found)
			{
				agnt->lexeme.type = SYM_CONSTANT_LEXEME;
			}
			else 
			{
				determine_type_of_constituent_string(agnt);
			}
		}

		/* --- Read and consume one pattern element.  Return 0 if error, 1 if "*",
		otherwise return 2 and set dest_sym to find_symbol() result. --- */
		int read_pattern_component (agent* agnt, Symbol **dest_sym) 
		{
			if (strcmp(agnt->lexeme.string,"*") == 0) return 1;
			switch (agnt->lexeme.type) 
			{
			case SYM_CONSTANT_LEXEME:
				*dest_sym = find_sym_constant (agnt, agnt->lexeme.string); return 2;
			case INT_CONSTANT_LEXEME:
				*dest_sym = find_int_constant (agnt, agnt->lexeme.int_val); return 2;
			case FLOAT_CONSTANT_LEXEME:
				*dest_sym = find_float_constant (agnt, agnt->lexeme.float_val); return 2;
			case IDENTIFIER_LEXEME:
				*dest_sym = find_identifier (agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number); return 2;
			case VARIABLE_LEXEME:
				*dest_sym = read_identifier_or_context_variable(agnt);
				if (*dest_sym) return 2;
				return 0;
			default:
				print (agnt, "Expected identifier or constant in wme pattern\n");
				print_location_of_most_recent_lexeme(agnt);
				return 0;
			}
		}

		void get_context_var_info ( agent* agnt, Symbol **dest_goal,
			Symbol **dest_attr_of_slot,
			Symbol **dest_current_value) 
		{
			Symbol *v, *g;
			int levels_up;
			wme *w;

			v = find_variable (agnt, agnt->lexeme.string);
			if (v==agnt->s_context_variable) {
				levels_up = 0;
				*dest_attr_of_slot = agnt->state_symbol;
			} else if (v==agnt->o_context_variable) {
				levels_up = 0;
				*dest_attr_of_slot = agnt->operator_symbol;
			} else if (v==agnt->ss_context_variable) {
				levels_up = 1;
				*dest_attr_of_slot = agnt->state_symbol;
			} else if (v==agnt->so_context_variable) {
				levels_up = 1;
				*dest_attr_of_slot = agnt->operator_symbol;
			} else if (v==agnt->sss_context_variable) {
				levels_up = 2;
				*dest_attr_of_slot = agnt->state_symbol;
			} else if (v==agnt->sso_context_variable) {
				levels_up = 2;
				*dest_attr_of_slot = agnt->operator_symbol;
			} else if (v==agnt->ts_context_variable) {
				levels_up = agnt->top_goal ? agnt->bottom_goal->id.level-agnt->top_goal->id.level : 0;
				*dest_attr_of_slot = agnt->state_symbol;
			} else if (v==agnt->to_context_variable) {
				levels_up = agnt->top_goal ? agnt->bottom_goal->id.level-agnt->top_goal->id.level : 0;
				*dest_attr_of_slot = agnt->operator_symbol;
			} else {
				*dest_goal = NIL;
				*dest_attr_of_slot = NIL;
				*dest_current_value = NIL;
				return;
			}

			g = agnt->bottom_goal;
			while (g && levels_up) {
				g = g->id.higher_goal;
				levels_up--;
			}
			*dest_goal = g;

			if (!g) {
				*dest_current_value = NIL;
				return;
			}

			if (*dest_attr_of_slot==agnt->state_symbol) {
				*dest_current_value = g;
			} else {
				w = g->id.operator_slot->wmes;
				*dest_current_value = w ? w->value : NIL;
			}
		}



		list *read_pattern_and_get_matching_wmes (agent* agnt) 
		{
			int parentheses_level;
			list *wmes;
			wme *w;
			Symbol *id, *attr, *value;
			int id_result, attr_result, value_result;
			bool acceptable;

			if (agnt->lexeme.type!=L_PAREN_LEXEME) {
				print (agnt, "Expected '(' to begin wme pattern not string '%s' or char '%c'\n", 
					agnt->lexeme.string, agnt->current_char);
				print_location_of_most_recent_lexeme(agnt);
				return NIL;
			}
			parentheses_level = current_lexer_parentheses_level(agnt);

			get_lexeme(agnt);
			id_result = read_pattern_component (agnt, &id);
			if (! id_result) {
				skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
				return NIL;
			}
			get_lexeme(agnt);
			if (agnt->lexeme.type!=UP_ARROW_LEXEME) {
				print (agnt, "Expected ^ in wme pattern\n");
				print_location_of_most_recent_lexeme(agnt);
				skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
				return NIL;
			}
			get_lexeme(agnt);
			attr_result = read_pattern_component (agnt, &attr);
			if (! attr_result) {
				skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
				return NIL;
			}
			get_lexeme(agnt);
			value_result = read_pattern_component (agnt, &value);
			if (! value_result) {
				skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
				return NIL;
			}
			get_lexeme(agnt);
			if (agnt->lexeme.type==PLUS_LEXEME) {
				acceptable = TRUE;
				get_lexeme(agnt);
			} else {
				acceptable = FALSE;
			}
			if (agnt->lexeme.type!=R_PAREN_LEXEME) {
				print (agnt, "Expected ')' to end wme pattern\n");
				print_location_of_most_recent_lexeme(agnt);
				skip_ahead_to_balanced_parentheses (agnt, parentheses_level-1);
				return NIL;
			}

			wmes = NIL;
			for (w=agnt->all_wmes_in_rete; w!=NIL; w=w->rete_next) {
				if ((id_result==1) || (id==w->id))
					if ((attr_result==1) || (attr==w->attr))
						if ((value_result==1) || (value==w->value))
							if (acceptable == (w->acceptable == TRUE))
								push (agnt, w, wmes);
			}
			return wmes;  
		}


		void  soar_alternate_input(agent *ai_agent,
			char  *ai_string, 
			char  *ai_suffix, 
			bool   ai_exit   )
		{
			ai_agent->alternate_input_string = ai_string;
			ai_agent->alternate_input_suffix = ai_suffix;
			ai_agent->current_char = ' ';
			ai_agent->alternate_input_exit = ai_exit;
			return;
		}


		void TgDWorkArounds::PrintSymbol(IAgent*     thisAgent,
			char*       arg, 
			bool        name_only, 
			bool        print_filename, 
			bool        internal,
			bool        full_prod,
			int         depth)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");

			cons *c;
			Symbol *id;
			bool output_arg;
			wme* w;
			list* wmes;

			agent* agnt = internalAgent->GetSoarAgent();

			get_lexeme_from_string(agnt, arg);

			switch (agnt->lexeme.type) 
			{
			case SYM_CONSTANT_LEXEME:
				output_arg = true; /* Soar-Bugs #161 */
				if (!name_only || print_filename) 
				{
					/* kjh CUSP(B11) begin */
					do_print_for_production_name (agnt, arg, internal, 
						print_filename, full_prod);
				} else 
				{
					print(agnt, "%s\n",arg);
				}
				break;

			case INT_CONSTANT_LEXEME:
				output_arg = true; /* Soar-Bugs #161 */
				for (w=internalAgent->GetSoarAgent()->all_wmes_in_rete; w!=NIL; w=w->rete_next)
					// RDF (08282002) Added the following cast to get rid of warning
					// message
					if (w->timetag == (unsigned long) internalAgent->GetSoarAgent()->lexeme.int_val) 
						break;
				if (w) 
				{
					do_print_for_wme (agnt, w, depth, internal);
				} else 
				{
					print(agnt, "No wme %ld in working memory.", agnt->lexeme.int_val);
					return;
				}
				break;

			case IDENTIFIER_LEXEME:
			case VARIABLE_LEXEME:
				output_arg = true; /* Soar-Bugs #161 */
				id = read_identifier_or_context_variable(agnt);
				if (id) 
					do_print_for_identifier (agnt, id, depth, internal);
				break;

			case QUOTED_STRING_LEXEME:
				output_arg = true; /* Soar-Bugs #161 */
				/* Soar-Bugs #54 TMH */
				soar_alternate_input(agnt, arg, ") ",true);
				/* ((agent *)clientData)->alternate_input_string = argv[next_arg];
				* ((agent *)clientData)->alternate_input_suffix = ") ";
				*/
				get_lexeme(agnt);
				wmes = read_pattern_and_get_matching_wmes (agnt);
				soar_alternate_input(agnt, NIL, NIL, FALSE); 
				agnt->current_char = ' ';
				for (c = wmes; c != NIL; c = c->rest)
					do_print_for_wme (agnt, (wme *)c->first, depth, internal);
				free_list (agnt, wmes);
				break;

			default:
				//            sprintf(interp->result,
				//               "Unrecognized argument to %s command: %s",
				//               print, arg);
				//            return TCL_ERROR;
				return;
			} /* end of switch statement */
			output_arg = true;  /* Soar-bugs #161 */
		}

		void TgDWorkArounds::PrintUser(IAgent*       thisAgent,
			char*         arg,
			bool          internal,
			bool          print_filename,
			bool          full_prod,
			unsigned int  productionType)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");

			//bool output_arg = true; /* TEST for Soar-Bugs #161 */
			for (production* prod=internalAgent->GetSoarAgent()->all_productions_of_type[productionType];  
				prod != NIL; prod = prod->next)
			{
				/* CUSP B11 kjh */
				do_print_for_production(internalAgent->GetSoarAgent(), prod,internal,
					print_filename,full_prod);
			}
		}


		slot *find_slot (Symbol *id, Symbol *attr) 
		{
			slot *s;

			if (!id) return NIL;  /* fixes bug #135 kjh */
			for (s=id->id.slots; s!=NIL; s=s->next)
				if (s->attr==attr) return s;
			return NIL;
		}

		/*
		*	This procedure parses a string to determine if it is a
		*      lexeme for an existing attribute.
		*
		* Side effects:
		*	None.
		*
		===============================
		*/

		bool read_attribute_from_string (agent* agnt, Symbol *id, char * the_lexeme, Symbol * * attr)
		{
			Symbol *attr_tmp;
			slot *s;

			/* skip optional '^' if present.  KJC added to Ken's code */
			if (*the_lexeme == '^')
			{
				the_lexeme++;
			}

			get_lexeme_from_string(agnt, the_lexeme);

			switch (agnt->lexeme.type) 
			{
			case SYM_CONSTANT_LEXEME:
				attr_tmp = find_sym_constant (agnt, agnt->lexeme.string);
				break;
			case INT_CONSTANT_LEXEME:
				attr_tmp = find_int_constant (agnt, agnt->lexeme.int_val);
				break;
			case FLOAT_CONSTANT_LEXEME:
				attr_tmp = find_float_constant (agnt, agnt->lexeme.float_val);
				break;
			case IDENTIFIER_LEXEME:
				attr_tmp = find_identifier (agnt, agnt->lexeme.id_letter,
					agnt->lexeme.id_number);
				break;
			case VARIABLE_LEXEME:
				attr_tmp = read_identifier_or_context_variable(agnt);
				if (!attr_tmp)
					return false;
				break;
			default:
				return false;
			}
			s = find_slot (id, attr_tmp);
			if (s) {
				*attr = attr_tmp;
				return true;
			} else
				return false;
		}


		/*
		*
		*	This procedure prints a preference and the production
		*      which is the source of the preference.
		*
		* Results:
		*	Tcl status code.
		*
		* Side effects:
		*	Prints the preference and its source production.
		*
		===============================
		*/
		void print_preference_and_source (agent* agnt, preference *pref,
			bool print_source,
			wme_trace_type wtt) 
		{
			print_string (agnt, "  ");
			print_object_trace (agnt, pref->value);
			print (agnt, " %c", preference_type_indicator (agnt, pref->type));
			if (preference_is_binary(pref->type)) print_object_trace (agnt, pref->referent);
			if (pref->o_supported) print (agnt, " :O ");
			print (agnt, "\n");
			if (print_source) {
				print (agnt, "    From ");
				print_instantiation_with_wmes (agnt, pref->inst, wtt, -1);
				print (agnt, "\n");
			}
		}


		/*
		*	This procedure parses a string to determine if it is a
		*      lexeme for an identifier or context variable.
		* 
		*      Many interface routines take identifiers as arguments.  
		*      These ids can be given as normal ids, or as special variables 
		*      such as <s> for the current state, etc.  This routine reads 
		*      (without consuming it) an identifier or context variable, 
		*      and returns a pointer (Symbol *) to the id.  (In the case of 
		*      context variables, the instantiated variable is returned.  If 
		*      any error occurs (e.g., no such id, no instantiation of the 
		*      variable), an error message is printed and NIL is returned.
		*
		* Results:
		*	Pointer to a symbol for the variable or NIL.
		*
		* Side effects:
		*	None.
		*
		===============================
		*/
		bool read_id_or_context_var_from_string (agent* agnt, char * the_lexeme,
			Symbol * * result_id) 
		{
			Symbol *id;
			Symbol *g, *attr, *value;

			get_lexeme_from_string(agnt, the_lexeme);

			if (agnt->lexeme.type == IDENTIFIER_LEXEME) 
			{
				id = find_identifier(agnt, agnt->lexeme.id_letter, agnt->lexeme.id_number);
				if (!id) 
				{
					return false;
				}
				else
				{
					*result_id = id;
					return true;
				}
			}

			if (agnt->lexeme.type==VARIABLE_LEXEME) 
			{
				get_context_var_info (agnt, &g, &attr, &value);

				if ((!attr) || (!value))
				{
					return false;
				}

				if (value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) 
				{
					return false;
				}

				*result_id = value;
				return true;
			}

			return false;
		}

		bool  string_match (char * string1, char * string2)
		{
			if ((string1 == NULL) && (string2 == NULL))
				return true;

			if (   (string1 != NULL) 
				&& (string2 != NULL) 
				&& !(strcmp(string1, string2)))
				return true;
			else
				return false;
		}

		bool string_match_up_to (char * string1, 
			char * string2, 
			unsigned int positions)
		{
			unsigned int i,num;

			/*  what we really want is to require a match over the length of
			the shorter of the two strings, with positions being a minimum */

			num = (unsigned int)strlen(string1);
			if (num > (unsigned int)strlen(string2)) num = (unsigned int)strlen(string2);
			if (positions < num)  positions = num;

			for (i = 0; i < positions; i++)
			{
				if (string1[i] != string2[i])
					return false;
			}

			return true;  
		}


		/*
		*
		*	This procedure parses a string to determine if it is a
		*      lexeme for the detail level indicator for the 'preferences'
		*      command.  If so, it sets the_lexeme and wme_trace_type accordingly
		*      and returns TCL_OK; otherwise, it leaves those parameters untouched
		*      and returns TCL_ERROR.
		*
		* Side effects:
		*	None.
		*
		===============================
		*/
		int read_pref_detail_from_string (char *the_lexeme,
			bool *print_productions,
			wme_trace_type *wtt)
		{
			if (string_match_up_to(the_lexeme, "-none", 3) || string_match(the_lexeme, "0")) 
			{
				*print_productions = FALSE;
				*wtt               = NONE_WME_TRACE;
			} 
			else if (string_match_up_to(the_lexeme, "-names", 3) || string_match(the_lexeme, "1")) 
			{
				*print_productions = TRUE;
				*wtt               = NONE_WME_TRACE;
			} 
			else if (string_match_up_to(the_lexeme, "-timetags", 2) || string_match(the_lexeme, "2")) 
			{
				*print_productions = TRUE;
				*wtt               = TIMETAG_WME_TRACE;
			} 
			else if (string_match_up_to(the_lexeme, "-wmes", 2) || string_match(the_lexeme, "3")) 
			{
				*print_productions = TRUE;
				*wtt               = FULL_WME_TRACE;
			} 
			else 
			{
				return false;
			}
			return true;
		}



		char * pref_names[] =
		{  
			"acceptable",
				"require",
				"reject",
				"prohibit",
				"reconsider",
				"unary indifferent",
				"unary parallel",
				"best",
				"worst",
				"binary indifferent",
				"binary parallel",
				"better",
				"worse" 
		};

		int soar_ecPrintPreferences(agent* soarAgent, char *szId, char *szAttr, bool print_prod, wme_trace_type wtt)
		{

			Symbol *id, *attr;
			slot *s;
			preference *p;
			int i;

			if (!read_id_or_context_var_from_string(soarAgent, szId, &id)) {
				print(soarAgent, "Could not find the id '%s'\n", szId);
				return -1;
			}
			if (!read_attribute_from_string(soarAgent, id, szAttr, &attr)) {
				print(soarAgent, "Could not find the id,attribute pair: %s ^%s\n", szId, szAttr);
				return -2;
			}

			s = find_slot(id, attr);
			if (!s) {
				print(soarAgent, "There are no preferences for %s ^%s.", szId, szAttr);
				return -3;
			}

			print_with_symbols(soarAgent, "Preferences for %y ^%y:\n", id, attr);

			for (i = 0; i < NUM_PREFERENCE_TYPES; i++) {
				if (s->preferences[i]) {
					print(soarAgent, "\n%ss:\n", preference_name[i]);
					for (p = s->preferences[i]; p; p = p->next) {
						print_preference_and_source(soarAgent, p, print_prod, wtt);
					}
				}
			}

			return 0;
		}

		bool TgDWorkArounds::Preferences(IAgent* thisAgent, int detail, const char* idString, const char* attrString)
		{
			static const int PREFERENCES_BUFFER_SIZE = 128;

			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to Preferences.");
			agent* soarAgent = internalAgent->GetSoarAgent();

			char id[PREFERENCES_BUFFER_SIZE];
			char attr[PREFERENCES_BUFFER_SIZE];

			// Establish defaults
			symbol_to_string(soarAgent, soarAgent->bottom_goal, TRUE, id, PREFERENCES_BUFFER_SIZE);
			symbol_to_string(soarAgent, soarAgent->operator_symbol, TRUE, attr, PREFERENCES_BUFFER_SIZE);

			// Override defaults
			if (idString) {
				strncpy(id, idString, PREFERENCES_BUFFER_SIZE);

				// Notice: attrString ignored if no idString passed
				if (attrString) {
					strncpy(attr, attrString, PREFERENCES_BUFFER_SIZE);
				}
			}

			bool print_productions = false;
			wme_trace_type wtt = NONE_WME_TRACE;

			switch (detail) {
				case 0:
					print_productions = false;
					wtt = NONE_WME_TRACE;
					break;
				case 1:
					print_productions = true;
					wtt = NONE_WME_TRACE;
					break;
				case 2:
					print_productions = true;
					wtt = TIMETAG_WME_TRACE;
					break;
				case 3:
					print_productions = true;
					wtt = FULL_WME_TRACE;
					break;
				default:
					MegaAssert(false, "Illegal detail level");
			}

			if (soar_ecPrintPreferences(soarAgent, id, attr, print_productions, wtt)) {
				print(soarAgent, "An Error occured trying to print the prefs.");
				return false;
			}
			return true;
		}

		typedef struct binding_structure {
			Symbol *from, *to;
		} Binding;


		Symbol *get_binding (Symbol *f, list *bindings) 
		{
			cons *c;

			for (c=bindings;c!=NIL;c=c->rest) 
			{
				if (((Binding *) c->first)->from == f)
					return ((Binding *) c->first)->to;
			}
			return NIL;
		}

		bool symbols_are_equal_with_bindings (agent* agnt, Symbol *s1, Symbol *s2, list **bindings) 
		{
			Binding *b;
			Symbol *bvar;

			/* SBH/MVP 7-5-94 */
			if ((s1 == s2) && (s1->common.symbol_type != VARIABLE_SYMBOL_TYPE))
				return TRUE;

			/* "*" matches everything. */
			if ((s1->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
				(!strcmp(s1->sc.name,"*"))) return TRUE;
			if ((s2->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) &&
				(!strcmp(s2->sc.name,"*"))) return TRUE;


			if ((s1->common.symbol_type != VARIABLE_SYMBOL_TYPE) ||
				(s2->common.symbol_type != VARIABLE_SYMBOL_TYPE))
				return FALSE;
			/* Both are variables */
			bvar = get_binding(s1,*bindings);
			if (bvar == NIL) {
				b = (Binding *) allocate_memory(agnt, sizeof(Binding),MISCELLANEOUS_MEM_USAGE);
				b->from = s1;
				b->to = s2;
				push(agnt, b,*bindings);
				return TRUE;
			}
			else if (bvar == s2) {
				return TRUE;
			}
			else return FALSE;
		}

		bool actions_are_equal_with_bindings (agent* agnt, action *a1, action *a2, list **bindings) 
		{
			//         if (a1->type == FUNCALL_ACTION) 
			//         {
			//            if ((a2->type == FUNCALL_ACTION)) 
			//            {
			//               if (funcalls_match(rhs_value_to_funcall_list(a1->value),
			//                  rhs_value_to_funcall_list(a2->value))) 
			//               {
			//                     return TRUE;
			//               }
			//               else return FALSE;
			//            }
			//            else return FALSE;
			//         }
			if (a2->type == FUNCALL_ACTION) return FALSE;

			/* Both are make_actions. */

			if (a1->preference_type != a2->preference_type) return FALSE;

			if (!symbols_are_equal_with_bindings(agnt, rhs_value_to_symbol(a1->id),
				rhs_value_to_symbol(a2->id),
				bindings)) return FALSE;

			if ((rhs_value_is_symbol(a1->attr)) && (rhs_value_is_symbol(a2->attr))) 
			{
				if (!symbols_are_equal_with_bindings(agnt, rhs_value_to_symbol(a1->attr),
					rhs_value_to_symbol(a2->attr), bindings)) 
				{
					return FALSE;
				}
			} else {
				//            if ((rhs_value_is_funcall(a1->attr)) && (rhs_value_is_funcall(a2->attr))) 
				//            {
				//               if (!funcalls_match(rhs_value_to_funcall_list(a1->attr),
				//                  rhs_value_to_funcall_list(a2->attr)))
				//               {
				//                  return FALSE;
				//               }
				//            }
			}

			/* Values are different. They are rhs_value's. */

			if ((rhs_value_is_symbol(a1->value)) && (rhs_value_is_symbol(a2->value))) 
			{
				if (symbols_are_equal_with_bindings(agnt, rhs_value_to_symbol(a1->value),
					rhs_value_to_symbol(a2->value), bindings)) 
				{
					return TRUE;
				}
				else 
				{
					return FALSE;
				}
			}
			if ((rhs_value_is_funcall(a1->value)) && (rhs_value_is_funcall(a2->value))) 
			{
				//            if (funcalls_match(rhs_value_to_funcall_list(a1->value),
				//               rhs_value_to_funcall_list(a2->value)))
				//            {
				//               return TRUE;
				//            }
				//            else 
				{
					return FALSE;
				}
			}
			return FALSE;
		}


		void reset_old_binding_point(agent* agnt, list **bindings, list **current_binding_point) 
		{
			cons *c,*c_next;

			c = *bindings;
			while (c != *current_binding_point) {
				c_next = c->rest;
				free_memory(agnt, c->first,MISCELLANEOUS_MEM_USAGE);
				free_cons (agnt, c);
				c = c_next;
			}

			bindings = current_binding_point;
		}

		void free_binding_list (agent* agnt, list *bindings) 
		{
			cons *c;

			for (c=bindings;c!=NIL;c=c->rest)
				free_memory(agnt, c->first,MISCELLANEOUS_MEM_USAGE);
			free_list(agnt, bindings);
		}

		void print_binding_list (agent* agnt, list *bindings) 
		{
			cons *c;

			for (c = bindings ; c != NIL ; c = c->rest)
			{
				print_with_symbols (agnt, "   (%y -> %y)\n",((Binding *) c->first)->from,((Binding *) c->first)->to);
			}
		}

		void read_rhs_pattern_and_get_matching_productions (agent* agnt,
			kernel* krnl,
			list **current_pf_list, 
			bool show_bindings,
			bool just_chunks, 
			bool no_chunks) 
		{
			action *a, *alist, *pa;
			int i;
			production *prod;
			list *bindings, *current_binding_point;
			bool match, match_this_a, parsed_ok;
			action *rhs; 
			condition *top_cond, *bottom_cond;

			bindings = NIL;
			current_binding_point = NIL;

			/*  print("Parsing as a rhs...\n"); */
			parsed_ok = (parse_rhs(agnt, &alist) == TRUE);
			if (!parsed_ok) {
				print(agnt, "Error: not a valid rhs.\n");
				current_pf_list = NIL;
				return;
			}

			/*
			print("Valid RHS:\n");
			print_action_list(alist,0,FALSE);
			print("\nMatches:\n");
			*/

			for (i=0; i<NUM_PRODUCTION_TYPES; i++)
			{
				if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) || (i != CHUNK_PRODUCTION_TYPE && !just_chunks))
				{
					for (prod=agnt->all_productions_of_type[i]; prod!=NIL; prod=prod->next) 
					{
						match = TRUE;

						free_binding_list(agnt, bindings);
						bindings = NIL;

						p_node_to_conditions_and_nots (agnt, prod->p_node, NIL, NIL, &top_cond,
							&bottom_cond, NIL, &rhs);
						deallocate_condition_list (agnt, top_cond);
						for (a=alist;a!=NIL;a=a->next) 
						{
							match_this_a= FALSE;
							current_binding_point = bindings;

							for (pa = rhs; pa != NIL; pa=pa->next) 
							{
								if (actions_are_equal_with_bindings(agnt, a,pa,&bindings)) 
								{
									match_this_a = TRUE;
									break;
								}
								else 
								{
									/* Remove new, incorrect bindings. */
									reset_old_binding_point(agnt, &bindings,&current_binding_point);
									bindings= current_binding_point;
								}
							}
							if (!match_this_a) 
							{
								match = FALSE; 
								break;
							}
						}

						deallocate_action_list (agnt, rhs);
						if (match) 
						{
							push(agnt,prod,(*current_pf_list));
							if (show_bindings) 
							{
								print_with_symbols(agnt, "%y, with bindings:\n",prod->name);
								print_binding_list(agnt,bindings);
							}
							else
							{
								print_with_symbols(agnt,"%y\n",prod->name);
							}
						}
					}      
				}
			}
			if (bindings) 
			{
				free_binding_list(agnt, bindings); /* DJP 4/3/96 -- To catch the last production */
			}
		}

#define dealloc_and_return(agnt,x,y) { deallocate_test(agnt, x) ; return (y) ; }

		/* DJP 4/3/96 -- changed t2 to test2 in declaration */
		bool tests_are_equal_with_bindings (agent* agnt, test t1, test test2, list **bindings) {
			cons *c1, *c2;
			complex_test *ct1, *ct2;
			Bool goal_test,impasse_test;

			/* DJP 4/3/96 -- The problem here is that sometimes test2 was being copied      */
			/*               and sometimes it wasn't.  If it was copied, the copy was never */
			/*               deallocated.  There's a few choices about how to fix this.  I  */
			/*               decided to just create a copy always and then always           */
			/*               deallocate it before returning.  Added a macro to do that.     */

			test t2;

			/* t1 is from the pattern given to "pf"; t2 is from a production's condition list. */
			if (test_is_blank_test(t1)) 
				return(test_is_blank_test(test2) == 0);

			/* If the pattern doesn't include "(state", but the test from the
			* production does, strip it out of the production's. 
			*/
			if ((!test_includes_goal_or_impasse_id_test(t1,TRUE,FALSE)) &&
				test_includes_goal_or_impasse_id_test(test2,TRUE,FALSE)) 
			{
				goal_test = FALSE;
				impasse_test = FALSE;
				t2 = copy_test_removing_goal_impasse_tests(agnt, test2, &goal_test, &impasse_test);
			}
			else
			{
				t2 = copy_test(agnt,test2) ; /* DJP 4/3/96 -- Always make t2 into a copy */
			}

			if (test_is_blank_or_equality_test(t1)) 
			{
				if (!(test_is_blank_or_equality_test(t2) && !(test_is_blank_test(t2))))
				{
					dealloc_and_return(agnt, t2,FALSE);
				}
				else 
				{
					if (symbols_are_equal_with_bindings(agnt, referent_of_equality_test(t1),
						referent_of_equality_test(t2),
						bindings))
					{
						dealloc_and_return(agnt, t2,TRUE);
					}
					else
					{
						dealloc_and_return(agnt, t2,FALSE);
					}
				}
			}

			ct1 = complex_test_from_test(t1);
			ct2 = complex_test_from_test(t2);

			if (ct1->type != ct2->type) 
			{
				dealloc_and_return(agnt, t2,FALSE);
			}

			switch(ct1->type) 
			{
			case GOAL_ID_TEST: 
				dealloc_and_return(agnt, t2,TRUE);
				break;
			case IMPASSE_ID_TEST: 
				dealloc_and_return(agnt, t2,TRUE);
				break;

			case DISJUNCTION_TEST:
				for (c1 = ct1->data.disjunction_list, c2=ct2->data.disjunction_list;
					((c1!=NIL)&&(c2!=NIL)); c1=c1->rest, c2=c2->rest)
				{
					if (c1->first != c2->first) 
					{
						dealloc_and_return(agnt, t2,FALSE)
					}
				}
				if (c1==c2) 
				{
					dealloc_and_return(agnt, t2,TRUE);  /* make sure they both hit end-of-list */
				}
				else
				{
					dealloc_and_return(agnt, t2,FALSE);
				}

			case CONJUNCTIVE_TEST:
				for (c1=ct1->data.conjunct_list, c2=ct2->data.conjunct_list;
					((c1!=NIL)&&(c2!=NIL)); c1=c1->rest, c2=c2->rest)
				{
					if (! tests_are_equal_with_bindings(agnt, (test)c1->first, (test)c2->first, bindings)) 
						dealloc_and_return(agnt, t2,FALSE)
				}
				if (c1==c2) 
				{
					dealloc_and_return(agnt, t2,TRUE);  /* make sure they both hit end-of-list */
				}
				else 
				{
					dealloc_and_return(agnt, t2,FALSE);
				}

			default:  /* relational tests other than equality */
				if (symbols_are_equal_with_bindings(agnt, ct1->data.referent,ct2->data.referent,bindings))
				{
					dealloc_and_return(agnt, t2,TRUE);
				}
				else
				{
					dealloc_and_return(agnt, t2,FALSE);
				}
			}
		}

		bool conditions_are_equal_with_bindings (agent* agnt, condition *c1, condition *c2, list **bindings) {
			if (c1->type != c2->type) return FALSE;
			switch (c1->type) 
			{
			case POSITIVE_CONDITION:
			case NEGATIVE_CONDITION:
				if (! tests_are_equal_with_bindings (agnt, c1->data.tests.id_test,
					c2->data.tests.id_test,bindings))
					return FALSE;
				if (! tests_are_equal_with_bindings (agnt, c1->data.tests.attr_test,
					c2->data.tests.attr_test,bindings))

					return FALSE;
				if (! tests_are_equal_with_bindings (agnt, c1->data.tests.value_test,
					c2->data.tests.value_test,bindings))
					return FALSE;
				if (c1->test_for_acceptable_preference != c2->test_for_acceptable_preference)
					return FALSE;
				return TRUE;

			case CONJUNCTIVE_NEGATION_CONDITION:
				for (c1=c1->data.ncc.top, c2=c2->data.ncc.top;
					((c1!=NIL)&&(c2!=NIL));
					c1=c1->next, c2=c2->next)
					if (! conditions_are_equal_with_bindings (agnt, c1,c2,bindings)) return FALSE;
				if (c1==c2) return TRUE;  /* make sure they both hit end-of-list */
				return FALSE;
			}
			return FALSE; /* unreachable, but without it, gcc -Wall warns here */
		}

		void read_pattern_and_get_matching_productions (agent* agnt,
			kernel* krnl,
			list **current_pf_list, 
			bool show_bindings,
			bool just_chunks,
			bool no_chunks) 
		{
			condition *c, *clist, *top, *bottom, *pc;
			int i;
			production *prod;
			list *bindings, *current_binding_point;
			bool match, match_this_c;


			bindings = NIL;
			current_binding_point = NIL;

			/*  print("Parsing as a lhs...\n"); */
			clist = (condition *) parse_lhs(agnt);
			if (!clist) {
				print(agnt, "Error: not a valid condition list.\n");
				current_pf_list = NIL;
				return;
			}
			/*
			print("Valid condition list:\n");
			print_condition_list(clist,0,FALSE);
			print("\nMatches:\n");
			*/

			/* For the moment match against productions of all types (user,chunk,default, justification).     Later on the type should be a parameter.
			*/

			for (i=0; i<NUM_PRODUCTION_TYPES; i++)
				if ((i == CHUNK_PRODUCTION_TYPE && !no_chunks) ||
					(i != CHUNK_PRODUCTION_TYPE && !just_chunks))
					for (prod=agnt->all_productions_of_type[i]; prod!=NIL; prod=prod->next) {

						/* Now the complicated part. */
						/* This is basically a full graph-match.  Like the rete.  Yikes! */
						/* Actually it's worse, because there are so many different KINDS of
						conditions (negated, >/<, acc prefs, ...) */
						/* Is there some way I could *USE* the rete for this?  -- for lhs
						positive conditions, possibly.  Put some fake stuff into WM
						(i.e. with make-wme's), see what matches all of them, and then
						yank out the fake stuff.  But that won't work for RHS or for
						negateds.       */

						/* Also note that we need bindings for every production.  Very expensive
						(but don't necc. need to save them -- maybe can just print them as we go). */

						match = TRUE;
						p_node_to_conditions_and_nots (agnt, prod->p_node, NIL, NIL, &top, &bottom,
							NIL, NIL);

						free_binding_list(agnt, bindings);
						bindings = NIL;

						for (c=clist;c!=NIL;c=c->next) {
							match_this_c= FALSE;
							current_binding_point = bindings;

							for (pc = top; pc != NIL; pc=pc->next) {
								if (conditions_are_equal_with_bindings(agnt, c,pc,&bindings)) {
									match_this_c = TRUE;
									break;}
								else {
									/* Remove new, incorrect bindings. */
									reset_old_binding_point(agnt, &bindings,&current_binding_point);
									bindings= current_binding_point;
								}
							}
							if (!match_this_c) {match = FALSE; break;}
						}
						deallocate_condition_list (agnt, top); /* DJP 4/3/96 -- Never dealloced */
						if (match) {
							push(agnt, prod,(*current_pf_list));
							if (show_bindings) {
								print_with_symbols(agnt, "%y, with bindings:\n",prod->name);
								print_binding_list(agnt, bindings);}
							else
								print_with_symbols(agnt, "%y\n",prod->name);
						}
					}
					if (bindings) free_binding_list(agnt, bindings); /* DJP 4/3/96 -- To catch the last production */
		}

		bool TgDWorkArounds::ProductionFind(IAgent*     thisAgent,
			agent*      agnt2,
			IKernel*    kernel,
			bool        lhs,
			bool        rhs,
			char*       arg,
			bool        show_bindings,
			bool        just_chunks,
			bool        no_chunks)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");
			agent* agnt = internalAgent->GetSoarAgent();

			Kernel* internalKernel = (Kernel*)(kernel);
			MegaAssert(kernel != 0, "Bad agent pointer passed to set_sysparams.");


			list* current_pf_list = 0;



			if (lhs) 
			{
				/* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
				/* Soar-Bugs #54 TMH */
				agnt->alternate_input_string = arg;
				agnt->alternate_input_suffix = ") ";

				get_lexeme(agnt);
				read_pattern_and_get_matching_productions (agnt, 
					internalKernel->GetSoarKernel(), 
					&current_pf_list,
					show_bindings,
					just_chunks, 
					no_chunks);
				agnt->current_char = ' ';
			}
			if (rhs)
			{
				/* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
				/* Soar-Bugs #54 TMH */
				agnt->alternate_input_string = arg;
				agnt->alternate_input_suffix = ") ";

				get_lexeme(agnt);
				read_rhs_pattern_and_get_matching_productions (agnt, internalKernel->GetSoarKernel(), &current_pf_list,
					show_bindings,
					just_chunks, 
					no_chunks);
				agnt->current_char = ' ';
			}
			if (current_pf_list == NIL) 
			{
				print(agnt, "No matches.\n");
			}

			free_list(agnt, current_pf_list);
			return true;
		} ////

		bool TgDWorkArounds::GDSPrint(IAgent* thisAgent)
		{
			Agent* internalAgent = (Agent*)(thisAgent);
			MegaAssert(internalAgent != 0, "Bad agent pointer passed to set_sysparams.");
			agent* agnt = internalAgent->GetSoarAgent();

			wme *w;
			Symbol *goal;


			print(agnt, "********************* Current GDS **************************\n");
			print(agnt, "stepping thru all wmes in rete, looking for any that are in a gds...\n");
			for (w=agnt->all_wmes_in_rete; w!=NIL; w=w->rete_next) 
			{
				if (w->gds)
				{
					if (w->gds->goal) 
					{
						print_with_symbols (agnt, "  For Goal  %y  ", w->gds->goal);
					} 
					else 
					{
						print(agnt, "  Old GDS value ");
					}
					print (agnt, "(%lu: ", w->timetag);
					print_with_symbols (agnt, "%y ^%y %y", w->id, w->attr, w->value);
					if (w->acceptable) 
					{
						print_string (agnt, " +");
					}
					print_string (agnt, ")");
					print (agnt, "\n");
				}
			}
			print(agnt, "************************************************************\n");
			for (goal=agnt->top_goal; goal!=NIL; goal=goal->id.lower_goal)
			{
				print_with_symbols (agnt, "  For Goal  %y  ", goal);
				if (goal->id.gds){
					/* Loop over all the WMEs in the GDS */
					print (agnt, "\n");
					for (w=goal->id.gds->wmes_in_gds; w!=NIL; w=w->gds_next)
					{
						print (agnt, "                (%lu: ", w->timetag);
						print_with_symbols (agnt, "%y ^%y %y", w->id, w->attr, w->value);
						if (w->acceptable) 
						{
							print_string (agnt, " +");
						}
						print_string (agnt, ")");
						print (agnt, "\n");
					}

				} 
				else 
				{
					print(agnt, ": No GDS for this goal.\n");
				}
			}

			print(agnt, "************************************************************\n");
			return true;
		}////

		void TgDWorkArounds::GetForceLearnStates(IAgent* pIAgent, std::string& res) {
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			cons *c;
			char buff[1024];

			for (c = pSoarAgent->chunky_problem_spaces; c != NIL; c = c->rest) {
				symbol_to_string(pSoarAgent, (Symbol *) (c->first), TRUE, buff, 1024);
				res += buff;
			}
		}

		void TgDWorkArounds::GetDontLearnStates(IAgent* pIAgent, std::string& res) {
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			cons *c;
			char buff[1024];

			for (c = pSoarAgent->chunk_free_problem_spaces; c != NIL; c = c->rest) {
				symbol_to_string(pSoarAgent, (Symbol *) (c->first), TRUE, buff, 1024);
				res += buff;
			}
		}

		void TgDWorkArounds::SetVerbosity(IAgent* pIAgent, bool setting) {
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			pSoarAgent->soar_verbose_flag = setting;

		}

		bool TgDWorkArounds::GetVerbosity(IAgent* pIAgent) {
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			return pSoarAgent->soar_verbose_flag ? true : false;
		}

		bool TgDWorkArounds::BeginTracingProduction(IAgent* pIAgent, const char* pProductionName) {
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			Symbol *sym;
			sym = find_sym_constant(pSoarAgent, pProductionName);

			if (!sym || !(sym->sc.production))
				return false;

			add_pwatch(pSoarAgent, sym->sc.production);
			return true;
		}

		bool TgDWorkArounds::StopTracingProduction(IAgent* pIAgent, const char* pProductionName) {
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			Symbol *sym;
			sym = find_sym_constant(pSoarAgent, pProductionName);

			if (!sym || !(sym->sc.production))
				return false;

			remove_pwatch(pSoarAgent, sym->sc.production);
			return true;
		}

		long TgDWorkArounds::AddWme(IAgent* pIAgent, const char* pIdString, const char* pAttrString, const char* pValueString, bool acceptable) {

			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			// Get ID
			Symbol* pId;
			if (!read_id_or_context_var_from_string(pSoarAgent, const_cast<char*>(pIdString), &pId)) return -1;

			// get optional '^', if present
			if (*pAttrString == '^') {
				pAttrString++;
			}

			// get attribute or '*'
			Symbol* pAttr;
			if (string_match("*", const_cast<char*>(pAttrString))) {
				pAttr = make_new_identifier(pSoarAgent, 'I', pId->id.level);
			} else {
				get_lexeme_from_string(pSoarAgent, const_cast<char*>(pAttrString));

				switch (pSoarAgent->lexeme.type) {
				case SYM_CONSTANT_LEXEME:
					pAttr = make_sym_constant(pSoarAgent, pSoarAgent->lexeme.string);
					break;
				case INT_CONSTANT_LEXEME:
					pAttr = make_int_constant(pSoarAgent, pSoarAgent->lexeme.int_val);
					break;
				case FLOAT_CONSTANT_LEXEME:
					pAttr = make_float_constant(pSoarAgent, pSoarAgent->lexeme.float_val);
					break;
				case IDENTIFIER_LEXEME:
				case VARIABLE_LEXEME:
					pAttr = read_identifier_or_context_variable(pSoarAgent);
					if (!pAttr) {
						return -2;
					}
					symbol_add_ref(pAttr);
					break;
				default:
					return -2;
				}
			}

			// get value or '*'
			Symbol* pValue;
			if (string_match("*", const_cast<char*>(pValueString)) == TRUE) {
				pValue = make_new_identifier(pSoarAgent, 'I', pId->id.level);
			} else {
				get_lexeme_from_string(pSoarAgent, const_cast<char*>(pValueString));
				switch (pSoarAgent->lexeme.type) {
				case SYM_CONSTANT_LEXEME:
					pValue = make_sym_constant(pSoarAgent, pSoarAgent->lexeme.string);
					break;
				case INT_CONSTANT_LEXEME:
					pValue = make_int_constant(pSoarAgent, pSoarAgent->lexeme.int_val);
					break;
				case FLOAT_CONSTANT_LEXEME:
					pValue = make_float_constant(pSoarAgent, pSoarAgent->lexeme.float_val);
					break;
				case IDENTIFIER_LEXEME:
				case VARIABLE_LEXEME:
					pValue = read_identifier_or_context_variable(pSoarAgent);
					if (!pValue) {
						symbol_remove_ref(pSoarAgent, pAttr);
						return -3;
					}
					symbol_add_ref(pValue);
					break;
				default:
					symbol_remove_ref(pSoarAgent, pAttr);
					return -3;
				}
			}

			// now create and add the wme
			wme* pWme = make_wme(pSoarAgent, pId, pAttr, pValue, acceptable);			

			symbol_remove_ref(pSoarAgent, pWme->attr);
			symbol_remove_ref(pSoarAgent, pWme->value);
			insert_at_head_of_dll(pWme->id->id.input_wmes, pWme, next, prev);
			add_wme_to_wm(pSoarAgent, pWme);

#ifdef USE_CAPTURE_REPLAY
			// TODO
#endif // USE_CAPTURE_REPLAY

			/* REW: begin 28.07.96 */
			/* OK.  This is an ugly hack.  Basically we want to keep track of kernel
			time and callback time.  add-wme is called from either a callback
			(for input routines) or from the command line (or someplace else?).
			Here, I'm just assuming that we'll normally call add-wme from an
			input routine so I turn off the input_function_timer and turn on
			the kernel timers before the call to the low-level function:
			do_buffered_wm_and_ownership_changes.

			This assumption is problematic because anytime add-wme is called 
			from some place other than the input function, there is a potential
			to get some erroneous (if start_kernel_tv wasn't set for the 
			input function) or just bad (if start_kernel_tv isn't defined)
			timing data.   The real problem is that this very high-level 
			routine is going deep into the kernel.  We can either ignore
			this and just call the time spent doing wm changes here time
			spent outside the kernel or we can try to do the accounting,
			what this hack is a first-attempt at doing.  

			However, my testing turned up no problems -- I was able to add
			and remove WMEs without messing up the timers.  So it`s a 
			hack that seems to work.  For now.  (there is a plan to 
			add routines for specifically adding and deleting input
			WMEs which should help clear up this isse)               REW */

			/* REW: end 28.07.96 */

#ifndef NO_TIMING_STUFF
			if (pSoarAgent->current_phase == INPUT_PHASE) {
				/* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
				stop_timer(pSoarAgent, &(pSoarAgent->start_kernel_tv), &(pSoarAgent->input_function_cpu_time));
				start_timer(pSoarAgent, &(pSoarAgent->start_kernel_tv));

#ifndef KERNEL_TIME_ONLY
				start_timer(pSoarAgent, &(pSoarAgent->start_phase_tv));
#endif // KERNEL_TIME_ONLY
			}
#endif // NO_TIMING_STUFF
			/* REW: end 28.07.96 */

			/* note: 
			* I don't completely understand this:
			* The deal seems to be that when NO_TOP_LEVEL_REFS is used, wmes on the i/o
			* link (obviously) have fewer references than thy would otherwise.
			* Although calling this here (in soar_cAddWme) doesn't seem to matter
			* one way or the other, calling it in soar_cRemoveWme really leads to 
			* problems.  What happens is that the i/o wme is removed prior to 
			* fully figuring out the match set.  This means that productions which
			* should have fired, dont.  However, if we comment this out for the
			* NO_TOP_LEVEL_REFS fix we don't seem to get this problem.  There might be
			* an underlying pathology here, but so far I don't know what it is.
			* This suspicion is heightened by the fact that even when this fix
			* is made, wmes are deallocated in a different place (e.g. at the end of
			* the input cycle) than using a normal build.
			*
			* an interesting aside seems to be that we don't need to do buffered
			* wme and own changes here regardless of whether or not L1R is used
			* so long as we test to make sure we're in the INPUT_PHASE.  I will 
			* look into this more later.
			*/

#ifndef NO_TOP_LEVEL_REFS
			do_buffered_wm_and_ownership_changes(pSoarAgent);
#endif // NO_TOP_LEVEL_REFS

			/* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
			if (pSoarAgent->current_phase == INPUT_PHASE) {
#ifndef KERNEL_TIME_ONLY
				stop_timer(pSoarAgent, &(pSoarAgent->start_phase_tv), &(pSoarAgent->decision_cycle_phase_timers[(pSoarAgent->current_phase)]));
#endif // KERNEL_TIME_ONLY
				stop_timer(pSoarAgent, &(pSoarAgent->start_kernel_tv), &(pSoarAgent->total_kernel_time));
				start_timer(pSoarAgent, &(pSoarAgent->start_kernel_tv));
			}
#endif // NO_TIMING_STUFF
			/* REW: end 28.07.96 */

			//*new_wme = (psoar_wme) pWme; // new_wme declared in parameter list in soar_cAddWme 8.5.2
			return static_cast<long>(pWme->timetag);
		}

		int RemoveWme(agent* pSoarAgent, wme* pWme)
		{
			//	wme *w, *w2;
			//	Symbol *id;
			//	slot *s;

			//	w = (wme *) the_wme;

			Symbol* pId = pWme->id;

			// remove w from whatever list of wmes it's on
			wme* pWme2;
			for (pWme2 = pId->id.input_wmes; pWme2 != NIL; pWme2 = pWme2->next)
				if (pWme == pWme2)
					break;

			if (pWme2) remove_from_dll(pId->id.input_wmes, pWme, next, prev);

			for (pWme2 = pId->id.impasse_wmes; pWme2 != NIL; pWme2 = pWme2->next)
				if (pWme == pWme2)
					break;

			if (pWme2) remove_from_dll(pId->id.impasse_wmes, pWme, next, prev);

			slot* s;
			for (s = pId->id.slots; s != NIL; s = s->next) {

				for (pWme2 = s->wmes; pWme2 != NIL; pWme2 = pWme2->next)
					if (pWme == pWme2)
						break;

				if (pWme2)
					remove_from_dll(s->wmes, pWme, next, prev);

				for (pWme2 = s->acceptable_preference_wmes; pWme2 != NIL; pWme2 = pWme2->next)
					if (pWme == pWme2)
						break;

				if (pWme2)
					remove_from_dll(s->acceptable_preference_wmes, pWme, next, prev);
			}

#ifdef USE_CAPTURE_REPLAY
			// TODO: ommitted
#endif // USE_CAPTURE_REPLAY

			/* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
			if (pSoarAgent->operand2_mode) {
#endif // SOAR_8_ONLY
				if (pWme->gds) {
					if (pWme->gds->goal != NIL) {

						gds_invalid_so_remove_goal(pSoarAgent, pWme);

						/* NOTE: the call to remove_wme_from_wm will take care of checking if
						GDS should be removed */
					}
				}
#ifndef SOAR_8_ONLY
			}
#endif // SOAR_8_ONLY
			/* REW: end   09.15.96 */

			// now remove w from working memory
			remove_wme_from_wm(pSoarAgent, pWme);

			/* REW: begin 28.07.96 */
			/* See AddWme for description of what's going on here */

			if (pSoarAgent->current_phase != INPUT_PHASE) {
#ifndef NO_TIMING_STUFF
				start_timer(pSoarAgent, &(pSoarAgent->start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
				start_timer(pSoarAgent, &(pSoarAgent->start_phase_tv));
#endif // KERNEL_TIME_ONLY
#endif // NO_TIMING_STUFF

				/* do_buffered_wm_and_ownership_changes(); */

#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
				stop_timer(pSoarAgent, &(pSoarAgent->start_phase_tv), &(pSoarAgent->decision_cycle_phase_timers[(pSoarAgent->current_phase)]));
#endif // KERNEL_TIME_ONLY
				stop_timer(pSoarAgent, &(pSoarAgent->start_kernel_tv), &(pSoarAgent->total_kernel_time));
				start_timer(pSoarAgent, &(pSoarAgent->start_kernel_tv));
#endif // NO_TIMING_STUFF
			}

			/* note: 
			*  See note at the NO_TOP_LEVEL_REFS flag in soar_cAddWme
			*/

#ifndef NO_TOP_LEVEL_REFS
			do_buffered_wm_and_ownership_changes(pSoarAgent);
#endif // NO_TOP_LEVEL_REFS

			return 0;
		}

		int TgDWorkArounds::RemoveWmeByTimetag(IAgent* pIAgent, int num)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			wme *pWme;

			for (pWme = pSoarAgent->all_wmes_in_rete; pWme != NIL; pWme = pWme->rete_next)
				if (pWme->timetag == (unsigned long) num)
					break;

			if (!pWme) return -1;
			if (!RemoveWme(pSoarAgent, pWme)) return 0; // this is the correct case
			return -2;                  /* Unspecified Failure */
		}

		void TgDWorkArounds::PrintInternalSymbols(IAgent* pIAgent)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			print_string(pSoarAgent, "\n--- Symbolic Constants: ---\n");
			do_for_all_items_in_hash_table(pSoarAgent, pSoarAgent->sym_constant_hash_table, print_sym, 0);
			print_string(pSoarAgent, "\n--- Integer Constants: ---\n");
			do_for_all_items_in_hash_table(pSoarAgent, pSoarAgent->int_constant_hash_table, print_sym, 0);
			print_string(pSoarAgent, "\n--- Floating-Point Constants: ---\n");
			do_for_all_items_in_hash_table(pSoarAgent, pSoarAgent->float_constant_hash_table, print_sym, 0);
			print_string(pSoarAgent, "\n--- Identifiers: ---\n");
			do_for_all_items_in_hash_table(pSoarAgent, pSoarAgent->identifier_hash_table, print_sym, 0);
			print_string(pSoarAgent, "\n--- Variables: ---\n");
			do_for_all_items_in_hash_table(pSoarAgent, pSoarAgent->variable_hash_table, print_sym, 0);
		}

		bool read_wme_filter_component(agent* pSoarAgent, const char *s, Symbol ** sym)
		{
			get_lexeme_from_string(pSoarAgent, const_cast<char*>(s));
			if (pSoarAgent->lexeme.type == IDENTIFIER_LEXEME) {
				if ((*sym = find_identifier(pSoarAgent, pSoarAgent->lexeme.id_letter, pSoarAgent->lexeme.id_number)) == NIL) {
					return false;          /* Identifier does not exist */
				}
			} else {
				*sym = make_symbol_for_current_lexeme(pSoarAgent);
			}
			// Added by voigtjr because if this function can 
			// legally return success with *sym == 0, my logic in AddWmeFilter will be broken.
			assert(*sym);
			return true;
		}

		int TgDWorkArounds::AddWMEFilter(IAgent* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			Symbol* pId = 0;
			if (!read_wme_filter_component(pSoarAgent, pIdString, &pId)) {
				return -1;
			}

			Symbol* pAttr = 0;
			if (!read_wme_filter_component(pSoarAgent, pAttrString, &pAttr)) {
				symbol_remove_ref(pSoarAgent, pId);
				return -2;
			}

			Symbol* pValue = 0;
			if (!read_wme_filter_component(pSoarAgent, pValueString, &pValue)) {
				symbol_remove_ref(pSoarAgent, pId);
				symbol_remove_ref(pSoarAgent, pAttr);
				return -3;
			}

			/* check to see if such a filter has already been added: */
			cons *c;
			wme_filter* existing_wf;
			for (c = pSoarAgent->wme_filter_list; c != NIL; c = c->rest) {

				existing_wf = static_cast<wme_filter*>(c->first);

				// check for duplicate
				if ((existing_wf->adds == adds) 
					&& (existing_wf->removes == removes)
					&& (existing_wf->id == pId) 
					&& (existing_wf->attr == pAttr)
					&& (existing_wf->value == pValue)) 
				{
					symbol_remove_ref(pSoarAgent, pId);
					symbol_remove_ref(pSoarAgent, pAttr);
					symbol_remove_ref(pSoarAgent, pValue);
					return -4; // Filter already exists
				}
			}

			wme_filter* wf = static_cast<wme_filter*>(allocate_memory(pSoarAgent, sizeof(wme_filter), MISCELLANEOUS_MEM_USAGE));
			wf->id = pId;
			wf->attr = pAttr;
			wf->value = pValue;
			wf->adds = adds;
			wf->removes = removes;

			/* Rather than add refs for the new filter symbols and then remove refs 
			* for the identical symbols created from the string parameters, skip
			* the two nullifying steps altogether and just return immediately
			* after pushing the new filter:
			*/
			push(pSoarAgent, wf, pSoarAgent->wme_filter_list);     
			return 0;
		}

		int TgDWorkArounds::RemoveWMEFilter(IAgent* pIAgent, const char *pIdString, const char *pAttrString, const char *pValueString, bool adds, bool removes)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			Symbol* pId = 0;
			if (!read_wme_filter_component(pSoarAgent, pIdString, &pId)) {
				return -1;
			}

			Symbol* pAttr = 0;
			if (!read_wme_filter_component(pSoarAgent, pAttrString, &pAttr)) {
				symbol_remove_ref(pSoarAgent, pId);
				return -2;
			}

			Symbol* pValue = 0;
			if (!read_wme_filter_component(pSoarAgent, pValueString, &pValue)) {
				symbol_remove_ref(pSoarAgent, pId);
				symbol_remove_ref(pSoarAgent, pAttr);
				return -3;
			}

			cons* c;
			cons** prev_cons_rest = &pSoarAgent->wme_filter_list;
			for (c = pSoarAgent->wme_filter_list; c != NIL; c = c->rest) {
				wme_filter* wf = static_cast<wme_filter*>(c->first);

				// check for duplicate
				if ((wf->adds == adds) 
					&& (wf->removes == removes)
					&& (wf->id == pId) 
					&& (wf->attr == pAttr)
					&& (wf->value == pValue)) 
				{
					*prev_cons_rest = c->rest;
					symbol_remove_ref(pSoarAgent, pId);
					symbol_remove_ref(pSoarAgent, pAttr);
					symbol_remove_ref(pSoarAgent, pValue);
					free_memory(pSoarAgent, wf, MISCELLANEOUS_MEM_USAGE);
					free_cons(pSoarAgent, c);
					return 0; /* assume that AddWMEFilter did not add duplicates */
				}
				prev_cons_rest = &(c->rest);
			}
			assert(!c);
			symbol_remove_ref(pSoarAgent, pId);
			symbol_remove_ref(pSoarAgent, pAttr);
			symbol_remove_ref(pSoarAgent, pValue);
			return -4;
		}

		bool TgDWorkArounds::ResetWMEFilters(IAgent* pIAgent, bool adds, bool removes)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			cons*c;
			bool didRemoveSome = false;
			cons** prev_cons_rest = &pSoarAgent->wme_filter_list;
			for (c = pSoarAgent->wme_filter_list; c != NIL; c = c->rest) {

				wme_filter* wf = static_cast<wme_filter*>(c->first);
				if ((adds && wf->adds) || (removes && wf->removes)) {
					*prev_cons_rest = c->rest;
					print_with_symbols(pSoarAgent, "Removed: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
					print(pSoarAgent, "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
					symbol_remove_ref(pSoarAgent, wf->id);
					symbol_remove_ref(pSoarAgent, wf->attr);
					symbol_remove_ref(pSoarAgent, wf->value);
					free_memory(pSoarAgent, wf, MISCELLANEOUS_MEM_USAGE);
					free_cons(pSoarAgent, c);
					didRemoveSome = true;
				}
				prev_cons_rest = &(c->rest);
			}
			return didRemoveSome;
		}

		void TgDWorkArounds::ListWMEFilters(IAgent* pIAgent, bool adds, bool removes)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			cons *c;
			for (c = pSoarAgent->wme_filter_list; c != NIL; c = c->rest) {
				wme_filter* wf = static_cast<wme_filter*>(c->first);

				if ((adds && wf->adds) || (removes && wf->removes)) {
					print_with_symbols(pSoarAgent, "wme filter: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
					print(pSoarAgent, "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
				}
			}
		}

		void TgDWorkArounds::ExplainListChunks(IAgent* pIAgent)
		{
			Agent* pAgent = (Agent*)(pIAgent);
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

		bool TgDWorkArounds::ExplainChunks(IAgent* pIAgent, const char* pProduction, int mode)
		{
			Agent* pAgent = (Agent*)(pIAgent);
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

		const char* TgDWorkArounds::GetChunkNamePrefix(IAgent* pIAgent)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			return pSoarAgent->chunk_name_prefix;
		}

		bool TgDWorkArounds::SetChunkNamePrefix(IAgent* pIAgent, const char* pPrefix)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			if (strchr(pPrefix, '*')) return false;

			// TODO: change to strncpy
			strcpy(pSoarAgent->chunk_name_prefix, pPrefix);
			return true;
		}

		unsigned long TgDWorkArounds::GetChunkCount(IAgent* pIAgent)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			return pSoarAgent->chunk_count;
		}

		void TgDWorkArounds::SetChunkCount(IAgent* pIAgent, unsigned long count)
		{
			Agent* pAgent = (Agent*)(pIAgent);
			agent* pSoarAgent = pAgent->GetSoarAgent();

			pSoarAgent->chunk_count = count;
		}

		void TgDWorkArounds::SeedRandomNumberGenerator(unsigned long int* pSeed)
		{
			if (pSeed) {
				SoarSeedRNG(*pSeed);
				return;
			}
			SoarSeedRNG();
		}
	}// class
}// namespace
