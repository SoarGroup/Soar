/*
 * smem_clI_commands.cpp
 *
 *  Created on: Sep 5, 2016
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
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "slot.h"
#include "symbol_manager.h"
#include "working_memory.h"
#include "xml.h"

bool SMem_Manager::CLI_add(const char* ltms_str, std::string** err_msg)
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
                    (*c_new)->lti_id = add_new_LTI();
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
                   LTM_to_DB((*c_new)->lti_id, (*c_new)->slots, false, false);
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

bool SMem_Manager::CLI_query(const char* ltms_str, std::string** err_msg, std::string** result_message, uint64_t number_to_retrieve)
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
                            value->id->smem_valid = smem_validation;
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
        std::list<Symbol*> root_cue_id_list;
        root_cue_id_list.push_back(root_cue_id);
        (*result_message) = new std::string();

        std::list<uint64_t> match_ids;
        command_line_activation_metadata acts;

        //adding stuff to support attention here. Ideally, this would actually be another parameter/argument passed through smem -q command. However,
        // for now I'm implementing this by having the command (here) instead inspect the state for an ^attention command and just using that.
        //thisAgent->bottom_goal->id->smem_info->cmd_wme->value->
        Symbol* attention_root = NIL;
        slot* smem_cmd_slots = thisAgent->bottom_goal->id->smem_info->cmd_wme->value->id->slots;
        while (smem_cmd_slots != NIL)
        {
            if (strcmp(smem_cmd_slots->attr->sc->name,"attention"))
            {
                smem_cmd_slots = smem_cmd_slots->next;
            }
            else
            {
                break;
            }
        }
        if (smem_cmd_slots != NIL)
        {
            attention_root = smem_cmd_slots->wmes->value;
        }

        process_query(NIL, root_cue_id_list, minus_ever ? negative_cues : NIL, NIL, prohibit, cue_wmes, meta_wmes, retrieval_wmes, qry_search, number_to_retrieve, &(match_ids), 1, fake_install, &(acts), attention_root);
        //This function is being modified to make it easier to debug spreading activation. Currently, it is difficult to inspect what are the different contributions to a retrieved node's activation.
        //"acts", the final argument of this function/method, was previously only a list of activation doubles. Now, it is a complex struct. It contains activation data that is broken into constituent parts.
        //The difficulty is that this struct must also be passed into the functions that update activations for use in process_query. In other words, I have to pass this again so that the activation data can be "record-kept"
        //a whole 'nother function deep.

        //What gets reported here after the results is a complicated list of activation data beyond what was previously only a list of activation values.
        if (!match_ids.empty())
        {
            for (std::list<uint64_t>::const_iterator id = match_ids.begin(), end = match_ids.end(); id != end; ++id)
            {
                print_LTM((*id), 1, *result_message); //"1" is the depth.
                std::string temp_act;
                //(*result_message)->append(temp_act);
                (*result_message)->append("[Total Activation: ");
                to_string(acts.recipient_decomposition_list.find(*id)->second.activation_total, temp_act,3,false);
                (*result_message)->append(temp_act);
                (*result_message)->append(", Base-level total: ");
                to_string(acts.recipient_decomposition_list.find(*id)->second.base_level_total, temp_act,3,false);
                (*result_message)->append(temp_act);
                (*result_message)->append(", Base-level inhibition: ");
                to_string(acts.recipient_decomposition_list.find(*id)->second.base_inhibition, temp_act,3,false);
                (*result_message)->append(temp_act);
                (*result_message)->append(", Base-level: ");
                to_string(acts.recipient_decomposition_list.find(*id)->second.base_level, temp_act,3,false);
                (*result_message)->append(temp_act);
                (*result_message)->append(", Spreading Total: ");
                to_string(acts.recipient_decomposition_list.find(*id)->second.spread_total, temp_act,3,false);
                (*result_message)->append(temp_act);
                (*result_message)->append("]\n");
                (*result_message)->append("[Sources of Spread:");
                //loop over all sources. for each source, give network and wma.
                std::set<uint64_t>::iterator source_it;
                std::set<uint64_t>::iterator source_begin = acts.recipient_decomposition_list.find(*id)->second.contributing_sources.begin();
                std::set<uint64_t>::iterator source_end = acts.recipient_decomposition_list.find(*id)->second.contributing_sources.end();
                uint64_t source;
                for (source_it = source_begin; source_it != source_end; ++source_it)
                {
                    source = *source_it;

                    (*result_message)->append(" LTI ");
                    to_string(source,temp_act);
                    (*result_message)->append(temp_act);
                    (*result_message)->append(", network factor ");
                    to_string(acts.recipient_decomposition_list.find(*id)->second.source_to_network_factor.find(source)->second, temp_act,4,false);
                    (*result_message)->append(temp_act);
                    (*result_message)->append(", WMA factor ");
                    to_string(acts.contributing_sources_to_WMA_factors.find(source)->second, temp_act,4,false);
                    (*result_message)->append(temp_act);
                    (*result_message)->append(", Attention factor ");
                    to_string(acts.contributing_sources_to_Attention_factors.find(source)->second, temp_act,4,false);
                    (*result_message)->append(temp_act);
                    (*result_message)->append(";");
                }
                (*result_message)->append("]\n");
                //Now, for each source, we also find each instance and compute here its wma.
            }
            /*(*result_message)->append("Activation values");
            for (std::list<double>::const_iterator act = acts.begin(), end = acts.end(); act != end; ++act)
            {
                (*result_message)->append(", ");
                std::string temp_act;
                to_string(*act, temp_act);
                (*result_message)->append(temp_act);
            }*/
            /*
             *
             *
             *
             * activation
             *
             *
             *
             *
             *
             * stuff
             *
             *
             *
             *
             */
        }
        else
        {
            (*result_message)->append("SMem| No results for query.");
        }
        // clear cache
        {
            symbol_triple_list::iterator mw_it;

            for (mw_it = retrieval_wmes.begin(); mw_it != retrieval_wmes.end(); mw_it++)
            {
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);
                thisAgent->memoryManager->free_with_pool(MP_sym_triple, (*mw_it));
            }
            retrieval_wmes.clear();

            for (mw_it = meta_wmes.begin(); mw_it != meta_wmes.end(); mw_it++)
            {
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);
                thisAgent->memoryManager->free_with_pool(MP_sym_triple, (*mw_it));
            }
            meta_wmes.clear();
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

bool SMem_Manager::CLI_remove(const char* ltms_str, std::string** err_msg, std::string** result_message, bool force)
{
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
                        /* Can this even happen any more? */
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
//                        temp_ltm->soar_id = (*triple_ptr_iter)->value;
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
//                        temp_ltm->soar_id = (*triple_ptr_iter)->value;
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
                                if (value->is_lti() && (*values)->val_lti.val_type == value_lti_t)
                                {
                                    if ((*values)->val_lti.val_value->lti_id == value->id->LTI_ID)
                                    {
                                        delete(*values)->val_lti.val_value;
                                        delete *values;
                                        (*(children.find(attribute))).second->erase(values);
                                        break;
                                    }
                                }
                                else if (!value->is_sti() && (*values)->val_const.val_type == value_const_t)
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
            LTM_to_DB(lti_id, &(children), true, false);
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

        {
            symbol_triple_list::iterator mw_it;

            for (mw_it = retrieval_wmes.begin(); mw_it != retrieval_wmes.end(); mw_it++)
            {
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);
                thisAgent->memoryManager->free_with_pool(MP_sym_triple, (*mw_it));
            }
            retrieval_wmes.clear();

            for (mw_it = meta_wmes.begin(); mw_it != meta_wmes.end(); mw_it++)
            {
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->id);
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->attr);
                thisAgent->symbolManager->symbol_remove_ref(&(*mw_it)->value);
                thisAgent->memoryManager->free_with_pool(MP_sym_triple, (*mw_it));
            }
            meta_wmes.clear();
        }
    }
    return good_command;
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

    if ((lexer->current_lexeme.type == AT_LEXEME) || (lexer->current_lexeme.type == VARIABLE_LEXEME))
    {
        good_at = true;

        if (lexer->current_lexeme.type == AT_LEXEME)
        {
            lexer->get_lexeme();

            good_at = (lexer->current_lexeme.type == INT_CONSTANT_LEXEME);
            if (good_at)
            {
                l_ltm->lti_id = lexer->current_lexeme.int_val;
            }
        }

        if (good_at)
        {
            if (lexer->current_lexeme.type == VARIABLE_LEXEME)
            {
                l_ltm_name.append(lexer->current_lexeme.string());
                l_ltm->lti_id = NIL;
            } else {
                good_at = ((lexer->current_lexeme.type == INT_CONSTANT_LEXEME) || (lexer->current_lexeme.type == IDENTIFIER_LEXEME));
                get_lti_name(l_ltm->lti_id, l_ltm_name);
            }
            if (good_at)
            {
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
                            //                        l_ltm_temp->soar_id = NIL;

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
                                bool dont_consume = false;
                                // value by type
                                l_ltm_value = NIL;
                                if ((lexer->current_lexeme.type == STR_CONSTANT_LEXEME)  || (lexer->current_lexeme.type == IDENTIFIER_LEXEME))
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
                                else if ((lexer->current_lexeme.type == AT_LEXEME) || (lexer->current_lexeme.type == VARIABLE_LEXEME))
                                {
                                    bool mistakenLTI = false;
                                    if (lexer->current_lexeme.type == AT_LEXEME)
                                    {
                                        lexer->get_lexeme();
                                        if (lexer->current_lexeme.type == STR_CONSTANT_LEXEME)
                                        {
                                            std::string fixedString("|@");
                                            fixedString.append(lexer->current_lexeme.string());
                                            fixedString.push_back('|');
                                            l_ltm_value = new ltm_value;
                                            l_ltm_value->val_const.val_type = value_const_t;
                                            l_ltm_value->val_const.val_value = thisAgent->symbolManager->make_str_constant(fixedString.c_str());
                                            mistakenLTI = true;
                                        } else {
                                            good_at = (lexer->current_lexeme.type == INT_CONSTANT_LEXEME);
                                        }
                                    }

                                    if (good_at && !mistakenLTI)
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
                                            //                                        l_ltm_temp->soar_id = NIL;

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
                                                //lexer->get_lexeme();

                                                // possibly a newbie (could be a self-loop)
                                                newbies->insert(l_ltm_temp);
                                            }
                                        }

                                    } else {
                                        /* Bad clause.  Print it out */
                                        thisAgent->outputManager->printa_sf(thisAgent, "Value of smem -add clause for @%u is invalid: %s\n", l_ltm->lti_id, lexer->current_lexeme.string());
                                    }
                                }

                                if (l_ltm_value != NIL)
                                {
                                    // consume
                                    /*if (!dont_consume)
                                    {
                                        lexer->get_lexeme();
                                    }*/
                                    lexer->get_lexeme();
                                    if (lexer->current_lexeme.type == L_PAREN_LEXEME)
                                    {
                                        lexer->get_lexeme();
                                        if (lexer->current_lexeme.type == FLOAT_CONSTANT_LEXEME)
                                        {
                                            l_ltm_value->val_lti.edge_weight = lexer->current_lexeme.float_val;
                                        }
                                        else if (lexer->current_lexeme.type == INT_CONSTANT_LEXEME)
                                        {
                                            l_ltm_value->val_lti.edge_weight = static_cast<double>(lexer->current_lexeme.int_val);
                                        }
                                        else
                                        {
                                            thisAgent->outputManager->printa_sf(thisAgent, "Edge weight input for smem -add clause for @%u is not float or integer: %s\n", l_ltm->lti_id, lexer->current_lexeme.string());
                                            //error: The syntax suggested that an edge-weight value was going to be provided
                                            //but this was not encountered
                                        }
                                        lexer->get_lexeme();
                                        if (lexer->current_lexeme.type == R_PAREN_LEXEME)
                                        {
                                            lexer->get_lexeme();
                                        }
                                        else
                                        {
                                            //horrible syntax error. missing right paren. fix.
                                            thisAgent->outputManager->printa_sf(thisAgent, "Edge weight input for smem -add clause for @%u is missing right paren.\n", l_ltm->lti_id);
                                        }
                                    }
                                    else
                                    {
                                        /*if (lexer->current_lexeme.type == R_PAREN_LEXEME)
                                        {
                                            dont_consume = true;
                                        }*/
                                        l_ltm_value->val_lti.edge_weight = 0;
                                    }

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
                        else
                        {
                            thisAgent->outputManager->printa_sf(thisAgent, "Attribute of smem -add clause for @%u is invalid: %s\n", l_ltm->lti_id, lexer->current_lexeme.string());
                        }
                    }
                }
            }
            else
            {
                thisAgent->outputManager->printa_sf(thisAgent, "Identifier of smem -add clause is invalid: %s\n", lexer->current_lexeme.string());
                delete l_ltm;
            }
        }
        else
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Identifier of smem -add clause is invalid: %s\n", lexer->current_lexeme.string());
            delete l_ltm;
        }
    }
    else if (lexer->current_lexeme.type == IDENTIFIER_LEXEME)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Identifier of smem -add clause is invalid: %s\n", lexer->current_lexeme.string());
        delete l_ltm;
    }
    else
    {
        delete l_ltm;
    }

    if (return_val)
    {
        // search for an existing ltm (occurs if value comes before id)
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

uint64_t SMem_Manager::spread_size()
{
    SQL->calc_spread_size_debug_cmd->execute();
    uint64_t number_spread_elements = SQL->calc_spread_size_debug_cmd->column_int(0);
    SQL->calc_spread_size_debug_cmd->reinitialize();
    return number_spread_elements;
}
