/*
 * smem_store.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_db.h"
#include "smem_stats.h"
#include "smem_settings.h"

#include "agent.h"
#include "episodic_memory.h"
#include "lexer.h"
#include "mem.h"
#include "misc.h"
#include "print.h"
#include "production.h"
#include "slot.h"
#include "symbol_manager.h"
#include "working_memory.h"
#include "xml.h"

void SMem_Manager::deallocate_ltm(ltm_object* pLTM, bool free_ltm )
{
    if (pLTM)
    {
        // proceed to slots
        if (pLTM->slots)
        {
            ltm_slot_map::iterator s;
            ltm_slot::iterator v;
            Symbol* lSym;
            // iterate over slots
            while (!pLTM->slots->empty())
            {
                s = pLTM->slots->begin();

                // proceed to slot contents
                if (s->second)
                {
                    // iterate over each value
                    for (v = s->second->begin(); v != s->second->end(); v = s->second->erase(v))
                    {
                        // de-allocation of value is dependent upon type
                        if ((*v)->val_const.val_type == value_const_t)
                        {
                            thisAgent->symbolManager->symbol_remove_ref(&(*v)->val_const.val_value);
                        }

                        delete(*v);
                    }

                    delete s->second;
                }

                // deallocate attribute for each corresponding value
                lSym = s->first;
                thisAgent->symbolManager->symbol_remove_ref(&lSym);

                pLTM->slots->erase(s);
            }

            // remove slots
            delete pLTM->slots;
            pLTM->slots = NULL;
        }

        // remove ltm itself
        if (free_ltm)
        {
            delete pLTM;
            pLTM = NULL;
        }
    }
}

Symbol* SMem_Manager::parse_constant_attr(soar::Lexeme* lexeme)
{
    Symbol* return_val = NIL;

    if ((*lexeme).type == STR_CONSTANT_LEXEME)
    {
        return_val = thisAgent->symbolManager->make_str_constant(static_cast<const char*>((*lexeme).string()));
    }
    else if ((*lexeme).type == INT_CONSTANT_LEXEME)
    {
        return_val = thisAgent->symbolManager->make_int_constant((*lexeme).int_val);
    }
    else if ((*lexeme).type == FLOAT_CONSTANT_LEXEME)
    {
        return_val = thisAgent->symbolManager->make_float_constant((*lexeme).float_val);
    }

    return return_val;
}

bool SMem_Manager::parse_add_clause(soar::Lexer* lexer, str_to_ltm_map* str_to_LTMs, ltm_set* newbies)
{
    bool return_val = false;

    ltm_object* l_ltm = new ltm_object;
    l_ltm->slots = NULL;

    std::string l_ltm_name;

    bool good_at;

    // consume left paren
    lexer->get_lexeme();

    if ((lexer->current_lexeme.type == AT_LEXEME) || (lexer->current_lexeme.type == IDENTIFIER_LEXEME) || (lexer->current_lexeme.type == VARIABLE_LEXEME))
    {
        good_at = true;

        if (lexer->current_lexeme.type == AT_LEXEME)
        {
            lexer->get_lexeme();

            good_at = (lexer->current_lexeme.type == INT_CONSTANT_LEXEME);
            if (good_at)
            {
                // l_ltm->lti_id = lti_exists(lexer->current_lexeme.int_val);
                // good_at = (l_ltm->lti_id != NIL);
                l_ltm->lti_id = lexer->current_lexeme.int_val;
            }
        }
        else if (lexer->current_lexeme.type == IDENTIFIER_LEXEME)
        {
            /* MToDo | May want to keep this case in case we want to support smem --add using existing STIs and their underlying LTI IDs.
             * For now, we'll just set to a bad parse. In the future, we find identifier with letter/number and use LTI_ID */
            good_at = false;
        }

        if (good_at)
        {
            if (lexer->current_lexeme.type == VARIABLE_LEXEME)
            {
                l_ltm_name.append(lexer->current_lexeme.string());
                l_ltm->lti_id = NIL;
            } else {
                assert ((lexer->current_lexeme.type == INT_CONSTANT_LEXEME) || (lexer->current_lexeme.type == IDENTIFIER_LEXEME));
                get_lti_name(l_ltm->lti_id, l_ltm_name);
            }
            l_ltm->soar_id = NIL;
            l_ltm->slots = new ltm_slot_map;

            // consume id
            lexer->get_lexeme();

            //

            uint64_t intermediate_counter = 1;
            ltm_object* l_ltm_intermediate_parent;
            ltm_object* l_ltm_temp;
            std::string temp_key;
            std::string temp_key2;
            Symbol*     l_ltm_attr;
            ltm_value*  l_ltm_value;
            ltm_slot*  l_ltm_slot;

            // populate slots
            while (lexer->current_lexeme.type == UP_ARROW_LEXEME)
            {
                l_ltm_intermediate_parent = l_ltm;

                // go on to attribute
                lexer->get_lexeme();

                // get the appropriate constant type
                l_ltm_attr = parse_constant_attr(&(lexer->current_lexeme));

                // if constant attribute, proceed to value
                if (l_ltm_attr != NIL)
                {
                    // consume attribute
                    lexer->get_lexeme();

                    // support for dot notation:
                    // when we encounter a dot, instantiate
                    // the previous attribute as a temporary
                    // identifier and use that as the parent
                    while (lexer->current_lexeme.type == PERIOD_LEXEME)
                    {
                        // create a new ltm
                        l_ltm_temp = new ltm_object;
                        l_ltm_temp->lti_id = NIL;
                        l_ltm_temp->slots = new ltm_slot_map;
                        l_ltm_temp->soar_id = NIL;

                        // add it as a child to the current parent
                        l_ltm_value = new ltm_value;
                        l_ltm_value->val_lti.val_type = value_lti_t;
                        l_ltm_value->val_lti.val_value = l_ltm_temp;
                        l_ltm_slot = make_ltm_slot(l_ltm_intermediate_parent->slots, l_ltm_attr);
                        l_ltm_slot->push_back(l_ltm_value);

                        // create a key guaranteed to be unique
                        temp_key.assign("<");
                        temp_key.append(1, ((l_ltm_attr->symbol_type == STR_CONSTANT_SYMBOL_TYPE) ? (static_cast<char>(static_cast<int>(l_ltm_attr->sc->name[0]))) : ('X')));
                        temp_key.append("#");
                        temp_key.append(std::to_string(++intermediate_counter));
                        temp_key.append(">");

                        // insert the new ltm
                        (*str_to_LTMs)[ temp_key ] = l_ltm_temp;

                        // definitely a new ltm
                        newbies->insert(l_ltm_temp);

                        // the new ltm is our parent for this set of values (or further dots)
                        l_ltm_intermediate_parent = l_ltm_temp;
                        l_ltm_temp = NULL;

                        // get the next attribute
                        lexer->get_lexeme();
                        l_ltm_attr = parse_constant_attr(&(lexer->current_lexeme));

                        // consume attribute
                        lexer->get_lexeme();
                    }

                    if (l_ltm_attr != NIL)
                    {
                        bool first_value = true;

                        do
                        {
                            // value by type
                            l_ltm_value = NIL;
                            if (lexer->current_lexeme.type == STR_CONSTANT_LEXEME)
                            {
                                l_ltm_value = new ltm_value;
                                l_ltm_value->val_const.val_type = value_const_t;
                                l_ltm_value->val_const.val_value = thisAgent->symbolManager->make_str_constant(static_cast<const char*>(lexer->current_lexeme.string()));
                            }
                            else if (lexer->current_lexeme.type == INT_CONSTANT_LEXEME)
                            {
                                l_ltm_value = new ltm_value;
                                l_ltm_value->val_const.val_type = value_const_t;
                                l_ltm_value->val_const.val_value = thisAgent->symbolManager->make_int_constant(lexer->current_lexeme.int_val);
                            }
                            else if (lexer->current_lexeme.type == FLOAT_CONSTANT_LEXEME)
                            {
                                l_ltm_value = new ltm_value;
                                l_ltm_value->val_const.val_type = value_const_t;
                                l_ltm_value->val_const.val_value = thisAgent->symbolManager->make_float_constant(lexer->current_lexeme.float_val);
                            }
                            else if ((lexer->current_lexeme.type == AT_LEXEME) || (lexer->current_lexeme.type == IDENTIFIER_LEXEME) || (lexer->current_lexeme.type == VARIABLE_LEXEME))
                            {
                                good_at = true;

                                if (lexer->current_lexeme.type == AT_LEXEME)
                                {
                                    lexer->get_lexeme();

                                    good_at = (lexer->current_lexeme.type == INT_CONSTANT_LEXEME);
                                }

                                if (good_at)
                                {
                                    // create new value
                                    l_ltm_value = new ltm_value;
                                    l_ltm_value->val_lti.val_type = value_lti_t;

                                    // get key
                                    if (lexer->current_lexeme.type == VARIABLE_LEXEME)
                                    {
                                        temp_key2.assign(lexer->current_lexeme.string());
                                    } else {
                                        assert ((lexer->current_lexeme.type == INT_CONSTANT_LEXEME) || (lexer->current_lexeme.type == IDENTIFIER_LEXEME));
                                        temp_key2.clear();
                                        get_lti_name(static_cast<uint64_t>(lexer->current_lexeme.int_val), temp_key2);
                                    }

                                    // search for an existing ltm
                                    str_to_ltm_map::iterator p = str_to_LTMs->find((temp_key2));

                                    // if exists, point; else create new
                                    if (p != str_to_LTMs->end())
                                    {
                                        l_ltm_value->val_lti.val_value = p->second;
                                    }
                                    else
                                    {
                                        // create new ltm
                                        l_ltm_temp = new ltm_object;
                                        l_ltm_temp->slots = NIL;
                                        l_ltm_temp->soar_id = NIL;

                                        if (lexer->current_lexeme.type == INT_CONSTANT_LEXEME)
                                        {
                                            l_ltm_temp->lti_id = static_cast<uint64_t>(lexer->current_lexeme.int_val);
                                            /* May want to verify that this is a legitimate id in the smem database */
                                            //l_ltm_temp->lti_id = lti_exists(static_cast<uint64_t>(lexer->current_lexeme.int_val));
                                            //if (l_ltm_temp->lti_id == NIL)
                                            //{
                                            //    good_at = false;
                                            //    delete l_ltm_temp;
                                            //    delete l_ltm_value;
                                            //    l_ltm_temp = NULL;
                                            //    l_ltm_value = NULL;
                                            //}

                                        } else {
                                            l_ltm_temp->lti_id = NIL;
                                        }
                                        if (l_ltm_temp)
                                        {
                                            // associate with value
                                            l_ltm_value->val_lti.val_value = l_ltm_temp;

                                            // add to ltms
                                            (*str_to_LTMs)[temp_key2] = l_ltm_temp;

                                            // possibly a newbie (could be a self-loop)
                                            newbies->insert(l_ltm_temp);
                                        }
                                    }
                                }
                            }

                            if (l_ltm_value != NIL)
                            {
                                // consume
                                lexer->get_lexeme();

                                // add to appropriate slot
                                l_ltm_slot = make_ltm_slot(l_ltm_intermediate_parent->slots, l_ltm_attr);
                                if (first_value && !l_ltm_slot->empty())
                                {
                                    // in the case of a repeated attribute, remove ref here to avoid leak
                                    thisAgent->symbolManager->symbol_remove_ref(&l_ltm_attr);
                                }
                                l_ltm_slot->push_back(l_ltm_value);

                                // if this was the last attribute
                                if (lexer->current_lexeme.type == R_PAREN_LEXEME)
                                {
                                    return_val = true;
                                    lexer->get_lexeme();
                                    l_ltm_value = NIL;
                                }

                                first_value = false;
                            }
                        }
                        while (l_ltm_value != NIL);
                    }
                }
            }
        }
        else
        {
            delete l_ltm;
        }
    }
    else
    {
        delete l_ltm;
    }

    if (return_val)
    {
        // search for an existing ltm (occurs if value comes before id)
        /* MToDo | Isn't this just indexing by the pointer to the string?  Which means that they're all unique and have
         *        don't really need the string to be unique? */
        ltm_object** p = & (*str_to_LTMs)[l_ltm_name];

        if (!(*p))
        {
            (*p) = l_ltm;
            newbies->insert(l_ltm);
        }
        else
        {
            // transfer slots
            if (!(*p)->slots)
            {
                // if none previously, can just use
                (*p)->slots = l_ltm->slots;
                l_ltm->slots = NULL;
            }
            else
            {
                // otherwise, copy

                ltm_slot_map::iterator ss_p;
                ltm_slot::iterator s_p;

                ltm_slot* source_slot;
                ltm_slot* target_slot;

                // for all slots
                for (ss_p = l_ltm->slots->begin(); ss_p != l_ltm->slots->end(); ss_p++)
                {
                    target_slot =make_ltm_slot((*p)->slots, ss_p->first);
                    source_slot = ss_p->second;

                    // for all values in the slot
                    for (s_p = source_slot->begin(); s_p != source_slot->end(); s_p++)
                    {
                        // copy each value
                        target_slot->push_back((*s_p));
                    }

                    // once copied, we no longer need the slot
                    delete source_slot;
                }

                // we no longer need the slots
                delete l_ltm->slots;
                l_ltm->slots = NULL;
            }

            // contents are new
            newbies->insert((*p));

            // deallocate
            deallocate_ltm(l_ltm);
        }
    }
    else
    {
        newbies->clear();
    }

    return return_val;
}

bool SMem_Manager::process_smem_add_object(const char* ltms_str, std::string** err_msg)
{
    bool return_val = false;
    uint64_t clause_count = 0;

    // parsing ltms requires an open semantic database
    attach();

    soar::Lexer lexer(thisAgent, ltms_str);

    bool good_ltm = true;

    str_to_ltm_map ltms;
    str_to_ltm_map::iterator c_old;

    ltm_set newbies;
    ltm_set::iterator c_new;

    // consume next token
    lexer.get_lexeme();

    if (lexer.current_lexeme.type != L_PAREN_LEXEME)
    {
        good_ltm = false;
    }

    // while there are ltms to consume
    while ((lexer.current_lexeme.type == L_PAREN_LEXEME) && (good_ltm))
    {
        good_ltm = parse_add_clause(&lexer, &(ltms), &(newbies));

        if (good_ltm)
        {
            // add all newbie lti's as appropriate
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
                if ((*c_new)->lti_id == NIL)
                {
                    if ((*c_new)->soar_id != NIL)
                    {
                        (*c_new)->lti_id = make_STI_instance_of_new_LTI((*c_new)->soar_id);
                    } else {
                        (*c_new)->lti_id = add_new_LTI();
                    }
                }
                else
                {
                    if (!lti_exists((*c_new)->lti_id))
                    {
                        add_specific_LTI((*c_new)->lti_id);
                    }
                }
            }

            // add all newbie contents (append, as opposed to replace, children)
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
                if ((*c_new)->slots != NIL)
                {
                    /* Third parameter determines whether smem will update LTM based on LTI_ID in an STI.
                     * For that to be useful here, parser must be changed to also accepts STIs for smem -add.
                     * May want to change to false until that is possible */
                   store_LTM_in_DB((*c_new)->lti_id, (*c_new)->slots, false);
                }
            }

            // deallocate *contents* of all newbies (need to keep around name->id association for future ltms)
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
               deallocate_ltm((*c_new), false);
            }

            // increment clause counter
            clause_count++;

            // clear newbie list
            newbies.clear();
        }
    };

    return_val = good_ltm;

    // deallocate all ltms
    {
        for (c_old = ltms.begin(); c_old != ltms.end(); c_old++)
        {
           deallocate_ltm(c_old->second, true);
        }
    }

    // produce error message on failure
    if (!return_val)
    {
        std::string num;
        to_string(clause_count, num);

        (*err_msg)->append("Error parsing clause #");
        (*err_msg)->append(num);
    }

    return return_val;
}

/* The following function is supposed to read in the lexemes
 * and turn them into the cue wme for a call to smem_process_query.
 * This is intended to be run from the command line and does not yet have
 * full functionality. It doesn't work with mathqueries, for example.
 * This is for debugging purposes.
 * -Steven 23-7-2014
 */

bool SMem_Manager::parse_cues(const char* ltms_str, std::string** err_msg, std::string** result_message, uint64_t number_to_retrieve)
{
    uint64_t clause_count = 0;  // This is counting up the number of parsed clauses
    // so that there is a pointer to a failure location.

    //Parsing requires an open semantic database.
    attach();
    clear_instance_mappings();

    soar::Lexer lexer(thisAgent, ltms_str);

    bool good_cue = true;   // This is a success or failure flag that will be checked periodically
    // and indicates whether or not we can call smem_process_query.

    std::map<std::string, Symbol*> cue_ids; //I want to keep track of previous references when adding a new element to the cue.

    //consume next token.
    lexer.get_lexeme();

    good_cue = lexer.current_lexeme.type == L_PAREN_LEXEME;

    Symbol* root_cue_id = NIL;    //This is the id that gets passed to smem_process_query.
    //It's main purpose is to contain augmentations
    Symbol* negative_cues = NULL;  //This is supposed to contain the negative augmentations.

    bool trigger_first = true; //Just for managing my loop.
    bool minus_ever = false; //Did a negative cue ever show up?
    bool first_attribute = true; //Want to make sure there is a positive attribute to begin with.

    // While there is parsing to be done:
    while ((lexer.current_lexeme.type == L_PAREN_LEXEME) && good_cue)
    {
        //First, consume the left paren.
        lexer.get_lexeme();

        if (trigger_first)
        {
            good_cue = lexer.current_lexeme.type == VARIABLE_LEXEME;

            if (good_cue)
            {
                root_cue_id = thisAgent->symbolManager->make_new_identifier((char) lexer.current_lexeme.string()[1], 1);
                cue_ids[lexer.current_lexeme.string()] = root_cue_id;
                negative_cues = thisAgent->symbolManager->make_new_identifier((char) lexer.current_lexeme.string()[1], 1);
            }
            else
            {
                (*err_msg)->append("Error: The cue must be a variable.\n");//Spit out that the cue must be a variable.
                break;
            }

            trigger_first = false;
        }
        else
        {
            //If this isn't the first time around, then this better be the same as the root_cue_id variable.
            good_cue = cue_ids[lexer.current_lexeme.string()] == root_cue_id;
            if (!good_cue)
            {
                (*err_msg)->append("Error: Additional clauses must share same variable.\n");//Spit out that additional clauses must share the same variable as the original cue variable.
                break;
            }
        }

        if (good_cue)
        {
            //Consume the root_cue_id
            lexer.get_lexeme();

            Symbol* attribute;
            slot* temp_slot;

            //Now, we process the attributes of the cue id contained in this clause.
            bool minus = false;

            //Loop as long as positive or negative cues keep popping up.
            while (good_cue && (lexer.current_lexeme.type == UP_ARROW_LEXEME || lexer.current_lexeme.type == MINUS_LEXEME))
            {
                if (lexer.current_lexeme.type == MINUS_LEXEME)
                {
                    minus_ever = true;
                    if (first_attribute)
                    {
                        good_cue = false;
                        break;
                    }
                    lexer.get_lexeme();
                    good_cue = lexer.current_lexeme.type == UP_ARROW_LEXEME;
                    minus = true;
                }
                else
                {
                    minus = false;
                }

                lexer.get_lexeme();//Consume the up arrow and move on to the attribute.

                if (lexer.current_lexeme.type == VARIABLE_LEXEME)
                {
                    //SMem doesn't suppose variable attributes ... YET.
                    good_cue = false;
                    break;
                }

                // TODO: test to make sure this is good. Previously there was no test
                // for the type of the lexeme so passing a "(" caused a segfault when making the slot.
                attribute = parse_constant_attr(&(lexer.current_lexeme));
                if (attribute == NIL)
                {
                    good_cue = false;
                    break;
                }

                Symbol* value;
                wme* temp_wme;

                if (minus)
                {
                    temp_slot = make_slot(thisAgent, negative_cues, attribute);
                }
                else
                {
                    temp_slot = make_slot(thisAgent, root_cue_id, attribute); //Make a slot for this attribute, or return slot it already has.
                }

                //consume the attribute.
                lexer.get_lexeme();
                bool hasAddedValue = false;

                do //Add value by type
                {
                    value = NIL;
                    if (lexer.current_lexeme.type == STR_CONSTANT_LEXEME)
                    {
                        value = thisAgent->symbolManager->make_str_constant(static_cast<const char*>(lexer.current_lexeme.string()));
                        lexer.get_lexeme();
                    }
                    else if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
                    {
                        value = thisAgent->symbolManager->make_int_constant(lexer.current_lexeme.int_val);
                        lexer.get_lexeme();
                    }
                    else if (lexer.current_lexeme.type == FLOAT_CONSTANT_LEXEME)
                    {
                        value = thisAgent->symbolManager->make_float_constant(lexer.current_lexeme.float_val);
                        lexer.get_lexeme();
                    }
                    else if (lexer.current_lexeme.type == AT_LEXEME)
                    {
                        lexer.get_lexeme();
                        uint64_t value_id = 0;
                        if (lexer.current_lexeme.type != INT_CONSTANT_LEXEME)
                        {
                            good_cue = false;
                            (*err_msg)->append("Error: @ must be followed by an integer to be a long-term identifier.\n");
                            break;
                        }
                        value_id = lti_exists(lexer.current_lexeme.int_val);
                        if (value_id == NIL)
                        {
                            good_cue = false;
                            (*err_msg)->append("Error: LTI was not found.\n");
                            break;
                        }
                        else
                        {
                            /* Not sure what we'd want to use here.  Will create an identifier for now with lti_id */
                            value = thisAgent->symbolManager->make_new_identifier('L', 1);
                            value->id->LTI_ID = lexer.current_lexeme.int_val;
                        }
                        lexer.get_lexeme();
                    }
                    else if (lexer.current_lexeme.type == VARIABLE_LEXEME || lexer.current_lexeme.type == IDENTIFIER_LEXEME)
                    {
                        std::map<std::basic_string<char>, Symbol*>::iterator value_iterator;
                        value_iterator = cue_ids.find(lexer.current_lexeme.string());

                        if (value_iterator == cue_ids.end())
                        {
                            value = thisAgent->symbolManager->make_new_identifier((char) lexer.current_lexeme.string()[0], 1);
                            cue_ids[lexer.current_lexeme.string()] = value; //Keep track of created symbols for deletion later.
                        }
                        lexer.get_lexeme();
                    }
                    else
                    {
                        if (((lexer.current_lexeme.type == R_PAREN_LEXEME || lexer.current_lexeme.type == UP_ARROW_LEXEME) || lexer.current_lexeme.type == MINUS_LEXEME) && hasAddedValue)
                        {
                            //good_cue = true;
                            break;
                        }
                        else
                        {
                            good_cue = false;
                            break;
                        }
                    }

                    if (value != NIL && good_cue)
                    {
                        //Value might be nil, but R_paren or next attribute could make it a good cue.
                        hasAddedValue = true;
                        if (minus)
                        {
                            temp_wme = make_wme(thisAgent, negative_cues, attribute, value, false);
                        }
                        else
                        {
                            temp_wme = make_wme(thisAgent, root_cue_id, attribute, value, false);
                        }
                        insert_at_head_of_dll(temp_slot->wmes, temp_wme, next, prev); //Put the wme in the slow for the attribute.
                    }
                }
                while (value != NIL);  //Loop until there are no more value lexemes to add to that attribute.

                first_attribute = false;

            }
        }
        else
        {
            break;
        }

        while (lexer.current_lexeme.type == R_PAREN_LEXEME)
        {
            lexer.get_lexeme();
        }

        clause_count++;
        trigger_first = false; //It is no longer the first clause.

    }
    if (!good_cue)
    {
        std::string num;
        to_string(clause_count, num);

        (*err_msg)->append("Error parsing clause #");
        (*err_msg)->append(num + ".");
    }
    else
    {
        id_set* prohibit = new id_set;
        wme_set cue_wmes;
        symbol_triple_list meta_wmes;
        symbol_triple_list retrieval_wmes;
        (*result_message) = new std::string();

        std::list<uint64_t> match_ids;

        process_query(NIL, root_cue_id, minus_ever ? negative_cues : NIL, NIL, prohibit, cue_wmes, meta_wmes, retrieval_wmes, qry_search, number_to_retrieve, &(match_ids), 1, fake_install);

        if (!match_ids.empty())
        {
            for (std::list<uint64_t>::const_iterator id = match_ids.begin(), end = match_ids.end(); id != end; ++id)
            {
               print_lti((*id), 1, *result_message); //"1" is the depth.
            }
        }
        else
        {
            (*result_message)->append("SMem| No results for query.");
        }
        delete prohibit;

    }

    /*
     * Below is the clean-up
     */
    if (root_cue_id != NIL)
    {
        slot* s;

        for (s = root_cue_id->id->slots; s != NIL; s = s->next)
        {
            //Remove all wme's from the slot.
            wme* delete_wme;
            for (delete_wme = s->wmes; delete_wme != NIL; delete_wme = delete_wme->next)
            {
                thisAgent->symbolManager->symbol_remove_ref(&delete_wme->value);
                deallocate_wme(thisAgent, delete_wme);
            }

            s->wmes = NIL;
            thisAgent->symbolManager->symbol_remove_ref(&s->attr);
            mark_slot_for_possible_removal(thisAgent, s);
        }//End of for-slots loop.
        root_cue_id->id->slots = NIL;

        for (s = negative_cues->id->slots; s != NIL; s = s->next)
        {
            //Remove all wme's from the slot.
            wme* delete_wme;
            for (delete_wme = s->wmes; delete_wme != NIL; delete_wme = delete_wme->next)
            {
                thisAgent->symbolManager->symbol_remove_ref(&delete_wme->value);
                deallocate_wme(thisAgent, delete_wme);
            }

            s->wmes = NIL;
            thisAgent->symbolManager->symbol_remove_ref(&s->attr);
            mark_slot_for_possible_removal(thisAgent, s);
        }//End of for-slots loop.
        negative_cues->id->slots = NIL;

        thisAgent->symbolManager->symbol_remove_ref(&root_cue_id);//gets rid of cue id.
        thisAgent->symbolManager->symbol_remove_ref(&negative_cues);//gets rid of negative cues id.
    }

    return good_cue;
}

void init_ltm_value_lti(ltm_value_lti& lti)
{
    lti.val_type = smem_cue_element_type_none;
    lti.val_value = NULL;
}

void init_ltm_value_constant(ltm_value_const& constant)
{
    constant.val_type = smem_cue_element_type_none;
    constant.val_value = NULL;
}

/*
 * This is intended to allow the user to remove part or all of information stored on a LTI.
 * (All attributes, selected attributes, or just values from particular attributes.)
 */
bool SMem_Manager::process_smem_remove(const char* ltms_str, std::string** err_msg, std::string** result_message, bool force)
{
    //TODO: need to fix so that err_msg and result_message are actually used or not passed.
    bool good_command = true;

    //parsing ltms requires an open semantic database
    attach();
    clear_instance_mappings();

    soar::Lexer lexer(thisAgent, ltms_str);

    lexer.get_lexeme();

    if (lexer.current_lexeme.type == L_PAREN_LEXEME)
    {
        lexer.get_lexeme();//Consumes the left paren
    }

    if (lexer.current_lexeme.type == AT_LEXEME && good_command)
    {
        lexer.get_lexeme();
    }

    uint64_t lti_id;
    ltm_object* l_ltm;

    if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
    {
        lti_id = lexer.current_lexeme.int_val;
    }
    else
    {
        good_command = false;
        (*err_msg)->append("Error: The lti id must be an integer.\n");
    }

    if (!lti_exists(lti_id))
    {
        good_command = false;
        (*err_msg)->append("Error: No LTI found for that id.\n");
    }

    symbol_triple_list retrieval_wmes;
    symbol_triple_list meta_wmes;

    if (good_command)
    {
        lexer.get_lexeme();//Consume the integer lti id.

        ltm_slot_map children;

        if (lexer.current_lexeme.type == UP_ARROW_LEXEME)
        {
            //Now that we know we have a good lti, we can do a NCBR so that we know what attributes and values we can delete.
            //"--force" will ignore attempts to delete that which isn't there, while the default will be to stop and report back.
            install_memory(NIL, lti_id, NULL, false, meta_wmes, retrieval_wmes, fake_install);
            //First, we'll create the slot_map according to retrieval_wmes, then we'll remove what we encounter during parsing.
            symbol_triple_list::iterator triple_ptr_iter;
            ltm_slot* temp_slot;
            for (triple_ptr_iter = retrieval_wmes.begin(); triple_ptr_iter != retrieval_wmes.end(); triple_ptr_iter++)
            {
                if (children.count((*triple_ptr_iter)->attr)) //If the attribute is already in the map.
                {
                    temp_slot = (children.find((*triple_ptr_iter)->attr)->second);
                    ltm_value* temp_val = new ltm_value;
                    if ((*triple_ptr_iter)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
                    {
                        //If the ltm was retrieved and it is an identifier it is lti.
                        ltm_value_lti temp_lti;
                        ltm_value_const temp_const;

                        temp_lti.val_type = smem_cue_element_type_none;
                        temp_lti.val_value = NULL;
                        temp_const.val_type = smem_cue_element_type_none;
                        temp_const.val_value = NULL;


                        temp_val->val_const = temp_const;
                        temp_val->val_const.val_type = value_lti_t;
                        temp_val->val_lti = temp_lti;
                        temp_val->val_lti.val_type = value_lti_t;
                        ltm_object* temp_ltm = new ltm_object;
                        temp_ltm->lti_id = (*triple_ptr_iter)->value->id->LTI_ID;
                        temp_ltm->soar_id = (*triple_ptr_iter)->value;
                        temp_val->val_lti.val_value = temp_ltm;
                    }
                    else //If the value is not an identifier, then it is a "constant".
                    {
                        ltm_value_lti temp_lti;
                        ltm_value_const temp_const;

                        temp_lti.val_type = smem_cue_element_type_none;
                        temp_lti.val_value = NULL;
                        temp_const.val_type = smem_cue_element_type_none;
                        temp_const.val_value = NULL;

                        temp_val->val_lti = temp_lti;
                        temp_val->val_lti.val_type = value_const_t;
                        temp_val->val_const.val_type = value_const_t;
                        temp_val->val_const.val_value = (*triple_ptr_iter)->value;
                    }
                    temp_slot->push_back(temp_val);
                }
                else //If the attribute is not in the map and we need to make a slot.
                {
                    temp_slot = new ltm_slot;
                    ltm_value* temp_val = new ltm_value;
                    if ((*triple_ptr_iter)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
                    {
                        //If the ltm was retrieved and it is an identifier it is lti.
                        ltm_value_lti temp_lti;
                        ltm_value_const temp_const;

                        temp_lti.val_type = smem_cue_element_type_none;
                        temp_lti.val_value = NULL;
                        temp_const.val_type = smem_cue_element_type_none;
                        temp_const.val_value = NULL;

                        temp_val->val_const = temp_const;
                        temp_val->val_const.val_type = value_lti_t;
                        temp_val->val_lti = temp_lti;
                        temp_val->val_lti.val_type = value_lti_t;
                        ltm_object* temp_ltm = new ltm_object;
                        temp_ltm->lti_id = (*triple_ptr_iter)->value->id->LTI_ID;
                        temp_ltm->soar_id = (*triple_ptr_iter)->value;
                        temp_val->val_lti.val_value = temp_ltm;
                    }
                    else //If the value is nt an identifier, then it is a "constant".
                    {
                        ltm_value_lti temp_lti;
                        ltm_value_const temp_const;

                        temp_lti.val_type = smem_cue_element_type_none;
                        temp_lti.val_value = NULL;
                        temp_const.val_type = smem_cue_element_type_none;
                        temp_const.val_value = NULL;

                        temp_val->val_lti = temp_lti;
                        temp_val->val_lti.val_type = value_const_t;
                        temp_val->val_const.val_type = value_const_t;
                        temp_val->val_const.val_value = (*triple_ptr_iter)->value;
                    }
                    temp_slot->push_back(temp_val);
                    children[(*triple_ptr_iter)->attr] = temp_slot;
                }
            }

            //Now we process attributes one at a time.
            while (lexer.current_lexeme.type == UP_ARROW_LEXEME && (good_command || force))
            {
                lexer.get_lexeme();// Consume the up arrow.

                Symbol* attribute = NIL;

                if (lexer.current_lexeme.type == STR_CONSTANT_LEXEME)
                {
                    attribute = thisAgent->symbolManager->find_str_constant(static_cast<const char*>(lexer.current_lexeme.string()));
                }
                else if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
                {
                    attribute = thisAgent->symbolManager->find_int_constant(lexer.current_lexeme.int_val);
                }
                else if (lexer.current_lexeme.type == FLOAT_CONSTANT_LEXEME)
                {
                    attribute = thisAgent->symbolManager->find_float_constant(lexer.current_lexeme.float_val);
                }

                if (attribute == NIL)
                {
                    good_command = false;
                    (*err_msg)->append("Error: Attribute was not found.\n");
                }
                else
                {
                    lexer.get_lexeme();//Consume the attribute.
                    good_command = true;
                }

                if (good_command && (lexer.current_lexeme.type != UP_ARROW_LEXEME && lexer.current_lexeme.type != R_PAREN_LEXEME)) //If there are values.
                {
                    Symbol* value;
                    do //Add value by type
                    {
                        value = NIL;
                        if (lexer.current_lexeme.type == STR_CONSTANT_LEXEME)
                        {
                            value = thisAgent->symbolManager->find_str_constant(static_cast<const char*>(lexer.current_lexeme.string()));
                            lexer.get_lexeme();
                        }
                        else if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
                        {
                            value = thisAgent->symbolManager->find_int_constant(lexer.current_lexeme.int_val);
                            lexer.get_lexeme();
                        }
                        else if (lexer.current_lexeme.type == FLOAT_CONSTANT_LEXEME)
                        {
                            value = thisAgent->symbolManager->find_float_constant(lexer.current_lexeme.float_val);
                            lexer.get_lexeme();
                        }
                        else if (lexer.current_lexeme.type == AT_LEXEME)
                        {
                            lexer.get_lexeme();
                            if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
                            {
                                value = get_current_iSTI_for_LTI(lexer.current_lexeme.int_val, NO_WME_LEVEL);
                                lexer.get_lexeme();
                            }
                            else
                            {
                                (*err_msg)->append("Error: '@' should be followed by an integer lti id.\n");
                                good_command = false;
                                break;
                            }
                        }
                        else
                        {
                            good_command = (lexer.current_lexeme.type == R_PAREN_LEXEME || lexer.current_lexeme.type == UP_ARROW_LEXEME);
                            if (!good_command)
                            {
                                (*err_msg)->append("Error: Expected ')' or '^'.\n... Neither was found.\n");
                            }
                        }

                        if (value != NIL && good_command) //Value might be nil, but that can be just fine.
                        {
                            //Given a value for this attribute, we have a symbol triple to remove.
                            ltm_slot::iterator values;
                            for (values = (children.find(attribute))->second->begin(); values != (children.find(attribute))->second->end(); values++)
                            {
                                if (value->symbol_type == IDENTIFIER_SYMBOL_TYPE && (*values)->val_lti.val_type == value_lti_t)
                                {
                                    if ((*values)->val_lti.val_value->soar_id == value)
                                    {
                                        delete(*values)->val_lti.val_value;
                                        delete *values;
                                        (*(children.find(attribute))).second->erase(values);
                                        break;
                                    }
                                }
                                else if (value->symbol_type != IDENTIFIER_SYMBOL_TYPE && (*values)->val_const.val_type == value_const_t)
                                {
                                    if ((*values)->val_const.val_value == value)
                                    {
                                        delete *values;
                                        (*(children.find(attribute))).second->erase(values);
                                        break;
                                    }
                                }
                            }
                            if (values == (children.find(attribute))->second->end())
                            {
                                (*err_msg)->append("Error: Value does not exist on attribute.\n");
                            }
                        }
                        else
                        {
                            if ((good_command && !force) && (lexer.current_lexeme.type != R_PAREN_LEXEME && lexer.current_lexeme.type != UP_ARROW_LEXEME))
                            {
                                (*err_msg)->append("Error: Attribute contained a value that could not be found.\n");
                                break;
                            }
                        }
                    }
                    while (good_command && (value != NIL || !(lexer.current_lexeme.type == R_PAREN_LEXEME || lexer.current_lexeme.type == UP_ARROW_LEXEME)));
                }
                else if (good_command && children.find(attribute) != children.end()) //If we didn't have any values, then we just get rid of everything on the attribute.
                {
                    ltm_slot* result = (children.find(attribute))->second;
                    ltm_slot::iterator values, end = result->end();
                    for (values = (children.find(attribute))->second->begin(); values != end; values++)
                    {
                        delete *values;
                    }
                    children.erase(attribute);
                }
                if (force)
                {
                    while ((lexer.current_lexeme.type != EOF_LEXEME && lexer.current_lexeme.type != UP_ARROW_LEXEME) && lexer.current_lexeme.type != R_PAREN_LEXEME) //Loop until the lexeme is EOF, another ^, or ")".
                    {
                        lexer.get_lexeme();
                    }
                }
            }
        }
        if (good_command && lexer.current_lexeme.type == R_PAREN_LEXEME)
        {
            store_LTM_in_DB(lti_id, &(children), true, NULL, false);
        }
        else if (good_command)
        {
            (*err_msg)->append("Error: Expected a ')'.\n");
        }

        //Clean up.
        ltm_slot_map::iterator attributes, end = children.end();
        for (attributes = children.begin(); attributes != end; attributes++)
        {
            ltm_slot* result = (children.find(attributes->first))->second;
            ltm_slot::iterator values, end = result->end();
            for (values = result->begin(); values != end; values++)
            {
                if ((*values)->val_lti.val_type == value_lti_t)
                {
                    delete(*values)->val_lti.val_value;
                }
                delete *values;
            }
            delete attributes->second;
        }

        symbol_triple_list::iterator triple_iterator, end2 = retrieval_wmes.end();
        for (triple_iterator = retrieval_wmes.begin(); triple_iterator != end2; triple_iterator++)
        {
            thisAgent->symbolManager->symbol_remove_ref(&(*triple_iterator)->id);
            thisAgent->symbolManager->symbol_remove_ref(&(*triple_iterator)->attr);
            thisAgent->symbolManager->symbol_remove_ref(&(*triple_iterator)->value);
            delete *triple_iterator;
        }
    }
    return good_command;
}

ltm_slot* SMem_Manager::make_ltm_slot(ltm_slot_map* slots, Symbol* attr)
{
    ltm_slot** s = & (*slots)[ attr ];

    if (!(*s))
    {
        (*s) = new ltm_slot;
    }

    return (*s);
}

void SMem_Manager::disconnect_ltm(uint64_t pLTI_ID)
{
    // adjust attr, attr/value counts
    {
        uint64_t pair_count = 0;

        uint64_t child_attr = 0;
        std::set<uint64_t> distinct_attr;

        // pairs first, accumulate distinct attributes and pair count
        SQL->web_all->bind_int(1, pLTI_ID);
        while (SQL->web_all->execute() == soar_module::row)
        {
            pair_count++;

            child_attr = SQL->web_all->column_int(0);
            distinct_attr.insert(child_attr);

            // null -> attr/lti
            if (SQL->web_all->column_int(1) != SMEM_AUGMENTATIONS_NULL)
            {
                // adjust in opposite direction ( adjust, attribute, const )
                SQL->wmes_constant_frequency_update->bind_int(1, -1);
                SQL->wmes_constant_frequency_update->bind_int(2, child_attr);
                SQL->wmes_constant_frequency_update->bind_int(3, SQL->web_all->column_int(1));
                SQL->wmes_constant_frequency_update->execute(soar_module::op_reinit);
            }
            else
            {
                // adjust in opposite direction ( adjust, attribute, lti )
                SQL->wmes_lti_frequency_update->bind_int(1, -1);
                SQL->wmes_lti_frequency_update->bind_int(2, child_attr);
                SQL->wmes_lti_frequency_update->bind_int(3, SQL->web_all->column_int(2));
                SQL->wmes_lti_frequency_update->execute(soar_module::op_reinit);
            }
        }
        SQL->web_all->reinitialize();

        // now attributes
        for (std::set<uint64_t>::iterator a = distinct_attr.begin(); a != distinct_attr.end(); a++)
        {
            // adjust in opposite direction ( adjust, attribute )
            SQL->attribute_frequency_update->bind_int(1, -1);
            SQL->attribute_frequency_update->bind_int(2, *a);
            SQL->attribute_frequency_update->execute(soar_module::op_reinit);
        }

        // update local statistic
        statistics->edges->set_value(statistics->edges->get_value() - pair_count);
    }

    // disconnect
    {
        SQL->web_truncate->bind_int(1, pLTI_ID);
        SQL->web_truncate->execute(soar_module::op_reinit);
    }
}

void SMem_Manager::store_LTM_in_DB(uint64_t pLTI_ID, ltm_slot_map* children, bool remove_old_children, Symbol* print_id, bool activate, bool preserve_previous_link)
{
    // if remove children, disconnect ltm -> no existing edges
    // else, need to query number of existing edges
    uint64_t existing_edges = 0;
    if (remove_old_children)
    {
        disconnect_ltm(pLTI_ID);

        // provide trace output
        if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ] && (print_id))
        {
            char buf[256];

            snprintf_with_symbols(thisAgent, buf, 256, "<=SMEM: (%y ^* *)\n", print_id);

            print(thisAgent, buf);
            xml_generate_warning(thisAgent, buf);
        }
    }
    else
    {
        SQL->act_lti_child_ct_get->bind_int(1, pLTI_ID);
        SQL->act_lti_child_ct_get->execute();

        existing_edges = static_cast<uint64_t>(SQL->act_lti_child_ct_get->column_int(0));

        SQL->act_lti_child_ct_get->reinitialize();
    }

    // get new edges
    // if didn't disconnect, entails lookups in existing edges
    std::set<smem_hash_id> attr_new;
    std::set< std::pair<smem_hash_id, smem_hash_id> > const_new;
    std::set< std::pair<smem_hash_id, uint64_t> > lti_new;
    {
        ltm_slot_map::iterator s;
        ltm_slot::iterator v;

        smem_hash_id attr_hash = 0;
        smem_hash_id value_hash = 0;
        uint64_t value_lti = 0;

        for (s = children->begin(); s != children->end(); s++)
        {
            attr_hash = hash(s->first);
            if (remove_old_children)
            {
                attr_new.insert(attr_hash);
            }
            else
            {
                // lti_id, attribute_s_id
                assert(pLTI_ID && attr_hash);
                SQL->web_attr_child->bind_int(1, pLTI_ID);
                SQL->web_attr_child->bind_int(2, attr_hash);
                if (SQL->web_attr_child->execute(soar_module::op_reinit) != soar_module::row)
                {
                    attr_new.insert(attr_hash);
                }
            }

            for (v = s->second->begin(); v != s->second->end(); v++)
            {
                if ((*v)->val_const.val_type == value_const_t)
                {
                    value_hash = hash((*v)->val_const.val_value);

                    if (remove_old_children)
                    {
                        const_new.insert(std::make_pair(attr_hash, value_hash));
                    }
                    else
                    {
                        // lti_id, attribute_s_id, val_const
                        assert(pLTI_ID && attr_hash && value_hash);
                        SQL->web_const_child->bind_int(1, pLTI_ID);
                        SQL->web_const_child->bind_int(2, attr_hash);
                        SQL->web_const_child->bind_int(3, value_hash);
                        if (SQL->web_const_child->execute(soar_module::op_reinit) != soar_module::row)
                        {
                            const_new.insert(std::make_pair(attr_hash, value_hash));
                        }
                    }

                    // provide trace output
                    if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ] && (print_id))
                    {
                        char buf[256];

                        snprintf_with_symbols(thisAgent, buf, 256, "=>SMEM: (%y ^%y %y)\n", print_id, s->first, (*v)->val_const.val_value);

                        print(thisAgent, buf);
                        xml_generate_warning(thisAgent, buf);
                    }
                }
                else
                {
                    value_lti = (*v)->val_lti.val_value->lti_id;
                    if (value_lti == NIL)
                    {
                        if ((*v)->val_lti.val_value->soar_id != NIL)
                        {
                            (*v)->val_lti.val_value->lti_id = make_STI_instance_of_new_LTI((*v)->val_lti.val_value->soar_id, preserve_previous_link);
                        } else {
                            (*v)->val_lti.val_value->lti_id = add_new_LTI();
                        }
                        value_lti = (*v)->val_lti.val_value->lti_id;
                    }

                    if (remove_old_children)
                    {
                        lti_new.insert(std::make_pair(attr_hash, value_lti));
                    }
                    else
                    {
                        // lti_id, attribute_s_id, val_lti
                        assert(pLTI_ID && attr_hash && value_lti);
                        SQL->web_lti_child->bind_int(1, pLTI_ID);
                        SQL->web_lti_child->bind_int(2, attr_hash);
                        SQL->web_lti_child->bind_int(3, value_lti);
                        if (SQL->web_lti_child->execute(soar_module::op_reinit) != soar_module::row)
                        {
                            lti_new.insert(std::make_pair(attr_hash, value_lti));
                        }
                    }

                    // provide trace output
                    if (thisAgent->sysparams[ TRACE_SMEM_SYSPARAM ] && (print_id))
                    {
                        char buf[256];

                        snprintf_with_symbols(thisAgent, buf, 256, "=>SMEM: (%y ^%y %y)\n", print_id, s->first, (*v)->val_lti.val_value->soar_id);

                        print(thisAgent, buf);
                        xml_generate_warning(thisAgent, buf);
                    }
                }
            }
        }
    }

    // activation function assumes proper thresholding state
    // thus, consider four cases of augmentation counts (w.r.t. thresh)
    // 1. before=below, after=below: good (activation will update smem_augmentations)
    // 2. before=below, after=above: need to update smem_augmentations->inf
    // 3. before=after, after=below: good (activation will update smem_augmentations, free transition)
    // 4. before=after, after=after: good (activation won't touch smem_augmentations)
    //
    // hence, we detect + handle case #2 here
    uint64_t new_edges = (existing_edges + const_new.size() + lti_new.size());
    bool after_above;
    double web_act = static_cast<double>(SMEM_ACT_MAX);
    {
        uint64_t thresh = static_cast<uint64_t>(settings->thresh->get_value());
        after_above = (new_edges >= thresh);

        // if before below
        if (existing_edges < thresh)
        {
            if (after_above)
            {
                // update smem_augmentations to inf
                SQL->act_set->bind_double(1, web_act);
                SQL->act_set->bind_int(2, pLTI_ID);
                SQL->act_set->execute(soar_module::op_reinit);
            }
        }
    }

    // update edge counter
    {
        SQL->act_lti_child_ct_set->bind_int(1, new_edges);
        SQL->act_lti_child_ct_set->bind_int(2, pLTI_ID);
        SQL->act_lti_child_ct_set->execute(soar_module::op_reinit);
    }

    // now we can safely activate the lti
    if (activate)
    {
        double lti_act = lti_activate(pLTI_ID, true, new_edges);

        if (!after_above)
        {
            web_act = lti_act;
        }
    }

    // insert new edges, update counters
    {
        // attr/const pairs
        {
            for (std::set< std::pair< smem_hash_id, smem_hash_id > >::iterator p = const_new.begin(); p != const_new.end(); p++)
            {
                // insert
                {
                    // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
                    SQL->web_add->bind_int(1, pLTI_ID);
                    SQL->web_add->bind_int(2, p->first);
                    SQL->web_add->bind_int(3, p->second);
                    SQL->web_add->bind_int(4, SMEM_AUGMENTATIONS_NULL);
                    SQL->web_add->bind_double(5, web_act);
                    SQL->web_add->execute(soar_module::op_reinit);
                }

                // update counter
                {
                    // check if counter exists (and add if does not): attribute_s_id, val
                    SQL->wmes_constant_frequency_check->bind_int(1, p->first);
                    SQL->wmes_constant_frequency_check->bind_int(2, p->second);
                    if (SQL->wmes_constant_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                    {
                        SQL->wmes_constant_frequency_add->bind_int(1, p->first);
                        SQL->wmes_constant_frequency_add->bind_int(2, p->second);
                        SQL->wmes_constant_frequency_add->execute(soar_module::op_reinit);
                    }
                    else
                    {
                        // adjust count (adjustment, attribute_s_id, val)
                        SQL->wmes_constant_frequency_update->bind_int(1, 1);
                        SQL->wmes_constant_frequency_update->bind_int(2, p->first);
                        SQL->wmes_constant_frequency_update->bind_int(3, p->second);
                        SQL->wmes_constant_frequency_update->execute(soar_module::op_reinit);
                    }
                }
            }
        }

        // attr/lti pairs
        {
            for (std::set< std::pair< smem_hash_id, uint64_t > >::iterator p = lti_new.begin(); p != lti_new.end(); p++)
            {
                // insert
                {
                    // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
                    SQL->web_add->bind_int(1, pLTI_ID);
                    SQL->web_add->bind_int(2, p->first);
                    SQL->web_add->bind_int(3, SMEM_AUGMENTATIONS_NULL);
                    SQL->web_add->bind_int(4, p->second);
                    SQL->web_add->bind_double(5, web_act);
                    SQL->web_add->execute(soar_module::op_reinit);
                }

                // update counter
                {
                    // check if counter exists (and add if does not): attribute_s_id, val
                    SQL->wmes_lti_frequency_check->bind_int(1, p->first);
                    SQL->wmes_lti_frequency_check->bind_int(2, p->second);
                    if (SQL->wmes_lti_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                    {
                        SQL->wmes_lti_frequency_add->bind_int(1, p->first);
                        SQL->wmes_lti_frequency_add->bind_int(2, p->second);
                        SQL->wmes_lti_frequency_add->execute(soar_module::op_reinit);
                    }
                    else
                    {
                        // adjust count (adjustment, attribute_s_id, lti)
                        SQL->wmes_lti_frequency_update->bind_int(1, 1);
                        SQL->wmes_lti_frequency_update->bind_int(2, p->first);
                        SQL->wmes_lti_frequency_update->bind_int(3, p->second);
                        SQL->wmes_lti_frequency_update->execute(soar_module::op_reinit);
                    }
                }
            }
        }

        // update attribute count
        {
            for (std::set< smem_hash_id >::iterator a = attr_new.begin(); a != attr_new.end(); a++)
            {
                // check if counter exists (and add if does not): attribute_s_id
                SQL->attribute_frequency_check->bind_int(1, *a);
                if (SQL->attribute_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                {
                    SQL->attribute_frequency_add->bind_int(1, *a);
                    SQL->attribute_frequency_add->execute(soar_module::op_reinit);
                }
                else
                {
                    // adjust count (adjustment, attribute_s_id)
                    SQL->attribute_frequency_update->bind_int(1, 1);
                    SQL->attribute_frequency_update->bind_int(2, *a);
                    SQL->attribute_frequency_update->execute(soar_module::op_reinit);
                }
            }
        }

        // update local edge count
        {
            statistics->edges->set_value(statistics->edges->get_value() + (const_new.size() + lti_new.size()));
        }
    }
}

void SMem_Manager::store_LTM(Symbol* pIdentifierSTI, smem_storage_type store_type, bool update_smem, tc_number tc)
{
    // transitive closure only matters for recursive storage
    if ((store_type == store_recursive) && (tc == NIL))
    {
        tc = get_new_tc_number(thisAgent);
    }
    symbol_list shorties;

    // get level
    wme_list* children = get_direct_augs_of_id(pIdentifierSTI, tc);
    wme_list::iterator w;

    // make the target an lti, so intermediary data structure has lti_id
    // (takes care of short-term id self-referencing)
    make_STI_instance_of_new_LTI(pIdentifierSTI, update_smem);

    // encode this level
    {
        sym_to_ltm_map sym_to_ltm;
        sym_to_ltm_map::iterator c_p;
        ltm_object** c;

        ltm_slot_map slots;
        ltm_slot_map::iterator s_p;
        ltm_slot::iterator v_p;
        ltm_slot* s;
        ltm_value* v;

        for (w = children->begin(); w != children->end(); w++)
        {
            // get slot
            s = make_ltm_slot(&(slots), (*w)->attr);

            // create value, per type
            v = new ltm_value;
            if ((*w)->value->is_constant())
            {
                v->val_const.val_type = value_const_t;
                v->val_const.val_value = (*w)->value;
            }
            else
            {
                v->val_lti.val_type = value_lti_t;

                /* This seems like funky map usage.  Following line will create entry in map if it doesn't
                 * exist.  This works because following code will use created entry anyway.  Should use
                 * iterator and find, then create if necessary. */
                c = & sym_to_ltm[(*w)->value ];

                // if doesn't exist, add; else use existing
                if (!(*c))
                {
                    (*c) = new ltm_object;
                    if (update_smem)
                    {
                        (*c)->lti_id = (*w)->value->id->LTI_ID;
                    } else {
                        (*c)->lti_id = NIL;
                    }
                    (*c)->slots = NULL;
                    (*c)->soar_id = (*w)->value;

                    // only traverse to short-term identifiers
                    if ((store_type == store_recursive) && ((*c)->lti_id == NIL))
                    {
                        shorties.push_back((*c)->soar_id);
                    }
                }

                v->val_lti.val_value = (*c);
            }

            // add value to slot
            s->push_back(v);
        }

        store_LTM_in_DB(pIdentifierSTI->id->LTI_ID, &(slots), true, pIdentifierSTI, true, update_smem);

        // clean up
        {
            // de-allocate slots
            for (s_p = slots.begin(); s_p != slots.end(); s_p++)
            {
                for (v_p = s_p->second->begin(); v_p != s_p->second->end(); v_p++)
                {
                    delete(*v_p);
                }

                delete s_p->second;
            }

            // de-allocate ltms
            for (c_p = sym_to_ltm.begin(); c_p != sym_to_ltm.end(); c_p++)
            {
                delete c_p->second;
            }

            delete children;
        }
    }

    // recurse as necessary
    for (symbol_list::iterator shorty = shorties.begin(); shorty != shorties.end(); shorty++)
    {
        store_LTM((*shorty), store_recursive, update_smem, tc);
    }
}
