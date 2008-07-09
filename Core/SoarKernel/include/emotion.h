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