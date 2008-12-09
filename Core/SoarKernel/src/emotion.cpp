#include <portability.h>

/*
 * TODO:
 * create commands
 *   enable/disable various appraisals
 *   set mood parameters
 *   turn entire system on/off
 * print current emotion/mood/feeling table
 * refactor headers so CLI and KernelSML aren't dependent on boost
 * fix BADBADs
 * use strings for invalid/none/error values in wm
 * use Symbols internally instead of raw types?  May allow appraisals to be treated more uniformly
 * consider replacing if/elseif blocks with map lookup? (probably requires Symbols since types aren't uniform otherwise)
 * can drop lexical_cast and other functions?
 * prefix all functions with "emotion"?
 * emotion_reset should clean up stuff currently handled in do_input_phase, and be called from there
 * consider generating feeling frame all at once, instead of one dimension at a time. Also, have feeling generation directly access emotion/mood from agent, instead of being passed in.
 */

// TODO:
// -- change appraisal registration to detect when appraisal has already been registered for that frame (and print a warning)
// -- Figure out how to get highest ela/joy when one step away from goal (must involve changing appraisals)
// -- Should dimensions with no values report a default value (e.g. 0) or should they report no value (e.g. fInvalidValue)?
// ---- currently reporting no value
// -- Should some dimensions (e.g. discrepancy & outcome_probability) not boost and decay? (e.g. something shouldn't seem more likely just because something else was likely recently)

#include "emotion.h"
#include "agent.h"
#include "symtab.h"
#include "wmem.h"
#include "io_soar.h"
#include "boost/foreach.hpp"
#include <list>

#include "AppraisalStatus.h"
#include "Mood.h"
#include "Feeling.h"

void emotion_clear_feeling_frame(agent* thisAgent, emotion_data* ed)
{
	if(!ed->feeling_frame) return;
	for(wme* w = ed->feeling_frame->value->id.input_wmes; w!=NIL; w=w->next)
	{
		release_io_symbol(thisAgent, w->id);  // Not sure why I have to do this explicitly
//		remove_input_wme(thisAgent, w);
	}
	remove_input_wme(thisAgent, ed->feeling_frame);
}

void cleanup_emotion_data(agent* thisAgent, emotion_data* ed)
{
	emotion_clear_feeling_frame(thisAgent, ed);
	delete ed->appraisalStatus; //BUGBUG: should use custom new/delete so memory usage is reported in a category
	delete ed->currentEmotion;
	delete ed->currentFeeling;
	delete ed->currentMood;
}

void register_appraisal(emotion_data* ed, wme* appraisal)
{
	if(appraisal->attr->sc.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE)
	{
		if(appraisal->id != ed->currentEmotion->id_sym) {
			ed->currentEmotion->Reset(appraisal->id, ed->currentEmotion->outcome_probability);
		}

		string result = ed->currentEmotion->SetAppraisalValue(appraisal->attr->sc.name, appraisal->value);
	}

}

void get_appraisals(Symbol* goal)
{
	if(!goal->id.emotion_header_appraisal) return;

	slot* frame_slot = goal->id.emotion_header_appraisal->id.slots;
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
								register_appraisal(goal->id.emotion_info, appraisal);
							}
						}
					}
				}
			}
		}
	}
}

void update_mood(Symbol* goal)
{
	emotion_data* ed = goal->id.emotion_info;
	ed->currentMood->Decay();
	ed->currentMood->MoveTowardEmotion(ed->currentEmotion);
}


inline void generate_feeling_appraisal_numeric(agent* thisAgent, const char* const name, emotion_data* ed)
{
	Symbol* tempAtt;
	Symbol* tempVal;

	tempAtt = make_sym_constant(thisAgent, name);
	tempVal = make_float_constant(thisAgent, ed->currentFeeling->GetNumericDimension(name, ed->currentEmotion, ed->currentMood->af, ed->appraisalStatus->GetStatus(name)));
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);
}

inline void generate_feeling_appraisal_categorical(agent* thisAgent, const char* const name, emotion_data* ed)
{
	Symbol* tempAtt;
	Symbol* tempVal;

	tempAtt = make_sym_constant(thisAgent, name);
	tempVal = make_sym_constant(thisAgent, ed->currentFeeling->GetCategoricalDimension(name, ed->currentEmotion, ed->currentMood->af, ed->appraisalStatus->GetStatus(name)).c_str());
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);
}

// BADBAD: should have pre-made Symbols for all of these attributes
// Shouldn't have to pass in status, mood, and emotion -- those should be directly available to the called function
void generate_feeling_frame(agent* thisAgent, Symbol * goal)
{
	emotion_data* ed = goal->id.emotion_info;

	// clear previous feeling frame (stored on agent structure)
	if(ed->feeling_frame) { emotion_clear_feeling_frame(thisAgent, ed); }

	// generate new frame
	Symbol* frame_att = make_sym_constant(thisAgent, "frame");
	ed->feeling_frame = add_input_wme(thisAgent, goal->id.emotion_header_feeling, frame_att, make_new_identifier(thisAgent, 'F', goal->id.level));
	symbol_remove_ref(thisAgent, frame_att);

	// TODO: can't all of this repeated stuff below be made into a function?

	generate_feeling_appraisal_numeric(thisAgent, "suddenness", ed);
	generate_feeling_appraisal_numeric(thisAgent, "unpredictability", ed);
	generate_feeling_appraisal_numeric(thisAgent, "intrinsic-pleasantness", ed);
	generate_feeling_appraisal_numeric(thisAgent, "goal-relevance", ed);
	generate_feeling_appraisal_numeric(thisAgent, "outcome-probability", ed);
	generate_feeling_appraisal_numeric(thisAgent, "discrepancy", ed);
	generate_feeling_appraisal_numeric(thisAgent, "conduciveness", ed);
	generate_feeling_appraisal_numeric(thisAgent, "control", ed);
	generate_feeling_appraisal_numeric(thisAgent, "power", ed);
	generate_feeling_appraisal_categorical(thisAgent, "causal-agent", ed);
	generate_feeling_appraisal_categorical(thisAgent, "causal-motive", ed);


	// create feeling intensity, valence
	Symbol* tempAtt;
	Symbol* tempVal;

	tempAtt = make_sym_constant(thisAgent, "intensity");
	tempVal = make_float_constant(thisAgent, ed->currentFeeling->af.CalculateIntensity());
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);

	tempAtt = make_sym_constant(thisAgent, "valence");
	tempVal = make_float_constant(thisAgent, ed->currentFeeling->af.CalculateValence());
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);

	//TODO: reward
}

void emotion_reset_data( agent *thisAgent )
{
	for(Symbol* goal = thisAgent->top_goal; goal; goal=goal->id.lower_goal)
	{
		emotion_clear_feeling_frame(thisAgent, goal->id.emotion_info);
	}
}

void emotion_update(agent* thisAgent, Symbol* goal)
{
	get_appraisals(goal);
	update_mood(goal);
	generate_feeling_frame(thisAgent, goal);
}