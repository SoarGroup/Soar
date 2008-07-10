/*
 * TODO:
 * generate feeling frames
 * create commands
 *   enable/disable various appraisals
 *   set mood parameters
 *   turn entire system on/off
 * refactor headers so CLI and KernelSML aren't dependent on boost
 * fix BADBADs
 * fix memory leaks
 */

#include "agent.h"
#include "symtab.h"
#include "wmem.h"

void register_appraisal(agent* thisAgent, wme* appraisal)
{
	if(appraisal->attr->sc.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE)
	{
		if(appraisal->id != thisAgent->currentEmotion.id_sym) {
			thisAgent->currentEmotion.Reset(appraisal->id, thisAgent->currentEmotion.outcome_probability);
		}

		string result = thisAgent->currentEmotion.SetAppraisalValue(appraisal->attr->sc.name, appraisal->value);
	}

}

void get_appraisals(agent* thisAgent)
{
	if(!thisAgent->emotion_header_appraisal) return;

	slot* frame_slot = thisAgent->emotion_header_appraisal->id.slots;
	slot* appraisal_slot;
	wme *frame, *appraisal;

	if ( frame_slot )
	{
		for ( ; frame_slot; frame_slot = frame_slot->next )
		{
			if(    frame_slot->attr->sc.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE
				&& !strcmp(frame_slot->attr->sc.name, "frame")) /* BADBAD: should store "frame" symbol in common symbols so can do direct comparison */
			{
				for ( frame = frame_slot->wmes ; frame; frame = frame->next)
				{
					if (frame->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
					{
						for ( appraisal_slot = frame->value->id.slots; appraisal_slot; appraisal_slot = appraisal_slot->next )
						{
							for ( appraisal = appraisal_slot->wmes; appraisal; appraisal = appraisal->next )
							{
								register_appraisal(thisAgent, appraisal);
							}
						}
					}
				}
			}
		}
	}
}

void update_mood(agent* thisAgent)
{
	thisAgent->currentMood.Decay();
	thisAgent->currentMood.MoveTowardEmotion(thisAgent->currentEmotion);
}

// BADBAD: should have pre-made Symbols for all of these attributes
// Shouldn't have to pass in status, mood, and emotion -- those should be directly available to the called function
void generate_feeling_frame(agent* thisAgent)
{
	// clear previous feeling frame (stored on agent structure)
	if(thisAgent->feeling_frame) { remove_input_wme(thisAgent, thisAgent->feeling_frame); }

	// generate new frame
	thisAgent->feeling_frame = add_input_wme(thisAgent, thisAgent->emotion_header_feeling, make_sym_constant(thisAgent, "frame"), make_new_identifier(thisAgent, 'F', TOP_GOAL_LEVEL));

	// generate new values for each appraisal
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "suddenness"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("suddenness", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("suddenness"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "unpredictability"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("unpredictability", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("unpredictability"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "intrinsic-pleasantness"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("intrinsic-pleasantness", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("intrinsic-pleasantness"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "goal-relevance"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("goal-relevance", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("goal-relevance"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "outcome-probability"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("outcome-probability", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("outcome-probability"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "discrepancy"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("discrepancy", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("discrepancy"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "conduciveness"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("conduciveness", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("conduciveness"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "control"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("control", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("control"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "power"), make_float_constant(thisAgent, 
		thisAgent->currentFeeling.GetNumericDimension("power", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("power"))));
	
	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "causal-agent"), make_sym_constant(thisAgent, 
		thisAgent->currentFeeling.GetCategoricalDimension("causal-agent", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("causal-agent")).c_str()));

	add_input_wme(thisAgent, thisAgent->feeling_frame->value, make_sym_constant(thisAgent, "causal-motive"), make_sym_constant(thisAgent, 
		thisAgent->currentFeeling.GetCategoricalDimension("causal-motive", thisAgent->currentEmotion, thisAgent->currentMood.af, thisAgent->appraisalStatus.GetStatus("causal-motive")).c_str()));

	// create feeling intensity, valence
}