#include <assert.h>
#include <math.h>
#include "soarkernel.h"
#include "emotion.h"

typedef enum appraisal_variable_type {
    DESIRABILITY = 0,
    LIKELIHOOD = 1
} appraisal_variable_type;

typedef struct appraisal_variable_info {
    appraisal_variable_type type;
    float value;
    unsigned long timetag;
} appraisal_variable_info;

bool emotion_appraisals_have_changed();
bool emotion_get_appraisal_variable_timetag(wme * appraisal_variable_wme, unsigned long * timetag);
bool emotion_get_appraisal_frame_timetag(slot * appraisal_frame_slot, unsigned long * timetag);
void emotion_get_appraisals();
void emotion_aggregate_variables();
void emotion_update_emotion();

void emotion_add_appraisal_to_list(appraisal_frame ** af_list, appraisal_frame * af);
void emotion_add_value_to_list(struct appraisal_variable_value ** avv, 
                               appraisal_variable_info * info);

void emotion_clear_appraisal_list(appraisal_frame ** af);
void emotion_clear_list(struct appraisal_variable_value ** avv);

bool emotion_get_appraisal_frame(struct slot_struct * appraisal_variable_slot, appraisal_frame ** af);
bool emotion_get_appraisal_variable_info(wme * appraisal_variable_wme, appraisal_variable_info ** info);


/*const int emotion_NUM_TYPES = 2;*/
#define emotion_NUM_TYPES 2
static const char * emotion_LABELS[] = {"desirability-agg","likelihood-agg"};
static const float emotion_NO_VALUE_FLOAT = -100.0;

void emotion_update() {

    /* if the e-link doesn't exist (i.e. it's been destructed), then don't do the update */
    if(current_agent(io_header_emotion) != NIL) {

        bool update_emotion;
        update_emotion = FALSE;

        /* get all the appraisals off of the e-link
           determine if any of them have changed and thus require the aggregates and emotion
           to be recomputed)
        */
        update_emotion = emotion_appraisals_have_changed();
        /*update_emotion = TRUE;*/

        if(update_emotion) {
            /* get the appraisals */
            emotion_get_appraisals();
            /* aggregate the values of the different variables */
            emotion_aggregate_variables();
            /* generate a new emotion if any of the appraisals have changed */
            emotion_update_emotion();
        }
    }
}

void emotion_update_aggregate(enum appraisal_variable_type type) {
    Symbol *wme_attr, *wme_value;
    float value;

    value = current_agent(emotions)->aggs[type];

    if(current_agent(emotions)->agg_wmes[type]) {        
        bool success = remove_input_wme(current_agent(emotions)->agg_wmes[type]);
        if(!success) print("Emotion: tried to remove aggregate value");
        assert(success);

        current_agent(emotions)->agg_wmes[type] = NIL;
    }

    if(value != emotion_NO_VALUE_FLOAT) {

        wme_attr = get_io_sym_constant ((char*)emotion_LABELS[type]);
        wme_value = get_io_float_constant (value);
    
        current_agent(emotions)->agg_wmes[type] =
            add_input_wme (current_agent(io_header_emotion),wme_attr,wme_value);
    }
}

//TODO: add excitement for when desirability == 0 (should intensity == likelihood then?)
void emotion_update_emotion() {
    Symbol *wme_emotion_name_attr, *wme_emotion_name_value,
        *wme_emotion_intensity_attr, *wme_emotion_intensity_value;

    float desirability, likelihood;

    desirability = current_agent(emotions)->aggs[DESIRABILITY];
    likelihood = current_agent(emotions)->aggs[LIKELIHOOD];

    if(current_agent(emotions)->emotion_wme) {
        bool success = remove_input_wme(current_agent(emotions)->emotion_wme);
        if(!success) print("Emotion: tried to remove emotion");
        assert(success);
        release_io_symbol (current_agent(emotions)->emotion_symbol);
        current_agent(emotions)->emotion_wme = NIL;
    }

    if(desirability != emotion_NO_VALUE_FLOAT &&
        likelihood != emotion_NO_VALUE_FLOAT) {

        if(desirability >= 0 && likelihood < 1.0) {
            wme_emotion_name_value = get_io_sym_constant ("hope");
        } else if (desirability >= 0 && likelihood == 1.0) {
            wme_emotion_name_value = get_io_sym_constant ("joy");
        } else if (desirability < 0 && likelihood < 1.0) {
            wme_emotion_name_value = get_io_sym_constant ("fear");
        } else if (desirability < 0 && likelihood == 1.0) {
            wme_emotion_name_value = get_io_sym_constant ("dismay");
        }
        else { return; }
     
        wme_emotion_name_attr = get_io_sym_constant ("name");
        wme_emotion_intensity_attr = get_io_sym_constant ("intensity");

        wme_emotion_intensity_value = get_io_float_constant((float)fabs(desirability)*likelihood);

        current_agent(emotions)->emotion_symbol = get_new_io_identifier('E');
        current_agent(emotions)->emotion_wme = add_input_wme (current_agent(io_header_emotion),
		       make_sym_constant("emotion"),
		       current_agent(emotions)->emotion_symbol);

        add_input_wme (current_agent(emotions)->emotion_symbol,wme_emotion_name_attr,wme_emotion_name_value);
        add_input_wme (current_agent(emotions)->emotion_symbol,wme_emotion_intensity_attr,wme_emotion_intensity_value);

		//add_input_wme (current_agent(io_header_input),wme_emotion_name_attr,wme_emotion_name_value);
        //add_input_wme (current_agent(io_header_input),wme_emotion_intensity_attr,wme_emotion_intensity_value);

    }
    
}

//TODO: check for correct substructure
void emotion_get_appraisals() {

    /*struct wme_struct * appraisal_variable_wme;*/
    union symbol_union * e_link;
    struct slot_struct * appraisal_variable_slot;
    struct slot_struct * appraisal_frame_slot;
    /*appraisal_variable_info * info;*/
    appraisal_variable_type i;
    appraisal_frame * cur_appraisal_frame;
    unsigned long num_appraisals;

    num_appraisals = 0;

    if(current_agent(emotions) == NIL) {
        emotion_init(&(current_agent(emotions)));
    }

    /* get the emotions link */
    e_link = current_agent(io_header_emotion);

    //appraisal_variable_slot = current_agent(io_header_emotion)->id.slots;
    appraisal_frame_slot = current_agent(io_header_emotion)->id.slots;

    /* we'll allocate this once and reuse it several times */
    /*info = allocate_memory_and_zerofill (sizeof(appraisal_variable_info), 0);*/
    cur_appraisal_frame = allocate_memory_and_zerofill (sizeof(appraisal_frame), 0);

    /* clear the existing lists */
    for(i = 0; i < emotion_NUM_TYPES; i++) {
        emotion_clear_list(&current_agent(emotions)->variables[i]);
    }

    emotion_clear_appraisal_list(&current_agent(emotions)->appraisal_frames);

    while(appraisal_frame_slot) {

        bool valid;

        if(appraisal_frame_slot->wmes && !strcmp(appraisal_frame_slot->attr->sc.name, "appraisal")) {

            appraisal_variable_slot = appraisal_frame_slot->wmes->value->id.slots;

            valid = emotion_get_appraisal_frame(appraisal_variable_slot, &cur_appraisal_frame);

            if(valid) {
                emotion_add_appraisal_to_list(&current_agent(emotions)->appraisal_frames, cur_appraisal_frame);
                num_appraisals++;

                /* TODO: should update timetag here, but then will need to store timetag in appraisal_frame or create appraisal_frame info type */
            }
        }

        appraisal_frame_slot = appraisal_frame_slot->next;
    }

    current_agent(emotions)->num_appraisals = num_appraisals;
}


bool emotion_appraisals_have_changed() {

    /*struct wme_struct * appraisal_variable_wme;*/
    union symbol_union * e_link;
    /*struct slot_struct * appraisal_variable_slot;*/
    struct slot_struct * appraisal_frame_slot;
    bool changed;
    unsigned long num_appraisals;

    num_appraisals = 0;
    changed = FALSE;

    if(current_agent(emotions) == NIL) {
        emotion_init(&(current_agent(emotions)));
    }

    /* get the emotions link */
    e_link = current_agent(io_header_emotion);

    appraisal_frame_slot = current_agent(io_header_emotion)->id.slots;

    /* BUGBUG: if a rule is written which modifies an existing appraisal, this may not detect that */
    while(appraisal_frame_slot) {

        bool valid;
        unsigned long timetag;

        valid = emotion_get_appraisal_frame_timetag(appraisal_frame_slot, &timetag);

        if(valid) {
            num_appraisals++;
            if(timetag > current_agent(emotions)->latest_timetag) {
                changed = TRUE;
                break;
            }
        }

        /*
        appraisal_variable_wme = appraisal_frame_slot->wmes;

        while(appraisal_variable_wme) {
            bool valid;
            unsigned long timetag;
            valid = emotion_get_appraisal_frame_timetag(appraisal_variable_wme, &timetag);
            if(valid) {
                num_appraisals++;
                if(timetag > current_agent(emotions)->latest_timetag) {
                    changed = TRUE;
                    break;
                }
            }

            appraisal_variable_wme = appraisal_variable_wme->next;
        }*/

        if(changed) { break; }

        appraisal_frame_slot = appraisal_frame_slot->next;
    }

    /* if there were no appraisals at all
       and there is a current emotion,
       then things must have changed
       */
    if(!changed &&
        ( ( num_appraisals != current_agent(emotions)->num_appraisals ) ||
        ( num_appraisals==0 && current_agent(emotions)->emotion_wme != NIL) ) ) {

        changed = TRUE;
    }

    return changed;
}

bool emotion_get_appraisal_frame(struct slot_struct * appraisal_variable_slot, appraisal_frame ** af) {

    struct wme_struct * appraisal_variable_wme;
    appraisal_variable_info * info;

    /* we'll allocate this once and reuse it several times */
    info = allocate_memory_and_zerofill (sizeof(appraisal_variable_info), 0);

    while(appraisal_variable_slot) {

        appraisal_variable_wme = appraisal_variable_slot->wmes;

        while(appraisal_variable_wme) {
            bool valid;

            valid = emotion_get_appraisal_variable_info(appraisal_variable_wme, &info);
            if(valid) {
                emotion_add_value_to_list(&current_agent(emotions)->variables[info->type], info);
                (*af)->variables[info->type] = info->value;

                /* update the latest timetag */
                if(info->timetag > current_agent(emotions)->latest_timetag) {
                    current_agent(emotions)->latest_timetag = info->timetag;
                }
            }
    
            appraisal_variable_wme = appraisal_variable_wme->next;
        }

        appraisal_variable_slot = appraisal_variable_slot->next;
    }

    free_memory(info,0);

    // TODO: check for actual valid appraisal frame (i.e. both variables were set; need new array of bools to check? -- maybe it can be in the appraisal frame info type)
    return TRUE;
}

bool emotion_get_appraisal_variable_info(wme * appraisal_variable_wme, appraisal_variable_info ** info) {

    wme * value_wme;
    appraisal_variable_type type;
    variable appraisal_variable_wme_attr_var;
    slot * appraisal_variable_wme_slot;
    unsigned long timetag;
    float value;
    
    value = 0.0;

    appraisal_variable_wme_attr_var = appraisal_variable_wme->attr->var;

    /* check to see if this is the right kind of wme */
    if(appraisal_variable_wme_attr_var.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {
        if(!strcmp(appraisal_variable_wme_attr_var.name,"desirability")) {
            type = DESIRABILITY;
        } else if (!strcmp(appraisal_variable_wme_attr_var.name,"likelihood")) {
            type = LIKELIHOOD;
        } else { return FALSE; }
    }
    else { return FALSE; }

    //TODO: check under what circumstances this is necessary (i.e. if an elaboration rule removes something from memory?)
    if(appraisal_variable_wme->value->id.slots == NULL) { return FALSE; }

    appraisal_variable_wme_slot = appraisal_variable_wme->value->id.slots;
    value_wme = appraisal_variable_wme_slot->wmes;

    /* look for a WME with attribute = "value" and a numerical value */
    while(value_wme) {
        if(!strcmp(value_wme->attr->var.name,"value")) {
            if(value_wme->value->var.common_symbol_info.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
                value = value_wme->value->fc.value;
                break;
            } else if(value_wme->value->var.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
                value = (float)value_wme->value->ic.value;
                break;
            } else { return FALSE; }
        }

        /* get the next wme, which will be in the next slot */
        if(appraisal_variable_wme_slot->next) {
            appraisal_variable_wme_slot = appraisal_variable_wme_slot->next;
            value_wme = appraisal_variable_wme_slot->wmes;
        }
        else {
            value_wme = NIL;
            break;
        }
    }

    if(!value_wme) { return FALSE; }

    timetag = value_wme->timetag;
    
    (*info)->type = type;
    (*info)->value = value;
    (*info)->timetag = timetag;

    return TRUE;
}   

bool emotion_get_appraisal_frame_timetag(slot * appraisal_frame_slot, unsigned long * timetag) {
    wme * appraisal_variable_wme;
    unsigned long cur_timetag;
    unsigned long max_timetag;

    max_timetag = 0;

    appraisal_variable_wme = appraisal_frame_slot->wmes;

    while(appraisal_variable_wme) {
        bool valid;

        valid = emotion_get_appraisal_variable_timetag(appraisal_variable_wme, &cur_timetag);
        
        if(valid && cur_timetag > max_timetag) {
            max_timetag = cur_timetag;
        }

        appraisal_variable_wme = appraisal_variable_wme->next;
    }

    (*timetag) = max_timetag;

    // TODO: check for actual valid appraisal frame (i.e. both variables were set; need new array of bools to check? -- maybe it can be in the appraisal frame info type)
    return TRUE;
}

bool emotion_get_appraisal_variable_timetag(wme * appraisal_variable_wme, unsigned long * timetag) {

    wme * value_wme;
    variable appraisal_variable_wme_attr_var;
    slot * appraisal_variable_wme_slot;
    
    appraisal_variable_wme_attr_var = appraisal_variable_wme->attr->var;

    /* check to see if this is the right kind of wme
       if it's not a symbol constant
         then it's invalid
       if it is a stmbol constant but it's name is not desirability or likelihood
         then it's invalid
    */
    if(appraisal_variable_wme_attr_var.common_symbol_info.symbol_type != SYM_CONSTANT_SYMBOL_TYPE) {
        return FALSE;
    } else {
        if(strcmp(appraisal_variable_wme_attr_var.name,"desirability") &&
         strcmp(appraisal_variable_wme_attr_var.name,"likelihood") )
        { return FALSE; }
    } 


    //TODO: check under what circumstances this is necessary (i.e. if an elaboration rule removes something from memory?)
    if(appraisal_variable_wme->value->id.slots == NULL) { return FALSE; }

    appraisal_variable_wme_slot = appraisal_variable_wme->value->id.slots;
    value_wme = appraisal_variable_wme_slot->wmes;

    /* look for a WME with attribute = "value" and a numerical value
       if it's not an attribute named "value"
          or (it's not a float and it's not an int)
       then it's invalid
       */
    while(value_wme) {
        if( strcmp(value_wme->attr->var.name,"value") ) {
            /* get the next wme, which will be in the next slot */
            if(appraisal_variable_wme_slot->next) {
                appraisal_variable_wme_slot = appraisal_variable_wme_slot->next;
                value_wme = appraisal_variable_wme_slot->wmes;
            }
            else {
                value_wme = NIL;
                break;
            }
        } else if(value_wme->value->var.common_symbol_info.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE && 
                  value_wme->value->var.common_symbol_info.symbol_type != INT_CONSTANT_SYMBOL_TYPE )
        { return FALSE; }
        else { break; }
        
    }

    if(!value_wme) { return FALSE; }

    /* if we made it this far, then it must be a legit appraisal variable,
       so grab the timetag
    */
    (*timetag) = value_wme->timetag;

    return TRUE;
}   

void emotion_add_appraisal_to_list(appraisal_frame ** af_list, appraisal_frame * af) {

    appraisal_frame * new_appraisal_frame;

    new_appraisal_frame = allocate_memory_and_zerofill (sizeof(appraisal_frame), 0);
    new_appraisal_frame->variables[0] = af->variables[0];
    new_appraisal_frame->variables[1] = af->variables[1];
    
    /*if this is the first item in the list, initialize the list*/
    if(*af_list == NIL) {
        (*af_list) = new_appraisal_frame;
    } else {
        insert_at_head_of_dll(*af_list,new_appraisal_frame,next,prev);
    }
}

void emotion_add_value_to_list(struct appraisal_variable_value ** avv, 
                               appraisal_variable_info * info) {

    appraisal_variable_value * new_value;

    new_value = allocate_memory_and_zerofill (sizeof(appraisal_variable_value), 0);
    new_value->value = info->value;
    
    /*if this is the first item in the list, initialize the list*/
    if(*avv == NIL) {
        (*avv) = new_value;
    } else {
        insert_at_head_of_dll(*avv,new_value,next,prev);
    }
}


void emotion_aggregate_variables() {

    int type;

    /*for(type=0; type < emotion_NUM_TYPES; type++) {

        struct appraisal_variable_value * avv;
        float total;
        int count;

        total = 0.0;
        count = 0;

        avv = current_agent(emotions)->variables[type];

        if(avv) {
            while(avv) {
                total += avv->value;
                count++;
                avv = avv->next;
            }

            current_agent(emotions)->aggs[type] = total/(float)count;
        } else {
            current_agent(emotions)->aggs[type] = emotion_NO_VALUE_FLOAT;
        }
        
        emotion_update_aggregate(type);
    }*/

    float sums[emotion_NUM_TYPES];
    int count;
    appraisal_frame * af;
    
    af = current_agent(emotions)->appraisal_frames;

    for(type=0; type < emotion_NUM_TYPES; type++) {
        sums[type] = 0;
    }
    
    count = 0;

    if(af) {
        while(af) {

            for(type=0; type < emotion_NUM_TYPES; type++) {
                sums[type] += af->variables[type];
            }

            count++;
            af = af->next;
        }

        for(type=0; type < emotion_NUM_TYPES; type++) {
            current_agent(emotions)->aggs[type] = sums[type]/(float)count;
        }

    } else {
        for(type=0; type < emotion_NUM_TYPES; type++) {
            current_agent(emotions)->aggs[type] = emotion_NO_VALUE_FLOAT;
        }
    }

}

void emotion_clear_appraisal_list(appraisal_frame ** af) {
    appraisal_frame * af_to_delete;

    while(*af) {
        af_to_delete = *af;

        fast_remove_from_dll(*af,*af,appraisal_frame,next,prev);
        free_memory(af_to_delete,0);
    }
}

void emotion_clear_list(struct appraisal_variable_value ** avv) {
    struct appraisal_variable_value * val_to_delete;

    while(*avv) {
        val_to_delete = *avv;

        fast_remove_from_dll(*avv,*avv,appraisal_variable_value,next,prev);
        free_memory(val_to_delete,0);
    }
}

void emotion_init(emotions_struct ** e) {
    int type;

    (*e) = allocate_memory_and_zerofill (sizeof(emotions_struct), 0);
    
    for(type=0; type < emotion_NUM_TYPES; type++) {
        (*e)->aggs[type] = emotion_NO_VALUE_FLOAT;
    }

    current_agent(emotions)->latest_timetag = 0;
    current_agent(emotions)->num_appraisals = 0;
}

void emotion_destructor() {

    int type;

    for(type=0; type < emotion_NUM_TYPES; type++) {

        /* reset wmes
           BUG: program crashes if we call remove_input_wme, so these wmes may not be getting cleaned up properly
        */
        current_agent(emotions)->agg_wmes[type] = NIL;
        /* clear lists */
        emotion_clear_list(&(current_agent(emotions)->variables[type]));
    }

    /* free any allocated emotion symbols, WMEs */
    if(current_agent(emotions)->emotion_wme) {
        release_io_symbol (current_agent(emotions)->emotion_symbol);
    }

    /* clean up emotions structure */
    free_memory(current_agent(emotions),0);
    current_agent(emotions) = NIL;

}