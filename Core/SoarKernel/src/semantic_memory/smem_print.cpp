/*
 * smem_print.cpp
 *
 *  Created on: Aug 21, 2016
 *      Author: mazzin
 */

#include "semantic_memory.h"
#include "smem_db.h"

#include "lexer.h"

void SMem_Manager::visualize_store(std::string* return_val)
{
    // header
    return_val->append("digraph smem {");
    return_val->append("\n");

    // LTIs
    return_val->append("node [ shape = doublecircle ];");
    return_val->append("\n");

    std::map< smem_lti_id, std::string > lti_names;
    std::map< smem_lti_id, std::string >::iterator n_p;
    {
        soar_module::sqlite_statement* q;

        smem_lti_id lti_id;

        std::string* lti_name;
        std::string temp_str;
        int64_t temp_int;
        double temp_double;

        // id, soar_letter, number
        q = thisAgent->SMem->smem_stmts->vis_lti;
        while (q->execute() == soar_module::row)
        {
            lti_id = q->column_int(0);

            lti_name = & lti_names[ lti_id ];
            lti_name->append(" @");
            to_string(lti_id, temp_str);
            lti_name->append(temp_str);

            return_val->append((*lti_name));
            return_val->append(" [ label=\"");
            return_val->append((*lti_name));
            return_val->append("\\n[");

            temp_double = q->column_double(1);
            to_string(temp_double, temp_str, 3, true);
            if (temp_double >= 0)
            {
                return_val->append("+");
            }
            return_val->append(temp_str);

            return_val->append("]\"");
            return_val->append(" ];");
            return_val->append("\n");
        }
        q->reinitialize();

        if (!lti_names.empty())
        {
            // terminal nodes first
            {
                std::map< smem_lti_id, std::list<std::string> > lti_terminals;
                std::map< smem_lti_id, std::list<std::string> >::iterator t_p;
                std::list<std::string>::iterator a_p;

                std::list<std::string>* my_terminals;
                std::list<std::string>::size_type terminal_num;

                return_val->append("\n");

                // proceed to terminal nodes
                return_val->append("node [ shape = plaintext ];");
                return_val->append("\n");

                // lti_id, attr_type, attr_hash, val_type, val_hash
                q = thisAgent->SMem->smem_stmts->vis_value_const;
                while (q->execute() == soar_module::row)
                {
                    lti_id = q->column_int(0);
                    my_terminals = & lti_terminals[ lti_id ];
                    lti_name = & lti_names[ lti_id ];

                    // parent prefix
                    return_val->append((*lti_name));
                    return_val->append("_");

                    // terminal count
                    terminal_num = my_terminals->size();
                    to_string(terminal_num, temp_str);
                    return_val->append(temp_str);

                    // prepare for value
                    return_val->append(" [ label = \"");

                    // output value
                    {
                        switch (q->column_int(3))
                        {
                            case STR_CONSTANT_SYMBOL_TYPE:
                                rhash__str(q->column_int(4), temp_str);
                                break;

                            case INT_CONSTANT_SYMBOL_TYPE:
                                temp_int = rhash__int(q->column_int(4));
                                to_string(temp_int, temp_str);
                                break;

                            case FLOAT_CONSTANT_SYMBOL_TYPE:
                                temp_double = rhash__float(q->column_int(4));
                                to_string(temp_double, temp_str);
                                break;

                            default:
                                temp_str.clear();
                                break;
                        }

                        return_val->append(temp_str);
                    }

                    // store terminal (attribute for edge label)
                    {
                        switch (q->column_int(1))
                        {
                            case STR_CONSTANT_SYMBOL_TYPE:
                                rhash__str(q->column_int(2), temp_str);
                                break;

                            case INT_CONSTANT_SYMBOL_TYPE:
                                temp_int = rhash__int(q->column_int(2));
                                to_string(temp_int, temp_str);
                                break;

                            case FLOAT_CONSTANT_SYMBOL_TYPE:
                                temp_double = rhash__float(q->column_int(2));
                                to_string(temp_double, temp_str);
                                break;

                            default:
                                temp_str.clear();
                                break;
                        }

                        my_terminals->push_back(temp_str);
                    }

                    // footer
                    return_val->append("\" ];");
                    return_val->append("\n");
                }
                q->reinitialize();

                // output edges
                {
                    unsigned int terminal_counter;

                    for (n_p = lti_names.begin(); n_p != lti_names.end(); n_p++)
                    {
                        t_p = lti_terminals.find(n_p->first);

                        if (t_p != lti_terminals.end())
                        {
                            terminal_counter = 0;

                            for (a_p = t_p->second.begin(); a_p != t_p->second.end(); a_p++)
                            {
                                return_val->append(n_p->second);
                                return_val ->append(" -> ");
                                return_val->append(n_p->second);
                                return_val->append("_");

                                to_string(terminal_counter, temp_str);
                                return_val->append(temp_str);
                                return_val->append(" [ label=\"");

                                return_val->append((*a_p));

                                return_val->append("\" ];");
                                return_val->append("\n");

                                terminal_counter++;
                            }
                        }
                    }
                }
            }

            // then links to other LTIs
            {
                // lti_id, attr_type, attr_hash, value_lti_id
                q = thisAgent->SMem->smem_stmts->vis_value_lti;
                while (q->execute() == soar_module::row)
                {
                    // source
                    lti_id = q->column_int(0);
                    lti_name = & lti_names[ lti_id ];
                    return_val->append((*lti_name));
                    return_val->append(" -> ");

                    // destination
                    lti_id = q->column_int(3);
                    lti_name = & lti_names[ lti_id ];
                    return_val->append((*lti_name));
                    return_val->append(" [ label =\"");

                    // output attribute
                    {
                        switch (q->column_int(1))
                        {
                            case STR_CONSTANT_SYMBOL_TYPE:
                                rhash__str(q->column_int(2), temp_str);
                                break;

                            case INT_CONSTANT_SYMBOL_TYPE:
                                temp_int = rhash__int(q->column_int(2));
                                to_string(temp_int, temp_str);
                                break;

                            case FLOAT_CONSTANT_SYMBOL_TYPE:
                                temp_double = rhash__float(q->column_int(2));
                                to_string(temp_double, temp_str);
                                break;

                            default:
                                temp_str.clear();
                                break;
                        }

                        return_val->append(temp_str);
                    }

                    // footer
                    return_val->append("\" ];");
                    return_val->append("\n");
                }
                q->reinitialize();
            }
        }
    }

    // footer
    return_val->append("}");
    return_val->append("\n");
}

void SMem_Manager::visualize_lti(smem_lti_id lti_id, unsigned int depth, std::string* return_val)
{
    // buffer
    std::string return_val2;

    soar_module::sqlite_statement* expand_q = thisAgent->SMem->smem_stmts->web_expand;

    uint64_t child_counter;

    std::string temp_str;
    std::string temp_str2;
    int64_t temp_int;
    double temp_double;

    std::queue<smem_vis_lti*> bfs;
    smem_vis_lti* new_lti;
    smem_vis_lti* parent_lti;

    std::map< smem_lti_id, smem_vis_lti* > close_list;
    std::map< smem_lti_id, smem_vis_lti* >::iterator cl_p;

    // header
    return_val->append("digraph smem_lti {");
    return_val->append("\n");

    // root
    {
        new_lti = new smem_vis_lti;
        new_lti->lti_id = lti_id;
        new_lti->level = 0;
        new_lti->lti_name = "@";
        new_lti->lti_name.append(std::to_string(lti_id));

        bfs.push(new_lti);
        close_list.insert(std::make_pair(lti_id, new_lti));

        new_lti = NULL;
    }

    // optionally depth-limited breadth-first-search of children
    while (!bfs.empty())
    {
        parent_lti = bfs.front();
        bfs.pop();

        child_counter = 0;

        // get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
        expand_q->bind_int(1, parent_lti->lti_id);
        while (expand_q->execute() == soar_module::row)
        {
            // identifier vs. constant
            if (expand_q->column_int(4) != SMEM_AUGMENTATIONS_NULL)
            {
                new_lti = new smem_vis_lti;
                new_lti->lti_id = expand_q->column_int(4);
                new_lti->level = (parent_lti->level + 1);

                // add node
                {
                    new_lti->lti_name.append("@");
                    to_string(new_lti->lti_id, temp_str);
                    new_lti->lti_name.append(temp_str);
                }


                // add linkage
                {
                    // get attribute
                    switch (expand_q->column_int(0))
                    {
                        case STR_CONSTANT_SYMBOL_TYPE:
                            rhash__str(expand_q->column_int(1), temp_str);
                            break;

                        case INT_CONSTANT_SYMBOL_TYPE:
                            temp_int = rhash__int(expand_q->column_int(1));
                            to_string(temp_int, temp_str);
                            break;

                        case FLOAT_CONSTANT_SYMBOL_TYPE:
                            temp_double = rhash__float(expand_q->column_int(1));
                            to_string(temp_double, temp_str);
                            break;

                        default:
                            temp_str.clear();
                            break;
                    }

                    // output linkage
                    return_val2.append(parent_lti->lti_name);
                    return_val2.append(" -> ");
                    return_val2.append(new_lti->lti_name);
                    return_val2.append(" [ label = \"");
                    return_val2.append(temp_str);
                    return_val2.append("\" ];");
                    return_val2.append("\n");
                }

                // prevent looping
                {
                    cl_p = close_list.find(new_lti->lti_id);
                    if (cl_p == close_list.end())
                    {
                        close_list.insert(std::make_pair(new_lti->lti_id, new_lti));

                        if ((depth == 0) || (new_lti->level < depth))
                        {
                            bfs.push(new_lti);
                        }
                    }
                    else
                    {
                        delete new_lti;
                    }
                }

                new_lti = NULL;
            }
            else
            {
                // add value node
                {
                    // get node name
                    {
                        temp_str2.assign(parent_lti->lti_name);
                        temp_str2.append("_");

                        to_string(child_counter, temp_str);
                        temp_str2.append(temp_str);
                    }

                    // get value
                    switch (expand_q->column_int(2))
                    {
                        case STR_CONSTANT_SYMBOL_TYPE:
                            rhash__str(expand_q->column_int(3), temp_str);
                            break;

                        case INT_CONSTANT_SYMBOL_TYPE:
                            temp_int = rhash__int(expand_q->column_int(3));
                            to_string(temp_int, temp_str);
                            break;

                        case FLOAT_CONSTANT_SYMBOL_TYPE:
                            temp_double = rhash__float(expand_q->column_int(3));
                            to_string(temp_double, temp_str);
                            break;

                        default:
                            temp_str.clear();
                            break;
                    }

                    // output node
                    return_val2.append("node [ shape = plaintext ];");
                    return_val2.append("\n");
                    return_val2.append(temp_str2);
                    return_val2.append(" [ label=\"");
                    return_val2.append(temp_str);
                    return_val2.append("\" ];");
                    return_val2.append("\n");
                }

                // add linkage
                {
                    // get attribute
                    switch (expand_q->column_int(0))
                    {
                        case STR_CONSTANT_SYMBOL_TYPE:
                            rhash__str(expand_q->column_int(1), temp_str);
                            break;

                        case INT_CONSTANT_SYMBOL_TYPE:
                            temp_int = rhash__int(expand_q->column_int(1));
                            to_string(temp_int, temp_str);
                            break;

                        case FLOAT_CONSTANT_SYMBOL_TYPE:
                            temp_double = rhash__float(expand_q->column_int(1));
                            to_string(temp_double, temp_str);
                            break;

                        default:
                            temp_str.clear();
                            break;
                    }

                    // output linkage
                    return_val2.append(parent_lti->lti_name);
                    return_val2.append(" -> ");
                    return_val2.append(temp_str2);
                    return_val2.append(" [ label = \"");
                    return_val2.append(temp_str);
                    return_val2.append("\" ];");
                    return_val2.append("\n");
                }

                child_counter++;
            }
        }
        expand_q->reinitialize();
    }

    // footer
    return_val2.append("}");
    return_val2.append("\n");

    // handle lti nodes at once
    {
        soar_module::sqlite_statement* act_q = thisAgent->SMem->smem_stmts->vis_lti_act;

        return_val->append("node [ shape = doublecircle ];");
        return_val->append("\n");

        for (cl_p = close_list.begin(); cl_p != close_list.end(); cl_p++)
        {
            return_val->append(cl_p->second->lti_name);
            return_val->append(" [ label=\"");
            return_val->append(cl_p->second->lti_name);
            return_val->append("\\n[");

            act_q->bind_int(1, cl_p->first);
            if (act_q->execute() == soar_module::row)
            {
                temp_double = act_q->column_double(0);
                to_string(temp_double, temp_str, 3, true);
                if (temp_double >= 0)
                {
                    return_val->append("+");
                }
                return_val->append(temp_str);
            }
            act_q->reinitialize();

            return_val->append("]\"");
            return_val->append(" ];");
            return_val->append("\n");

            delete cl_p->second;
        }
    }

    // transfer buffer after nodes
    return_val->append(return_val2);
}

std::set< smem_lti_id > SMem_Manager::print_lti(smem_lti_id lti_id, double lti_act, std::string* return_val, std::list<uint64_t>* history)
{
    std::set< smem_lti_id > next;

    std::string temp_str, temp_str2, temp_str3;
    int64_t temp_int;
    double temp_double;

    std::map< std::string, std::list< std::string > > augmentations;
    std::map< std::string, std::list< std::string > >::iterator lti_slot;
    std::list< std::string >::iterator slot_val;

    attach();

    soar_module::sqlite_statement* expand_q = thisAgent->SMem->smem_stmts->web_expand;

    return_val->append("@");
    to_string(lti_id, temp_str);
    return_val->append(temp_str);

    bool possible_id, possible_ic, possible_fc, possible_sc, possible_var, is_rereadable;

    // get direct children: attr_type, attr_hash, value_type, value_hash, value_letter, value_num, value_lti
    expand_q->bind_int(1, lti_id);
    while (expand_q->execute() == soar_module::row)
    {
        // get attribute
        switch (expand_q->column_int(0))
        {
            case STR_CONSTANT_SYMBOL_TYPE:
            {
                rhash__str(expand_q->column_int(1), temp_str);

                if (count(temp_str.begin(), temp_str.end(), ' ') > 0)
                {
                    temp_str.insert(0, "|");
                    temp_str += '|';
                    break;
                }

                soar::Lexer::determine_possible_symbol_types_for_string(temp_str.c_str(),
                        strlen(temp_str.c_str()),
                        &possible_id,
                        &possible_var,
                        &possible_sc,
                        &possible_ic,
                        &possible_fc,
                        &is_rereadable);

                bool has_angle_bracket = temp_str[0] == '<' || temp_str[temp_str.length() - 1] == '>';

                if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
                        (!is_rereadable) ||
                        has_angle_bracket)
                {
                    /* BUGBUG if in context where id's could occur, should check
                     possible_id flag here also */
                    temp_str.insert(0, "|");
                    temp_str += '|';
                }
                break;
            }
            case INT_CONSTANT_SYMBOL_TYPE:
                temp_int = rhash__int(expand_q->column_int(1));
                to_string(temp_int, temp_str);
                break;

            case FLOAT_CONSTANT_SYMBOL_TYPE:
                temp_double = rhash__float(expand_q->column_int(1));
                to_string(temp_double, temp_str);
                break;

            default:
                temp_str.clear();
                break;
        }

        // identifier vs. constant
        if (expand_q->column_int(4) != SMEM_AUGMENTATIONS_NULL)
        {
            temp_str2.clear();
            temp_str2.push_back('@');

            // lti
            temp_str2.push_back(static_cast<smem_lti_id>(expand_q->column_int(4)));

            // add to next
            next.insert(static_cast< smem_lti_id >(expand_q->column_int(4)));
        }
        else
        {
            switch (expand_q->column_int(2))
            {
                case STR_CONSTANT_SYMBOL_TYPE:
                {
                    rhash__str(expand_q->column_int(3), temp_str2);

                    if (count(temp_str2.begin(), temp_str2.end(), ' ') > 0)
                    {
                        temp_str2.insert(0, "|");
                        temp_str2 += '|';
                        break;
                    }

                    soar::Lexer::determine_possible_symbol_types_for_string(temp_str2.c_str(),
                            temp_str2.length(),
                            &possible_id,
                            &possible_var,
                            &possible_sc,
                            &possible_ic,
                            &possible_fc,
                            &is_rereadable);

                    bool has_angle_bracket = temp_str2[0] == '<' || temp_str2[temp_str2.length() - 1] == '>';

                    if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
                            (!is_rereadable) ||
                            has_angle_bracket)
                    {
                        /* BUGBUG if in context where id's could occur, should check
                         possible_id flag here also */
                        temp_str2.insert(0, "|");
                        temp_str2 += '|';
                    }
                    break;
                }
                case INT_CONSTANT_SYMBOL_TYPE:
                    temp_int = rhash__int(expand_q->column_int(3));
                    to_string(temp_int, temp_str2);
                    break;

                case FLOAT_CONSTANT_SYMBOL_TYPE:
                    temp_double = rhash__float(expand_q->column_int(3));
                    to_string(temp_double, temp_str2);
                    break;

                default:
                    temp_str2.clear();
                    break;
            }
        }

        augmentations[ temp_str ].push_back(temp_str2);
    }
    expand_q->reinitialize();

    // output augmentations nicely
    {
        for (lti_slot = augmentations.begin(); lti_slot != augmentations.end(); lti_slot++)
        {
            return_val->append(" ^");
            return_val->append(lti_slot->first);

            for (slot_val = lti_slot->second.begin(); slot_val != lti_slot->second.end(); slot_val++)
            {
                return_val->append(" ");
                return_val->append((*slot_val));
            }
        }
    }
    augmentations.clear();

    return_val->append(" [");
    to_string(lti_act, temp_str, 3, true);
    if (lti_act >= 0)
    {
        return_val->append("+");
    }
    return_val->append(temp_str);
    return_val->append("]");
    return_val->append(")\n");

    if (history != NIL)
    {
        std::ostringstream temp_string;
        return_val->append("SMem Access Cycle History\n");
        return_val->append("[-");
        for (std::list<uint64_t>::iterator history_item = (*history).begin(); history_item != (*history).end(); ++history_item)
        {
            if (history_item != (*history).begin())
            {
                return_val->append(", -");
            }
            temp_string << ((int64_t)thisAgent->SMem->smem_max_cycle - (int64_t)*history_item);
            return_val->append(temp_string.str());
            temp_string.str("");
        }
        return_val->append("]\n");
    }

    return next;
}

void SMem_Manager::print_store(std::string* return_val)
{
    // id, soar_letter, number
    soar_module::sqlite_statement* q = thisAgent->SMem->smem_stmts->vis_lti;
    while (q->execute() == soar_module::row)
    {
        print_smem_object(q->column_int(0), q->column_double(1), return_val);
    }
    q->reinitialize();
}

void SMem_Manager::print_smem_object(smem_lti_id lti_id, uint64_t depth, std::string* return_val, bool history)
{
    std::set< smem_lti_id > visited;
    std::pair< std::set< smem_lti_id >::iterator, bool > visited_ins_result;

    std::queue< std::pair< smem_lti_id, unsigned int > > to_visit;
    std::pair< smem_lti_id, unsigned int > c;

    std::set< smem_lti_id > next;
    std::set< smem_lti_id >::iterator next_it;

    soar_module::sqlite_statement* act_q = thisAgent->SMem->smem_stmts->vis_lti_act;
    soar_module::sqlite_statement* hist_q = thisAgent->SMem->smem_stmts->history_get;
    soar_module::sqlite_statement* lti_access_q = thisAgent->SMem->smem_stmts->lti_access_get;
    unsigned int i;


    // initialize queue/set
    to_visit.push(std::make_pair(lti_id, 1u));
    visited.insert(lti_id);

    while (!to_visit.empty())
    {
        c = to_visit.front();
        to_visit.pop();

        // output leading spaces ala depth
        for (i = 1; i < c.second; i++)
        {
            return_val->append("  ");
        }

        // get lti info
        {
            act_q->bind_int(1, c.first);
            act_q->execute();

            //Look up activation history.
            std::list<uint64_t> access_history;
            if (history)
            {
                lti_access_q->bind_int(1, c.first);
                lti_access_q->execute();
                uint64_t n = lti_access_q->column_int(0);
                lti_access_q->reinitialize();
                hist_q->bind_int(1, c.first);
                hist_q->execute();
                for (int i = 0; i < n && i < 10; ++i) //10 because of the length of the history record kept for smem.
                {
                    if (thisAgent->SMem->smem_stmts->history_get->column_int(i) != 0)
                    {
                        access_history.push_back(hist_q->column_int(i));
                    }
                }
                hist_q->reinitialize();
            }

            if (history && !access_history.empty())
            {
                next = print_lti(c.first, act_q->column_double(0), return_val, &(access_history));
            }
            else
            {
                next = print_lti(c.first, act_q->column_double(0), return_val);
            }

            // done with lookup
            act_q->reinitialize();

            // consider further depth
            if (c.second < depth)
            {
                for (next_it = next.begin(); next_it != next.end(); next_it++)
                {
                    visited_ins_result = visited.insert((*next_it));
                    if (visited_ins_result.second)
                    {
                        to_visit.push(std::make_pair((*next_it), c.second + 1u));
                    }
                }
            }
        }
    }
}
