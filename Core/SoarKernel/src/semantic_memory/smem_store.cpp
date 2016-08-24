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

void SMem_Manager::deallocate_chunk(smem_chunk* chunk, bool free_chunk )
{
    if (chunk)
    {
        // proceed to slots
        if (chunk->slots)
        {
            smem_slot_map::iterator s;
            smem_slot::iterator v;
            Symbol* lSym;
            // iterate over slots
            while (!chunk->slots->empty())
            {
                s = chunk->slots->begin();

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
                        else
                        {
                            // we never deallocate the lti chunk, as we assume
                            // it will exist elsewhere for deallocation
                            // delete (*s)->val_lti.val_value;
                        }

                        delete(*v);
                    }

                    delete s->second;
                }

                // deallocate attribute for each corresponding value
                lSym = s->first;
                thisAgent->symbolManager->symbol_remove_ref(&lSym);

                chunk->slots->erase(s);
            }

            // remove slots
            delete chunk->slots;
            chunk->slots = NULL;
        }

        // remove chunk itself
        if (free_chunk)
        {
            delete chunk;
            chunk = NULL;
        }
    }
}

std::string* SMem_Manager::parse_lti_name(soar::Lexeme* lexeme, char* id_letter, uint64_t* id_number)
{
    std::string* return_val = new std::string;

    if ((*lexeme).type == IDENTIFIER_LEXEME)
    {
        std::string soar_number;
        to_string((*lexeme).id_number, soar_number);

        return_val->append(1, (*lexeme).id_letter);
        return_val->append(soar_number);

        (*id_letter) = (*lexeme).id_letter;
        (*id_number) = (*lexeme).id_number;

        char counter_index = (*id_letter - static_cast<char>('A'));
        uint64_t* letter_ct = thisAgent->symbolManager->get_id_counter(counter_index);
        if (*id_number >= *letter_ct)
        {
            *letter_ct = *id_number + 1;
        }
    }
    else
    {
        return_val->assign((*lexeme).string());

        (*id_letter) = static_cast<char>(toupper((*lexeme).string()[1]));
        (*id_number) = 0;
    }

    return return_val;
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

bool SMem_Manager::process_smem_add(soar::Lexer* lexer, smem_str_to_chunk_map* chunks, smem_chunk_set* newbies)
{
    bool return_val = false;

    smem_chunk* new_chunk = new smem_chunk;
    new_chunk->slots = NULL;

    std::string* chunk_name = NULL;

    char temp_letter;
    uint64_t temp_number;

    bool good_at;

    // consume left paren
    lexer->get_lexeme();

    if ((lexer->current_lexeme.type == AT_LEXEME) || (lexer->current_lexeme.type == IDENTIFIER_LEXEME) || (lexer->current_lexeme.type == VARIABLE_LEXEME))
    {
        good_at = true;

        if (lexer->current_lexeme.type == AT_LEXEME)
        {
            lexer->get_lexeme();

            good_at = (lexer->current_lexeme.type == IDENTIFIER_LEXEME);
        }

        if (good_at)
        {
            // save identifier
            chunk_name = parse_lti_name(&(lexer->current_lexeme), &(temp_letter), &(temp_number));
            new_chunk->lti_letter = temp_letter;
            new_chunk->lti_number = temp_number;
            new_chunk->lti_id = NIL;
            new_chunk->soar_id = NIL;
            new_chunk->slots = new smem_slot_map;

            // consume id
            lexer->get_lexeme();

            //

            uint64_t intermediate_counter = 1;
            smem_chunk* intermediate_parent;
            smem_chunk* temp_chunk;
            std::string temp_key;
            std::string* temp_key2;
            Symbol* chunk_attr;
            smem_chunk_value* chunk_value;
            smem_slot* s;

            // populate slots
            while (lexer->current_lexeme.type == UP_ARROW_LEXEME)
            {
                intermediate_parent = new_chunk;

                // go on to attribute
                lexer->get_lexeme();

                // get the appropriate constant type
                chunk_attr = parse_constant_attr(&(lexer->current_lexeme));

                // if constant attribute, proceed to value
                if (chunk_attr != NIL)
                {
                    // consume attribute
                    lexer->get_lexeme();

                    // support for dot notation:
                    // when we encounter a dot, instantiate
                    // the previous attribute as a temporary
                    // identifier and use that as the parent
                    while (lexer->current_lexeme.type == PERIOD_LEXEME)
                    {
                        // create a new chunk
                        temp_chunk = new smem_chunk;
                        temp_chunk->lti_letter = ((chunk_attr->symbol_type == STR_CONSTANT_SYMBOL_TYPE) ? (static_cast<char>(static_cast<int>(chunk_attr->sc->name[0]))) : ('X'));
                        temp_chunk->lti_number = (intermediate_counter++);
                        temp_chunk->lti_id = NIL;
                        temp_chunk->slots = new smem_slot_map;
                        temp_chunk->soar_id = NIL;

                        // add it as a child to the current parent
                        chunk_value = new smem_chunk_value;
                        chunk_value->val_lti.val_type = value_lti_t;
                        chunk_value->val_lti.val_value = temp_chunk;
                        s = make_smem_slot(intermediate_parent->slots, chunk_attr);
                        s->push_back(chunk_value);

                        // create a key guaranteed to be unique
                        std::string temp_key3;
                        to_string(temp_chunk->lti_number, temp_key3);
                        temp_key.assign("<");
                        temp_key.append(1, temp_chunk->lti_letter);
                        temp_key.append("#");
                        temp_key.append(temp_key3);
                        temp_key.append(">");

                        // insert the new chunk
                        (*chunks)[ temp_key ] = temp_chunk;

                        // definitely a new chunk
                        newbies->insert(temp_chunk);

                        // the new chunk is our parent for this set of values (or further dots)
                        intermediate_parent = temp_chunk;
                        temp_chunk = NULL;

                        // get the next attribute
                        lexer->get_lexeme();
                        chunk_attr = parse_constant_attr(&(lexer->current_lexeme));

                        // consume attribute
                        lexer->get_lexeme();
                    }

                    if (chunk_attr != NIL)
                    {
                        bool first_value = true;

                        do
                        {
                            // value by type
                            chunk_value = NIL;
                            if (lexer->current_lexeme.type == STR_CONSTANT_LEXEME)
                            {
                                chunk_value = new smem_chunk_value;
                                chunk_value->val_const.val_type = value_const_t;
                                chunk_value->val_const.val_value = thisAgent->symbolManager->make_str_constant(static_cast<const char*>(lexer->current_lexeme.string()));
                            }
                            else if (lexer->current_lexeme.type == INT_CONSTANT_LEXEME)
                            {
                                chunk_value = new smem_chunk_value;
                                chunk_value->val_const.val_type = value_const_t;
                                chunk_value->val_const.val_value = thisAgent->symbolManager->make_int_constant(lexer->current_lexeme.int_val);
                            }
                            else if (lexer->current_lexeme.type == FLOAT_CONSTANT_LEXEME)
                            {
                                chunk_value = new smem_chunk_value;
                                chunk_value->val_const.val_type = value_const_t;
                                chunk_value->val_const.val_value = thisAgent->symbolManager->make_float_constant(lexer->current_lexeme.float_val);
                            }
                            else if ((lexer->current_lexeme.type == AT_LEXEME) || (lexer->current_lexeme.type == IDENTIFIER_LEXEME) || (lexer->current_lexeme.type == VARIABLE_LEXEME))
                            {
                                good_at = true;

                                if (lexer->current_lexeme.type == AT_LEXEME)
                                {
                                    lexer->get_lexeme();

                                    good_at = (lexer->current_lexeme.type == IDENTIFIER_LEXEME);
                                }

                                if (good_at)
                                {
                                    // create new value
                                    chunk_value = new smem_chunk_value;
                                    chunk_value->val_lti.val_type = value_lti_t;

                                    // get key
                                    temp_key2 = parse_lti_name(&(lexer->current_lexeme), &(temp_letter), &(temp_number));

                                    // search for an existing chunk
                                    smem_str_to_chunk_map::iterator p = chunks->find((*temp_key2));

                                    // if exists, point; else create new
                                    if (p != chunks->end())
                                    {
                                        chunk_value->val_lti.val_value = p->second;
                                    }
                                    else
                                    {
                                        // create new chunk
                                        temp_chunk = new smem_chunk;
                                        temp_chunk->lti_id = NIL;
                                        temp_chunk->lti_letter = temp_letter;
                                        temp_chunk->lti_number = temp_number;
                                        temp_chunk->lti_id = NIL;
                                        temp_chunk->slots = NIL;
                                        temp_chunk->soar_id = NIL;

                                        // associate with value
                                        chunk_value->val_lti.val_value = temp_chunk;

                                        // add to chunks
                                        (*chunks)[(*temp_key2) ] = temp_chunk;

                                        // possibly a newbie (could be a self-loop)
                                        newbies->insert(temp_chunk);
                                    }

                                    delete temp_key2;
                                }
                            }

                            if (chunk_value != NIL)
                            {
                                // consume
                                lexer->get_lexeme();

                                // add to appropriate slot
                                s = make_smem_slot(intermediate_parent->slots, chunk_attr);
                                if (first_value && !s->empty())
                                {
                                    // in the case of a repeated attribute, remove ref here to avoid leak
                                    thisAgent->symbolManager->symbol_remove_ref(&chunk_attr);
                                }
                                s->push_back(chunk_value);

                                // if this was the last attribute
                                if (lexer->current_lexeme.type == R_PAREN_LEXEME)
                                {
                                    return_val = true;
                                    lexer->get_lexeme();
                                    chunk_value = NIL;
                                }

                                first_value = false;
                            }
                        }
                        while (chunk_value != NIL);
                    }
                }
            }
        }
        else
        {
            delete new_chunk;
        }
    }
    else
    {
        delete new_chunk;
    }

    if (return_val)
    {
        // search for an existing chunk (occurs if value comes before id)
        smem_chunk** p = & (*chunks)[(*chunk_name) ];

        if (!(*p))
        {
            (*p) = new_chunk;

            // a newbie!
            newbies->insert(new_chunk);
        }
        else
        {
            // transfer slots
            if (!(*p)->slots)
            {
                // if none previously, can just use
                (*p)->slots = new_chunk->slots;
                new_chunk->slots = NULL;
            }
            else
            {
                // otherwise, copy

                smem_slot_map::iterator ss_p;
                smem_slot::iterator s_p;

                smem_slot* source_slot;
                smem_slot* target_slot;

                // for all slots
                for (ss_p = new_chunk->slots->begin(); ss_p != new_chunk->slots->end(); ss_p++)
                {
                    target_slot =make_smem_slot((*p)->slots, ss_p->first);
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
                delete new_chunk->slots;
                new_chunk->slots = NULL;
            }

            // contents are new
            newbies->insert((*p));

            // deallocate
            deallocate_chunk(new_chunk);
        }
    }
    else
    {
        newbies->clear();
    }

    // de-allocate id name
    if (chunk_name)
    {
        delete chunk_name;
    }

    return return_val;
}

bool SMem_Manager::process_smem_add_object(const char* chunks_str, std::string** err_msg)
{
    bool return_val = false;
    uint64_t clause_count = 0;

    // parsing chunks requires an open semantic database
    attach();

    soar::Lexer lexer(thisAgent, chunks_str);

    bool good_chunk = true;

    smem_str_to_chunk_map chunks;
    smem_str_to_chunk_map::iterator c_old;

    smem_chunk_set newbies;
    smem_chunk_set::iterator c_new;

    // consume next token
    lexer.get_lexeme();

    if (lexer.current_lexeme.type != L_PAREN_LEXEME)
    {
        good_chunk = false;
    }

    // while there are chunks to consume
    while ((lexer.current_lexeme.type == L_PAREN_LEXEME) && (good_chunk))
    {
        good_chunk = process_smem_add(&lexer, &(chunks), &(newbies));

        if (good_chunk)
        {
            // add all newbie lti's as appropriate
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
                if ((*c_new)->lti_id == NIL)
                {
                    // deal differently with variable vs. lti
                    if ((*c_new)->lti_number == NIL)
                    {
                        // add a new lti id (we have a guarantee this won't be in Soar's WM)
                        char counter_index = ((*c_new)->lti_letter - static_cast<char>('A'));
                        uint64_t* letter_ct = thisAgent->symbolManager->get_id_counter(counter_index);

                        (*c_new)->lti_number = (*letter_ct)++;
                        (*c_new)->lti_id =add_new_lti_id();
                    }
                    else
                    {
                        // should ALWAYS be the case (it's a newbie and we've initialized lti_id to NIL)
                        if ((*c_new)->lti_id == NIL)
                        {
                            // get existing
                            (*c_new)->lti_id = lti_exists((*c_new)->lti_number);

                            // if doesn't exist, add it
                            if ((*c_new)->lti_id == NIL)
                            {
                                (*c_new)->lti_id =add_new_lti_id();
                            }
                        }
                    }
                }
            }

            // add all newbie contents (append, as opposed to replace, children)
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
                if ((*c_new)->slots != NIL)
                {
                   add_semantic_object_to_smem((*c_new)->lti_id, (*c_new)->slots, false);
                }
            }

            // deallocate *contents* of all newbies (need to keep around name->id association for future chunks)
            for (c_new = newbies.begin(); c_new != newbies.end(); c_new++)
            {
               deallocate_chunk((*c_new), false);
            }

            // increment clause counter
            clause_count++;

            // clear newbie list
            newbies.clear();
        }
    };

    return_val = good_chunk;

    // deallocate all chunks
    {
        for (c_old = chunks.begin(); c_old != chunks.end(); c_old++)
        {
           deallocate_chunk(c_old->second, true);
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

bool SMem_Manager::parse_cues(const char* chunks_str, std::string** err_msg, std::string** result_message, uint64_t number_to_retrieve)
{
    uint64_t clause_count = 0;  // This is counting up the number of parsed clauses
    // so that there is a pointer to a failure location.

    //Parsing requires an open semantic database.
    attach();

    soar::Lexer lexer(thisAgent, chunks_str);

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
                        /* Need to re-do for identifier_lexeme instead.  */
//                        //If the LTI isn't recognized, then it cannot be a good cue.
//                        lexer.get_lexeme();
//                        smem_lti_id value_id = lti_get_id(lexer.current_lexeme.id_letter, lexer.current_lexeme.id_number);
//                        if (value_id == NIL)
//                        {
//                            good_cue = false;
//                            (*err_msg)->append("Error: LTI was not found.\n");
//                            break;
//                        }
//                        else
//                        {
//                            value = lti_soar_make(value_id, lexer.current_lexeme.id_letter, lexer.current_lexeme.id_number, SMEM_LTI_UNKNOWN_LEVEL);
//                        }
//                        lexer.get_lexeme();
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
        smem_lti_set* prohibit = new smem_lti_set;
        wme_set cue_wmes;
        symbol_triple_list meta_wmes;
        symbol_triple_list retrieval_wmes;
        (*result_message) = new std::string();

        std::list<smem_lti_id> match_ids;

        process_query(NIL, root_cue_id, minus_ever ? negative_cues : NIL, NIL, prohibit, cue_wmes, meta_wmes, retrieval_wmes, qry_search, number_to_retrieve, &(match_ids), 1, fake_install);

        if (!match_ids.empty())
        {
            for (std::list<smem_lti_id>::const_iterator id = match_ids.begin(), end = match_ids.end(); id != end; ++id)
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

void initialize_smem_chunk_value_lti(smem_chunk_value_lti& lti)
{
    lti.val_type = smem_cue_element_type_none;
    lti.val_value = NULL;
}

void initialize_smem_chunk_value_constant(smem_chunk_value_constant& constant)
{
    constant.val_type = smem_cue_element_type_none;
    constant.val_value = NULL;
}

/*
 * This is intended to allow the user to remove part or all of information stored on a LTI.
 * (All attributes, selected attributes, or just values from particular attributes.)
 */
bool SMem_Manager::process_smem_remove(const char* chunks_str, std::string** err_msg, std::string** result_message, bool force)
{
    /* MToDo | Fix after we fully remove shared symbol.  Then we can use either a separate symbol table or what we currently
     *         store in the db. */
    return true;

    //    //TODO: need to fix so that err_msg and result_message are actually used or not passed.
//    bool good_command = true;
//
//    //parsing chunks requires an open semantic database
//    attach();
//
//    soar::Lexer lexer(thisAgent, chunks_str);
//
//    lexer.get_lexeme();
//
//    if (lexer.current_lexeme.type == L_PAREN_LEXEME)
//    {
//        lexer.get_lexeme();//Consumes the left paren
//    }
//
//    if (lexer.current_lexeme.type == AT_LEXEME && good_command)
//    {
//        lexer.get_lexeme();
//    }
//
//    good_command = lexer.current_lexeme.type == IDENTIFIER_LEXEME;
//
//    smem_lti_id lti_id = 0;
//
//    if (good_command)
//    {
//        lti_id = lti_get_id(lexer.current_lexeme.id_letter, lexer.current_lexeme.id_number);
//    }
//    else
//    {
//        (*err_msg)->append("Error: No LTI found for that letter and number.\n");
//    }
//
//    symbol_triple_list retrieval_wmes;
//    symbol_triple_list meta_wmes;
//
//    if (good_command && lti_id != NIL)
//    {
//        Symbol* lti = lti_soar_make(lti_id, lexer.current_lexeme.id_letter, lexer.current_lexeme.id_number, SMEM_LTI_UNKNOWN_LEVEL);
////        Symbol* lti = get_sti_for_lti(lti_id, SMEM_LTI_UNKNOWN_LEVEL);
//
//        lexer.get_lexeme();//Consume the identifier.
//
//        smem_slot_map children;
//
//        if (lexer.current_lexeme.type == UP_ARROW_LEXEME)
//        {
//            //Now that we know we have a good lti, we can do a NCBR so that we know what attributes and values we can delete.
//            //"--force" will ignore attempts to delete that which isn't there, while the default will be to stop and report back.
//            install_memory(NIL, lti_id, lti, false, meta_wmes, retrieval_wmes, fake_install);
//
//            //First, we'll create the slot_map according to retrieval_wmes, then we'll remove what we encounter during parsing.
//            symbol_triple_list::iterator triple_ptr_iter;
//            smem_slot* temp_slot;
//            for (triple_ptr_iter = retrieval_wmes.begin(); triple_ptr_iter != retrieval_wmes.end(); triple_ptr_iter++)
//            {
//                if (children.count((*triple_ptr_iter)->attr)) //If the attribute is already in the map.
//                {
//                    temp_slot = (children.find((*triple_ptr_iter)->attr)->second);
//                    smem_chunk_value* temp_val = new smem_chunk_value;
//                    if ((*triple_ptr_iter)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
//                    {
//                        //If the chunk was retrieved and it is an identifier it is lti.
//                        smem_chunk_value_lti temp_lti;
//                        smem_chunk_value_constant temp_const;
//
//                        initialize_smem_chunk_value_lti(temp_lti);
//                        initialize_smem_chunk_value_constant(temp_const);
//
//                        temp_val->val_const = temp_const;
//                        temp_val->val_const.val_type = value_lti_t;
//                        temp_val->val_lti = temp_lti;
//                        temp_val->val_lti.val_type = value_lti_t;
//                        smem_chunk* temp_chunk = new smem_chunk;
//                        temp_chunk->lti_id = (*triple_ptr_iter)->value->id->LTI_ID;
//                        temp_chunk->lti_letter = (*triple_ptr_iter)->value->id->name_letter;
//                        temp_chunk->lti_number = (*triple_ptr_iter)->value->id->name_number;
//                        temp_chunk->soar_id = (*triple_ptr_iter)->value;
//                        temp_val->val_lti.val_value = temp_chunk;
//                    }
//                    else //If the value is not an identifier, then it is a "constant".
//                    {
//                        smem_chunk_value_constant temp_const;
//                        smem_chunk_value_lti temp_lti;
//
//                        initialize_smem_chunk_value_lti(temp_lti);
//                        initialize_smem_chunk_value_constant(temp_const);
//
//                        temp_val->val_lti = temp_lti;
//                        temp_val->val_lti.val_type = value_const_t;
//                        temp_val->val_const.val_type = value_const_t;
//                        temp_val->val_const.val_value = (*triple_ptr_iter)->value;
//                    }
//                    (*temp_slot).push_back(temp_val);
//                }
//                else //If the attribute is not in the map and we need to make a slot.
//                {
//                    temp_slot = new smem_slot;
//                    smem_chunk_value* temp_val = new smem_chunk_value;
//                    if ((*triple_ptr_iter)->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
//                    {
//                        //If the chunk was retrieved and it is an identifier it is lti.
//                        smem_chunk_value_lti temp_lti;
//                        smem_chunk_value_constant temp_const;
//
//                        initialize_smem_chunk_value_lti(temp_lti);
//                        initialize_smem_chunk_value_constant(temp_const);
//
//                        temp_val->val_const = temp_const;
//                        temp_val->val_const.val_type = value_lti_t;
//                        temp_val->val_lti = temp_lti;
//                        temp_val->val_lti.val_type = value_lti_t;
//                        smem_chunk* temp_chunk = new smem_chunk;
//                        temp_chunk->lti_id = (*triple_ptr_iter)->value->id->LTI_ID;
//                        temp_chunk->lti_letter = (*triple_ptr_iter)->value->id->name_letter;
//                        temp_chunk->lti_number = (*triple_ptr_iter)->value->id->name_number;
//                        temp_chunk->soar_id = (*triple_ptr_iter)->value;
//                        temp_val->val_lti.val_value = temp_chunk;
//                    }
//                    else //If the value is nt an identifier, then it is a "constant".
//                    {
//                        smem_chunk_value_constant temp_const;
//                        smem_chunk_value_lti temp_lti;
//
//                        initialize_smem_chunk_value_lti(temp_lti);
//                        initialize_smem_chunk_value_constant(temp_const);
//
//                        temp_val->val_lti = temp_lti;
//                        temp_val->val_lti.val_type = value_const_t;
//                        temp_val->val_const.val_type = value_const_t;
//                        temp_val->val_const.val_value = (*triple_ptr_iter)->value;
//                    }
//                    (*temp_slot).push_back(temp_val);
//                    children[(*triple_ptr_iter)->attr] = temp_slot;
//                }
//            }
//
//            //Now we process attributes one at a time.
//            while (lexer.current_lexeme.type == UP_ARROW_LEXEME && (good_command || force))
//            {
//                lexer.get_lexeme();// Consume the up arrow.
//
//                Symbol* attribute = NIL;
//
//                if (lexer.current_lexeme.type == STR_CONSTANT_LEXEME)
//                {
//                    attribute = thisAgent->symbolManager->find_str_constant(static_cast<const char*>(lexer.current_lexeme.string()));
//                }
//                else if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
//                {
//                    attribute = thisAgent->symbolManager->find_int_constant(lexer.current_lexeme.int_val);
//                }
//                else if (lexer.current_lexeme.type == FLOAT_CONSTANT_LEXEME)
//                {
//                    attribute = thisAgent->symbolManager->find_float_constant(lexer.current_lexeme.float_val);
//                }
//
//                if (attribute == NIL)
//                {
//                    good_command = false;
//                    (*err_msg)->append("Error: Attribute was not found.\n");
//                }
//                else
//                {
//                    lexer.get_lexeme();//Consume the attribute.
//                    good_command = true;
//                }
//
//                if (good_command && (lexer.current_lexeme.type != UP_ARROW_LEXEME && lexer.current_lexeme.type != R_PAREN_LEXEME)) //If there are values.
//                {
//                    Symbol* value;
//                    do //Add value by type
//                    {
//                        value = NIL;
//                        if (lexer.current_lexeme.type == STR_CONSTANT_LEXEME)
//                        {
//                            value = thisAgent->symbolManager->find_str_constant(static_cast<const char*>(lexer.current_lexeme.string()));
//                            lexer.get_lexeme();
//                        }
//                        else if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
//                        {
//                            value = thisAgent->symbolManager->find_int_constant(lexer.current_lexeme.int_val);
//                            lexer.get_lexeme();
//                        }
//                        else if (lexer.current_lexeme.type == FLOAT_CONSTANT_LEXEME)
//                        {
//                            value = thisAgent->symbolManager->find_float_constant(lexer.current_lexeme.float_val);
//                            lexer.get_lexeme();
//                        }
//                        else if (lexer.current_lexeme.type == AT_LEXEME)
//                        {
//                            lexer.get_lexeme();
//                            if (lexer.current_lexeme.type == IDENTIFIER_LEXEME)
//                            {
//                                value = thisAgent->symbolManager->find_identifier(lexer.current_lexeme.id_letter, lexer.current_lexeme.id_number);
//                                lexer.get_lexeme();
//                            }
//                            else
//                            {
//                                (*err_msg)->append("Error: '@' should be followed by an identifier.\n");
//                                good_command = false;
//                                break;
//                            }
//                        }
//                        else
//                        {
//                            good_command = (lexer.current_lexeme.type == R_PAREN_LEXEME || lexer.current_lexeme.type == UP_ARROW_LEXEME);
//                            if (!good_command)
//                            {
//                                (*err_msg)->append("Error: Expected ')' or '^'.\n... The value was likely not found.\n");
//                            }
//                        }
//
//                        if (value != NIL && good_command) //Value might be nil, but that can be just fine.
//                        {
//                            //Given a value for this attribute, we have a symbol triple to remove.
//                            smem_slot::iterator values;
//                            for (values = (children.find(attribute))->second->begin(); values != (children.find(attribute))->second->end(); values++)
//                            {
//                                if (value->symbol_type == IDENTIFIER_SYMBOL_TYPE && (*values)->val_lti.val_type == value_lti_t)
//                                {
//                                    if ((*values)->val_lti.val_value->soar_id == value)
//                                    {
//                                        delete(*values)->val_lti.val_value;
//                                        delete *values;
//                                        (*(children.find(attribute))).second->erase(values);
//                                        break;
//                                    }
//                                }
//                                else if (value->symbol_type != IDENTIFIER_SYMBOL_TYPE && (*values)->val_const.val_type == value_const_t)
//                                {
//                                    if ((*values)->val_const.val_value == value)
//                                    {
//                                        delete *values;
//                                        (*(children.find(attribute))).second->erase(values);
//                                        break;
//                                    }
//                                }
//                            }
//                            if (values == (children.find(attribute))->second->end())
//                            {
//                                (*err_msg)->append("Error: Value does not exist on attribute.\n");
//                            }
//                        }
//                        else
//                        {
//                            if ((good_command && !force) && (lexer.current_lexeme.type != R_PAREN_LEXEME && lexer.current_lexeme.type != UP_ARROW_LEXEME))
//                            {
//                                (*err_msg)->append("Error: Attribute contained a value that could not be found.\n");
//                                break;
//                            }
//                        }
//                    }
//                    while (good_command && (value != NIL || !(lexer.current_lexeme.type == R_PAREN_LEXEME || lexer.current_lexeme.type == UP_ARROW_LEXEME)));
//                }
//                else if (good_command && children.find(attribute) != children.end()) //If we didn't have any values, then we just get rid of everything on the attribute.
//                {
//                    smem_slot* result = (children.find(attribute))->second;
//                    smem_slot::iterator values, end = result->end();
//                    for (values = (children.find(attribute))->second->begin(); values != end; values++)
//                    {
//                        delete *values;
//                    }
//                    children.erase(attribute);
//                }
//                if (force)
//                {
//                    while ((lexer.current_lexeme.type != EOF_LEXEME && lexer.current_lexeme.type != UP_ARROW_LEXEME) && lexer.current_lexeme.type != R_PAREN_LEXEME) //Loop until the lexeme is EOF, another ^, or ")".
//                    {
//                        lexer.get_lexeme();
//                    }
//                }
//            }
//        }
//        if (good_command && lexer.current_lexeme.type == R_PAREN_LEXEME)
//        {
//            store_chunk(lti_id, &(children), true, NULL, false);
//        }
//        else if (good_command)
//        {
//            (*err_msg)->append("Error: Expected a ')'.\n");
//        }
//
//        //Clean up.
//        smem_slot_map::iterator attributes, end = children.end();
//        for (attributes = children.begin(); attributes != end; attributes++)
//        {
//            smem_slot* result = (children.find(attributes->first))->second;
//            smem_slot::iterator values, end = result->end();
//            for (values = result->begin(); values != end; values++)
//            {
//                if ((*values)->val_lti.val_type == value_lti_t)
//                {
//                    delete(*values)->val_lti.val_value;
//                }
//                delete *values;
//            }
//            delete attributes->second;
//        }
//
//        symbol_triple_list::iterator triple_iterator, end2 = retrieval_wmes.end();
//        for (triple_iterator = retrieval_wmes.begin(); triple_iterator != end2; triple_iterator++)
//        {
//            thisAgent->symbolManager->symbol_remove_ref(&(*triple_iterator)->id);
//            thisAgent->symbolManager->symbol_remove_ref(&(*triple_iterator)->attr);
//            thisAgent->symbolManager->symbol_remove_ref(&(*triple_iterator)->value);
//            delete *triple_iterator;
//        }
//        thisAgent->symbolManager->symbol_remove_ref(&lti);
//    }
//    return good_command;
}

smem_slot* SMem_Manager::make_smem_slot(smem_slot_map* slots, Symbol* attr)
{
    smem_slot** s = & (*slots)[ attr ];

    if (!(*s))
    {
        (*s) = new smem_slot;
    }

    return (*s);
}

void SMem_Manager::disconnect_chunk(smem_lti_id lti_id)
{
    // adjust attr, attr/value counts
    {
        uint64_t pair_count = 0;

        smem_lti_id child_attr = 0;
        std::set<smem_lti_id> distinct_attr;

        // pairs first, accumulate distinct attributes and pair count
        smem_stmts->web_all->bind_int(1, lti_id);
        while (smem_stmts->web_all->execute() == soar_module::row)
        {
            pair_count++;

            child_attr = smem_stmts->web_all->column_int(0);
            distinct_attr.insert(child_attr);

            // null -> attr/lti
            if (smem_stmts->web_all->column_int(1) != SMEM_AUGMENTATIONS_NULL)
            {
                // adjust in opposite direction ( adjust, attribute, const )
                smem_stmts->wmes_constant_frequency_update->bind_int(1, -1);
                smem_stmts->wmes_constant_frequency_update->bind_int(2, child_attr);
                smem_stmts->wmes_constant_frequency_update->bind_int(3, smem_stmts->web_all->column_int(1));
                smem_stmts->wmes_constant_frequency_update->execute(soar_module::op_reinit);
            }
            else
            {
                // adjust in opposite direction ( adjust, attribute, lti )
                smem_stmts->wmes_lti_frequency_update->bind_int(1, -1);
                smem_stmts->wmes_lti_frequency_update->bind_int(2, child_attr);
                smem_stmts->wmes_lti_frequency_update->bind_int(3, smem_stmts->web_all->column_int(2));
                smem_stmts->wmes_lti_frequency_update->execute(soar_module::op_reinit);
            }
        }
        smem_stmts->web_all->reinitialize();

        // now attributes
        for (std::set<smem_lti_id>::iterator a = distinct_attr.begin(); a != distinct_attr.end(); a++)
        {
            // adjust in opposite direction ( adjust, attribute )
            smem_stmts->attribute_frequency_update->bind_int(1, -1);
            smem_stmts->attribute_frequency_update->bind_int(2, *a);
            smem_stmts->attribute_frequency_update->execute(soar_module::op_reinit);
        }

        // update local statistic
        smem_stats->slots->set_value(smem_stats->slots->get_value() - pair_count);
    }

    // disconnect
    {
        smem_stmts->web_truncate->bind_int(1, lti_id);
        smem_stmts->web_truncate->execute(soar_module::op_reinit);
    }
}

void SMem_Manager::add_semantic_object_to_smem(smem_lti_id lti_id, smem_slot_map* children, bool remove_old_children, Symbol* print_id, bool activate)
{
    // if remove children, disconnect chunk -> no existing edges
    // else, need to query number of existing edges
    uint64_t existing_edges = 0;
    if (remove_old_children)
    {
        disconnect_chunk(lti_id);

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
        smem_stmts->act_lti_child_ct_get->bind_int(1, lti_id);
        smem_stmts->act_lti_child_ct_get->execute();

        existing_edges = static_cast<uint64_t>(smem_stmts->act_lti_child_ct_get->column_int(0));

        smem_stmts->act_lti_child_ct_get->reinitialize();
    }

    // get new edges
    // if didn't disconnect, entails lookups in existing edges
    std::set<smem_hash_id> attr_new;
    std::set< std::pair<smem_hash_id, smem_hash_id> > const_new;
    std::set< std::pair<smem_hash_id, smem_lti_id> > lti_new;
    {
        smem_slot_map::iterator s;
        smem_slot::iterator v;

        smem_hash_id attr_hash = 0;
        smem_hash_id value_hash = 0;
        smem_lti_id value_lti = 0;

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
                smem_stmts->web_attr_child->bind_int(1, lti_id);
                smem_stmts->web_attr_child->bind_int(2, attr_hash);
                if (smem_stmts->web_attr_child->execute(soar_module::op_reinit) != soar_module::row)
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
                        smem_stmts->web_const_child->bind_int(1, lti_id);
                        smem_stmts->web_const_child->bind_int(2, attr_hash);
                        smem_stmts->web_const_child->bind_int(3, value_hash);
                        if (smem_stmts->web_const_child->execute(soar_module::op_reinit) != soar_module::row)
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
                        value_lti = add_new_lti_id();
                        (*v)->val_lti.val_value->lti_id = value_lti;

                        if ((*v)->val_lti.val_value->soar_id != NIL)
                        {
                            (*v)->val_lti.val_value->soar_id->id->LTI_ID = value_lti;
                            (*v)->val_lti.val_value->soar_id->id->smem_valid = thisAgent->EpMem->epmem_validation;
                        }
                    }

                    if (remove_old_children)
                    {
                        lti_new.insert(std::make_pair(attr_hash, value_lti));
                    }
                    else
                    {
                        // lti_id, attribute_s_id, val_lti
                        smem_stmts->web_lti_child->bind_int(1, lti_id);
                        smem_stmts->web_lti_child->bind_int(2, attr_hash);
                        smem_stmts->web_lti_child->bind_int(3, value_lti);
                        if (smem_stmts->web_lti_child->execute(soar_module::op_reinit) != soar_module::row)
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
        uint64_t thresh = static_cast<uint64_t>(smem_params->thresh->get_value());
        after_above = (new_edges >= thresh);

        // if before below
        if (existing_edges < thresh)
        {
            if (after_above)
            {
                // update smem_augmentations to inf
                smem_stmts->act_set->bind_double(1, web_act);
                smem_stmts->act_set->bind_int(2, lti_id);
                smem_stmts->act_set->execute(soar_module::op_reinit);
            }
        }
    }

    // update edge counter
    {
        smem_stmts->act_lti_child_ct_set->bind_int(1, new_edges);
        smem_stmts->act_lti_child_ct_set->bind_int(2, lti_id);
        smem_stmts->act_lti_child_ct_set->execute(soar_module::op_reinit);
    }

    // now we can safely activate the lti
    if (activate)
    {
        double lti_act = lti_activate(lti_id, true, new_edges);

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
                    smem_stmts->web_add->bind_int(1, lti_id);
                    smem_stmts->web_add->bind_int(2, p->first);
                    smem_stmts->web_add->bind_int(3, p->second);
                    smem_stmts->web_add->bind_int(4, SMEM_AUGMENTATIONS_NULL);
                    smem_stmts->web_add->bind_double(5, web_act);
                    smem_stmts->web_add->execute(soar_module::op_reinit);
                }

                // update counter
                {
                    // check if counter exists (and add if does not): attribute_s_id, val
                    smem_stmts->wmes_constant_frequency_check->bind_int(1, p->first);
                    smem_stmts->wmes_constant_frequency_check->bind_int(2, p->second);
                    if (smem_stmts->wmes_constant_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                    {
                        smem_stmts->wmes_constant_frequency_add->bind_int(1, p->first);
                        smem_stmts->wmes_constant_frequency_add->bind_int(2, p->second);
                        smem_stmts->wmes_constant_frequency_add->execute(soar_module::op_reinit);
                    }
                    else
                    {
                        // adjust count (adjustment, attribute_s_id, val)
                        smem_stmts->wmes_constant_frequency_update->bind_int(1, 1);
                        smem_stmts->wmes_constant_frequency_update->bind_int(2, p->first);
                        smem_stmts->wmes_constant_frequency_update->bind_int(3, p->second);
                        smem_stmts->wmes_constant_frequency_update->execute(soar_module::op_reinit);
                    }
                }
            }
        }

        // attr/lti pairs
        {
            for (std::set< std::pair< smem_hash_id, smem_lti_id > >::iterator p = lti_new.begin(); p != lti_new.end(); p++)
            {
                // insert
                {
                    // lti_id, attribute_s_id, val_const, value_lti_id, activation_value
                    smem_stmts->web_add->bind_int(1, lti_id);
                    smem_stmts->web_add->bind_int(2, p->first);
                    smem_stmts->web_add->bind_int(3, SMEM_AUGMENTATIONS_NULL);
                    smem_stmts->web_add->bind_int(4, p->second);
                    smem_stmts->web_add->bind_double(5, web_act);
                    smem_stmts->web_add->execute(soar_module::op_reinit);
                }

                // update counter
                {
                    // check if counter exists (and add if does not): attribute_s_id, val
                    smem_stmts->wmes_lti_frequency_check->bind_int(1, p->first);
                    smem_stmts->wmes_lti_frequency_check->bind_int(2, p->second);
                    if (smem_stmts->wmes_lti_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                    {
                        smem_stmts->wmes_lti_frequency_add->bind_int(1, p->first);
                        smem_stmts->wmes_lti_frequency_add->bind_int(2, p->second);
                        smem_stmts->wmes_lti_frequency_add->execute(soar_module::op_reinit);
                    }
                    else
                    {
                        // adjust count (adjustment, attribute_s_id, lti)
                        smem_stmts->wmes_lti_frequency_update->bind_int(1, 1);
                        smem_stmts->wmes_lti_frequency_update->bind_int(2, p->first);
                        smem_stmts->wmes_lti_frequency_update->bind_int(3, p->second);
                        smem_stmts->wmes_lti_frequency_update->execute(soar_module::op_reinit);
                    }
                }
            }
        }

        // update attribute count
        {
            for (std::set< smem_hash_id >::iterator a = attr_new.begin(); a != attr_new.end(); a++)
            {
                // check if counter exists (and add if does not): attribute_s_id
                smem_stmts->attribute_frequency_check->bind_int(1, *a);
                if (smem_stmts->attribute_frequency_check->execute(soar_module::op_reinit) != soar_module::row)
                {
                    smem_stmts->attribute_frequency_add->bind_int(1, *a);
                    smem_stmts->attribute_frequency_add->execute(soar_module::op_reinit);
                }
                else
                {
                    // adjust count (adjustment, attribute_s_id)
                    smem_stmts->attribute_frequency_update->bind_int(1, 1);
                    smem_stmts->attribute_frequency_update->bind_int(2, *a);
                    smem_stmts->attribute_frequency_update->execute(soar_module::op_reinit);
                }
            }
        }

        // update local edge count
        {
            smem_stats->slots->set_value(smem_stats->slots->get_value() + (const_new.size() + lti_new.size()));
        }
    }
}

void SMem_Manager::store_in_smem(Symbol* pIdentifierSTI, smem_storage_type store_type, tc_number tc)
{
    // transitive closure only matters for recursive storage
    if ((store_type == store_recursive) && (tc == NIL))
    {
        tc = get_new_tc_number(thisAgent);
    }
    smem_sym_list shorties;

    // get level
    smem_wme_list* children = get_direct_augs_of_id(pIdentifierSTI, tc);
    smem_wme_list::iterator w;

    // make the target an lti, so intermediary data structure has lti_id
    // (takes care of short-term id self-referencing)
    link_sti_to_lti(pIdentifierSTI);

    // encode this level
    {
        smem_sym_to_chunk_map sym_to_chunk;
        smem_sym_to_chunk_map::iterator c_p;
        smem_chunk** c;

        smem_slot_map slots;
        smem_slot_map::iterator s_p;
        smem_slot::iterator v_p;
        smem_slot* s;
        smem_chunk_value* v;

        for (w = children->begin(); w != children->end(); w++)
        {
            // get slot
            s = make_smem_slot(&(slots), (*w)->attr);

            // create value, per type
            v = new smem_chunk_value;
            if ((*w)->value->is_constant())
            {
                v->val_const.val_type = value_const_t;
                v->val_const.val_value = (*w)->value;
            }
            else
            {
                v->val_lti.val_type = value_lti_t;

                // try to find existing chunk
                c = & sym_to_chunk[(*w)->value ];

                // if doesn't exist, add; else use existing
                if (!(*c))
                {
                    (*c) = new smem_chunk;
                    (*c)->lti_id = (*w)->value->id->LTI_ID;
                    (*c)->lti_letter = (*w)->value->id->name_letter;
                    (*c)->lti_number = (*w)->value->id->name_number;
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

        add_semantic_object_to_smem(pIdentifierSTI->id->LTI_ID, &(slots), true, pIdentifierSTI);

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

            // de-allocate chunks
            for (c_p = sym_to_chunk.begin(); c_p != sym_to_chunk.end(); c_p++)
            {
                delete c_p->second;
            }

            delete children;
        }
    }

    // recurse as necessary
    for (smem_sym_list::iterator shorty = shorties.begin(); shorty != shorties.end(); shorty++)
    {
        store_in_smem((*shorty), store_recursive, tc);
    }
}
