#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  decision_manipulation.cpp
 *
 * =======================================================================
 * Description  :  Predict/Select functionality
 * =======================================================================
 */

#include "decision_manipulation.h"

#include "agent.h"
#include "soar_rand.h"

#include "decide.h"
#include "misc.h"

/***************************************************************************
 * Function     : select_init
 **************************************************************************/
void select_init( agent *my_agent )
{
	my_agent->select->select_enabled = false;
	my_agent->select->select_operator.clear();
}

/***************************************************************************
 * Function     : select_next_operator
 **************************************************************************/
void select_next_operator( agent *my_agent, const char *operator_id )
{
	select_init( my_agent );
	std::string& op = my_agent->select->select_operator;
	
	my_agent->select->select_enabled = true;
	op.assign(operator_id);
	
	assert( !op.empty() );

	// lazy users may use a lower-case letter
	std::string::iterator iter = op.begin();
	*iter = static_cast< char >( toupper( *iter ) );
}

/***************************************************************************
 * Function     : select_get_operator
 **************************************************************************/
const char *select_get_operator( agent *my_agent )
{
	if ( !my_agent->select->select_enabled )
		return NULL;

	return my_agent->select->select_operator.c_str();
}

/***************************************************************************
 * Function     : select_force
 **************************************************************************/
preference *select_force( agent *my_agent, preference *candidates, bool reinit )
{
	preference *return_val = NULL;
	preference *cand = candidates;
	std::string temp;

	if ( my_agent->select->select_enabled )
	{
		// go through the list till we find a match or done
		while ( cand && !return_val )
		{
			if ( cand->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
			{
				// clear comparison string
				temp = "";

				// get first letter of comparison string
				temp += cand->value->id.name_letter;

				// get number of comparison string
				std::string temp2;
				to_string( cand->value->id.name_number, temp2 );
				temp += temp2;

				if ( !my_agent->select->select_operator.compare( temp ) )
					return_val = cand;
			}
			
			cand = cand->next;
		}

		if ( return_val && reinit )
			select_init( my_agent );
	}

	return return_val;
}

/***************************************************************************
 * Function     : predict_init
 **************************************************************************/
void predict_init( agent *my_agent )
{
	my_agent->predict_seed = 0;
	(*my_agent->prediction) = "";
}

/***************************************************************************
 * Function     : predict_srand_store_snapshot
 **************************************************************************/
void predict_srand_store_snapshot( agent *my_agent )
{
	unsigned long storage_val = 0;

	while ( !storage_val )
		storage_val = SoarRandInt();

	my_agent->predict_seed = storage_val;
}

/***************************************************************************
 * Function     : predict_srand_restore_snapshot
 **************************************************************************/
void predict_srand_restore_snapshot( agent *my_agent, bool clear_snapshot )
{
	if ( my_agent->predict_seed )
		SoarSeedRNG( my_agent->predict_seed );

	if ( clear_snapshot )
		predict_init( my_agent );
}

/***************************************************************************
 * Function     : predict_set
 **************************************************************************/
void predict_set( agent *my_agent, const char *prediction)
{
	(*my_agent->prediction) = prediction;
}

/***************************************************************************
 * Function     : predict_get
 **************************************************************************/
const char *predict_get( agent *my_agent )
{
	predict_srand_store_snapshot( my_agent );
	do_decision_phase( my_agent, true );

	return my_agent->prediction->c_str();
}
