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
    bool object; /* if object == TRUE, then type and value don't hold meaningful information */
    unsigned long timetag;
} appraisal_variable_info;

bool emotion_appraisals_have_changed();
bool emotion_get_appraisal_variable_timetag(wme * appraisal_variable_wme, unsigned long * timetag);
bool emotion_get_appraisal_frame_timetag(identifier appraisal_variable_id, unsigned long * timetag);
void emotion_get_appraisals();
void emotion_aggregate_variables();
void emotion_update_emotion();
void emotion_update_test_aggregate(wme ** agg_wme, float value, char * label);

void emotion_add_appraisal_to_list(appraisal_frame ** af_list, appraisal_frame * af);

void emotion_clear_appraisal_list(appraisal_frame ** af);

bool emotion_get_appraisal_frame(struct slot_struct * appraisal_variable_slot, appraisal_frame ** af);
bool emotion_get_appraisal_variable_info(wme * appraisal_variable_wme, appraisal_variable_info ** info);

const int emotion_NUM_TYPES = 2;
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

    float desirability, likelihood, signedIntensity;

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

        signedIntensity = desirability*likelihood;

        /* For now, ties are broken in favor of not being angry. this fixes the problem where the overall intensity is zero and there are no objects leading to anger */
        /* If we wanted to break the tie in favor of anger, then we would need a flag which said that there are actually objects available to be angry at */
        if(signedIntensity > 0 || soar_agent->emotions->average_object_frame_intensity <= soar_agent->emotions->average_no_object_frame_intensity) {

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
        } else {
            wme_emotion_name_value = get_io_sym_constant ("anger");
        }
     
        wme_emotion_name_attr = get_io_sym_constant ("name");
        wme_emotion_intensity_attr = get_io_sym_constant ("intensity");

        wme_emotion_intensity_value = get_io_float_constant((float)fabs(signedIntensity));

        current_agent(emotions)->emotion_symbol = get_new_io_identifier('E');
        current_agent(emotions)->emotion_wme = add_input_wme (current_agent(io_header_emotion),
		       make_sym_constant("emotion"),
		       current_agent(emotions)->emotion_symbol);

        add_input_wme (current_agent(emotions)->emotion_symbol,wme_emotion_name_attr,wme_emotion_name_value);
        add_input_wme (current_agent(emotions)->emotion_symbol,wme_emotion_intensity_attr,wme_emotion_intensity_value);
    }
}

//TODO: check for correct substructure
void emotion_get_appraisals() {

    union symbol_union * e_link;
    struct slot_struct * appraisal_variable_slot;
    struct slot_struct * appraisal_frame_slot;
    wme * appraisal_frame_wme;
    appraisal_frame * cur_appraisal_frame;
    unsigned long num_appraisals;

    num_appraisals = 0;

    if(current_agent(emotions) == NIL) {
        emotion_init(&(current_agent(emotions)));
    }

    /* get the emotions link */
    e_link = current_agent(io_header_emotion);

    appraisal_frame_slot = current_agent(io_header_emotion)->id.slots;

    /* we'll allocate this once and reuse it several times */
    cur_appraisal_frame = allocate_memory_and_zerofill (sizeof(appraisal_frame), 0);

    emotion_clear_appraisal_list(&current_agent(emotions)->appraisal_frames);

    while(appraisal_frame_slot) {

        bool valid;

        appraisal_frame_wme = appraisal_frame_slot->wmes;

        if(appraisal_frame_wme && !strcmp(appraisal_frame_slot->attr->sc.name, "appraisal")) {

            while(appraisal_frame_wme) {
                appraisal_variable_slot = appraisal_frame_wme->value->id.slots;

                valid = emotion_get_appraisal_frame(appraisal_variable_slot, &cur_appraisal_frame);

                if(valid) {
                    emotion_add_appraisal_to_list(&current_agent(emotions)->appraisal_frames, cur_appraisal_frame);
                    num_appraisals++;

                    /* TODO: should update timetag here, but then will need to store timetag in appraisal_frame or create appraisal_frame info type */
                }

                appraisal_frame_wme = appraisal_frame_wme->next;
            }
        }

        appraisal_frame_slot = appraisal_frame_slot->next;
    }

    current_agent(emotions)->num_appraisals = num_appraisals;
}

//TODO: should really just call get appraisals again and have it return a list which we can check; the timetags can be in the list
bool emotion_appraisals_have_changed() {

    union symbol_union * e_link;
    struct slot_struct * appraisal_frame_slot;
    wme * appraisal_frame_wme;
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

    while(appraisal_frame_slot) {

        appraisal_frame_wme = appraisal_frame_slot->wmes;

        while(appraisal_frame_wme) {

            bool valid;
            unsigned long timetag;

            
            valid = emotion_get_appraisal_frame_timetag(appraisal_frame_wme->value->id, &timetag);

            if(valid) {
                num_appraisals++;
                if(timetag > current_agent(emotions)->latest_timetag) {
                    changed = TRUE;
                    break;
                }
            }
            appraisal_frame_wme = appraisal_frame_wme->next;
        }

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
    bool valid_frame;

    valid_frame = FALSE;

    /* we'll allocate this once and reuse it several times */
    info = allocate_memory_and_zerofill (sizeof(appraisal_variable_info), 0);

    while(appraisal_variable_slot) {

        appraisal_variable_wme = appraisal_variable_slot->wmes;

        while(appraisal_variable_wme) {
            bool valid_variable;

            valid_variable = emotion_get_appraisal_variable_info(appraisal_variable_wme, &info);
            if(valid_variable) {
                valid_frame = TRUE;
                (*af)->object = info->object;
                if(!(*af)->object) {
                    (*af)->variables[info->type] = info->value;
                }

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

    // TODO: check for actual valid appraisal frame (i.e. all required variables were set; need new array of bools to check? -- maybe it can be in the appraisal frame info type)
    return valid_frame;
}

bool emotion_get_appraisal_variable_info(wme * appraisal_variable_wme, appraisal_variable_info ** info) {

    appraisal_variable_type type;
    float value;
    bool object;

    /* initialize these to placate the compiler */
    type = 0;
    object = FALSE;
    value = emotion_NO_VALUE_FLOAT;

    /* check for correct variable type */
    if( !strcmp(appraisal_variable_wme->attr->var.name,"desirability") ){
        type = DESIRABILITY;
    } else if( !strcmp(appraisal_variable_wme->attr->var.name,"likelihood") ) {
        type = LIKELIHOOD;
    } else if( !strcmp(appraisal_variable_wme->attr->var.name,"object") ) {
        object = TRUE;
    } else {
        return FALSE;
    }

    /* check for legal value type */
    if(appraisal_variable_wme->value->var.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
        value = (float)appraisal_variable_wme->value->ic.value;
    } else if(appraisal_variable_wme->value->var.common_symbol_info.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
        value = appraisal_variable_wme->value->fc.value;
    } else if(!object) { /* if the variable type is object, then we can skip this check */
        return FALSE;
    }

    (*info)->type = type;
    (*info)->value = value;
    (*info)->timetag = appraisal_variable_wme->timetag;
    (*info)->object = object;

    return TRUE;
}

bool emotion_get_appraisal_variable_infoOLD(wme * appraisal_variable_wme, appraisal_variable_info ** info) {

    wme * value_wme;
    appraisal_variable_type type;
    variable appraisal_variable_wme_attr_var;
    slot * appraisal_variable_wme_slot;
    unsigned long timetag;
    float value;
    bool object;
    
    object = FALSE;
    value = 0.0;
    type = 0;

    appraisal_variable_wme_attr_var = appraisal_variable_wme->attr->var;

    /* check to see if this is the right kind of wme */
    if(appraisal_variable_wme_attr_var.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {
        if(!strcmp(appraisal_variable_wme_attr_var.name,"desirability")) {
            type = DESIRABILITY;
        } else if (!strcmp(appraisal_variable_wme_attr_var.name,"likelihood")) {
            type = LIKELIHOOD;
        } else if (!strcmp(appraisal_variable_wme_attr_var.name,"object")) {
            object = TRUE;
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

            if(!object) { /* if it's not an object, then it must have a numberic value */
                if(value_wme->value->var.common_symbol_info.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
                    value = value_wme->value->fc.value;
                    break;
                } else if(value_wme->value->var.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
                    value = (float)value_wme->value->ic.value;
                    break;
                } else { return FALSE; }
            } else {
                break; /* if it is an object, then the substructure can be arbitrary.  We only check for existence */
            }
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
    (*info)->object = object;

    return TRUE;
}   

bool emotion_get_appraisal_frame_timetag(identifier appraisal_frame_id, unsigned long * timetag) {
    slot * appraisal_variable_slot;
    wme * appraisal_variable_wme;
    unsigned long cur_timetag;
    unsigned long max_timetag;
    bool valid_frame;

    valid_frame = FALSE;
    max_timetag = 0;

    appraisal_variable_slot = appraisal_frame_id.slots;

    while(appraisal_variable_slot) {
        appraisal_variable_wme = appraisal_variable_slot->wmes;

        while(appraisal_variable_wme) {
            bool valid_variable;

            valid_variable = emotion_get_appraisal_variable_timetag(appraisal_variable_wme, &cur_timetag);
        
            if(valid_variable) {
                valid_frame = TRUE;
                if(cur_timetag > max_timetag) {
                    max_timetag = cur_timetag;
                }
            }

            appraisal_variable_wme = appraisal_variable_wme->next;
        }
        appraisal_variable_slot = appraisal_variable_slot->next;
    }

    (*timetag) = max_timetag;

    // TODO: check for actual valid appraisal frame (i.e. all required variables were set; need new array of bools to check? -- maybe it can be in the appraisal frame info type)
    return valid_frame;
}

bool emotion_get_appraisal_variable_timetag(wme * appraisal_variable_wme, unsigned long * timetag) {
    
    /* if the wme isn't one of the variables, then it isn't valid */
    if( (!strcmp(appraisal_variable_wme->attr->var.name,"desirability") || 
        !strcmp(appraisal_variable_wme->attr->var.name,"likelihood"))
        &&
        (appraisal_variable_wme->value->var.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE ||
        appraisal_variable_wme->value->var.common_symbol_info.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE))
    {
        (*timetag) = appraisal_variable_wme->timetag;
        return TRUE;
    } else if ( !strcmp(appraisal_variable_wme->attr->var.name,"object") ) {
        if(appraisal_variable_wme->value->var.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {
            // fixme: get all child wmes and return the largest timetag
            //wme ** wmelist = get_augs_of_id(appraisal_variable_wme->value->id, ???, ???)
            (*timetag) = appraisal_variable_wme->timetag;
            return TRUE;
        } else {
            (*timetag) = appraisal_variable_wme->timetag;
            return TRUE;
        }
    } else { return FALSE; }

}

bool emotion_get_appraisal_variable_timetagOLD(wme * appraisal_variable_wme, unsigned long * timetag) {

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
    new_appraisal_frame->object = af->object;
    
    /*if this is the first item in the list, initialize the list*/
    if(*af_list == NIL) {
        (*af_list) = new_appraisal_frame;
    } else {
        insert_at_head_of_dll(*af_list,new_appraisal_frame,next,prev);
    }
}


void emotion_aggregate_variables() {

    int type;
    appraisal_frame * af;
    float total;
    int count;
    int pos_obj_count, pos_no_obj_count, neg_obj_count, neg_no_obj_count, obj_count, no_obj_count;
    float intensity;
    
    for(type=0; type < emotion_NUM_TYPES; type++) {

        total = 0.0;
        count = 0;

        af = current_agent(emotions)->appraisal_frames;

        if(af) {
            while(af) {
                total += af->variables[type];
                count++;
                af = af->next;
            }

            current_agent(emotions)->aggs[type] = total/(float)count;
        } else {
            current_agent(emotions)->aggs[type] = emotion_NO_VALUE_FLOAT;
        }
        
        emotion_update_aggregate(type);
    }

    soar_agent->emotions->max_negative_no_object_frame_intensity = -100.0;
    soar_agent->emotions->max_negative_object_frame_intensity = -100.0;
    soar_agent->emotions->max_no_object_frame_intensity = -100.0;
    soar_agent->emotions->max_object_frame_intensity = -100.0;
    soar_agent->emotions->max_positive_no_object_frame_intensity = -100.0;
    soar_agent->emotions->max_positive_object_frame_intensity = -100.0;
    
    soar_agent->emotions->sum_negative_no_object_frame_intensity = 0.0;
    soar_agent->emotions->sum_negative_object_frame_intensity = 0.0;
    soar_agent->emotions->sum_no_object_frame_intensity = 0.0;
    soar_agent->emotions->sum_object_frame_intensity = 0.0;
    soar_agent->emotions->sum_positive_no_object_frame_intensity = 0.0;
    soar_agent->emotions->sum_positive_object_frame_intensity = 0.0;

    count = 0;
    pos_obj_count = 0;
    pos_no_obj_count = 0;
    neg_obj_count = 0;
    neg_no_obj_count = 0;
    obj_count = 0;
    no_obj_count = 0;

    af = current_agent(emotions)->appraisal_frames;

    if(af) {
        while(af) {
            
            /* Note that, while intensity is always positive, we allow negatives here
               to take advantage of opposite signs (so intensities can cancel each other out)
               Once the signed intensity is obtained, we will make it positive (at the end)
            */
            intensity = af->variables[DESIRABILITY]*af->variables[LIKELIHOOD];

            if(af->object) {
                obj_count++;

                soar_agent->emotions->sum_object_frame_intensity += intensity;

                if(intensity > soar_agent->emotions->max_object_frame_intensity) {
                    soar_agent->emotions->max_object_frame_intensity = intensity;
                }

                if(af->variables[DESIRABILITY] > 0) {
                    pos_obj_count++;
                
                    soar_agent->emotions->sum_positive_object_frame_intensity += intensity;

                    if(intensity > soar_agent->emotions->max_positive_object_frame_intensity) {
                        soar_agent->emotions->max_positive_object_frame_intensity = intensity;
                    }

                } else {
                    neg_obj_count++;
                
                    soar_agent->emotions->sum_negative_object_frame_intensity += intensity;

                    if(intensity > soar_agent->emotions->max_negative_object_frame_intensity) {
                        soar_agent->emotions->max_negative_object_frame_intensity = intensity;
                    }
                }
            } else {
                no_obj_count++;
                
                soar_agent->emotions->sum_no_object_frame_intensity += intensity;

                if(intensity > soar_agent->emotions->max_no_object_frame_intensity) {
                    soar_agent->emotions->max_no_object_frame_intensity = intensity;
                }

                if(af->variables[DESIRABILITY] > 0) {
                    pos_no_obj_count++;
                
                    soar_agent->emotions->sum_positive_no_object_frame_intensity += intensity;

                    if(intensity > soar_agent->emotions->max_positive_no_object_frame_intensity) {
                        soar_agent->emotions->max_positive_no_object_frame_intensity = intensity;
                    }

                } else {
                    neg_no_obj_count++;
                
                    soar_agent->emotions->sum_negative_no_object_frame_intensity += intensity;

                    if(intensity > soar_agent->emotions->max_negative_no_object_frame_intensity) {
                        soar_agent->emotions->max_negative_no_object_frame_intensity = intensity;
                    }
                }
            }

            count++;
            af = af->next;
        }

        /* calculate the averages from the sums.  Take the absolute values for the possibly negative intensities (intensity is always positive) */
        /* if the count of something is 0, then its intensity is 0 */
        if(neg_no_obj_count > 0) {
            soar_agent->emotions->average_negative_no_object_frame_intensity = (float)fabs(soar_agent->emotions->sum_negative_no_object_frame_intensity / (float)neg_no_obj_count);
        } else {
            soar_agent->emotions->average_negative_no_object_frame_intensity = 0;
        }

        if(neg_obj_count > 0) {
            soar_agent->emotions->average_negative_object_frame_intensity = (float)fabs(soar_agent->emotions->sum_negative_object_frame_intensity / (float)neg_obj_count);
        } else {
            soar_agent->emotions->average_negative_object_frame_intensity = 0;
        }

        if(no_obj_count > 0) {
            soar_agent->emotions->average_no_object_frame_intensity = (float)fabs(soar_agent->emotions->sum_no_object_frame_intensity / (float)no_obj_count);
        } else {
            soar_agent->emotions->average_no_object_frame_intensity = 0;
        }

        if(obj_count > 0) {
            soar_agent->emotions->average_object_frame_intensity = (float)fabs(soar_agent->emotions->sum_object_frame_intensity / (float)obj_count);
        } else {
            soar_agent->emotions->average_object_frame_intensity = 0;
        }

        if(pos_no_obj_count > 0) {
            soar_agent->emotions->average_positive_no_object_frame_intensity = soar_agent->emotions->sum_positive_no_object_frame_intensity / (float)pos_no_obj_count;
        } else {
            soar_agent->emotions->average_positive_no_object_frame_intensity = 0;
        }

        if(pos_obj_count > 0) {
            soar_agent->emotions->average_positive_object_frame_intensity = soar_agent->emotions->sum_positive_object_frame_intensity / (float)pos_obj_count;
        } else {
            soar_agent->emotions->average_positive_object_frame_intensity = 0;
        }
        
        /* take the absolute values of the possibily negative intensities (intensity is always positive) */
        /* if the numbers are still at their defaults (i.e. there were no instances found), then set them to 0 */
        if(soar_agent->emotions->max_negative_no_object_frame_intensity == emotion_NO_VALUE_FLOAT) {
            soar_agent->emotions->max_negative_no_object_frame_intensity = 0;
        } else {
            soar_agent->emotions->max_negative_no_object_frame_intensity = (float)fabs(soar_agent->emotions->max_negative_no_object_frame_intensity);
        }

        if(soar_agent->emotions->max_negative_object_frame_intensity == emotion_NO_VALUE_FLOAT) {
            soar_agent->emotions->max_negative_object_frame_intensity = 0;
        } else {
            soar_agent->emotions->max_negative_object_frame_intensity = (float)fabs(soar_agent->emotions->max_negative_object_frame_intensity);
        }

        if(soar_agent->emotions->max_no_object_frame_intensity == emotion_NO_VALUE_FLOAT) {
            soar_agent->emotions->max_no_object_frame_intensity = 0;
        } else {
            soar_agent->emotions->max_no_object_frame_intensity = (float)fabs(soar_agent->emotions->max_no_object_frame_intensity);
        }

        if(soar_agent->emotions->max_object_frame_intensity == emotion_NO_VALUE_FLOAT) {
            soar_agent->emotions->max_object_frame_intensity = 0;
        } else {
            soar_agent->emotions->max_object_frame_intensity = (float)fabs(soar_agent->emotions->max_object_frame_intensity);
        }

        if(soar_agent->emotions->max_positive_no_object_frame_intensity == emotion_NO_VALUE_FLOAT) {
            soar_agent->emotions->max_positive_no_object_frame_intensity = 0;
        }

        if(soar_agent->emotions->max_positive_object_frame_intensity == emotion_NO_VALUE_FLOAT) {
            soar_agent->emotions->max_positive_object_frame_intensity = 0;
        }

        /* take the absolute value to prevent negative intensities */
        soar_agent->emotions->sum_negative_no_object_frame_intensity = (float)fabs(soar_agent->emotions->sum_negative_no_object_frame_intensity);
        soar_agent->emotions->sum_negative_object_frame_intensity = (float)fabs(soar_agent->emotions->sum_negative_object_frame_intensity);
        soar_agent->emotions->sum_no_object_frame_intensity = (float)fabs(soar_agent->emotions->sum_no_object_frame_intensity);
        soar_agent->emotions->sum_object_frame_intensity = (float)fabs(soar_agent->emotions->sum_object_frame_intensity);

    } else { /* if there are no appraisals, then we don't want these appearing on the e-link */
        soar_agent->emotions->average_negative_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->average_negative_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->average_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->average_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->average_positive_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->average_positive_object_frame_intensity = emotion_NO_VALUE_FLOAT;

        soar_agent->emotions->max_negative_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->max_negative_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->max_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->max_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->max_positive_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->max_positive_object_frame_intensity = emotion_NO_VALUE_FLOAT;
    
        soar_agent->emotions->sum_negative_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->sum_negative_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->sum_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->sum_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->sum_positive_no_object_frame_intensity = emotion_NO_VALUE_FLOAT;
        soar_agent->emotions->sum_positive_object_frame_intensity = emotion_NO_VALUE_FLOAT;
    }

    /* update aggregates on e-link */
    
    emotion_update_test_aggregate(&soar_agent->emotions->average_negative_no_object_frame_intensity_wme, soar_agent->emotions->average_negative_no_object_frame_intensity, "average_negative_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->average_negative_object_frame_intensity_wme, soar_agent->emotions->average_negative_object_frame_intensity, "average_negative_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->average_no_object_frame_intensity_wme, soar_agent->emotions->average_no_object_frame_intensity, "average_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->average_object_frame_intensity_wme, soar_agent->emotions->average_object_frame_intensity, "average_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->average_positive_no_object_frame_intensity_wme, soar_agent->emotions->average_positive_no_object_frame_intensity, "average_positive_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->average_positive_object_frame_intensity_wme, soar_agent->emotions->average_positive_object_frame_intensity, "average_positive_object_frame_intensity");

    emotion_update_test_aggregate(&soar_agent->emotions->max_negative_no_object_frame_intensity_wme, soar_agent->emotions->max_negative_no_object_frame_intensity, "max_negative_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->max_negative_object_frame_intensity_wme, soar_agent->emotions->max_negative_object_frame_intensity, "max_negative_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->max_no_object_frame_intensity_wme, soar_agent->emotions->max_no_object_frame_intensity, "max_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->max_object_frame_intensity_wme, soar_agent->emotions->max_object_frame_intensity, "max_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->max_positive_no_object_frame_intensity_wme, soar_agent->emotions->max_positive_no_object_frame_intensity, "max_positive_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->max_positive_object_frame_intensity_wme, soar_agent->emotions->max_positive_object_frame_intensity, "max_positive_object_frame_intensity");

    emotion_update_test_aggregate(&soar_agent->emotions->sum_negative_no_object_frame_intensity_wme, soar_agent->emotions->sum_negative_no_object_frame_intensity, "sum_negative_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->sum_negative_object_frame_intensity_wme, soar_agent->emotions->sum_negative_object_frame_intensity, "sum_negative_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->sum_no_object_frame_intensity_wme, soar_agent->emotions->sum_no_object_frame_intensity, "sum_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->sum_object_frame_intensity_wme, soar_agent->emotions->sum_object_frame_intensity, "sum_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->sum_positive_no_object_frame_intensity_wme, soar_agent->emotions->sum_positive_no_object_frame_intensity, "sum_positive_no_object_frame_intensity");
    emotion_update_test_aggregate(&soar_agent->emotions->sum_positive_object_frame_intensity_wme, soar_agent->emotions->sum_positive_object_frame_intensity, "sum_positive_object_frame_intensity");

}

void emotion_update_test_aggregate(wme ** agg_wme, float value, char * label) {
    Symbol *wme_attr, *wme_value;

    if((*agg_wme)) {        
        bool success = remove_input_wme((*agg_wme));
        if(!success) print("Emotion: tried to remove aggregate value");
        assert(success);

        (*agg_wme) = NIL;
    }

    if(value != emotion_NO_VALUE_FLOAT) {

        wme_attr = get_io_sym_constant (label);
        wme_value = get_io_float_constant (value);
    
        (*agg_wme) =
            add_input_wme (current_agent(io_header_emotion),wme_attr,wme_value);
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
    }

    /* clear lists */
    emotion_clear_appraisal_list(&current_agent(emotions)->appraisal_frames);

    /* free any allocated emotion symbols, WMEs */
    if(current_agent(emotions)->emotion_wme) {
        release_io_symbol (current_agent(emotions)->emotion_symbol);
    }

    /* clean up emotions structure */
    free_memory(current_agent(emotions),0);
    current_agent(emotions) = NIL;

}