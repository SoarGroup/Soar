#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  reinforcement_learning.cpp
 *
 * =======================================================================
 * Description  :  Various functions for Soar-RL
 * =======================================================================
 */

#include <cstdlib>
#include <cmath>
#include <vector>

#include "reinforcement_learning.h"
#include "production.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "rete.h"
#include "wmem.h"
#include "tempmem.h"
#include "print.h"
#include "xml.h"
#include "utilities.h"

extern Symbol *instantiate_rhs_value (agent* thisAgent, rhs_value rv, goal_stack_level new_id_level, char new_id_letter, struct token_struct *tok, wme *w);
extern void variablize_symbol (agent* thisAgent, Symbol **sym);
extern void variablize_nots_and_insert_into_conditions (agent* thisAgent, not_struct *nots, condition *conds);
extern void variablize_condition_list (agent* thisAgent, condition *cond);

/////////////////////////////////////////////////////
// Visitors
/////////////////////////////////////////////////////

struct Visitor {
	enum Instruction {Repeat, Next, Done};

	const condition * next(const condition * const &cond) { return cond->next; }
	condition * next(condition * const &cond) { return cond->next; }

	const Symbol * next(const Symbol * const &goal) { return goal->id.lower_goal; }
	Symbol * next(Symbol * const &goal) { return goal->id.lower_goal; }

	const preference * next(const preference * const &pref) { return pref->next; }
	preference * next(preference * const &pref) { return pref->next; }

	rl_et_map::const_iterator next(rl_et_map::const_iterator iter) { return ++iter; }
	rl_et_map::iterator next(rl_et_map::iterator iter) { return ++iter; }

	rl_rule_list::const_iterator next(rl_rule_list::const_iterator rule) { return ++rule; }
	rl_rule_list::iterator next(rl_rule_list::iterator rule) { return ++rule; }

	const wme * next(const wme * const &w) { return w->next; }
	wme * next(wme * const &w) { return w->next; }
};

struct Visitor_Instantiation : public Visitor {
	const instantiation * next(const instantiation * const &inst) { return inst->next; }
	instantiation * next(instantiation * const &inst) { return inst->next; }

	const preference * next(const preference * const &pref) { return pref->inst_next; }
	preference * next(preference * const &pref) { return pref->inst_next; }
};

template <typename VISITOR, typename NODE>
inline VISITOR visit(VISITOR visitor, NODE begin, NODE const &end = NODE()) {
	while(begin != end) {
		switch(visitor(begin)) {
		case Visitor::Next:
			begin = visitor.next(begin);
			break;

		case Visitor::Repeat:
			break;

		case Visitor::Done:
		default:
			return visitor;
		}
	}

	return visitor;
}

template <typename VISITOR, typename MEMBER_FUNCTION, typename NODE>
inline void visit(VISITOR &visitor, MEMBER_FUNCTION function, NODE begin, NODE const &end = NODE()) {
	while(begin != end) {
		switch((visitor.*function)(begin)) {
		case Visitor::Next:
			begin = visitor.next(begin);
			break;

		case Visitor::Repeat:
			break;

		case Visitor::Done:
		default:
			return;
		}
	}
}

/////////////////////////////////////////////////////
// Parameters
/////////////////////////////////////////////////////

rl_param_container::rl_param_container( agent *new_agent ): soar_module::param_container( new_agent )
{
	learning = new rl_learning_param( "learning", soar_module::off, new soar_module::f_predicate<soar_module::boolean>(), new_agent );
	add( learning );

	// per-state controls
	granular_control = new soar_module::boolean_param( "granular-control", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
	add( granular_control );

	discount_rate = new soar_module::decimal_param( "discount-rate", 0.9, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
	add( discount_rate );

	learning_rate = new soar_module::decimal_param( "learning-rate", 0.3, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
	add( learning_rate );

	learning_policy = new soar_module::constant_param<learning_choices>( "learning-policy", sarsa, new soar_module::f_predicate<learning_choices>() );
	learning_policy->add_mapping( sarsa, "sarsa" );
	learning_policy->add_mapping( q, "q-learning" );
	add( learning_policy );

	// eligibility-trace-decay-rate
	et_decay_rate = new soar_module::decimal_param( "eligibility-trace-decay-rate", 0, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
	add( et_decay_rate );

	// eligibility-trace-tolerance
	et_tolerance = new soar_module::decimal_param( "eligibility-trace-tolerance", 0.001, new soar_module::gt_predicate<double>( 0, false ), new soar_module::f_predicate<double>() );
	add( et_tolerance );

	temporal_extension = new soar_module::boolean_param( "temporal-extension", soar_module::on, new soar_module::f_predicate<soar_module::boolean>() );
	add( temporal_extension );

	hrl_discount = new soar_module::boolean_param( "hrl-discount", soar_module::on, new soar_module::f_predicate<soar_module::boolean>() );
	add( hrl_discount );
};

//

rl_learning_param::rl_learning_param( const char *new_name, soar_module::boolean new_value, soar_module::predicate<soar_module::boolean> *new_prot_pred, agent *new_agent ) : soar_module::boolean_param( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

void rl_learning_param::set_value( soar_module::boolean new_value )
{
	if ( new_value == soar_module::on && my_agent->rl_first_switch )
	{
		my_agent->rl_first_switch = false;
		exploration_set_policy( my_agent, USER_SELECT_E_GREEDY );

		const char * const msg = "Exploration Mode changed to epsilon-greedy";
		print( my_agent, const_cast<char *>( msg ) );
		xml_generate_message( my_agent, const_cast<char *>( msg ) );
	}

	value = new_value;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Stats
/////////////////////////////////////////////////////

rl_stat_container::rl_stat_container( agent *new_agent ): stat_container( new_agent )
{
	// update-error
	update_error = new soar_module::decimal_stat( "update-error", 0, new soar_module::f_predicate<double>() );
	add( update_error );

	// total-reward
	total_reward = new soar_module::decimal_stat( "total-reward", 0, new soar_module::f_predicate<double>() );
	add( total_reward );

	// global-reward
	global_reward = new soar_module::decimal_stat( "global-reward", 0, new soar_module::f_predicate<double>() );
	add( global_reward );
};


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// quick shortcut to determine if rl is enabled
inline bool rl_enabled( agent *my_agent )
{
	return my_agent->rl_params->learning->get_value() == soar_module::on;
}

struct RL_Reset_Datum : public Visitor {
	Instruction operator()(Symbol * const &goal) {
		rl_data * const &data = goal->id.rl_info;

		data->eligibility_traces->clear();
		data->prev_op_rl_rules->clear();

		data->previous_q = 0;
		data->reward = 0;

		data->gap_age = 0;
		data->hrl_age = 0;

		return Next;
	}
};

// resets rl data structures
void rl_reset_data( agent *my_agent )
{
	visit(RL_Reset_Datum(), my_agent->top_goal);
}

class RL_Remove_Ref_For_Prod : public Visitor {
public:
	RL_Remove_Ref_For_Prod(production * const &prod_) : prod(prod_) {}

	Instruction operator()(Symbol * const &state) {
		state->id.rl_info->eligibility_traces->erase(prod);

		visit(*this, &RL_Remove_Ref_For_Prod::remove_ref, state->id.rl_info->prev_op_rl_rules->begin(), state->id.rl_info->prev_op_rl_rules->end());

		return Next;
	}

private:
	Instruction remove_ref(const rl_rule_list::iterator &p) {
		if(*p == prod)
			*p = NIL;

		return Next;
	}

	production * prod;
};

// removes rl references to a production (used for excise)
void rl_remove_refs_for_prod( agent *my_agent, production *prod )
{
	visit(RL_Remove_Ref_For_Prod(prod), my_agent->top_state);
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// returns true if a template is valid
bool rl_valid_template( production *prod )
{
	return	prod->action_list &&
		!prod->action_list->next &&
		prod->action_list->type == MAKE_ACTION &&
		( prod->action_list->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE ||
		 ( prod->action_list->preference_type == BINARY_INDIFFERENT_PREFERENCE_TYPE &&
		   rhs_value_is_symbol( prod->action_list->referent ) &&
		   rhs_value_to_symbol( prod->action_list->referent )->id.common_symbol_info.symbol_type == VARIABLE_SYMBOL_TYPE ) );
}

// returns true if an rl rule is valid
bool rl_valid_rule( production *prod )
{
	return	prod->action_list &&
		!prod->action_list->next &&
		prod->action_list->type == MAKE_ACTION &&
		prod->action_list->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE;
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// gets the auto-assigned id of a template instantiation
int rl_get_template_id( const char *prod_name )
{
	const size_t len = strlen(prod_name);

	// has to be at least "rl*a*#" (where a is a single letter/number/etc)
	if ( len < 6 )
		return -1;

	// check first three letters are "rl*"
	if ( prod_name[0] != 'r' || prod_name[1] != 'l' || prod_name[2] != '*' )
		return -1;

	// find last * to isolate id
	const char * const end = prod_name + len;
	const char * last_star = end;
	do {
		--last_star;

		if ( last_star == prod_name + 2 )
			return -1;
	} while ( *last_star != '*' );

	// make sure there's something left after last_star
	const char * const id_str = last_star + 1;
	if ( id_str == end )
		return -1;

	// make sure id is a valid natural number
	if ( !is_natural_number( id_str ) )
		return -1;

	// convert id
	int id;
	from_string( id, id_str );
	return id;
}

// initializes the max rl template counter
void rl_initialize_template_tracking( agent *my_agent )
{
	my_agent->rl_template_count = 1;
}

// updates rl template counter for a rule
void rl_update_template_tracking( agent *my_agent, const char *rule_name )
{
	const int new_id = rl_get_template_id( rule_name );

	if ( new_id != -1 && new_id > my_agent->rl_template_count )
		my_agent->rl_template_count = new_id + 1;
}

// gets the next template-assigned id
int rl_next_template_id( agent *my_agent )
{
	return my_agent->rl_template_count++;
}

// gives back a template-assigned id (on auto-retract)
void rl_revert_template_id( agent *my_agent )
{
	--my_agent->rl_template_count;
}

// builds a template instantiation
 Symbol *rl_build_template_instantiation( agent *my_agent, instantiation *my_template_instance, struct token_struct *tok, wme *w )
{
	Bool chunk_var = my_agent->variablize_this_chunk;
	my_agent->variablize_this_chunk = TRUE;

	const production * const & my_template = my_template_instance->prod;
	Symbol *new_name_symbol;

	{	// make unique production name
		const std::string new_name_start = std::string("rl*") + my_template->name->sc.name + "*";

		for(;;)
		{
			const int new_id = rl_next_template_id( my_agent );
			std::string temp_id;
			to_string( new_id, temp_id );

			const std::string new_name = new_name_start + temp_id;

			if ( !find_sym_constant( my_agent, new_name.c_str() ) ) {
				new_name_symbol = make_sym_constant( my_agent, new_name.c_str() );
				break;
			}
		}
	}

 	condition *cond_top, *cond_bottom;

	{	// prep conditions
		copy_condition_list( my_agent, my_template_instance->top_of_instantiated_conditions, &cond_top, &cond_bottom );
		rl_add_goal_or_impasse_tests_to_conds( my_agent, cond_top );
		reset_variable_generator( my_agent, cond_top, NIL );
		my_agent->variablization_tc = get_new_tc_number( my_agent );
		variablize_condition_list( my_agent, cond_top );
		variablize_nots_and_insert_into_conditions( my_agent, my_template_instance->nots, cond_top );
	}

	Symbol *referent;
	action *new_action;

	{
		const action * const &my_action = my_template->action_list;

		// get the preference value
		Symbol * const id = instantiate_rhs_value( my_agent, my_action->id, -1, 's', tok, w );
		Symbol * const attr = instantiate_rhs_value( my_agent, my_action->attr, id->id.level, 'a', tok, w );
		const char first_letter = first_letter_from_symbol( attr );
		Symbol * const value = instantiate_rhs_value( my_agent, my_action->value, id->id.level, first_letter, tok, w );
		referent = instantiate_rhs_value( my_agent, my_action->referent, id->id.level, first_letter, tok, w );

		// clean up after yourself :)
		symbol_remove_ref( my_agent, id );
		symbol_remove_ref( my_agent, attr );
		symbol_remove_ref( my_agent, value );
		symbol_remove_ref( my_agent, referent );

		// make new action list
		new_action = rl_make_simple_action( my_agent, id, attr, value, referent );
		new_action->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;
	}

	// make new production
	production * const new_production = make_production( my_agent, USER_PRODUCTION_TYPE, new_name_symbol, &cond_top, &cond_bottom, &new_action, false );
	my_agent->variablize_this_chunk = chunk_var; // restored to original value

	// set initial expected reward values
	new_production->rl_efr = get_number_from_symbol( referent );

	// attempt to add to rete, remove if duplicate
	if ( add_production_to_rete( my_agent, new_production, cond_top, NULL, FALSE, TRUE ) == DUPLICATE_PRODUCTION )
	{
		excise_production( my_agent, new_production, false );
		rl_revert_template_id( my_agent );

		new_name_symbol = NULL;
	}

	deallocate_condition_list( my_agent, cond_top );

	return new_name_symbol;
}

inline void rl_MSA_make_value( agent * const &my_agent, char * &rhs_value, Symbol * symbol /* must be a copy? */)
{
	symbol_add_ref( symbol );
	variablize_symbol( my_agent, &symbol );
	rhs_value = symbol_to_rhs_value( symbol );
}

// creates an action for a template instantiation
action *rl_make_simple_action( agent *my_agent, Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym )
{
	action *rhs;

	allocate_with_pool( my_agent, &my_agent->action_pool, &rhs );
	rhs->next = NIL;
	rhs->type = MAKE_ACTION;

	rl_MSA_make_value( my_agent, rhs->id, id_sym ); //< id
	rl_MSA_make_value( my_agent, rhs->attr, attr_sym ); //< attribute
	rl_MSA_make_value( my_agent, rhs->value, val_sym ); //< value
	rl_MSA_make_value( my_agent, rhs->referent, ref_sym ); //< referent

	return rhs;
}

class RL_Add_Goal_or_Impasse_Test_To_Cond : public Visitor {
public:
	RL_Add_Goal_or_Impasse_Test_To_Cond(agent * const my_agent_) : my_agent(my_agent_), tc(get_new_tc_number(my_agent)) {}

	Instruction operator()(condition * const &cond) {
		if ( cond->type != POSITIVE_CONDITION )
			return Next;

		Symbol * const id = referent_of_equality_test( cond->data.tests.id_test );
		if ( ( !id->id.isa_goal && !id->id.isa_impasse ) || id->id.tc_num == tc )
			return Next;

		complex_test * ct;
		allocate_with_pool( my_agent, &my_agent->complex_test_pool, &ct );
		ct->type = static_cast<byte>( id->id.isa_goal ? GOAL_ID_TEST : IMPASSE_ID_TEST );
		const test t = make_test_from_complex_test( ct );
		add_new_test_to_test( my_agent, &cond->data.tests.id_test, t );

		// mark each id as we add a test for it, so we don't add a test for the same id in two different places
		id->id.tc_num = tc;

		return Next;
	}

private:
	agent * my_agent;
	tc_number tc;
};

void rl_add_goal_or_impasse_tests_to_conds( agent *my_agent, condition *all_conds )
{
	visit(RL_Add_Goal_or_Impasse_Test_To_Cond(my_agent), all_conds);
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

class RL_Tabulate_Reward_Value_For_WME : public Visitor {
public:
	RL_Tabulate_Reward_Value_For_WME(agent * const &my_agent_) : my_agent(my_agent_), reward(0.0) {}

	Instruction operator()(wme * const &w) {
		if(w->value->common.symbol_type != IDENTIFIER_SYMBOL_TYPE)
			return Next;

		const slot * const t = make_slot(my_agent, w->value, my_agent->rl_sym_value);
		if(!t)
			return Next;

		visit(*this, &RL_Tabulate_Reward_Value_For_WME::add_value, t->wmes);

		return Next;
	}

	const double & get_reward() const { return reward; }

private:
	Instruction add_value(wme * const &w) {
		if(w->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE || w->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
			reward += get_number_from_symbol(w->value);

		return Next;
	}

	agent * my_agent;
	double reward;
};

// gathers discounted reward for a state
void rl_tabulate_reward_value_for_goal( agent *my_agent, Symbol *goal )
{
	rl_data * const &data = goal->id.rl_info;

	if ( !data->prev_op_rl_rules->empty() )
	{
		const slot * const s = make_slot( my_agent, goal->id.reward_header, my_agent->rl_sym_reward );

		RL_Tabulate_Reward_Value_For_WME tabulator(my_agent);
		const double discount_rate = my_agent->rl_params->discount_rate->get_value();

		if ( s )
		{
			tabulator = visit(tabulator, s->wmes);

			data->reward += tabulator.get_reward() * pow( discount_rate, static_cast< double >( data->gap_age + data->hrl_age ) );
		}

		// update stats
		const double global_reward = my_agent->rl_stats->global_reward->get_value();
		my_agent->rl_stats->total_reward->set_value( tabulator.get_reward() );
		my_agent->rl_stats->global_reward->set_value( global_reward + tabulator.get_reward() );

		if ( goal != my_agent->bottom_goal && my_agent->rl_params->hrl_discount->get_value() == soar_module::on )
			++data->hrl_age;
	}
}

class Tabulate_Reward_Value : public Visitor {
public:
	Tabulate_Reward_Value(agent * const &my_agent_) : my_agent(my_agent_) {}

	Instruction operator()(Symbol * const goal) {
		rl_tabulate_reward_value_for_goal( my_agent, goal );

		return Next;
	}

private:
	agent * my_agent;
};

// gathers reward for all states
void rl_tabulate_reward_values( agent *my_agent )
{
	visit(Tabulate_Reward_Value(my_agent), my_agent->top_goal);
}

class RL_List_Just_First_Prods : public Visitor {
public:
	RL_List_Just_First_Prods(rl_data * const &data_, preference * const &cand) : data(data_), op(cand->value), just_fired(false) {}

	Instruction operator()(const preference * const &pref) {
		if(op != pref->value || !pref->inst->prod->rl_rule)
			return Next;

		if(!just_fired) {
			data->prev_op_rl_rules->clear();
			just_fired = true;
		}

		data->prev_op_rl_rules->push_back(pref->inst->prod);

		return Next;
	}

	const bool & prods_just_fired() const { return just_fired; }

private:
	rl_data * data;
	const Symbol * op;
	bool just_fired;
};

// stores rl info for a state w.r.t. a selected operator
void rl_store_data( agent *my_agent, Symbol *goal, preference *cand )
{
	rl_data * const &data = goal->id.rl_info;
	const bool using_gaps = my_agent->rl_params->temporal_extension->get_value() == soar_module::on;

	// Make list of just-fired prods
	RL_List_Just_First_Prods lister(data, cand);
	lister = visit(lister, goal->id.operator_slot->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]);

	if ( lister.prods_just_fired() )
	{
		data->previous_q = cand->numeric_value;
	}
	else if ( !using_gaps )
	{
		data->prev_op_rl_rules->clear();
		data->previous_q = cand->numeric_value;
	}
	else if ( !data->prev_op_rl_rules->empty() )
	{
		if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] && data->gap_age == 0 )
		{
			char buf[256];
			SNPRINTF( buf, 254, "gap started (%c%llu)", goal->id.name_letter, goal->id.name_number );

			print( my_agent, buf );
			xml_generate_warning( my_agent, buf );
		}

		++data->gap_age;
	}
}

class RL_ET_Updater : public Visitor {
public:
	RL_ET_Updater(agent * my_agent_, rl_data * const &data_, const double &op_value_, const bool &update_efr_)
		: my_agent(my_agent_),
		data(data_),
		op_value(op_value_),
		update_efr(update_efr_),
		alpha(my_agent->rl_params->learning_rate->get_value()),
		lambda(my_agent->rl_params->et_decay_rate->get_value()),
		gamma(my_agent->rl_params->discount_rate->get_value()),
		tolerance(my_agent->rl_params->et_tolerance->get_value()),
		discount(pow(gamma, data->gap_age + data->hrl_age + 1.0)),
		trace_increment(data->prev_op_rl_rules->empty() ? 0.0 : 1.0 / data->prev_op_rl_rules->size())
	{
	}

	class Preference_Updater : public Visitor_Instantiation {
	public:
		Preference_Updater(agent * const &my_agent_, const double &new_combined_) : my_agent(my_agent_), new_combined(new_combined_) {}

		Instruction operator()(const instantiation * const &inst) {
			visit(*this, &Preference_Updater::preference, inst->preferences_generated);

			return Next;
		}

	private:
		Instruction preference(preference * const &pref) {
			symbol_remove_ref(my_agent, pref->referent);
			pref->referent = make_float_constant(my_agent, new_combined);

			return Next;
		}

		agent * my_agent;
		double new_combined;
	};

	Instruction operator()(rl_et_map::iterator &iter) {
		production * const &prod = iter->first;

		// get old vals
		const double old_combined = get_number_from_symbol( rhs_value_to_symbol( prod->action_list->referent ) );
		const double old_ecr = prod->rl_ecr;
		const double old_efr = prod->rl_efr;

		// calculate updates
		const double delta_ecr = alpha * iter->second * ( data->reward - old_ecr );
		const double delta_efr = update_efr ? alpha * iter->second * ( discount * op_value - old_efr ) : 0.0;

		// calculate new vals
		const double new_ecr = old_ecr + delta_ecr;
		const double new_efr = old_efr + delta_efr;
		const double new_combined = new_ecr + new_efr;

		// print as necessary
		if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] )
		{
			std::string msg = std::string("updating RL rule ") +  prod->name->sc.name + " from (";
			std::string temp_str;

			// old ecr
			to_string( old_ecr, temp_str );
			msg.append( temp_str );

			// old efr
			to_string( old_efr, temp_str );
			msg.append( ", " );
			msg.append( temp_str );

			// old combined
			to_string( old_combined, temp_str );
			msg.append( ", " );
			msg.append( temp_str );

			msg.append( ") to (" );

			// new ecr
			to_string( new_ecr, temp_str );
			msg.append( temp_str );

			// new efr
			to_string( new_efr, temp_str );
			msg.append( ", " );
			msg.append( temp_str );

			// new combined
			to_string( new_combined, temp_str );
			msg.append( ", " );
			msg.append( temp_str );
			msg.append( ")" );

			print( my_agent, const_cast<char *>( msg.c_str() ) );
			xml_generate_message( my_agent, const_cast<char *>( msg.c_str() ) );
		}

		// Change value of rule
		symbol_remove_ref( my_agent, rhs_value_to_symbol( prod->action_list->referent ) );
		prod->action_list->referent = symbol_to_rhs_value( make_float_constant( my_agent, new_combined ) );
		prod->rl_update_count += 1;
		prod->rl_ecr = new_ecr;
		prod->rl_efr = new_efr;

		// Change value of preferences generated by current instantiations of this rule
		if ( prod->instantiations )
			visit(Preference_Updater(my_agent, new_combined), prod->instantiations);

		return Next;
	}

	Instruction just_fired_only(rl_rule_list::iterator &p) {
		if(!*p)
			return Next;

		rl_et_map::iterator iter = data->eligibility_traces->find(*p);

		if (iter != data->eligibility_traces->end())
			iter->second += trace_increment;
		else
			(*data->eligibility_traces)[*p] = trace_increment;

		return Next;
	}

	Instruction decay(rl_et_map::iterator &iter) {
		iter->second *= lambda;
		iter->second *= discount;

		if(iter->second >= tolerance)
			return Next;

		data->eligibility_traces->erase(iter++);
		return Repeat;
	}

	const double & get_lambda() const { return lambda; }

private:
	agent * my_agent;
	rl_data * data;
	double op_value;
	bool update_efr;

	double alpha;
	double lambda;
	double gamma;
	double tolerance;
	double discount;

	double trace_increment;
};

// performs the rl update at a state
void rl_perform_update( agent *my_agent, double op_value, bool op_rl, Symbol *goal, bool update_efr )
{
	const bool using_gaps = my_agent->rl_params->temporal_extension->get_value() == soar_module::on;

	if ( using_gaps && !op_rl )
		return;

	rl_data * const &data = goal->id.rl_info;
	
	if ( !data->prev_op_rl_rules->empty() )
	{
		RL_ET_Updater updater(my_agent, data, op_value, update_efr);

		// notify of gap closure
		if ( data->gap_age && using_gaps && my_agent->sysparams[ TRACE_RL_SYSPARAM ] )
		{
			char buf[256];
			SNPRINTF( buf, 254, "gap ended (%c%llu)", goal->id.name_letter, goal->id.name_number );

			print( my_agent, buf );
			xml_generate_warning( my_agent, buf );
		}

		// Iterate through eligibility_traces, decay traces. If less than TOLERANCE, remove from map.
		if(updater.get_lambda() == 0.0)
			data->eligibility_traces->clear();
		else
			visit(updater, &RL_ET_Updater::decay, data->eligibility_traces->begin(), data->eligibility_traces->end());

		// Update trace for just fired prods
		if(!data->prev_op_rl_rules->empty())
			visit(updater, &RL_ET_Updater::just_fired_only, data->prev_op_rl_rules->begin(), data->prev_op_rl_rules->end());

		// For each prod with a trace, perform update
		visit(updater, data->eligibility_traces->begin(), data->eligibility_traces->end());
	}

	data->gap_age = 0;
	data->hrl_age = 0;
	data->reward = 0.0;
}

// clears eligibility traces 
void rl_watkins_clear( agent * /*my_agent*/, Symbol *goal )
{
	goal->id.rl_info->eligibility_traces->clear();
}
