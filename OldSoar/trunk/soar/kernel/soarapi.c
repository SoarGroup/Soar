/**
 * \file soarapi.c
 *                      The High Level Interface to Soar
 *
 *
 * Copyright 1995-2004 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 *
 * $Id$
 *
 */

#include "soarkernel.h"
#include "soarapi_datatypes.h"
#include "soarapi.h"
#include "soarapiUtils.h"
#include "soarapiCallbacks.h"
#include "scheduler.h"

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 1:    CRITICAL FUNCTIONS
 *
 *               - (Re)Initializing Soar
 *	         - Creating Agents
 *               - Destroying Agents
 *               - Starting and Stopping Agents
 *
 * *************************************************************************
 * *************************************************************************
 */

/*
 *----------------------------------------------------------------------
 *
 * soar_ReInitSoar
 *
 *----------------------------------------------------------------------
 */

int soar_ReInitSoar(int argc, const char *argv[], soarResult * res)
{

    argv = argv;
    argc = argc;

    clearSoarResultResult(res);
    soar_cReInitSoar();

    return SOAR_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_CreateAgent
 *
 *----------------------------------------------------------------------
 */

int soar_CreateAgent(int argc, const char *argv[], soarResult * res)
{
    char too_few[] = "Too few arguments, a name must be supplied.";
    char too_many[] = "Too many arguments, only a name may be supplied.";

    if (argc < 2) {
        setSoarResultResult(res, too_few);
        return SOAR_ERROR;
    }

    if (argc > 3) {
        setSoarResultResult(res, too_many);
        return SOAR_ERROR;
    }

    soar_cCreateAgent(argv[1]);
    clearSoarResultResult(res);
    return SOAR_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_Run --
 *
 *----------------------------------------------------------------------
 */

int soar_Run(int argc, const char *argv[], soarResult * res)
{
    agent *the_agent;
    cons *c;
    int parse_result;
    long go_number;
    enum go_type_enum go_type;
    Symbol *go_slot_attr;
    goal_stack_level go_slot_level;
    bool self_only = FALSE;
    bool single_agent = FALSE;

    static bool executing = FALSE;

    if (executing == TRUE) {
        /* 
         * Disallow attempts to recursively enter run-related actions.
         * This would lead to seg faults since the agent code is not
         * re-entrant.  Note that this is a general problem, so this
         * does not solve the problem in a general fashion.  This
         * strategy should probably be used on long running commands,
         * however, unless a more general solution is found.
         *
         * This possibility is easy to generate when "run" is tied to
         * a button on the GUI and the user clicks faster than "run"
         * can run.
         */
        clearSoarResultResult(res);
        return SOAR_OK;
    } else {
        executing = TRUE;
    }

    the_agent = soar_agent;

    c = all_soar_agents;
    if (c->rest == NIL)
        single_agent = TRUE;

    go_number = 1;              /* was the_agent->go_number */
    go_type = GO_DECISION;      /* was the_agent->go_type   */
    /*  go_slot_attr  = the_agent->go_slot_attr;
       go_slot_level = the_agent->go_slot_level;
     */
    parse_result = parse_run_command(argc, argv, &go_number, &go_type, &go_slot_attr, &go_slot_level, &self_only, res);
    if (parse_result == SOAR_OK) {

        if ((self_only) || (single_agent)) {

            run_current_agent(go_number, go_type, go_slot_attr, go_slot_level);
            current_agent(stop_soar) = TRUE; /* fix for bugzilla bug #353 */
        } else {                /* run all agents */

            /* set the params for all agents...
             * this will have to be different for context slots ???
             */

            run_all_agents(go_number, go_type, go_slot_attr, go_slot_level);

            /* fix for bugzilla bug #353 */
            for (c = all_soar_agents; c != NIL; c = c->rest) {
                the_agent = (agent *) c->first;
                the_agent->stop_soar = TRUE;
            }

        }
        executing = FALSE;
        clearSoarResultResult(res);
        return SOAR_OK;
    } else {
        executing = FALSE;
        return SOAR_ERROR;
    }

}

/*
 *----------------------------------------------------------------------
 *
 * soar_DestroyAgent --
 *
 *----------------------------------------------------------------------
 */

int soar_DestroyAgent(int argc, const char *argv[], soarResult * res)
{

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments");
        return SOAR_ERROR;
    }

    if (argc < 2) {
        setSoarResultResult(res, "Expected agent name but none given.");
        return SOAR_ERROR;
    }

    soar_cDestroyAgentByName(argv[1]);
    clearSoarResultResult(res);
    return SOAR_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_Quit --
 *
 *----------------------------------------------------------------------
 */

int soar_Quit(int argc, const char *argv[], soarResult * res)
{

    argv = argv;
    argc = argc;
    res = res;

    soar_cQuit();
    return SOAR_OK;
}

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 2:    MODIFYING AGENT MEMORY
 *
 *               - Production Memory
 *	         - Working Memory
 *
 * *************************************************************************
 * *************************************************************************
 */

/*
 *----------------------------------------------------------------------
 *
 * soar_ReteNet --
 *
 *----------------------------------------------------------------------
 */

int soar_ReteNet(int argc, const char *argv[], soarResult * res)
{

    if (argc < 3) {
        setSoarResultResult(res, "Too few arguments.\nUsage: rete-net {-save | -load} filename.");
        return SOAR_ERROR;
    }

    if (argc > 3) {
        setSoarResultResult(res, "Too many arguments.\nUsage: rete-net {-save | -load} filename.");
        return SOAR_ERROR;
    }

    if (string_match("-save", argv[1])) {
        if (!soar_cSaveReteNet(argv[2])) {
            clearSoarResultResult(res);
            return SOAR_OK;
        }
        /* TODO:
         *  fill in string based on return value 
         */
        setSoarResultResult(res, "Failed to save rete net\n");
        return SOAR_ERROR;
    } else if (string_match("-load", argv[1])) {
        if (!soar_cLoadReteNet(argv[2])) {
            clearSoarResultResult(res);
            return SOAR_OK;
        }
        /* TODO:
         *  fill in string based on return value 
         */
        setSoarResultResult(res, "Failed to load rete net\n");
        return SOAR_ERROR;
    } else {
        setSoarResultResult(res, "Unrecognized argument to ReteNet command: %s. Should be -save|-load", argv[1]);
        return SOAR_ERROR;
    }

}

/*
 *----------------------------------------------------------------------
 *
 * soar_AddWme --
 *
 *----------------------------------------------------------------------
 */

int soar_AddWme(int argc, const char *argv[], soarResult * res)
{

    static char *too_few_args = "Too few arguments.\nUsage: add-wme id [^] { attribute | '*'} { value | '*' } [+]";
    static char *too_many_args = "Too many arguments.\nUsage: add-wme id [^] { attribute | '*'} { value | '*' } [+]";

    int attr_index;
    bool acceptable;
    psoar_wme psw;

    /* to next command argument       */
    if (argc < 4) {
        setSoarResultResult(res, too_few_args);
        return SOAR_ERROR;
    }

    if (string_match(argv[2], "^"))
        attr_index = 3;
    else
        attr_index = 2;

    if (argc < (attr_index + 2)) {
        setSoarResultResult(res, too_few_args);
        return SOAR_ERROR;
    }

    if (argc > (attr_index + 3)) {
        setSoarResultResult(res, too_many_args);
        return SOAR_ERROR;
    }

    acceptable = FALSE;

    if (argc > (attr_index + 2)) {
        if (string_match("+", argv[attr_index + 2])) {
            acceptable = TRUE;
        } else {
            setSoarResultResult(res, "Expected acceptable preference (+) or nothing, got %s\n", argv[attr_index + 2]);
            return SOAR_ERROR;
        }

    }

    if (soar_cAddWme(argv[1], argv[attr_index], argv[attr_index + 1], acceptable, &psw) <= 0) {
        return SOAR_ERROR;
    }

    /* SW NOTE
     * The old way to do this is commented out below
     * the reason for the change is that print_wme_for_tcl
     * in only used here, and moreover, we don't want to 
     * use a print call back since wmes added using this method
     * will get added into the log file which really doesn't
     * make a whole lot of sense.
     */
    /*
       soar_cPushCallback((soar_callback_agent) soar_agent, 
       PRINT_CALLBACK,
       (soar_callback_fn) cb_soarResult_AppendResult, 
       (soar_callback_data) res,
       (soar_callback_free_fn) NULL);

       print_wme_for_tcl((wme *)psw);
       soar_cPopCallback((soar_callback_agent) soar_agent, PRINT_CALLBACK);
     */

    setSoarResultResult(res, "%lu: ", ((wme *) psw)->timetag);
    appendSymbolsToSoarResultResult(res, "%y ^%y %y", ((wme *) psw)->id, ((wme *) psw)->attr, ((wme *) psw)->value);
    if (((wme *) psw)->acceptable)
        appendSoarResultResult(res, " +");

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_RemoveWme --
 *
 *----------------------------------------------------------------------
 */

int soar_RemoveWme(int argc, const char *argv[], soarResult * res)
{
    int num;

    if (argc == 1) {
        setSoarResultResult(res, "Too few arguments, should be: remove-wme integer");
        return SOAR_ERROR;
    }

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments, should be: remove-wme integer");
        return SOAR_ERROR;
    }

    if (getInt(argv[1], &num) == SOAR_OK) {
        switch (soar_cRemoveWmeUsingTimetag(num)) {

        case -1:
            setSoarResultResult(res, "No wme with timetag %d", num);
            return SOAR_ERROR;
            break;

        case -2:
            setSoarResultResult(res, "Failed to remove wme with timetag %d", num);
            return SOAR_ERROR;
            break;
        }

    } else {
        setSoarResultResult(res, "Unrecognized argument to remove-wme command: %s", argv[1]);
        return SOAR_ERROR;
    }

    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Excise --
 *
 *----------------------------------------------------------------------
 */

int soar_Excise(int argc, const char *argv[], soarResult * res)
{
    int i;

    if (argc == 1) {
        setSoarResultResult(res,
                            "No arguments given.\nUsage: excise production-name | -chunks | -default | -task | -user | -all");
        return SOAR_ERROR;
    }

    for (i = 1; i < argc; i++) {
        if (string_match_up_to(argv[i], "-chunks", 2)) {
            soar_cExciseAllProductionsOfType(CHUNK_PRODUCTION_TYPE);
            soar_cExciseAllProductionsOfType(JUSTIFICATION_PRODUCTION_TYPE);
        } else if (string_match_up_to(argv[i], "-default", 2))
            soar_cExciseAllProductionsOfType(DEFAULT_PRODUCTION_TYPE);
        else if (string_match_up_to(argv[i], "-task", 2))
            soar_cExciseAllTaskProductions();
        else if (string_match_up_to(argv[i], "-user", 2))
            soar_cExciseAllProductionsOfType(USER_PRODUCTION_TYPE);
        else if (string_match_up_to(argv[i], "-all", 2))
            soar_cExciseAllProductions();
        else if (soar_cExciseProductionByName(argv[i])) {
            setSoarResultResult(res, "Unrecognized Production name or argument: %s", argv[i]);
            return SOAR_ERROR;
        }
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/* *************************************************************************
 * *************************************************************************
 *   
 * SECTION 3:    ACCESSING, MODIFYING & WATCHING THE AGENT'S STATE
 *
 *
 * *************************************************************************
 * *************************************************************************
 */

#ifdef USE_CAPTURE_REPLAY

/*
 *----------------------------------------------------------------------
 *
 * soar_CaptureInput --
 *
 *----------------------------------------------------------------------
 */
int soar_CaptureInput(int argc, const char *argv[], soarResult * res)
{

    char *too_many = "Too many arguments, should be: capture-input [-open pathname | -query | -close]";
    char *too_few = "Too few arguments, should be: capture-input [-open pathname | -query | -close]";

    print(" %d args\n", argc);
    if (argc < 2) {
        setSoarResultResult(res, "The capture file is ");

        if (current_agent(capture_fileID)) {
            appendSoarResultResult(res, "open.  Use capture-input -close to close the file.");
        } else {
            appendSoarResultResult(res, "closed.");
        }
        return SOAR_OK;
    }

    if (argc > 3) {
        setSoarResultResult(res, too_many);
        return SOAR_ERROR;
    }

    if (string_match_up_to("-query", argv[1], 2)) {
        if (argc == 2) {
            if (current_agent(capture_fileID)) {
                setSoarResultResult(res, "open");
            } else {
                setSoarResultResult(res, "closed");
            }
        } else {
            setSoarResultResult(res, too_many);
            return SOAR_ERROR;
        }
    } else if (string_match_up_to("-close", argv[1], 2)) {
        if (argc == 2) {
            if (current_agent(capture_fileID)) {
                if (!(soar_ecCaptureInput(NIL))) {
                    setSoarResultResult(res, "capture file closed");
                    return SOAR_OK;
                }
                setSoarResultResult(res, "Error closing file");
                return SOAR_ERROR;

            } else {
                setSoarResultResult(res, "Attempt to close non-existant capture file");
                return SOAR_ERROR;
            }
        } else if (argc > 2) {
            setSoarResultResult(res, too_many);
            return SOAR_ERROR;
        }
    } else if (string_match_up_to("-open", argv[1], 2)) {
        if (argc == 3) {
            if (current_agent(capture_fileID)) {
                setSoarResultResult(res, "Attempt to open new capture file before closing an old capture file");
                return SOAR_ERROR;
            } else {
                if (!soar_ecCaptureInput(argv[2])) {
                    return SOAR_OK;
                }
                setSoarResultResult(res, "Error opening file.");
                return SOAR_ERROR;
            }
        } else {
            setSoarResultResult(res, too_few);
            return SOAR_ERROR;
        }
    } else {
        setSoarResultResult(res, "Invalid Arguments.  Use: capture-input [-open pathname | -query | -close]");
        return SOAR_ERROR;
    }

    return SOAR_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Replay --
 *
 *----------------------------------------------------------------------
 */
int soar_ReplayInput(int argc, const char *argv[], soarResult * res)
{

    char *too_few = "Too few arguments, should be: replay-input [-open pathname | -query | -close]";
    char *too_many = "Too many arguments, should be: replay-input [-open pathname | -query | -close]";

    if (argc < 2) {
        setSoarResultResult(res, "The replay file is ");
        if (current_agent(replay_fileID)) {
            appendSoarResultResult(res, "open.  Use replay-input -close to close the file.");
        } else {
            appendSoarResultResult(res, "closed.");
        }

        return SOAR_OK;
    }
    if (argc > 3) {
        setSoarResultResult(res, too_many);
        return SOAR_ERROR;
    }

    if (string_match_up_to("-query", argv[1], 2)) {
        if (argc == 2) {
            if (current_agent(replay_fileID)) {
                setSoarResultResult(res, "open");
            } else {
                setSoarResultResult(res, "closed");
            }
        } else {
            setSoarResultResult(res, too_many);
            return SOAR_ERROR;
        }
    }

    else if (string_match_up_to("-close", argv[1], 2)) {
        if (argc == 2) {

            if (!soar_ecReplayInput(NIL)) {
                setSoarResultResult(res, "Replay Terminated.");
                return SOAR_OK;
            } else {
                setSoarResultResult(res, "Error Terminating Replay");
                return SOAR_ERROR;
            }
        } else if (argc > 2) {
            setSoarResultResult(res, too_many);
            return SOAR_ERROR;
        }
    } else {                    /* Either we have a file open request or there is an error */

        if (string_match_up_to("-open", argv[1], 2)) {
            if (argc < 3) {
                setSoarResultResult(res, too_few);
                return SOAR_ERROR;
            } else if (!soar_ecReplayInput(argv[2])) {
                setSoarResultResult(res, "opened.");
                return SOAR_OK;
            } else {
                setSoarResultResult(res, "Error opening replay-file.");
                return SOAR_ERROR;
            }
        }
    }
    setSoarResultResult(res, "Invalid arguments");
    return SOAR_ERROR;
}

#endif

/*
 *----------------------------------------------------------------------
 *
 * soar_ChunkNameFormat --
 *
 *----------------------------------------------------------------------
 */
int soar_ChunkNameFormat(int argc, const char *argv[], soarResult * res)
{
    unsigned long tmp_chunk_count;
    int i;
    bool seen_long_or_short;

    if (argc == 1) {
        setSoarResultResult(res,
                            "No arguments given.\nUsage: chunk-name-format [-short|-long] [-prefix [<prefix>]] [-count [<start-chunk-number>]]");
        return SOAR_ERROR;
    }

    seen_long_or_short = FALSE;
    for (i = 1; i < argc; i++) {

        if (string_match_up_to(argv[i], "-short", 2)) {
            if (seen_long_or_short) {
                setSoarResultResult(res, "-long and -short are exclusive options", argv[i]);
                return SOAR_ERROR;
            } else {
                seen_long_or_short = TRUE;
                soar_cSetChunkNameLong(FALSE);
            }

        } else if (string_match_up_to(argv[i], "-long", 2)) {
            if (seen_long_or_short) {
                setSoarResultResult(res, "-long and -short are exclusive options", argv[i]);
                return SOAR_ERROR;
            } else {
                seen_long_or_short = TRUE;
                soar_cSetChunkNameLong(TRUE);
            }

        } else if (string_match_up_to(argv[i], "-prefix", 2)) {
            if ((i + 1 >= argc) || (*argv[i + 1] == '-'))
                print("%s\n", current_agent(chunk_name_prefix));

            else if (soar_cSetChunkNamePrefix(argv[++i]) == SOAR_ERROR) {
                setSoarResultResult(res, "Prefix-string contains illegal characters.");
                return SOAR_ERROR;
            }

        } else if (string_match_up_to(argv[i], "-count", 2)) {
            if ((i + 1 >= argc) || (*argv[i + 1] == '-'))
                print("%lu\n", current_agent(chunk_count));
            else if (sscanf(argv[i + 1], "%lu", &tmp_chunk_count) == 1) {
                i++;

                if (soar_cSetChunkNameCount(tmp_chunk_count))
                    setSoarResultResult(res, "chunk-name-format: couldn't do that");
                return SOAR_ERROR;

            } else {
                setSoarResultResult(res, "chunk-name-format: expected number after -count; got \"%s\"", argv[i]);
                return SOAR_ERROR;
            }
        } else {
            setSoarResultResult(res, "Unrecognized argument: %s", argv[i]);
            return SOAR_ERROR;
        }
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/* kjh (B14) end */

/*
 *----------------------------------------------------------------------
 *
 * soar_Learn --
 *
 *----------------------------------------------------------------------
 */

#define SOAR_LEARN_BUFF_SIZE 1024
int soar_Learn(int argc, const char *argv[], soarResult * res)
{

    if (argc == 1) {
        print_current_learn_settings();
        clearSoarResultResult(res);
        return SOAR_OK;
    }

    {
        int i;

        for (i = 1; i < argc; i++) {
            if (string_match("-on", argv[i]))
                soar_cSetLearning(ON);
            else if (string_match_up_to("-only", argv[i], 3))
                soar_cSetLearning(ONLY);
            else if (string_match_up_to("-except", argv[i], 2))
                soar_cSetLearning(EXCEPT);
            else if (string_match_up_to("-off", argv[i], 3))
                soar_cSetLearning(OFF);
            else if (string_match_up_to("-all-levels", argv[i], 2))
                soar_cSetLearning(ALL_LEVELS);
            else if (string_match_up_to("-bottom-up", argv[i], 2))
                soar_cSetLearning(BOTTOM_UP);
            else if (string_match_up_to("-list", argv[i], 2)) {
                cons *c;
                char buff[SOAR_LEARN_BUFF_SIZE];

                print_current_learn_settings();
                setSoarResultResult(res, "force-learn states (when learn = -only):\n");
                for (c = current_agent(chunky_problem_spaces); c != NIL; c = c->rest) {
                    symbol_to_string((Symbol *) (c->first), TRUE, buff, SOAR_LEARN_BUFF_SIZE);
                    appendSoarResultResult(res, buff);
                }
                appendSoarResultResult(res, "\ndont-learn states (when learn = -except):\n");
                for (c = current_agent(chunk_free_problem_spaces); c != NIL; c = c->rest) {
                    symbol_to_string((Symbol *) (c->first), TRUE, buff, SOAR_LEARN_BUFF_SIZE);
                    appendSoarResultResult(res, buff);
                }
                return SOAR_OK;
            } else {
                setSoarResultResult(res, "Unrecognized argument to learn command: %s", argv[i]);
                return SOAR_ERROR;
            }
        }
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_MaxElaborations --
 *
 *----------------------------------------------------------------------
 */

int soar_MaxElaborations(int argc, const char *argv[], soarResult * res)
{
    int num;

    if (argc == 1) {
        setSoarResultResult(res, "%ld", current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM]);
        return SOAR_OK;
    }

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments, should be: max-elaborations [integer]");
        return SOAR_ERROR;
    }

    if (getInt(argv[1], &num) == SOAR_OK) {
        set_sysparam(MAX_ELABORATIONS_SYSPARAM, num);
    } else {
        setSoarResultResult(res, "Expected integer for new maximum elaborations count: %s", argv[1]);
        return SOAR_ERROR;
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_MaxChunks --
 *
 *----------------------------------------------------------------------
 */

int soar_MaxChunks(int argc, const char *argv[], soarResult * res)
{
    int num;

    if (argc == 1) {
        setSoarResultResult(res, "%ld", current_agent(sysparams)[MAX_CHUNKS_SYSPARAM]);
        return SOAR_OK;
    }

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments, should be: max-chunks [integer]");
        return SOAR_ERROR;
    }

    if (getInt(argv[1], &num) == SOAR_OK) {
        set_sysparam(MAX_CHUNKS_SYSPARAM, num);
    } else {
        setSoarResultResult(res, "Expected integer for new maximum chunks count: %s", argv[1]);
        return SOAR_OK;
    }

    clearSoarResultResult(res);
    return SOAR_OK;
}

/* REW: begin 09.15.96 */
/*
 *----------------------------------------------------------------------
 *
 * soar_Operand2 --
 *
 *----------------------------------------------------------------------
 */

#define SOAR_OPERAND2_BUFFER_SIZE 1024
int soar_Operand2(int argc, const char *argv[], soarResult * res)
{

#ifndef SOAR_8_ONLY
    char buffer[SOAR_OPERAND2_BUFFER_SIZE];
    bool turnOn;

    if (argc == 1) {
        setSoarResultResult(res, "Soar8 Mode is %s", (current_agent(operand2_mode) == TRUE) ? "ON" : "OFF");
        return SOAR_OK;
    }

    if (argc != 2) {
        setSoarResultResult(res, "argument should be -on|-off\n");
        return SOAR_ERROR;
    }

    if (string_match("-on", argv[1]))
        turnOn = TRUE;
    else if (string_match("-off", argv[1]))
        turnOn = FALSE;
    else {
        setSoarResultResult(res, "unrecognized argument, should be -on|-off\n");
        return SOAR_ERROR;
    }

    if (!soar_cSetOperand2(turnOn)) {

        if (turnOn) {

#if MICRO_VERSION_NUMBER > 0
            snprintf(buffer, SOAR_OPERAND2_BUFFER_SIZE,
                     "Soar%d.%d.%d %s on : reinitializing Soar",
                     MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, MICRO_VERSION_NUMBER, OPERAND2_MODE_NAME);
            buffer[SOAR_OPERAND2_BUFFER_SIZE - 1] = 0;  /* snprintf doesn't set last char to null if output is truncated */
#else
            snprintf(buffer, SOAR_OPERAND2_BUFFER_SIZE,
                     "Soar%d.%d %s on : reinitializing Soar",
                     MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, OPERAND2_MODE_NAME);
            buffer[SOAR_OPERAND2_BUFFER_SIZE - 1] = 0;  /* snprintf doesn't set last char to null if output is truncated */
#endif

        } else {

#if MICRO_VERSION_NUMBER > 0
            snprintf(buffer, SOAR_OPERAND2_BUFFER_SIZE,
                     "Soar%d.%d.%d - running in Soar7 mode:  reinitializing Soar",
                     MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, MICRO_VERSION_NUMBER);
            buffer[SOAR_OPERAND2_BUFFER_SIZE - 1] = 0;  /* snprintf doesn't set last char to null if output is truncated */
#else
            snprintf(buffer, SOAR_OPERAND2_BUFFER_SIZE,
                     "Soar%d.%d - running in Soar7 mode: reinitializing Soar",
                     MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER);
            buffer[SOAR_OPERAND2_BUFFER_SIZE - 1] = 0;  /* snprintf doesn't set last char to null if output is truncated */
#endif

        }
        print("%s\n", buffer);
        return SOAR_OK;
    }
    setSoarResultResult(res, "Cannot change Soar8 mode after productions have been loaded.  Excise all productions first.");
    return SOAR_ERROR;

#else

    if (argc == 1) {
        setSoarResultResult(res, "Soar8 Mode is ON");
        return SOAR_OK;
    }

    if (argc == 2 && string_match(argv[2], "-on")) {
        setSoarResultResult(res, "Soar8 Mode is already ON");
        return SOAR_OK;
    } else {
        setSoarResultResult(res, "Soar8 Mode must remain on in this build");
        return SOAR_ERROR;
    }

#endif

}

/* REW: end   09.15.96 */

/* REW: begin 10.24.97 */
/*
 *----------------------------------------------------------------------
 *
 * soar_WaitSNC --
 *
 *----------------------------------------------------------------------
 */

int soar_WaitSNC(int argc, const char *argv[], soarResult * res)
{

    if (argc == 1) {
        print("waitsnc is %s\n\n", (current_agent(waitsnc) == TRUE) ? "ON" : "OFF");
        return SOAR_OK;
    }

    {
        int i;

        for (i = 1; i < argc; i++) {
            if (string_match("-on", argv[i])) {
                soar_cSetWaitSNC(TRUE);
                print("waitsnc is ON\n\n");
            }

            else if (string_match_up_to("-off", argv[i], 3)) {
                soar_cSetWaitSNC(FALSE);
                print("waitsnc is OFF\n\n");

            } else {
                setSoarResultResult(res, "Unrecognized argument to the WaitSNC command: %s", argv[i]);
                return SOAR_ERROR;
            }
        }
    }

    return SOAR_OK;
}

/* REW: end   10.24.97 */

/*
 *----------------------------------------------------------------------
 *
 * soar_InputPeriod --
 *
 *----------------------------------------------------------------------
 */

int soar_InputPeriod(int argc, const char *argv[], soarResult * res)
{
    int period;

    if (argc == 1) {
        setSoarResultResult(res, "%d", soar_cGetInputPeriod());
        return SOAR_OK;
    }

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments, should be: input-period [integer]");
        return SOAR_ERROR;
    }

    if (getInt(argv[1], &period) == SOAR_OK) {

        if (soar_cSetInputPeriod(period) < 0) {
            setSoarResultResult(res, "Integer for new input period must be >= 0, not %s", argv[1]);
            return SOAR_ERROR;
        }
    } else {
        setSoarResultResult(res, "Expected integer for new input period: %s", argv[1]);
        return SOAR_ERROR;
    }

    clearSoarResultResult(res);
    return SOAR_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_MultiAttributes --
 *
 *----------------------------------------------------------------------
 */

int soar_MultiAttributes(int argc, const char *argv[], soarResult * res)
{
    int num;

    if (argc == 1) {
        print_multi_attribute_symbols();
        clearSoarResultResult(res);
        return SOAR_OK;
    }

    if (argc > 3) {
        setSoarResultResult(res, "Too many arguments, should be: multi-attribute [symbol] [value]");
        return SOAR_ERROR;
    }

    if (argc < 3) {

        if (soar_cMultiAttributes(argv[1], 10) == -1) {
            setSoarResultResult(res, "Expected symbolic constant for symbol but got: %s", argv[1]);
            return SOAR_ERROR;
        }
    } else {
        if (getInt(argv[2], &num) != SOAR_OK) {
            setSoarResultResult(res, "Non-integer given for attribute count: %s", argv[2]);
            return SOAR_ERROR;
        } else if (soar_cMultiAttributes(argv[1], num) != 0) {

            setSoarResultResult(res, "Invalid arguments.  Usage: multi-attributes <attr> <val>");
            return SOAR_ERROR;
        }

    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_NumericIndifferentMode --
 *
 *----------------------------------------------------------------------
 */

int soar_NumericIndifferentMode(int argc, const char *argv[], soarResult * res)
{

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments.\nUsage: numeric-indifferent-mode -sum | -avg");
        return SOAR_ERROR;
    }

    if (argc == 2) {
        if (string_match_up_to("-sum", argv[1], 2)) {
            current_agent(numeric_indifferent_mode) = NUMERIC_INDIFFERENT_MODE_SUM;
        } else if (string_match_up_to("-avg", argv[1], 2)) {
            current_agent(numeric_indifferent_mode) = NUMERIC_INDIFFERENT_MODE_AVG;
        } else {
            setSoarResultResult(res,
                                "Unrecognized argument to %s: %s.  either '-avg' or '-sum' was expected.",
                                argv[0], argv[1]);
            return SOAR_ERROR;
        }
    }

    switch (current_agent(numeric_indifferent_mode)) {
    case NUMERIC_INDIFFERENT_MODE_SUM:
        setSoarResultResult(res, "-sum");
        break;
    case NUMERIC_INDIFFERENT_MODE_AVG:
        setSoarResultResult(res, "-avg");
        break;
    default:
        setSoarResultResult(res, "???");
        break;
    }

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_OSupportMode --
 *
 *----------------------------------------------------------------------
 */

int soar_OSupportMode(int argc, const char *argv[], soarResult * res)
{

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments.\nUsage: o-support-mode 0|1|2|3");
        return SOAR_ERROR;
    }

    if (argc == 2) {
        if (!strcmp(argv[1], "0")) {
            current_agent(o_support_calculation_type) = 0;
        } else if (!strcmp(argv[1], "1")) {
            current_agent(o_support_calculation_type) = 1;
        } else if (!strcmp(argv[1], "2")) {
            current_agent(o_support_calculation_type) = 2;
        } else if (!strcmp(argv[1], "3")) {
            current_agent(o_support_calculation_type) = 3;
        } else if (!strcmp(argv[1], "4")) {
            current_agent(o_support_calculation_type) = 4;
        } else {
            setSoarResultResult(res,
                                "Unrecognized argument to %s: %s.  Integer 0, 1, 2, or 3 expected.", argv[0], argv[1]);
            return SOAR_ERROR;
        }
    }

    setSoarResultResult(res, "%d", current_agent(o_support_calculation_type));
    return SOAR_OK;
}

/* End of core commands */

/*
 *----------------------------------------------------------------------
 *
 * soar_ExplainBacktraces --
 *
 *----------------------------------------------------------------------
 */

int soar_ExplainBacktraces(int argc, const char *argv[], soarResult * res)
{

    if (argc == 1) {
        explain_list_chunks();
        clearSoarResultResult(res);
        return SOAR_OK;
    }

    {
        int cond_num;

        get_lexeme_from_string(argv[1]);

        if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) {
            if (argc > 2) {
                if (string_match("-full", argv[2])) {
                    /* handle the 'explain name -full' case */

                    soar_ecExplainChunkTrace(current_agent(lexeme.string));
                } else if (getInt(argv[2], &cond_num) == SOAR_OK) {
                    /* handle the 'explain name <cond-num>' case */

                    soar_ecExplainChunkCondition(current_agent(lexeme.string), cond_num);
                } else {
                    setSoarResultResult(res,
                                        "Unexpected argument to %s %s: %s.  Should be -full or integer.",
                                        argv[0], argv[1], argv[2]);
                    return SOAR_ERROR;
                }
            } else {
                /* handle the 'explain name' case */

                soar_ecExplainChunkConditionList(current_agent(lexeme.string));
            }
        } else {
            setSoarResultResult(res,
                                "Unexpected argument to %s: %s.  Should be symbolic constant or integer.",
                                argv[0], argv[1]);
            return SOAR_ERROR;
        }
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_FiringCounts --
 *
 *----------------------------------------------------------------------
 */

int soar_FiringCounts(int argc, const char *argv[], soarResult * res)
{
    int num_requested = 0;

    res = res;

    if (argc > 1) {
        if (getInt(argv[1], &num_requested) != SOAR_OK) {
            int i;

            for (i = 1; i < argc; i++) {
                soar_ecPrintFiringsForProduction(argv[i]);
            }

            return SOAR_OK;
        }
    }

    soar_ecPrintTopProductionFirings((argc == 1) ? 20 : num_requested);
    return SOAR_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_FormatWatch --
 *
 *----------------------------------------------------------------------
 */

int soar_FormatWatch(int argc, const char *argv[], soarResult * res)
{
    static char *too_few_args =
        "Too few arguments.\nUsage: format-watch {-object | -stack} [{{ -add {s|o|*} [name] \"format\" }|{-remove {s|o|*} [name]}}]";
    static char *too_many_args =
        "Too many arguments.\nUsage: format-watch {-object | -stack} [{{ -add {s|o|*} [name] \"format\" }|{-remove {s|o|*} [name]}}]";

    bool stack_trace;
    int type_restriction;
    Symbol *name_restriction = NIL;
    bool remove;
    int format_arg = 0;         /* Initialized to placate gcc -Wall */

    if (argc == 1) {
        setSoarResultResult(res, too_few_args);
        return SOAR_ERROR;
    }

    /* --- set stack_trace depending on which option was given --- */
    if (string_match("-stack", argv[1])) {
        stack_trace = TRUE;
    } else if (string_match("-object", argv[1])) {
        stack_trace = FALSE;
    } else {
        setSoarResultResult(res, "Unrecognized option to format-watch : %s", argv[1]);
        return SOAR_ERROR;
    }

    /* --- if no further args, print all trace formats of that type --- */

    if (argc == 2) {
        print_all_trace_formats(stack_trace);
        clearSoarResultResult(res);
        return SOAR_OK;
    }

    /* --- next argument must be either -add or -remove --- */
    if (string_match("-add", argv[2])) {
        remove = FALSE;
    } else if (string_match("-remove", argv[2])) {
        remove = TRUE;
    } else {
        setSoarResultResult(res, "Unrecognized option to format-watch %s: %s", argv[1], argv[2]);
        return SOAR_ERROR;
    }

    if (argc == 3) {
        setSoarResultResult(res, too_few_args);
        return SOAR_ERROR;
    }

    /* --- read context item argument: s, o, or '*' --- */

    if (string_match("s", argv[3])) {
        type_restriction = FOR_STATES_TF;
    } else if (string_match("o", argv[3])) {
        type_restriction = FOR_OPERATORS_TF;
    } else if (string_match("*", argv[3])) {
        type_restriction = FOR_ANYTHING_TF;
    } else {
        setSoarResultResult(res, "Unrecognized option to %s %s %s: %s", argv[0], argv[1], argv[2], argv[3]);
        return SOAR_ERROR;
    }

    if (argc > 4) {
        get_lexeme_from_string(argv[4]);

        /* --- read optional name restriction --- */
        if (current_agent(lexeme).type == SYM_CONSTANT_LEXEME) {
            name_restriction = make_sym_constant(argv[4]);
            format_arg = 5;
        } else {
            format_arg = 4;
        }

        if ((remove && (argc > format_arg))
            || (!remove && (argc > (format_arg + 1)))) {
            setSoarResultResult(res, too_many_args);
            return SOAR_ERROR;
        }
    }

    /* --- finally, execute the command --- */
    if (remove) {
        remove_trace_format(stack_trace, type_restriction, name_restriction);
    } else {
        if (argc == (format_arg + 1)) {
            add_trace_format(stack_trace, type_restriction, name_restriction, argv[format_arg]);
        } else {
            if (name_restriction) {
                symbol_remove_ref(name_restriction);
            }

            setSoarResultResult(res, "Missing format string");

            return SOAR_ERROR;
        }
    }

    if (name_restriction) {
        symbol_remove_ref(name_restriction);
    }

    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_IndifferentSelection --
 *
 *----------------------------------------------------------------------
 */

/* AGR 615  Adding the "last" option to this command was pretty simple
   and the changes are integrated into this entire function.  94.11.08 */

int soar_IndifferentSelection(int argc, const char *argv[], soarResult * res)
{

    if (argc > 2) {
        setSoarResultResult(res,
                            "Too many arguments, should be: indifferent-selection [-first | -last | -ask | -random ]");
        return SOAR_ERROR;
    }
    if (argc == 2) {
        if (string_match_up_to(argv[1], "-ask", 2)) {
            set_sysparam(USER_SELECT_MODE_SYSPARAM, USER_SELECT_ASK);
        } else if (string_match_up_to(argv[1], "-first", 2)) {
            set_sysparam(USER_SELECT_MODE_SYSPARAM, USER_SELECT_FIRST);
        } else if (string_match_up_to(argv[1], "-last", 2)) {
            set_sysparam(USER_SELECT_MODE_SYSPARAM, USER_SELECT_LAST);
        } else if (string_match_up_to(argv[1], "-random", 2)) {
            set_sysparam(USER_SELECT_MODE_SYSPARAM, USER_SELECT_RANDOM);
        } else {
            setSoarResultResult(res, "Unrecognized argument to indifferent-selection: %s", argv[1]);
            return SOAR_ERROR;
        }
    }

    switch (current_agent(sysparams)[USER_SELECT_MODE_SYSPARAM]) {

    case USER_SELECT_FIRST:
        setSoarResultResult(res, "-first");
        break;
    case USER_SELECT_LAST:
        setSoarResultResult(res, "-last");
        break;
    case USER_SELECT_ASK:
        setSoarResultResult(res, "-ask");
        break;
    case USER_SELECT_RANDOM:
        setSoarResultResult(res, "-random");
        break;

    }

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_InternalSymbols --
 *
 *----------------------------------------------------------------------
 */

int soar_InternalSymbols(int argc, const char *argv[], soarResult * res)
{

    argv = argv;

    if (argc > 1) {
        setSoarResultResult(res, "Too many arguments, should be: internal-symbols");
        return SOAR_ERROR;
    }

    soar_ecPrintInternalSymbols();

    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Matches --
 *
 *----------------------------------------------------------------------
 */

int soar_Matches(int argc, const char *argv[], soarResult * res)
{
    wme_trace_type wtt = NONE_WME_TRACE;
    ms_trace_type mst = MS_ASSERT_RETRACT;
    int curr_arg = 1;
    int prod_arg = 0;

    while (curr_arg < argc) {
        if (string_match_up_to("-assertions", argv[curr_arg], 2)) {
            mst = MS_ASSERT;
        } else if (string_match_up_to("-retractions", argv[curr_arg], 2)) {
            mst = MS_RETRACT;
        } else if (string_match_up_to("-names", argv[curr_arg], 2)
                   || string_match_up_to("-count", argv[curr_arg], 2)
                   || string_match("0", argv[curr_arg])) {
            wtt = NONE_WME_TRACE;
        } else if (string_match_up_to("-timetags", argv[curr_arg], 2)
                   || string_match("1", argv[curr_arg])) {
            wtt = TIMETAG_WME_TRACE;
        } else if (string_match_up_to("-wmes", argv[curr_arg], 2)
                   || string_match("2", argv[curr_arg])) {
            wtt = FULL_WME_TRACE;
        } else {
            prod_arg = curr_arg;
        }
        curr_arg++;
    }

    if (prod_arg) {
        if (soar_ecPrintMatchInfoForProduction(argv[prod_arg], wtt)) {
            setSoarResultResult(res, "There is no production named %s\n", argv[prod_arg]);
            return SOAR_ERROR;
        }
    } else {
        soar_ecPrintMatchSet(wtt, mst);
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Memories --
 *
 *----------------------------------------------------------------------
 */

int soar_Memories(int argc, const char *argv[], soarResult * res)
{
    int i;
    int num;
    int num_items = -1;
    int mems_to_print[NUM_PRODUCTION_TYPES];
    bool set_mems = FALSE;
    production *prod;

    for (i = 0; i < NUM_PRODUCTION_TYPES; i++)
        mems_to_print[i] = FALSE;

    for (i = 1; i < argc; i++) {
        if (string_match_up_to(argv[i], "-chunks", 2)) {
            mems_to_print[CHUNK_PRODUCTION_TYPE] = TRUE;
            set_mems = TRUE;

        } else if (string_match_up_to(argv[i], "-user", 2)) {
            mems_to_print[USER_PRODUCTION_TYPE] = TRUE;
            set_mems = TRUE;

        } else if (string_match_up_to(argv[i], "-defaults", 2)) {
            mems_to_print[DEFAULT_PRODUCTION_TYPE] = TRUE;
            set_mems = TRUE;

        } else if (string_match_up_to(argv[i], "-justifications", 2)) {
            mems_to_print[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
            set_mems = TRUE;

        } else {
            prod = name_to_production(argv[i]);
            if (prod) {
                print("\n Memory use for %s: %ld\n\n", argv[i], count_rete_tokens_for_production(prod));
                set_mems = TRUE;

            } else if (getInt(argv[i], &num) == SOAR_OK) {
                if (num <= 0) {
                    setSoarResultResult(res, "Count argument to memories must be a positive integer, not: %s", argv[i]);
                    return SOAR_ERROR;
                } else {
                    num_items = num;
                }

            } else {
                setSoarResultResult(res, "Unrecognized argument to memories: %s", argv[i]);
                return SOAR_ERROR;
            }
        }
    }

    if (!set_mems) {
        mems_to_print[JUSTIFICATION_PRODUCTION_TYPE] = TRUE;
        mems_to_print[CHUNK_PRODUCTION_TYPE] = TRUE;
        mems_to_print[USER_PRODUCTION_TYPE] = TRUE;
        mems_to_print[DEFAULT_PRODUCTION_TYPE] = TRUE;
    }
    /*printf("chunkflag = %d\nuserflag = %d\ndefflag = %d\njustflag = %d\n",
     *     mems_to_print[CHUNK_PRODUCTION_TYPE],
     *     mems_to_print[USER_PRODUCTION_TYPE],
     *     mems_to_print[DEFAULT_PRODUCTION_TYPE],
     *     mems_to_print[JUSTIFICATION_PRODUCTION_TYPE]);
     */
    soar_ecPrintMemories(num_items, mems_to_print);
    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_ProductionFind --
 *
 *----------------------------------------------------------------------
 */

int soar_ProductionFind(int argc, const char *argv[], soarResult * res)
{
    int i;

    list *current_pf_list = NIL;

    bool lhs = TRUE;
    bool rhs = FALSE;
    bool show_bindings = FALSE;
    bool just_chunks = FALSE;
    bool no_chunks = FALSE;
    bool clause_found = FALSE;

    if (argc == 1) {
        setSoarResultResult(res,
                            "No arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}");
        return SOAR_ERROR;
    }

    /* the args parsing should really be done in a while loop, where we
       have better control over the loop vars, which we'll use later.
       But using clause_found will do for now...  */

    for (i = 1; i < argc; i++) {
        if (string_match_up_to(argv[i], "-lhs", 2)) {
            lhs = TRUE;
        } else if (string_match_up_to(argv[i], "-rhs", 2)) {
            rhs = TRUE;
            lhs = FALSE;
        } else if (string_match_up_to(argv[i], "-show-bindings", 2)) {
            show_bindings = TRUE;
        } else if (string_match_up_to(argv[i], "-chunks", 2)) {
            just_chunks = TRUE;
        } else if (string_match_up_to(argv[i], "-nochunks", 2)) {
            no_chunks = TRUE;
        } else if (strchr(argv[i], '(') != 0)
            /* strchr allows for leading whitespace */
        {
            /* it's the clause */
            clause_found = TRUE;
            break;
        } else {
            setSoarResultResult(res, "Unrecognized argument to %s command: %s", argv[0], argv[i]);
            return SOAR_ERROR;
        }
    }

    if (!clause_found) {
        setSoarResultResult(res,
                            "No clause found.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}");
        return SOAR_ERROR;
    }

    if ((*argv[i] == '-') || (strchr(argv[i], '(') != 0)) {
        if (argc > i + 1) {
            setSoarResultResult(res,
                                "Too many arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}");
            return SOAR_ERROR;
        }
        if ((*argv[i] == '-') && (argc < i + 1)) {
            setSoarResultResult(res,
                                "Too few arguments given.\nUsage: production-find [-rhs|-lhs] [-chunks|-nochunks] [-show-bindings] {clauses}");
            return SOAR_ERROR;
        }
        if (lhs) {
            /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
            /* Soar-Bugs #54 TMH */
            /*  soar_alternate_input((agent *)clientData, argv[1], ") ", TRUE); */
            current_agent(alternate_input_string) = argv[i];
            current_agent(alternate_input_suffix) = ") ";

            /*
               print ( "Set Alternate Input to '$s'\n", argv[i] );
               print( "EXIT? %s\n", (current_agent(alternate_input_exit)
               ? "TRUE":"FALSE") );
             */
            get_lexeme();
            read_pattern_and_get_matching_productions(&current_pf_list, show_bindings, just_chunks, no_chunks);
            /* soar_alternate_input((agent *)clientData, NIL, NIL, FALSE);  */
            current_agent(current_char) = ' ';
        }
        if (rhs) {
            /* this patch failed for -rhs, so I removed altogether.  KJC 3/99 */
            /* Soar-Bugs #54 TMH */
            /* soar_alternate_input((agent *)clientData, argv[1], ") ", TRUE);  */
            current_agent(alternate_input_string) = argv[i];
            current_agent(alternate_input_suffix) = ") ";

            /*
               print ("Set alternate Input to '%s'\n", argv[i] ); 
               print( "EXIT? %s\n", (current_agent(alternate_input_exit) ? "TRUE":"FALSE") );
             */
            get_lexeme();
            read_rhs_pattern_and_get_matching_productions(&current_pf_list, show_bindings, just_chunks, no_chunks);
            /* soar_alternate_input((agent *)clientData, NIL, NIL, FALSE); */
            current_agent(current_char) = ' ';

        }
        if (current_pf_list == NIL) {
            print("No matches.\n");
        }

        free_list(current_pf_list);
    } else {
        setSoarResultResult(res, "Unknown argument to %s command: %s", argv[0], argv[i]);
        return SOAR_ERROR;
    }

    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Preferences --
 *
 *----------------------------------------------------------------------
 */

/* kjh (CUSP-B7): Replace samed named procedure in soarCommands.c */
#define SOAR_PREFERENCES_BUFF_SIZE 128
int soar_Preferences(int argc, const char *argv[], soarResult * res)
{
/* kjh (CUSP-B7) begin */
    static char *too_many_args = "Too many arguments.\nUsage: preferences [id] [attribute] [detail]";
    static char *wrong_args = "Usage: preferences [id] [attribute] [detail]";

    Symbol *id, *id_tmp, *attr, *attr_tmp;
    bool print_productions;
    wme_trace_type wtt;
    char buff1[SOAR_PREFERENCES_BUFF_SIZE];
    char buff2[SOAR_PREFERENCES_BUFF_SIZE];

    /* Establish argument defaults: */
    id = current_agent(bottom_goal);
    id_tmp = NIL;
    attr = current_agent(operator_symbol);
    attr_tmp = NIL;
    print_productions = FALSE;
    wtt = NONE_WME_TRACE;

    /* Parse provided arguments: */
    switch (argc) {
    case 1:
        /* No arguments; defaults suffice. */
        break;
    case 2:
        /* One argument; replace one of the defaults: */
        if ((read_id_or_context_var_from_string(argv[1], &id_tmp)
             == SOAR_ERROR)
            && (read_attribute_from_string(id, argv[1], &attr_tmp)
                == SOAR_ERROR)
            && (read_pref_detail_from_string(argv[1], &print_productions, &wtt)
                == SOAR_ERROR)) {
            setSoarResultResult(res, wrong_args);
            return SOAR_ERROR;
        }
        break;
    case 3:
        /* Two arguments; replace two of the defaults: */
        if (read_id_or_context_var_from_string(argv[1], &id_tmp) == SOAR_ERROR) {
            id_tmp = id;
            if (read_attribute_from_string(id, argv[1], &attr_tmp) == SOAR_ERROR) {
                setSoarResultResult(res, wrong_args);
                return SOAR_ERROR;
            }
        }
        if ((read_attribute_from_string(id_tmp, argv[2], &attr_tmp)
             == SOAR_ERROR)
            && (read_pref_detail_from_string(argv[2], &print_productions, &wtt)
                == SOAR_ERROR)) {
            setSoarResultResult(res, wrong_args);
            return SOAR_ERROR;
        }
        break;

    case 4:
        /* Three arguments; replace (all) three of the defaults: */
        if ((read_id_or_context_var_from_string(argv[1], &id_tmp)
             == SOAR_ERROR)
            || (read_attribute_from_string(id_tmp, argv[2], &attr_tmp)
                == SOAR_ERROR)
            || (read_pref_detail_from_string(argv[3], &print_productions, &wtt)
                == SOAR_ERROR)) {
            setSoarResultResult(res, wrong_args);
            return SOAR_ERROR;
        }
        break;
    default:
        /* Too many arguments; complain: */
        setSoarResultResult(res, too_many_args);
        return SOAR_ERROR;
        break;
    }

/* kjh (CUSP-B7) end */

    /* --- print the preferences --- */
    if (id_tmp != NIL)
        id = id_tmp;
    if (attr_tmp != NIL)
        attr = attr_tmp;

    if (id == NIL) {
        clearSoarResultResult(res);
        return (SOAR_OK);
    }

    /* SW BUG?
     * Ironically, now we turn the symbols back into strings
     * so we can pass them to the ecore function.  Making
     * the symbols only serves to determine which arguments were supplied
     */
    symbol_to_string(id, TRUE, buff1, SOAR_PREFERENCES_BUFF_SIZE);
    symbol_to_string(attr, TRUE, buff2, SOAR_PREFERENCES_BUFF_SIZE);
    if (soar_ecPrintPreferences(buff1, buff2, print_productions, wtt)) {
        setSoarResultResult(res, "An Error occured trying to print the prefs.");
        return SOAR_ERROR;
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Print --
 *
 *----------------------------------------------------------------------
 */

int soar_Print(int argc, const char *argv[], soarResult * res)
{
    static char *too_few_args = "Too few arguments.\nUsage: print [-depth n] [-internal] arg*";

    bool internal;
    bool name_only, full_prod;
    bool output_arg;            /* Soar-Bugs #161 */
    bool print_filename;        /* CUSP (B11) kjh */

    int depth;
    Symbol *id;
    wme *w;
    list *wmes;
    cons *c;
    int i, next_arg;

    internal = FALSE;
    depth = current_agent(default_wme_depth);   /* AGR 646 */
    name_only = FALSE;
    full_prod = FALSE;
    print_filename = FALSE;     /* kjh CUSP(B11) */
    output_arg = FALSE;         /* Soar-Bugs #161 */

    if (argc == 1) {
        setSoarResultResult(res, too_few_args);
        return SOAR_ERROR;
    }

    clearSoarResultResult(res);

    next_arg = 1;

    /* --- see if we have the -stack option --- */
    if (string_match_up_to("-stack", argv[next_arg], 4)) {
        bool print_states;
        bool print_operators;

        /* Determine context items to print */

        if (argc == 2) {        /* No options given */
            print_states = TRUE;
            print_operators = TRUE;
        } else {
            int i;

            print_states = FALSE;
            print_operators = FALSE;

            next_arg++;
            for (i = next_arg; i < argc; i++) {
                if (string_match_up_to(argv[i], "-states", 2))
                    print_states = TRUE;
                else if (string_match_up_to(argv[i], "-operators", 2))
                    print_operators = TRUE;
                else {
                    setSoarResultResult(res,
                                        "Unknown option passed to print -stack: (%s).\nUsage print -stack [-state | -operator]*",
                                        argv[next_arg]);
                    return SOAR_ERROR;
                }
            }
        }

        {
            Symbol *g;

            for (g = current_agent(top_goal); g != NIL; g = g->id.lower_goal) {
                if (print_states) {
                    print_stack_trace(g, g, FOR_STATES_TF, FALSE);
                    print("\n");
                }
                if (print_operators && g->id.operator_slot->wmes) {
                    print_stack_trace(g->id.operator_slot->wmes->value, g, FOR_OPERATORS_TF, FALSE);
                    print("\n");
                }
            }
        }

        return SOAR_OK;
    }

    /* End of string_match "-stack" */
    /* --- repeat: read one arg and print it --- */
    while (next_arg < argc) {

        /* --- read optional -depth flag --- */
        if (string_match_up_to("-depth", argv[next_arg], 4)) {
            if ((argc - next_arg) <= 1) {
                setSoarResultResult(res, too_few_args);
                return SOAR_ERROR;
            } else {
                if (getInt(argv[++next_arg], &depth) != SOAR_OK) {
                    setSoarResultResult(res, "Integer expected after %s, not %s.", argv[next_arg - 1], argv[next_arg]);
                    return SOAR_ERROR;
                }
            }
        }

        /* --- read optional -internal flag --- */
        else if (string_match_up_to("-internal", argv[next_arg], 2)) {
            internal = TRUE;
        }

        /* kjh CUSP(B11) begin */
        /* --- read optional -filename flag --- */

        else if (string_match_up_to("-filename", argv[next_arg], 2)) {
            print_filename = TRUE;
        }
        /* kjh CUSP(B11) end */

        /* --- read optional -name flag --- */
        else if (string_match_up_to("-name", argv[next_arg], 2)) {
            name_only = TRUE;
            full_prod = FALSE;  /* Soar-Bugs #161 */
        }
        /* --- read optional -full flag --- */
        else if (string_match_up_to("-full", argv[next_arg], 2)) {
            full_prod = TRUE;
            name_only = FALSE;  /* Soar-Bugs #161 */
        } else if (string_match_up_to("-all", argv[next_arg], 2)) {
            output_arg = TRUE;  /* Soar-Bugs #161 */
            for (i = 0; i < NUM_PRODUCTION_TYPES; i++) {
                soar_ecPrintAllProductionsOfType(i, internal, print_filename, full_prod);
            }
        } else if (string_match_up_to("-defaults", argv[next_arg], 4)) {
            output_arg = TRUE;  /* Soar-Bugs #161 */
            /* SW 042000: Using this new API function, all prods are
             * printed in the reverse order they were loaded...
             */
            soar_ecPrintAllProductionsOfType(DEFAULT_PRODUCTION_TYPE, internal, print_filename, full_prod);

        } else if (string_match_up_to("-user", argv[next_arg], 2)) {
            output_arg = TRUE;  /* TEST for Soar-Bugs #161 */
            soar_ecPrintAllProductionsOfType(USER_PRODUCTION_TYPE, internal, print_filename, full_prod);
        } else if (string_match_up_to("-chunks", argv[next_arg], 2)) {
            output_arg = TRUE;  /* Soar-Bugs #161 */
            soar_ecPrintAllProductionsOfType(CHUNK_PRODUCTION_TYPE, internal, print_filename, full_prod);
        } else if (string_match_up_to("-justifications", argv[next_arg], 2)) {
            output_arg = TRUE;  /* Soar-Bugs #161 */
            soar_ecPrintAllProductionsOfType(JUSTIFICATION_PRODUCTION_TYPE, internal, print_filename, full_prod);
        } else if (string_match_up_to("-", argv[next_arg], 1)) {
            setSoarResultResult(res, "Unrecognized option to print command: %s", argv[next_arg]);
            return SOAR_ERROR;
        } else {                /* check for production name or wme */

            get_lexeme_from_string(argv[next_arg]);

            switch (current_agent(lexeme).type) {
            case SYM_CONSTANT_LEXEME:
                output_arg = TRUE;      /* Soar-Bugs #161 */
                if (!name_only || print_filename) {
                    /* kjh CUSP(B11) begin */
                    do_print_for_production_name(argv[next_arg], internal, print_filename, full_prod);
                } else {
                    print("%s\n", argv[next_arg]);
                }
                break;

            case INT_CONSTANT_LEXEME:
                output_arg = TRUE;      /* Soar-Bugs #161 */
                for (w = current_agent(all_wmes_in_rete); w != NIL; w = w->rete_next)
                    /* if (w->timetag == current_agent(lexeme).int_val) break; */
                    if (w->timetag == (unsigned long) current_agent(lexeme).int_val)
                        break;
                /* rmarinie: added explicit conversion to unsigned long -- 
                   timetags always positive, so this conversion seemed reasonable */
                if (w) {
                    do_print_for_wme(w, depth, internal);
                } else {
                    setSoarResultResult(res, "No wme %ld in working memory", current_agent(lexeme).int_val);
                    return SOAR_ERROR;
                }
                break;

            case IDENTIFIER_LEXEME:
            case VARIABLE_LEXEME:

                output_arg = TRUE;      /* Soar-Bugs #161 */
                id = read_identifier_or_context_variable();
                if (id)
                    do_print_for_identifier(id, depth, internal);
                break;

            case QUOTED_STRING_LEXEME:
                output_arg = TRUE;      /* Soar-Bugs #161 */
                /* Soar-Bugs #54 TMH */
                soar_alternate_input(soar_agent, argv[next_arg], ") ", TRUE);
                /*  print( "Set alternate Input to '%s'\n", argv[next_arg] ); */
                /* ((agent *)clientData)->alternate_input_string = argv[next_arg];
                 * ((agent *)clientData)->alternate_input_suffix = ") ";
                 */
                get_lexeme();
                wmes = read_pattern_and_get_matching_wmes();
                soar_alternate_input(soar_agent, NIL, NIL, FALSE);
                current_agent(current_char) = ' ';
                for (c = wmes; c != NIL; c = c->rest)
                    do_print_for_wme(c->first, depth, internal);
                free_list(wmes);
                break;

            default:
                setSoarResultResult(res, "Unrecognized argument to %s command: %s", argv[0], argv[next_arg]);
                return SOAR_ERROR;
            }                   /* end of switch statement */
            output_arg = TRUE;  /* Soar-bugs #161 */
        }                       /* end of if-else stmt */
        next_arg++;
    }                           /* end of while loop */

    /* Soar-Bugs #161 */
    if (!output_arg) {
        setSoarResultResult(res, too_few_args);
        return SOAR_ERROR;
    } else
        return SOAR_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_PWatch --
 *
 *----------------------------------------------------------------------
 */

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

int soar_PWatch(int argc, const char *argv[], soarResult * res)
{
    bool trace_productions = TRUE;
    int next_arg = 1;

    clearSoarResultResult(res);
    if (argc == 1) {
        cons *c;

        for (c = current_agent(productions_being_traced); c != NIL; c = c->rest)
            print_with_symbols(" %y\n", ((production *) (c->first))->name);

        return SOAR_OK;
    }

    /* Check for optional argument */

    if (string_match_up_to(argv[1], "-on", 3)) {
        trace_productions = TRUE;
        next_arg++;
    }
    if (string_match_up_to(argv[1], "-off", 3)) {
        trace_productions = FALSE;
        next_arg++;
    }

    if ((argc == 2) && (next_arg != 1)) {
        /* Turn on/off all productions */

        if (trace_productions) {
            /* List all productions that are currently being traced */
            soar_ecPrintProductionsBeingTraced();
            return SOAR_OK;
        } else {
            /* Stop tracing all productions */
            soar_ecStopAllProductionTracing();
            return SOAR_OK;
        }
    }

    /* Otherwise, we have a list of productions to process */
    {

        if (trace_productions) {
            if (soar_ecBeginTracingProductions((argc - next_arg), &argv[next_arg])) {
                setSoarResultResult(res, "Could not begin tracing");
                return SOAR_ERROR;
            }

        } else {
            if (soar_ecStopTracingProductions((argc - next_arg), &argv[next_arg])) {
                setSoarResultResult(res, "Could not stop tracing");
                return SOAR_ERROR;
            }
        }

        return SOAR_OK;
    }

}

#endif

#ifdef USE_DEBUG_UTILS

/* TLD = top level disambiguation, not tomato and lettuce donut */
#define TLD 4

int soar_Pool(int argc, const char *argv[], soarResult * res)
{

    bool print_free;
    int pool_arg;

    if (argc < 2) {
        setSoarResultResult(res, "Need more arguments.");
        return SOAR_ERROR;
    }

    if (argc < 3) {
        print_free = FALSE;
        pool_arg = 1;
    } else {
        if (string_match("-all", argv[1])) {
            pool_arg = 2;
            print_free = TRUE;
        } else {
            setSoarResultResult(res, "Bad args.");
            return SOAR_ERROR;
        }
    }

    if (string_match_up_to("-wme", argv[pool_arg], TLD)) {
        examine_memory_pool(&current_agent(wme_pool));
        print_all_wmes_in_block(0, print_free);
    } else if (string_match_up_to("-condition", argv[pool_arg], TLD)) {
        examine_memory_pool(&current_agent(condition_pool));
        print_all_conditions_in_block(0, print_free);
    } else if (string_match_up_to("-preference", argv[pool_arg], TLD)) {
        examine_memory_pool(&current_agent(preference_pool));
        print_all_preferences_in_block(0, print_free);

    } else if (string_match_up_to("-instantiation", argv[pool_arg], TLD)) {
        examine_memory_pool(&current_agent(instantiation_pool));

        if (argc < (pool_arg + 1)) {
            setSoarResultResult(res, "Need more arguments for -instantiation. Either -none|-full");
            return SOAR_ERROR;
        }
        if (string_match_up_to("-none", argv[pool_arg + 1], 2)) {
            print_all_instantiations_in_block(0, NONE_WME_TRACE, print_free);
        } else if (string_match_up_to("-full", argv[pool_arg + 1], 2)) {
            print_all_instantiations_in_block(0, FULL_WME_TRACE, print_free);

        }
    } else if (string_match_up_to("-identifier", argv[pool_arg], TLD)) {
        examine_memory_pool(&current_agent(identifier_pool));
        print_all_identifiers_in_block(0, print_free);
    } else if (string_match_up_to("-production", argv[pool_arg], TLD)) {
        examine_memory_pool(&current_agent(production_pool));

        if (argc < (pool_arg + 1)) {
            setSoarResultResult(res, "Need more arguments for -production. Either -name|-full");
            return SOAR_ERROR;
        }
        if (string_match_up_to("-name", argv[pool_arg + 1], 2)) {
            print_all_productions_in_block(0, FALSE, print_free);
        } else if (string_match_up_to("-full", argv[pool_arg + 1], 2)) {
            print_all_productions_in_block(0, TRUE, print_free);

        } else {
            setSoarResultResult(res, "Argument for -production must be either -name|-full");
            return SOAR_ERROR;
        }
    }

    else {
        setSoarResultResult(res, "Don't know what to do with your arg: %s", argv[pool_arg]);
        return SOAR_ERROR;
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

#endif

/*
 *----------------------------------------------------------------------
 *
 * soar_Sp --
 *
 *----------------------------------------------------------------------
 */

int soar_Sp(int argc, const char *argv[], soarResult * res)
{

    if (argc == 1) {
        setSoarResultResult(res, "Too few arguments.\nUsage: sp {rule} sourceFile.");
        return SOAR_ERROR;
    }

    if (argc > 3) {
        setSoarResultResult(res, "Too many arguments.\nUsage: sp {rule} sourceFile.");
        return SOAR_ERROR;
    }

    if (argc == 2)
        soar_ecSp(argv[1], NULL);
    else
        soar_ecSp(argv[1], argv[2]);

    /* Errors are non-fatal, return SOAR_OK */
    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Stats --
 *
 *----------------------------------------------------------------------
 */

int soar_Stats(int argc, const char *argv[], soarResult * res)
{

    if ((argc == 1)
        || string_match_up_to("-system", argv[1], 2)) {
        return parse_system_stats(argc, argv, res);
    } else if (string_match_up_to("-memory", argv[1], 2)) {
        return parse_memory_stats(argc, argv, res);
    } else if (string_match_up_to("-rete", argv[1], 2)) {
        return parse_rete_stats(argc, argv, res);
    }
#ifdef DC_HISTOGRAM
    else if (string_match_up_to("-dc_histogram", argv[1], 2)) {
        return soar_ecPrintDCHistogram();
    }
#endif
#ifdef KT_HISTOGRAM
    else if (string_match_up_to("-kt_histogram", argv[1], 2)) {
        return soar_ecPrintKTHistogram();
    }
#endif
#ifndef NO_TIMING_STUFF
    else if (string_match_up_to("-timers", argv[1], 2)) {
        return printTimingInfo();
    }
#endif
    else {
        setSoarResultResult(res, "Unrecognized argument to stats: %s", argv[1]);
        return SOAR_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Stop --
 *
 *----------------------------------------------------------------------
 */

int soar_Stop(int argc, const char *argv[], soarResult * res)
{
    const char *reason;

    if (argc > 3) {
        setSoarResultResult(res, "Too many arguments, should be:\n\t stop-soar [-s[elf] {reason_string}");

        return SOAR_ERROR;
    }

    if (argc == 1) {
        soar_cStopAllAgents();
    } else if (string_match_up_to("-self", argv[1], 1)) {

        if (argc == 3)
            reason = argv[2];
        else
            reason = NIL;

        soar_cStopCurrentAgent(reason);

    } else {
        setSoarResultResult(res, "Unrecognized argument to stop-soar: %s", argv[1]);
        return SOAR_ERROR;
    }
    clearSoarResultResult(res);
    return SOAR_OK;
}

/* RCHONG: begin 10.11 */
/*
 *----------------------------------------------------------------------
 *
 * soar_Verbose --
 *
 *----------------------------------------------------------------------
 */
int soar_Verbose(int argc, const char *argv[], soarResult * res)
{
    int i;

    clearSoarResultResult(res);
    if (argc == 1) {
        print("VERBOSE is %s\n\n", (soar_cGetVerbosity() == TRUE) ? "ON" : "OFF");
        return SOAR_OK;
    }

    for (i = 1; i < argc; i++) {
        if (string_match("-on", argv[i])) {
            soar_cSetVerbosity(TRUE);
            print("VERBOSE is %s\n\n", (soar_cGetVerbosity() == TRUE) ? "ON" : "OFF");
        }

        else if (string_match_up_to("-off", argv[i], 3)) {
            soar_cSetVerbosity(FALSE);
            print("VERBOSE is %s\n\n", (soar_cGetVerbosity() == TRUE) ? "ON" : "OFF");
        } else {
            setSoarResultResult(res, "Unrecognized argument to the VERBOSE command: %s", argv[i]);
            return SOAR_ERROR;
        }
    }

    return SOAR_OK;
}

/* RCHONG: end 10.11 */

/*
 *----------------------------------------------------------------------
 *
 * soar_Warnings --
 *
 *----------------------------------------------------------------------
 */

int soar_Warnings(int argc, const char *argv[], soarResult * res)
{

    if (argc == 1) {
        setSoarResultResult(res, "%s", current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM]
                            ? "on" : "off");
        return SOAR_OK;
    }

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments, should be: warnings [-on | -off]");
        return SOAR_ERROR;
    }

    if (string_match_up_to(argv[1], "-on", 3)) {
        set_sysparam(PRINT_WARNINGS_SYSPARAM, TRUE);
    } else if (string_match_up_to(argv[1], "-off", 3)) {
        set_sysparam(PRINT_WARNINGS_SYSPARAM, FALSE);
    } else {
        setSoarResultResult(res, "Unrecognized option to warnings command: %s", argv[1]);
        return SOAR_ERROR;
    }

    setSoarResultResult(res, "%s", current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM]
                        ? "on" : "off");
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Log --
 *
 *----------------------------------------------------------------------
 */

int soar_Log(int argc, const char *argv[], soarResult * res)
{
    char *too_few =
        "Too few arguments, should be: log [-new | -existing] pathname | log -add string | log -query | log -off";
    char *too_many =
        "Too many arguments, should be: log [-new | -existing] pathname | log -add string | log -query | log -off";

    if (argc < 2) {
        setSoarResultResult(res, "The log file is ");

        if (soar_exists_callback(soar_agent, LOG_CALLBACK)) {
            appendSoarResultResult(res, "open.  Use log -off to close the file.");
        } else {
            appendSoarResultResult(res, "closed.");
        }

        return SOAR_OK;
    }

    if (argc > 3) {
        setSoarResultResult(res, too_many);
        return SOAR_ERROR;
    }

    if (string_match_up_to("-add", argv[1], 2)) {
        if (argc == 3) {
            soar_invoke_first_callback(soar_agent, LOG_CALLBACK, argv[2]);
        } else if (argc < 3) {
            setSoarResultResult(res, too_few);
            return SOAR_ERROR;
        } else {
            setSoarResultResult(res, too_many);
            return SOAR_ERROR;
        }

    } else if (string_match_up_to("-query", argv[1], 2)) {
        if (argc == 2) {
            if (soar_exists_callback(soar_agent, LOG_CALLBACK)) {
                setSoarResultResult(res, "open");
            } else {
                setSoarResultResult(res, "closed");
            }

        } else {
            setSoarResultResult(res, too_many);
            return SOAR_ERROR;
        }

    } else if (string_match_up_to("-off", argv[1], 2)) {
        if (argc == 2) {
            if (soar_ecCloseLog() == 0) {
                setSoarResultResult(res, "log file closed");
            } else {
                setSoarResultResult(res, "Attempt to close non-existant log file");
            }
        } else if (argc > 2) {
            setSoarResultResult(res, too_many);
            return SOAR_ERROR;
        }

    } else {                    /* Either we have a file open/append request or there is an error */
        const char *filename;
        char *mode;

        if (argc == 2) {
            filename = argv[1];
            mode = "w";
        } else if (string_match_up_to("-new", argv[1], 2)) {
            filename = argv[2];
            mode = "w";
        } else if (string_match_up_to("-new", argv[2], 2)) {
            filename = argv[1];
            mode = "w";
        } else if (string_match_up_to("-existing", argv[1], 2)) {
            filename = argv[2];
            mode = "a";
        } else if (string_match_up_to("-existing", argv[2], 2)) {
            filename = argv[1];
            mode = "a";
        } else {
            setSoarResultResult(res, "log: unrecognized arguments: %s %s", argv[1], argv[2]);
            return SOAR_ERROR;
        }

        if (soar_ecOpenLog(filename, mode) == 0) {
            /* Soar-Bugs #74 TMH */
            setSoarResultResult(res, "log file '%s' opened", filename);

        } else {
            setSoarResultResult(res, "log: Error: unable to open '%s'", filename);
            return SOAR_ERROR;
        }
    }

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_AttributePreferencesMode --
 *
 *----------------------------------------------------------------------
 */

int soar_AttributePreferencesMode(int argc, const char *argv[], soarResult * res)
{

    int i;

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments, expected  single integer 0|1|2\n");
        return SOAR_ERROR;
    }

    if (argc == 2) {
        if (getInt(argv[1], &i) == SOAR_ERROR) {
            setSoarResultResult(res, "Expected a single integer argument 0|1|2\n");
            return SOAR_ERROR;
        }
        switch (soar_cAttributePreferencesMode(i)) {
        case 0:
            return SOAR_OK;

        case -1:
            setSoarResultResult(res, "Attribute Preferences Mode must be 2 when using Soar 8\n");
            return SOAR_ERROR;

        case -2:
            setSoarResultResult(res, "Expected a single integer argument 0|1|2\n");
            return SOAR_ERROR;
        }
        return SOAR_ERROR;      /* Unknown return val */

    }

    setSoarResultResult(res, "%d", current_agent(attribute_preferences_mode));
    return SOAR_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * soar_Watch --
 *
 *----------------------------------------------------------------------
 */

int soar_Watch(int argc, const char *argv[], soarResult * res)
{

    clearSoarResultResult(res);
    if (argc == 1) {
        print_current_watch_settings();
    }

    {
        int i;

        for (i = 1; i < argc; i++) {
            if (string_match("none", argv[i]) || string_match("0", argv[i])) {
                if (soar_ecWatchLevel(0)) {
                    setSoarResultResultStdError(res);
                    return SOAR_ERROR;
                }
            } else if (string_match("1", argv[i])) {
                if (soar_ecWatchLevel(1)) {
                    return SOAR_ERROR;
                }
            } else if (string_match("decisions", argv[i])) {
                /* check if -on|-off|-inc follows */
                if ((i + 1) < argc) {
                    if ((string_match("-on", argv[i + 1])) ||
                        (string_match("-off", argv[i + 1])) || (string_match_up_to("-inclusive", argv[i + 1], 3))) {
                        if (set_watch_setting(TRACE_CONTEXT_DECISIONS_SYSPARAM, argv[i], argv[++i], res)
                            != SOAR_OK) {
                            return SOAR_ERROR;
                        }
                    } else {    /* something else follows setting, so it's inclusive */

                        if (soar_ecWatchLevel(1)) {
                            return SOAR_ERROR;
                        }
                    }
                } else {        /* nothing else on cmd line */

                    if (soar_ecWatchLevel(1)) {
                        return SOAR_ERROR;
                    }
                }
            } else if (string_match("2", argv[i])) {
                if (soar_ecWatchLevel(2)) {
                    return SOAR_ERROR;
                }
            } else if (string_match("phases", argv[i])) {
                /* check if -on|-off|-inc follows */
                if ((i + 1) < argc) {
                    if ((string_match("-on", argv[i + 1])) ||
                        (string_match("-off", argv[i + 1])) || (string_match_up_to("-inclusive", argv[i + 1], 3))) {
                        if (set_watch_setting(TRACE_PHASES_SYSPARAM, argv[i], argv[++i], res)
                            != SOAR_OK) {
                            return SOAR_ERROR;
                        }
                    } else {    /* something else follows setting, so it's inclusive */

                        if (soar_ecWatchLevel(2)) {
                            return SOAR_ERROR;
                        }
                    }
                } else {        /* nothing else on cmd line */

                    if (soar_ecWatchLevel(2)) {
                        return SOAR_ERROR;
                    }
                }
            } else if (string_match("3", argv[i])) {
                if (soar_ecWatchLevel(3)) {
                    return SOAR_ERROR;
                }
            } else if (string_match("productions", argv[i])) {
                int t;

                /* productions is a different beast, since there are four
                   separate categories and more flags possible */
                /* check if -on|-off|-inc follows */
                if ((i + 1) < argc) {
                    if (string_match("-on", argv[i + 1])) {
                        for (t = 0; t < NUM_PRODUCTION_TYPES; t++)
                            set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM + t, TRUE);
                        i++;
                    } else if (string_match("-off", argv[i + 1])) {
                        for (t = 0; t < NUM_PRODUCTION_TYPES; t++)
                            set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM + t, FALSE);
                        i++;
                    } else if (string_match_up_to("-inclusive", argv[i + 1], 3)) {
                        if (soar_ecWatchLevel(3)) {
                            return SOAR_ERROR;
                        }
                        i++;
                    }

                    /* check for specific production types */

                    else if ((string_match("-all", argv[i + 1])) || (string_match("-a", argv[i + 1]))) {
                        i++;
                        t = i + 1;
                        if (set_watch_prod_group_setting(0, argv[i], argv[t], res)) {
                            return SOAR_ERROR;
						}
                        i++;
                    } else if ((string_match("-chunks", argv[i + 1])) || (string_match("-c", argv[i + 1]))) {
                        i++;
                        t = i + 1;
                        if (set_watch_prod_group_setting(1, argv[i], argv[t], res)) {
                            return SOAR_ERROR;
						}
                        i++;
                    } else if ((string_match("-defaults", argv[i + 1])) || (string_match("-d", argv[i + 1]))) {
                        i++;
                        t = i + 1;
                        if (set_watch_prod_group_setting(2, argv[i], argv[t], res)) {
                            return SOAR_ERROR;
						}
                        i++;
                    } else if ((string_match("-justifications", argv[i + 1])) || (string_match("-j", argv[i + 1]))) {
                        i++;
                        t = i + 1;
                        if (set_watch_prod_group_setting(3, argv[i], argv[t], res)) {
                            return SOAR_ERROR;
						}
                        i++;
                    } else if ((string_match("-user", argv[i + 1])) || (string_match("-u", argv[i + 1]))) {
                        i++;
                        t = i + 1;
                        if (set_watch_prod_group_setting(4, argv[i], argv[t], res)) {
                            return SOAR_ERROR;
						}
                        i++;
                    }

                    else {      /* something else follows setting, so it's inclusive */

                        if (soar_ecWatchLevel(3)) {
                            return SOAR_ERROR;
                        }
                    }
                } else {        /* nothing else on cmd line */

                    if (soar_ecWatchLevel(3)) {
                        return SOAR_ERROR;
                    }
                }
            } else if (string_match("4", argv[i])) {
                print("Receive Watch 4\n");

                if (soar_ecWatchLevel(4)) {

                    return SOAR_ERROR;
                }
            } else if (string_match("wmes", argv[i])) {
/* kjh(CUSP-B2) begin */
                char *wmes_option_syntax_msg = "\
watch wmes syntax:\n\
   wmes [ -on |\n\
          -off |\n\
          -inc[lusive] |\n\
         {-add-filter    type filter} |\n\
         {-remove-filter type filter} |\n\
         {-reset-filter  type} |\n\
         {-list-filter   type} ]\n\
        where\n\
          type   = -adds|-removes|-both\n\
          filter = {id|*} {attribute|*} {value|*}";

                if (i + 1 >= argc) {    /* nothing else on cmd line, so it's inclusive */
                    if (soar_ecWatchLevel(4)) {
                        return SOAR_ERROR;
                    }
                } else if ((string_match(argv[i + 1], "-on")) ||
                           (string_match(argv[i + 1], "-off")) || (string_match_up_to("-inclusive", argv[i + 1], 3))) {
                    if (set_watch_setting(TRACE_WM_CHANGES_SYSPARAM, argv[i], argv[i + 1], res)
                        != SOAR_OK)
                        return SOAR_ERROR;
                    else
                        i += 1;
                } else if (i + 2 >= argc) {
                    setSoarResultResult(res, wmes_option_syntax_msg);
                    return SOAR_ERROR;
                } else if (string_match(argv[i + 1], "-add-filter")) {
                    bool forAdds, forRemoves;
                    if ((i + 5 >= argc)
                        || (parse_filter_type(argv[i + 2], &forAdds, &forRemoves) == SOAR_ERROR)) {
                        appendSoarResultResult(res, wmes_option_syntax_msg);
                        return SOAR_ERROR;
                    } else {
                        if (soar_ecAddWmeFilter(argv[i + 3], argv[i + 4], argv[i + 5], forAdds, forRemoves) != 0) {
                            setSoarResultResult(res, "Error: Filter not added.");
                            return SOAR_ERROR;
                        } else {
                            setSoarResultResult(res, "Filter added.");
                        }
                    }
                    i += 5;
                } else if (string_match(argv[i + 1], "-remove-filter")) {
                    bool forAdds, forRemoves;
                    if ((i + 5 >= argc)
                        || (parse_filter_type(argv[i + 2], &forAdds, &forRemoves) == SOAR_ERROR)) {
                        appendSoarResultResult(res, wmes_option_syntax_msg);
                        return SOAR_ERROR;
                    } else {
                        if (soar_ecRemoveWmeFilter(argv[i + 3], argv[i + 4], argv[i + 5], forAdds, forRemoves) != 0) {
                            setSoarResultResult(res, "Error: Bad args or filter not found");
                            return SOAR_ERROR;
                        } else {
                            appendSoarResultResult(res, "Filter removed.");
                        }
                    }
                    i += 5;
                } else if (string_match(argv[i + 1], "-reset-filter")) {
                    bool forAdds, forRemoves;
                    if ((i + 2 >= argc)
                        || (parse_filter_type(argv[i + 2], &forAdds, &forRemoves) == SOAR_ERROR)) {
                        appendSoarResultResult(res, wmes_option_syntax_msg);
                        return SOAR_ERROR;
                    } else {
                        if (soar_ecResetWmeFilters(forAdds, forRemoves) != 0) {
                            appendSoarResultResult(res, "No filters were removed.");
                            return SOAR_ERROR;
                        }
                    }
                    i += 2;
                } else if (string_match(argv[i + 1], "-list-filter")) {
                    bool forAdds, forRemoves;
                    if ((i + 2 >= argc) || (parse_filter_type(argv[i + 2], &forAdds, &forRemoves) == SOAR_ERROR)) {

                        appendSoarResultResult(res, wmes_option_syntax_msg);
                        return SOAR_ERROR;
                    } else {
                        soar_ecListWmeFilters(forAdds, forRemoves);
                    }
                    i += 2;
                }
            }
/* kjh(CUSP-B2) end */
            /*  kjc note:  not sure CUSP-B2 solution accounts for other
             *  non-wme args following "wmes" which should make it -inc 
             */

            else if (string_match("5", argv[i])) {
                if (soar_ecWatchLevel(5)) {
                    return SOAR_ERROR;
                }
            } else if (string_match("preferences", argv[i])) {
                /* check if -on|-off|-inc follows */
                if ((i + 1) < argc) {
                    if ((string_match("-on", argv[i + 1])) ||
                        (string_match("-off", argv[i + 1])) || (string_match_up_to("-inclusive", argv[i + 1], 3))) {
                        if (set_watch_setting(TRACE_FIRINGS_PREFERENCES_SYSPARAM, argv[i], argv[++i], res)
                            != SOAR_OK) {
                            return SOAR_ERROR;
                        }
                    } else {    /* something else follows setting, so it's inclusive */

                        if (soar_ecWatchLevel(5)) {
                            return SOAR_ERROR;
                        }
                    }
                } else {        /* nothing else on cmd line */

                    if (soar_ecWatchLevel(5)) {
                        return SOAR_ERROR;
                    }
                }
            } else if ((string_match("-all", argv[i])) || (string_match("-a", argv[i]))) {
                int t = i + 1;
                if(set_watch_prod_group_setting(0, argv[i], argv[t], res)) {
					return SOAR_ERROR;
				}
                i++;
            } else if ((string_match("-chunks", argv[i])) || (string_match("-c", argv[i]))) {
                int t = i + 1;
                if(set_watch_prod_group_setting(1, argv[i], argv[t], res)) {
					return SOAR_ERROR;
				}
                i++;
            } else if ((string_match("-defaults", argv[i])) || (string_match("-d", argv[i]))) {
                int t = i + 1;
                if(set_watch_prod_group_setting(2, argv[i], argv[t], res)) {
					return SOAR_ERROR;
				}
                i++;
            } else if ((string_match("-justifications", argv[i])) || (string_match("-j", argv[i]))) {
                int t = i + 1;
                if(set_watch_prod_group_setting(3, argv[i], argv[t], res)) {
					return SOAR_ERROR;
				}
                i++;
            } else if ((string_match("-user", argv[i])) || (string_match("-u", argv[i]))) {
                int t = i + 1;
                if(set_watch_prod_group_setting(4, argv[i], argv[t], res)) {
					return SOAR_ERROR;
				}
                i++;
            } else if (string_match_up_to("-nowmes", argv[i], 4)) {
                set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
            } else if (string_match_up_to("-timetags", argv[i], 3)) {
                set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, TIMETAG_WME_TRACE);
            } else if (string_match_up_to("-fullwmes", argv[i], 6)) {
                set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
            } else if (string_match_up_to("-prefs", argv[i], 4)) {
                set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, TRUE);
            } else if (string_match_up_to("-noprefs", argv[i], 6)) {
                set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM, FALSE);
            }
/* REW: begin 10.22.97 */
            else if (string_match_up_to("-soar8", argv[i], 6)) {
                set_sysparam(TRACE_OPERAND2_REMOVALS_SYSPARAM, TRUE);
            } else if (string_match_up_to("-nosoar8", argv[i], 6)) {
                set_sysparam(TRACE_OPERAND2_REMOVALS_SYSPARAM, FALSE);
            }
/* REW: end   10.22.97 */
            else if (string_match_up_to("learning", argv[i], 2)) {
                /* check if -print|-noprint|-fullprint follows */
                if ((i + 1) < argc) {
                    if (string_match("-print", argv[i + 1])) {
                        set_sysparam(TRACE_CHUNK_NAMES_SYSPARAM, TRUE);
                        set_sysparam(TRACE_CHUNKS_SYSPARAM, FALSE);
                        set_sysparam(TRACE_JUSTIFICATION_NAMES_SYSPARAM, TRUE);
                        set_sysparam(TRACE_JUSTIFICATIONS_SYSPARAM, FALSE);
                        i++;
                    } else if (string_match("-noprint", argv[i + 1])) {
                        set_sysparam(TRACE_CHUNK_NAMES_SYSPARAM, FALSE);
                        set_sysparam(TRACE_CHUNKS_SYSPARAM, FALSE);
                        set_sysparam(TRACE_JUSTIFICATION_NAMES_SYSPARAM, FALSE);
                        set_sysparam(TRACE_JUSTIFICATIONS_SYSPARAM, FALSE);
                        i++;
                    } else if (string_match_up_to("-fullprint", argv[i + 1], 3)) {
                        set_sysparam(TRACE_CHUNK_NAMES_SYSPARAM, TRUE);
                        set_sysparam(TRACE_CHUNKS_SYSPARAM, TRUE);
                        set_sysparam(TRACE_JUSTIFICATION_NAMES_SYSPARAM, TRUE);
                        set_sysparam(TRACE_JUSTIFICATIONS_SYSPARAM, TRUE);
                        i++;
                    } else {    /* error: no arg for learning */
                        setSoarResultResult(res,
                                            "Missing setting for watch learning, should be -noprint|-print|-fullprint");
                        return SOAR_ERROR;
                    }
                } else {        /* error: no arg for learning */
                    setSoarResultResult(res,
                                        "Missing setting for watch learning, should be -noprint|-print|-fullprint");
                    return SOAR_ERROR;
                }
            } else if (string_match("backtracing", argv[i])) {
                if (set_watch_setting(TRACE_BACKTRACING_SYSPARAM, argv[i], argv[++i], res)
                    != SOAR_OK) {
                    return SOAR_ERROR;
                }
            } else if (string_match("loading", argv[i])) {
                if (set_watch_setting(TRACE_LOADING_SYSPARAM, argv[i], argv[++i], res)
                    != SOAR_OK) {
                    return SOAR_ERROR;
                }
            } else if (string_match("indifferent-selection", argv[i])) {
				if (set_watch_setting(TRACE_INDIFFERENT_SYSPARAM, argv[i], argv[++i], res)
					!= SOAR_OK) {
					return SOAR_ERROR;
				}
			}
            /* Pushed to interface  081699 
               else if (string_match("aliases", argv[i]))
               {
               if (argv[i+1] == NULL)
               {
               setSoarResultResult( res, 
               "Missing setting for watch alias, should be -on|-off");
               return SOAR_ERROR;
               }
               else if (string_match("-on",argv[i+1]))
               {
               Interface_SetVar(soar_agent, "print_alias_switch","on");
               i++;
               }
               else if (string_match("-off",argv[i+1]))
               {
               Interface_SetVar(soar_agent,"print_alias_switch","off");
               i++;
               }
               else
               {
               setSoarResultResult( res, 
               "Unrecognized argument to watch alias : %s",
               argv[i+1]);
               return SOAR_ERROR;
               }
               }
             */
            else {
                setSoarResultResult(res, "Unrecognized argument to watch command: %s", argv[i]);
                return SOAR_ERROR;
            }
        }
    }

    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_DefaultWmeDepth --
 *
 *----------------------------------------------------------------------
 */

int soar_DefaultWmeDepth(int argc, const char *argv[], soarResult * res)
{
    int depth;

    if (argc == 1) {
        setSoarResultResult(res, "%d", current_agent(default_wme_depth));
        return SOAR_OK;
    }

    if (argc > 2) {
        setSoarResultResult(res, "Too many arguments, should be: default-wme-depth [integer]");
        return SOAR_ERROR;
    }

    if (getInt(argv[1], &depth) == SOAR_OK) {
        soar_ecSetDefaultWmeDepth(depth);
    } else {
        setSoarResultResult(res, "Expected integer for new default print depth: %s", argv[1]);
        return SOAR_ERROR;
    }

    clearSoarResultResult(res);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_BuildInfo --
 *
 *----------------------------------------------------------------------
 */

int soar_BuildInfo(int argc, const char *argv[], soarResult * res)
{

    argv = argv;
    argc = argc;

    soar_ecBuildInfo();
    setSoarResultResult(res, soar_version_string);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_ExcludedBuildInfo --
 *
 *----------------------------------------------------------------------
 */

int soar_ExcludedBuildInfo(int argc, const char *argv[], soarResult * res)
{

    argv = argv;
    argc = argc;

    soar_ecExcludedBuildInfo();
    setSoarResultResult(res, soar_version_string);
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_GDSPrint --
 *
 *----------------------------------------------------------------------
 */

int soar_GDSPrint(int argc, const char *argv[], soarResult * res)
{

    res = res;
    argc = argc;
    argv = argv;

    soar_ecGDSPrint();
    return SOAR_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * soar_Interrupt --
 *
 *----------------------------------------------------------------------
 */

int soar_Interrupt(int argc, const char *argv[], soarResult * res)
{
    enum soar_apiInterruptSetting interrupt_setting;
    Symbol *sym;
    production *prod;
    bool valid_args;
    int i;

    clearSoarResultResult(res);

    if(argc > 3) {
        setSoarResultResult(res, "Too many arguments\n");
    }
    if (argc == 1 || argc > 3) {
        appendSoarResultResult(res, "Usage: interrupt [-on|-off] [production name]");
        return SOAR_ERROR;
    }

    interrupt_setting = INTERRUPT_PRINT;
    prod = NULL;
    valid_args = TRUE;

    for (i = 1; i < argc; i++) {
        if (string_match("-on", argv[i])) {
	        interrupt_setting = INTERRUPT_ON;
        } else if (string_match("-off", argv[i])) {
	        interrupt_setting = INTERRUPT_OFF;
        } else {
            sym = find_sym_constant(argv[i]);
            if (sym && sym->sc.production) {
                prod = sym->sc.production;
            } else {
                setSoarResultResult(res, "Can't find production named %s", argv[i]);
                valid_args = FALSE;
                break;
            }
        }
    }

    if(!valid_args) {
        return SOAR_ERROR;
    } else {
        if(prod) {
            if(interrupt_setting == INTERRUPT_ON) {
                prod->interrupt = TRUE;
            } else if(interrupt_setting == INTERRUPT_OFF) {
                prod->interrupt = FALSE;
            } else {
                print("%s%s", prod->name->var.name, ": ");
                if(prod->interrupt) {
                    print("%s\n", "on");
                } else {
                    print("%s\n", "off");
                }
            }
        } else {
            soar_ecPrintAllProductionsWithInterruptSetting(interrupt_setting);
        }
    }

    return SOAR_OK;
}
