#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include "soar_rand.h"
#include "decide.h"

#include "decision_manipulation.h"
#include "misc.h"

/***************************************************************************
 * Function     : init_select
 **************************************************************************/
void init_select( agent *my_agent )
{
	my_agent->select->select_enabled = false;
	my_agent->select->select_operator.clear();
}

/***************************************************************************
 * Function     : select_next_operator
 **************************************************************************/
void select_next_operator( agent *my_agent, const char *operator_id )
{
	init_select( my_agent );
	
	my_agent->select->select_enabled = true;
	my_agent->select->select_operator = operator_id;
}

/***************************************************************************
 * Function     : get_selected_operator
 **************************************************************************/
const char *get_selected_operator( agent *my_agent )
{
	if ( !my_agent->select->select_enabled )
		return NULL;

	return my_agent->select->select_operator.c_str();
}

/***************************************************************************
 * Function     : force_selection
 **************************************************************************/
preference *force_selection( agent *my_agent, preference *candidates, bool reinit )
{
	preference *return_val = NULL;
	preference *cand = candidates;
	std::string temp;
	std::string *temp2;

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
				temp2 = to_string( cand->value->id.name_number );
				temp += (*temp2);
				delete temp2;

				if ( !my_agent->select->select_operator.compare( temp ) )
					return_val = cand;
			}
			
			cand = cand->next;
		}

		if ( reinit )
			init_select( my_agent );
	}

	return return_val;
}

/***************************************************************************
 * Function     : init_predict
 **************************************************************************/
void init_predict( agent *my_agent )
{
	my_agent->predict_seed = 0;
	(*my_agent->prediction) = "";
}

/***************************************************************************
 * Function     : srand_store_snapshot
 **************************************************************************/
void srand_store_snapshot( agent *my_agent )
{
	unsigned long storage_val = 0;

	while ( !storage_val )
		storage_val = SoarRandInt();

	my_agent->predict_seed = storage_val;
}

/***************************************************************************
 * Function     : srand_restore_snapshot
 **************************************************************************/
void srand_restore_snapshot( agent *my_agent, bool clear_snapshot )
{
	if ( my_agent->predict_seed )
		SoarSeedRNG( my_agent->predict_seed );

	if ( clear_snapshot )
		init_predict( my_agent );
}

/***************************************************************************
 * Function     : set_prediction
 **************************************************************************/
void set_prediction( agent *my_agent, const char *prediction)
{
	(*my_agent->prediction) = prediction;
}

/***************************************************************************
 * Function     : get_prediction
 **************************************************************************/
const char *get_prediction( agent *my_agent )
{
	srand_store_snapshot( my_agent );
	do_decision_phase( my_agent, true );

	return my_agent->prediction->c_str();
}