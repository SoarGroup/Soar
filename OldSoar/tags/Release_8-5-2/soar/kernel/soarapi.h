/**
 * \file soarapi.h
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

#ifndef _SOARAPI_H__            /* excludeFromBuildInfo */
#define _SOARAPI_H__

#include "soarkernel.h"
#include "soar_core_api.h"
#include "soar_ecore_api.h"
#include "soarapi_datatypes.h"

#ifdef USE_DEBUG_UTILS
#include "debugutil.h"
#endif

/* *************************************************************************
 * *************************************************************************/

/**
 * @name Interaction With Soar
 *
 *       This section deals with four main areas of interaction
 *               \arg (Re)Initializing Soar
 *	         \arg Creating Agents
 *               \arg Destroying Agents
 *               \arg Starting and Stopping Agents
 *     
 */

/* *************************************************************************
 * *************************************************************************/

/*@{*/

/*
 *
 *   Initialize Soar 
 * 
 *   Soar must be initialized before any of the API functions are 
 *   called.  Because this initialization must only be done once,
 *   it makes little sense to make it a part of this high level API.
 *   (it will never be called using a UI).  Thus initialization
 *   must be done with the core api function soar_cInitializeSoar()
 *
 *   See soar_core_api.h for the real documentation of this function
 */

/**
 *     soar_ReInitSoar --      
 *
 * \brief  This is the command procedure for the "init-soar" command,
 *         it (re)initializes the Soar agent.
 * 
 *         This command removes all wmes from working memory, wiping
 *         out the goal stack, and resets all statistics (except the
 *         counts tracking how many times each individual production
 *         has fired which is used by the "firing-counts" command).
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *                   function. 
 * \par    Syntax:
   \verbatim  
           init-soar
   \endverbatim
 *
 * \return  A standard SOAR completion code.
 *
 * \par     SoarResult:
 *            \arg On \c SOAR_ERROR:  Contains details about the error.
 *            \arg On \c SOAR_OK:     Is empty
 *
 * \par     Side effects:
 *            Empties working memory, removes the goal stack, and resets
 *            statistics.
 *
 *  
 */

extern int soar_ReInitSoar(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_CreateAgent --
 *
 * \brief  This command creates a new agent.
 *
 *         If there were no agents previously, the current agent is
 *         defined as the newly created agent.  Otherwise, the
 *         current agent remains the same.
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           create-agent [name]
   \endverbatim
 *
 * \return Returns a standard SOAR completion code.
 *
 * \par    SoarResult:
 *            \arg On \c SOAR_ERROR:  Contains details about error
 *            \arg On \c SOAR_OK:     Is empty
 *
 *
 * \par    Side effects: 
 *            Creates a new agent, and possibly changes the
 *            current agent
 *
 * 
 */

extern int soar_CreateAgent(int argc, const char *argv[], soarResult * res);

/**
 * 
 *
 * soar_Run --
 *
 * \brief  This is the command procedure for the "run" command which
 *         runs the Soar agent. 
 * 
 *         This is the command for running Soar agents.  It takes two
 *         optional arguments, one specifying how many things to run,
 *         and one specifying what type of things to run (by default
 *         all agents are run).  The following types are available:
 *          - \b p  - run Soar for n phases.  A phase is either an input 
 *                    phase, preference phase, working memory phase, output 
 *                    phase, or decision phase.
 *          - \b e  - run Soar for n elaboration cycles.  (For purposes of 
 *                    this command, decision phase is counted as an 
 *                    elaboration cycle.)
 *          - \b d  - run Soar for n decision cycles.
 *          - \b s  - run Soar until the nth time a state is selected.
 *          - \b o  - run Soar until the nth time an operator is selected.
 *          - \b out - run Soar by decision cycles until output is generated.
 *          - \e context-variable - run Soar until the nth time a selection 
 *                     is made for that particular context slot, or until the 
 *                     context stack pops to above that context.
 *         Unlike Soar 6 "go" , "run" has no memory of settings from
 *         previous run commands.
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * 
 * \par    Syntax:
   \verbatim
           run [integer | 'forever'] [type] [-self]
           type ::= p | e | d | s | o | context-variable
           if no integer given, but [type] is specified, n = 1 assumed
           if no [type] given, but [int] specified, decisions assumed
  \endverbatim
 *
 *
 * \par Examples:
 *   \arg run 5 d     -> run all agents for 5 decision cycles
 *   \arg run d -self -> run the current agent only for 5 decision cycles
 *   \arg run 3       -> run all agents for 3 decision cycles
 *   \arg run e       -> run all agents for 1 elaboration cycle
 *   \arg run 1 s     -> run all agents until the next state is selected 
 *                            (i.e., until the next time an impasse arises)
 *   \arg run <so>    -> run until the next superoperator is selected 
 *                           (or until the supergoal goes away)
 *   \arg run 3 <o>   -> run for 3 operator selections at this level 
 *                          (continuing through any subgoals that arise)
 * 
 *
 * \return Returns a standard SOAR completion code.
 * 
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 * 
 * 
 * \par    Side effects:
 *           Runs the Soar agents for the indicated duration
 * 
 * */
extern int soar_Run(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_DestroyAgent --
 *
 * \brief  This is the command procedure for the "destroy-agent" command, 
 *	   which destroys a Soar agent.
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:  
   \verbatim
           destroy-agent [agent-name]
   \endverbatim
 *
 * \return Returns a standard SOAR completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *	     Destroys a Soar agent;  resets the current agent if necessary
 *
 *
 */
extern int soar_DestroyAgent(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Quit --
 *
 * \brief  This is the command procedure for the "quit" command, 
 *	   which prepares Soar to be exited.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 *
 * \par    Syntax:
   \verbatim
           quit
   \endverbatim
 *
 * \return Returns a standard SOAR completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           turns logging off, does some clean up.
 *
 *
 */
extern int soar_Quit(int argc, const char *argv[], soarResult * res);

/*@}*/

/* *************************************************************************
 * *************************************************************************/

/**
 *   
 *  @name Modifying Agent Memory
 *
 *
 *       This section deals with two areas of the agent:
 *               \arg Production Memory
 *	         \arg Working Memory
 *
 */

/* *************************************************************************
 * *************************************************************************/

/*@{*/

/**
 *
 *
 * soar_ReteNet --
 *
 * \brief  This is the command procedure for the "rete-net" command, 
 *         which saves or loads a rete-network.
 *
 *         The rete-net is a binary file containing the contents of 
 *         production memory.  Because productions are decomposed 
 *         into their constituent conditions before being added to
 *         the network, the productions themselves are difficult to 
 *         recognize.  Although this file format is not encrypted
 *         in any real sense, it does obfuscate productions to a 
 *         degree similar to compilation of source code.
 *
 *         Moreoever, if productions are saved as a rete-net, they
 *         can be loaded by a version of Soar which does not contain
 *         a productions parser (and supporting elements) this allows
 *         a significantly smaller footprint for the executable when
 *         memory is at a premium.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           rete-net {-save | -load} {filename}
   \endverbatim
 *
 * \return Returns a standard SOAR completion code
 * 
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Adds the given wme to working memory.
 *
 *
 */

extern int soar_ReteNet(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_AddWme --
 *
 * \brief  This is the command procedure for the "add-wme" command, 
 *         which adds an element to working memory.
 *
 *         This command surgically modifies Soar's working memory.  
 *         Add-wme adds a new wme with the given id, attribute, value,
 *         and optional acceptable preference.  The given id must be
 *         an existing identifier.  If '*' is given in place of the
 *         attribute or value, Soar creates a new identifier
 *         (gensym) for that field.
 *
 *         \b WARNING: this command is inherently unstable and may have 
 *         weird side effects (possibly even including system
 *         crashes).  For example, the chunker can't backtrace
 *         through wmes created via add-wme.  Removing input wmes or
 *         context/impasse wmes may have unexpected side effects.
 *         You've been warned.
 *
 * \see    soar_RemoveWme
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           add-wme id [^] { attribute | '*'} { value | '*' } [+]
   \endverbatim
 *
 * \return Returns a standard SOAR completion code
 * 
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Contains information about the new 
 *                                   wme in the following format: 
 *                                   "<timetag>: <state> ^<attribute> <value>"
 *
 * \par    Side effects:
 *           Adds the given wme to working memory.
 *
 * */
extern int soar_AddWme(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_RemoveWme --
 *
 * \brief  This is the command procedure for the "remove-wme" command, 
 *         which removes a wme from Soar's memory.
 *
 *         \b WARNING: this command is inherently unstable and may
 *         have weird side effects (possibly even including system
 *         crashes).  For example, the chunker can't backtrace
 *         through wmes created via add-wme.  Removing input wmes or
 *         context/impasse wmes may have unexpected side effects. 
 *         You've been warned.
 *
 * \see    soar_AddWme
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           remove-wme integer
   \endverbatim
 *
 * \return Returns a standard SOAR completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 *
 * \par    Side effects:
 *           Removes the selected working memory element from Soar.
 *
 * */

extern int soar_RemoveWme(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Excise --
 *
 * \brief  This is the command procedure for the "excise" command, 
 *         which removes productions from Soar's memory.
 *
 *        "excise -chunks" removes all chunks and justifications from
 *        the agent.  "excise -task" removes all non-default
 *        productions from the agent and performs an init-soar
 *        command.  excise -all removes all productions from the
 *        agent.  The switches may be abbreviated to 2 characters
 *        (e.g, -c for chunks).
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           excise production-name | -chunks | -default | -task 
                                  |-user | -all
   \endverbatim
 *
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 *
 * \par    Side effects:
 *           Removes the indicated productions from the agent memory.
 *           For -all and -task, does an init-soar.
 *
 * */
extern int soar_Excise(int argc, const char *argv[], soarResult * res);

/*@}*/

/* *************************************************************************
 * *************************************************************************/

/**
 *   @name Modifying the Agents Parameters
 *
 *
 */

/* *************************************************************************
 * *************************************************************************/
/*@{*/

#ifdef USE_CAPTURE_REPLAY

/**
 *
 *
 * soar_CaptureInput --
 *
 * \brief  This is the command procedure for the "capture-input" command
 *         which records input wme commands (add|remove) from the INPUT phase.
 *
 *         This command may be used to start and stop the recording of
 *         input wmes as created by an external simulation.  wmes are
 *         recorded decision cycle by decision cycle.  Use the
 *         command replay-input to replay the sequence.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           capture-input <action>
            <action> ::= -open pathname 
            <action> ::= -query
            <action> ::= -close
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Contains the status of the capture file
 *
 * \par    Side effects:
 *           Opens and/or closes captured input files.  
 *
 * */
extern int soar_CaptureInput(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_ReplayInput --
 *
 * \brief  This is the command procedure for the "replay-input" command
 *         which is used to playback previously recorded input.
 *
 *         Input originally obtained using the "capture-input" command
 *         is replayed with this command
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           replay-input <action>
            <action> ::= -open pathname 
            <action> ::= -query
            <action> ::= -close
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Contains the status of the replay file
 *
 * \par    Side effects:
 *           Opens and/or closes replay files.  
 *
 * */
extern int soar_ReplayInput(int argc, const char *argv[], soarResult * res);
#endif

/**
 *
 *
 * soar_ChunkNameFormat --
 *
 * \brief  This command  specifies the format of names of newly created 
 *         chunks.
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par Syntax:
   \verbatim
           chunk-name-format  [-short|-long]
                              [-prefix [<prefix>]]
                              [-count [<start-chunk-number>]]
   \endverbatim
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Sets indicated format style, prefix, and starting chunk number.
 *           Prints current <prefix> if -prefix is used without a <prefix>.
 *           Prints current <start-chunk-number> if -count is used without
 *           a <start-chunk-number>.
 *
 *
 */
extern int soar_ChunkNameFormat(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Learn --
 *
 * \brief  This is the command procedure for the "learn" command.
 *
 *         With no arguments, this command prints out the current learning 
 *         status.  Any of the following arguments may be given:
 *
 * \arg \c on         - turns all learning on 
 * \arg \c off        - turns all learning off 
 * \arg \c only       - learning is off except as specified by RHS force-learn
 * \arg \c except     - learning is on except as specified by RHS dont-learn
 * \arg \c list       - print lists of force-learn and dont-learn states
 * \arg \c all-goals  - when learning is on, this allows learning at all 
 *                      goal stack levels (in contrast to bottom-up 
 *                      learning)
 * \arg \c bottom-up  - when learning is on, this allows learning at only 
 *                      the lowest goal stack level; i.e., a chunk is 
 *                      learned at a given level only if no chunk has yet 
 *                      been learned at a lower level.
 *
 * \see    chunk-free-problem-spaces
 * \see    soar_Watch
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           learn arg*
              arg  ::=  -on | -only | -except | -off 
              arg  ::=  -all-goals | -bottom-up
              arg  ::=  -list
   \endverbatim
 *
 * \return Returns a standard Tcl completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty or contains info from 
 *                                   the 'list' sub-command.
 *
 * \par    Side effects:
 *           Sets booleans for whether or not chunking done.
 *
 *
 */
extern int soar_Learn(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_MaxElaborations --
 *
 * \brief  This is the command procedure for the "max-elaborations" 
 *         command, which sets/prints the maximum elaboration cycles
 *         allowed.
 *
 *         With no arguments, this command returns the current value
 *         of the system variable "max-elaborations" in the
 *         SoarResult.  With an integer argument, it sets the
 *         current value.  This variable controls the maximum number
 *         of elaboration cycles allowed in a single decision cycle.
 *         After this many elabloration cycles have been executed,
 *         Soar proceeds to decision phase even if quiescence hasn't
 *         really been reached yet.  (Max-elaborations is initially
 *         100.)
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           max-elaborations [integer]
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty or contains the current 
 *                                   value of max-elaborations
 *
 * \par    Side effects:
 *           Sets the maximum elaborations allowed.
 *
 *---------------------------------------------------------------------- */
extern int soar_MaxElaborations(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_MaxChunks --
 *
 * \brief  This is the command procedure for the "max-chunks"  command, 
 *         which sets/prints the maximum number of chunks allowed.
 *
 *         With no arguments, this command returns the current value
 *         of the system variable "max-chunks" in the SoarResult.
 *         With an integer argument, it sets the current value.
 *         This variable controls the maximum number of chunks
 *         allowed in a single decision cycle.  After this many
 *         chunks have been executed, Soar proceeds to decision
 *         phase even if quiescence hasn't really been reached yet.
 *         (Max-chunks is initially 50.)  The maximum number of
 *         chunks can also be limited by setting the soar variable
 *         max_chunks.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           max-chunks [integer]
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty or contains the current 
 *                                   value of max-chunks
 *
 * \par    Side effects:
 *           Sets the maximum chunks allowed.
 *
 *---------------------------------------------------------------------- */
extern int soar_MaxChunks(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Operand2 --
 *
 * \brief  This is the command procedure for the "soar8" command. 
 *
 *         With no arguments, this command prints out the current
 *         operand state.  Otherwise it turns on or off soar8 mode.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           operand2 arg*
              arg  ::=  -on | -off 
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty  or contains the current operand state.
 *
 *
 * \par    Side effects:
 *           re-initializes soar and puts it in the requested mode.
 *           Also modifies the soar_version_string.
 *
 * */
extern int soar_Operand2(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_WaitSNC --
 *
 * \brief  This is the command procedure for the "waitsnc" command.
 *         
 *         With no arguments, this command stores the current wait/snc
 *         state in the SoarResult. If this mode is on, soar waits 
 *         (in its current state) for new things to happen. If it is off,
 *         then soar generates impasses when the state does not change.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           waitsnc arg*
              arg  ::=  -on | -off 
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty  or contains the current wait/snc state.
 *
 * \par    Side effects:
 *           Changes the wait/snc state
 *
 *---------------------------------------------------------------------- */
extern int soar_WaitSNC(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_InputPeriod --
 *
 * \brief  This is the command procedure for the "input-period" 
 *         command, which sets/prints the Soar input period.
 *
 *         With no arguments, this command stores the current period
 *         used to control the input rate to the Soar agent in the
 *         SoarResult.  With an integer argument, it sets the
 *         current input period.  If the argument is 0, Soar behaves
 *         as it did in versions before 6.2.4, accepting input every
 *         elaboration cycle.  An input period of "n" means that
 *         input is accepted only every "n" decision cycles.  The
 *         input period is initially set to 0.  Negative integer
 *         values are disallowed.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           input-period [integer]
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty or contains the current input period.
 *
 * \par    Side effects:
 *           Sets/prints the input period for the agent.
 *
 *---------------------------------------------------------------------- */
extern int soar_InputPeriod(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_MultiAttributes --
 *
 * \brief  This is the command procedure for the "multi-attributes" command, 
 *         which enables a symbol to have multiple attribute values.
 * 
 *         If given, the value must be a positive integer > 1.  The 
 *         default is 10.  If no args are given on the cmdline, the
 *         list of symbols that are multi-attributed is printed.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           multi-attributes symbol [value]
   \endverbatim
 *
 * \return Returns a Soar  completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Defines the symbol to be multi-attributed or prints a list
 *           of symbols that are declared to multi-attributed.
 *
 * */
extern int soar_MultiAttributes(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_NumericIndifferentMode --
 *
 * \brief  This is the command procedure for the "numeric-indifferent-mode" , 
 *         command which controls the way o-support calculations are done
 *         (for the current agent).  
 * 
 *         It takes a single string argument: 
 * \arg \c -avg  - do numeric indiffernce by averaging all values asserted 
 *                   by the rules.  Indifferent preferrences with no explicit
 *                   value are assigned the numeric weight of 50.
 * \arg \c -sum  - do numeric indifference by summing all values asserted by
 *                   the rules.  Indifferent prefferences with no explicit
 *                   value are assigned the numeric weight of 0.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           numeric-indifferent-mode arg*
					    arg ::=   -avg | -sum
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Contains the current o-support mode.
 *              
 *
 * \par    Side effects:
 *	     Sets current_agent(numeric_indifferent_mode).
 *
 *
 */
extern int soar_NumericIndifferentMode(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_OSupportMode --
 *
 * \brief  This is the command procedure for the "o-support-mode" command, 
 *         which controls the way o-support calculations are done (for the
 *         current agent).  
 * 
 *         It takes a single numeric argument: 
 * \arg \c 0  - do o-support calculations the normal (Soar 6) way.
 * \arg \c 1  - do o-support calculations the normal (Soar 6) way, but
 *                 print a warning message whenever a preference is created
 *                 that would get different support under Doug's proposal.
 * \arg \c 2  - do o-support calculations according to Doug Pearson's
 *                 proposal.  (See osupport.c for details.)
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           o-support-mode {0 | 1 | 2}
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Contains the current o-support mode.
 *              
 *
 * \par    Side effects:
 *	     Sets current_agent(o_support_calculation_type).
 *
 *
 */
extern int soar_OSupportMode(int argc, const char *argv[], soarResult * res);

/* End of core commands */

/**
 *
 *
 * soar_ExplainBacktraces --
 *
 * \brief This is the command procedure for the "explain-backtraces"
 *        command.
 *
 *        Explain provides some interpretation of backtraces generated
 *        during chunking.  Explain mode must be ON when the chunk/ 
 *        justification is CREATED or no explanation will be
 *        available.  Explain mode is toggled using the
 *        save_backtraces variable.  When explain mode is on, more
 *        memory is used, and building chunks/justifications will be
 *        slower.
 *
 *        The two most useful commands are 'explain-backtraces <name>'
 *        and *'explain-backtraces <name> <cond-num>'.  The first
 *        command lists all of the conditions for the named 
 *        chunk/justification, and the 'ground' which resulted in 
 *        inclusion in the chunk/justification.  A 'ground' is a WME 
 *        which was tested in the supergoal.  Just knowing which WME
 *        was tested may be enough to explain why the
 *        chunk/justification exists.  If not, the conditions can be
 *        listed with an integer value.  This value can be used in
 *        'explain <name> <cond-num>' to obtain a list of the
 *        productions which fired to obtain this condition in the
 *        chunk/justification (and the crucial WMEs tested along the
 *        way).  Why use an integer value to specify the condition?
 *        To save a big parsing job.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           explain-backtraces arg
           arg := ''                 list chunks/justifications if 
                                     explain is on
           arg := <name>             list all conditions & grounds for 
                                     a chunk/justification
           arg := <name> -full       give the backtrace for a named
                                     chunk/justification
           arg := <name> <cond-num>  explain why this condition is in 
                                     the chunk/justification
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           None.
 *
 * */
extern int soar_ExplainBacktraces(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_FiringCounts --
 *
 * \brief  This is the command procedure for the "firing-counts" command, 
 *         which prints out how many times a production has fired.
 *
 *         This command prints how many times certain productions have
 *         fired.  With no arguments, it lists all the productions
 *         sorted according to how many times they have fired.  If
 *         an integer argument (call it k) is given, only the top k
 *         productions are listed.  If k=0, only the productions
 *         which haven't fired at all are listed.  Note that firing
 *         counts are not reset by an (init-soar); the counts
 *         indicate the number of firings since the productions were
 *         loaded or built.
 *
 *         NB:  this is slow, because the sorting takes time O(n*log n)
 *
 *         With one or more production names as arguments, this
 *         command prints how many times each of those productions
 *         fired.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           firing-counts [[integer] | production-name* ]
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  (Currently this never happens)
 *           \arg On \c SOAR_OK:     Is unmodified.
 *
 * \par    Side effects:
 *           Prints information about production firings.
 *
 * */
extern int soar_FiringCounts(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_FormatWatch --
 *
 * \brief  This is the command procedure for the "format-watch" 
 *         command, which defines the format to use when printing 
 *         objects and the Soar goal stack.
 *
 *         Object trace formats control how Soar prints an object--
 *         e.g., a certain operator, problem-space, etc.  (This is 
 *         like trace-attributes in Soar 5.)  Stack trace formats 
 *         control how Soar prints its context stack selections in
 *         'watch 0' and 'pgs' printouts.  You specify a trace 
 *         format by indicating two things: 
 *           - a format string, indicating the printout format to be used
 *           - what things this format string can be applied to
 *         The format string can be any string.  Certain 'escape
 *         sequences' can be used within the string; for example,
 *         '%dc' means print the current decision cycle number.
 * 
 *        There are two ways to restrict what objects a format string
 *        applies to.  The {s|o|*} argument restricts the types of
 *        objects: 's' indicates that the format only applies to 
 *        states; 'o' means it only applies to operators; and '*'
 *        means it applies to any type of object.  The [object-name] 
 *        argument (for object trace formats), if given, means it only
 *        applies to objects with that ^name.  The [ps-name]
 *        argument (for stack trace formats) means it only applies
 *        within problem spaces with that ^name.
 * 
 *        With an -add argument, these commands add new trace formats
 *        (replacing any existing ones with identical applicability
 *        conditions).  With a -remove argument, they remove trace 
 *        formats with the given applicability conditions.  With no 
 *        arguments, they print out all current trace formats.
 *
 *        The following escape sequences can be used within trace
 *        format strings.  The S indicates the sequence is ONLY usable
 *        in stack traces:
 *
 *  \arg %cs                S- print the current state
 *  \arg %co                S- print the current operator 
 *                          * The %cg, %cp, %cs, and %co sequences use 
 *                            the appropriate object trace format.
 *  \arg %dc                S- print the current decision cycle number 
 *  \arg %ec                S- print the current elaboration cycle number
 *                          * The %dc and %ec sequences are NOT
 *                          * meaningful in stack traces produced by 
 *                            the "pgs" command.  In these cases, nothing 
 *                            is printed.
 *  \arg %sd                S- print the current subgoal depth
 *                          * The %sd sequence uses 0 as the top level).
 *  \arg %rsd[pattern]      S- repeat (subgoal depth) times: print the 
 *                            given pattern.
 *  \arg %left[num,pattern]  - print the pattern left justified in a
 *                            field of num spaces.
 *  \arg %right[num,pattern] - print the pattern right justified in a
 *                            field of num spaces.
 *  \arg %id                 - print the identifier of the current object.
 *  \arg %v[foo]             - print the value(s) of attribute ^foo on the 
 *                            current object.  If there is no ^foo on the 
 *                            current object, nothing is printed.
 *  \arg %v[foo.bar.baz]     - same as the above, only follow the given 
 *                            attribute path to get the value(s).
 *  \arg %v[*]               - print all values of all attributes on the 
 *                            current object.
 *  \arg %o[args]            - same as %v, except that if the value is an 
 *                            identifier it is printed using the 
 *                            appropriate object trace format.
 *  \arg %av[args]           - same as %v, except the printed value is 
 *                            preceeded with "^attr " to indicate the 
 *                            attribute name.
 *  \arg %ao[args]           - a combination of the above two.
 *  \arg %ifdef[pattern]     - print the given pattern if and only if all 
 *                            escape sequences inside it are "meaningful" 
 *                            or "well-defined."  For example, 
 *                            "%ifdef[foo has value: %v[foo]]" will print 
 *                            nothing if there is no ^foo on the current 
 *                            object.
 *  \arg %nl                 - print a newline.
 *  \arg %%                  - print a percent sign.
 *  \arg %[                  - print a left bracket.
 *  \arg %]                  - print a right bracket.
 * 
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           format-watch {-object | -stack} 
                     [{ { -add    {s|o|*} [name] format}  | 
                        { -remove {s|o|*} [name]       }  }]
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Sets the indicated format.
 *
 *
 */
extern int soar_FormatWatch(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_IndifferentSelection --
 *
 * \brief  This is the command procedure for the "indifferent-selection" 
 *         command which controls indifferent preference arbitration.
 *
 *         With no arguments, this command prints the current setting
 *         of indifferent-selection.  With an argument, it sets
 *         indifferent-selection to the given value.  This controls
 *         how Soar's decision procedure chooses between multiple
 *         indifferent items: 
 * \arg \c -first -- just choose the first one found (deterministically)
 * \arg \c -last -- choose the last one found (deterministically) 
 * \arg \c -ask -- ask the user to choose one of the items
 * \arg \c -random -- choose one randomly
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           indifferent-selection [-first | -ask | -last | -random ]
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Modifies the setting of the indifferent-selection option.
 *
 * */
extern int soar_IndifferentSelection(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_InternalSymbols --
 *
 * \brief  This is the command procedure for the "internal-symbols"
 *         command which prints information about the Soar agent symbol
 *         table.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           internal-symbols
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Prints the current Soar agent symbol table
 *
 *
 */
extern int soar_InternalSymbols(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Matches --
 *
 * \brief  This is the command procedure for the "matches" command.
 *
 *         This command prints partial match information for a
 *         selected production.  It also prints information about the
 *         current match set.
 *
 *         The match set is a list of productions that are about to
 *         fire or retract in the next preference phase.  With no
 *         arguments, this command prints out the production names for
 *         both Assertions and Retractions.  The first optional
 *         argument specifies listing of either Assertions or
 *         Retractions.
 *
 *         The last optional argument specifies the level of detail
 *         wanted: -counts (the default for single productions) prints
 *         out just the partial match counts; -names (the default for
 *         match sets) prints out just the production names; -timetags
 *         also prints the timetags of wmes matched; and -wmes prints
 *         the wmes rather than just their timetags.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           matches production-name [ -count | -timetags | -wmes]
           matches production-name [ 0 | 1 | 2 ]
           ms [-assertions | -retractions] [-names| -timetags | -wmes]
           ms [-assertions | -retractions] [ 0 | 1 | 2 ]
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Prints production match information.
 *
 * */
extern int soar_Matches(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Memories --
 *
 * \brief  This is the command procedure for the "memories" command, 
 *         which prints information about memory use.
 *      
 *         Information is displayed about the memory usage in tokens,
 *         of partial matches of productions.  If a production-name is
 *         given, memory use for that production is printed.  If no
 *         production name is given, memories will list 'count'
 *         productions of the specified type (or all types, if no type
 *         is specified).  If 'count' is omitted, memory use of all
 *         productions is printed.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           memories [arg*]
            arg  ::=  -chunk | -user | -default | -justification 
                             | production-name | count
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Prints information about memory usage of partial matches.
 *
 * */
extern int soar_Memories(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_ProductionFind --
 *
 * \brief  This is the command procedure for the "production-find"
 *         command, which finds Soar productions by characteristic.
 *
 *         pf is a production finding facility.  It allows you to find
 *         productions that either test a particular LHS pattern or
 *         produce particular RHS preferences.
 *
 *         The syntax of the lhs-conditions or rhs-actions is exactly
 *         their syntax within SP's.  In addition, the symbol '*' may
 *         be used as a wildcard for an attribute or value.  Note that
 *         variable names do not have to match the specific names used
 *         in productions.
 * 
 *         Specifying -chunks means only chunks are searched (and
 *         -nochunks means no chunks are searched).
 *
 *         Note that leading blanks in the clause will cause pf to fail.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * Examples:
 *
 *      Find productions that test that some object gumby has 
 *      attribute ^alive t, and test an operator named foo:
 *        production-find {(<s> ^gumby <gv> ^operator.name foo)(<gv> ^alive t)}
 *
 *      Find productions that propose foo:
 *        production-find -rhs {(<x> ^operator <op> +)(<op> ^name foo)}
 *
 *      Find productions that test the attribute ^pokey:
 *        production-find {(<x> ^pokey *)}
 *
 *      Find productions that test the operator foo
 *        production-find {(<s> ^operator.name foo)}
 *
 * \par    Syntax:
   \verbatim
           production-find [side] [chnks] [-show-bindings] {clauses}
	        side  :==  -rhs | -lhs
		chnks :==  -chunks | -nochunks
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Prints the productions found
 *
 * */
extern int soar_ProductionFind(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Preferences --
 *
 * \brief  This is the command procedure for the "preferences" command, 
 *         which prints all the preferences for the given slot.
 *
 *         This command prints all the preferences for the slot given
 *         by the 'id' and 'attribute' arguments.  The optional
 *         'detail' argument must be one of the following (-none is
 *         the default):
 *
 * \arg \c -none     -- prints just the preferences themselves
 * \arg \c -names    -- also prints the names of the productions that 
 *                      generated them
 * \arg \c -timetags -- also prints the timetags of the wmes matched by 
 *                      the productions
 * \arg \c -wmes     -- prints the whole wmes, not just their timetags.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           preferences [id] [attribute] [-none | -names | -timetags | -wmes]
	   preferences [id] [attribute] [ 0 | 1 | 2 | 3 ]
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Prints preference information.
 *
 * */
extern int soar_Preferences(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Print --
 *
 * \brief  This is the command procedure for the "print" command, which 
 *         prints various Soar items.
 * 
 *         The print command is used to print items from production
 *         memory or working memory.  It can take several kinds of
 *         arguments.  When printing items from working memory, the
 *         objects are printed unless the -internal flag is used, in
 *         in which case the wmes themselves are printed.
 *
 * \see    default-wme-depth
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           print <stack>|<items>
            <stack>  ::= -stack [-state | -operator]*
            <items>  ::= [-depth n] [-internal] [-filename] <arg>*
            <arg>    ::= production-name  print that production
            <arg>    ::= production type  [-name | -full] -all | -chunks | 
                                          -defaults | -user | -justifications
            <arg>    ::= identifier       id of the object to print
            <arg>    ::= integer          timetag of wme--the identifier 
                                          from the wme indicates the object 
                                          to be printed
            <arg>    ::= <pattern>        pattern--same as if you listed 
                                          as arguments the timetags of all 
                                          wmes matching the pattern,
                                          often results in multiple copies
                                          of the same object being printed
   
          <pattern> ::= '{' '('  {identifier | '*'} 
                               ^ {attribute  | '*'} 
                                 {value      | '*'} 
                                 [+] ')' '}'
   \endverbatim
 *
 *      The optional [-depth n] argument overrides default-wme-depth.
 *      The optional [-internal] argument tells Soar to print things 
 *      in their internal form.  For productions, this means leaving 
 *      conditions in their reordered (rete net) form.  For wmes, this 
 *      means printing the individual wmes with their timetags, rather 
 *      than the objects.  The optional [-filename] argument tells Soar
 *      to print the filename for source'd productions.
 * 
 *      -depth 0 is meaningful only for integer and pattern arguments.  
 *      It causes only the matching wmes to be printed, instead of all 
 *      wmes whose id is an id in one of the matching wmes.
 *
 *      -name | -full apply to printing productions.  For the specified
 *      production type, only the production name will be printed,
 *      unless the -full flag is specified.  For named productions, the
 *      default is to print the entire production unless -name is specified.
 *      (-name for an individual prod seems silly, but it's included
 *      for completeness...)
 *
 *      The -depth n, -internal, -name, and -full flags apply only to the
 *      args that appear after them on the command line.
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Prints the selected objects.
 *
 * kjh CUSP(B11) =  
 *     "Soar should know the file name from which a given production was loaded."  
 *
 *      The changes here to PrintCmd are meant to be 
 *      more illustrative of how to provide a production's file name to the
 *      user than they are meant to be final changes to PrintCmd, which 
 *      is undergoing concurrent work at this time.
 *
 *      print -filename <production-name>+
 *
 * */
extern int soar_Print(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_PWatch --
 *
 * \brief  This is the command procedure for the "pwatch" command
 *         which enables the tracing of production firing.
 *
 *         This command enables tracing of the firings and retractions
 *         of individual productions.  (This mechanism is orthogonal
 *         to the watch -firings mechanism.  See the "watch" command
 *         for more information.  PWatch, with no arguments, lists the
 *         productions currently being traced.  With one or more
 *         production name arguments, it enables tracing of those
 *         productions.  Using the -on or -off option explicitly turns
 *         tracing on or off for the given productions.  Tracing
 *         persists until disabled, or until the production is
 *         excised.
 *
 * \see    soar_Watch
 * \see    soar_Excise
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           pwatch [-on | -off] production-name*
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Enables the tracing of the specified productions.
 *
 * */

extern int soar_PWatch(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Pool --
 *
 * \brief  This is the command procedure for the "pool" command, which 
 *         yields debugging information on various internal memory pools
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           pool [-all] <pool-name> <pool-args>
   \endverbatim
 *
 *         pool-name  is one of:
 *            -wme
 *            -condition
 *            -preference
 *            -instantiation
 *            -identifier
 *            -production
 *
 *         pool-arguements:
 *        
 *         for -instantiation, one of:
 *                  -none            (no wme information)
 *                  -full            (full wme information)
 *
 *         for -production, one of:
 *                  -name           (print production name only)
 *                  -full           (print entire production)
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 *
 * \par    Side effects:
 *           None
 *
 *
 */
#ifdef USE_DEBUG_UTILS
extern int soar_Pool(int argc, const char *argv[], soarResult * res);
#endif

/**
 *
 *
 * soar_Sp --
 *
 * \brief  This is the command procedure for the "sp" command, which 
 *         defines a new Soar production.
 *
 *         This command adds a new production to the system.  (If
 *         another production with the same name already exists, it is
 *         excised.)  The optional flags are as follows:
 *
 * \arg \c :o-support --  specifies that all the RHS actions are to 
 *                         be given o-support when the production fires
 * \arg \c :no-support -- specifies that all the RHS actions are only 
 *                         to be given i-support when the production fires
 * \arg \c :default --    specifies that this production is a default 
 *                         production (this matters for (excise-task) and 
 *                         (watch task))
 * \arg \c :chunk --      specifies that this production is a chunk (this 
 *                         matters for (learn trace))
 *
 * See also:  lhs-grammar, rhs-grammar
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           sp {rule-body}
            rule-body := name 
                        ["documentation-string"] 
                        [ flag ]*
                        LHS
                        ->
                        RHS
  
            flag  ::=  :o-support
            flag  ::=  :i-support
            flag  ::=  :default
            flag  ::=  :chunk
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty -- note OK so long as 
 *                                   the proper number of arguments 
 *                                   is supplied.  Invalid productions 
 *                                   don't cause an error....
 *
 * \par    Side effects:
 *           Runs the agent for the indicated number of decision cycles.
 *
 * */

extern int soar_Sp(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Stats --
 *
 * \brief  This is the command procedure for the "stats" command, which 
 *         prints out internal statistical information.
 *
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           stats [-system <stype> | -memory <mtype> | -rete <rtype>] 
           <stype> ::= -default-production-count
                       -user-production-count
                       -chunk-count
                       -justification-count
                       -all-productions-count
                       -dc-count
                       -ec-count
                       -ecs/dc
                       -firings-count
                       -firings/ec
                       -wme-change-count
                       -wme-addition-count
                       -wme-removal-count
                       -wme-count
                       -wme-avg-count
                       -wme-max-count
                       -total-time             |T
                       -ms/dc                  |T
                       -ms/ec                  |T
                       -ms/firing              |T
                       -ms/wme-change          |T
                       -match-time             |D
                       -ownership-time         |D
                       -chunking-time          |D
  
           The items marked |T are available when Soar has been
           compiled with the NO_TIMING_STUFF flag NOT SET and 
           the items marked |D are available when Soar has been
           compiled with the DETAILED_TIMING_STATS flag SET.
   
           <mtype> ::= -total
                       -overhead
                       -strings
                       -hash-table
                       -pool [-total | pool-name [<aspect>]]
                       -misc
          <aspect> ::= -used                   |M
                       -free                   |M
                       -item-size
                       -total-bytes
  
            The items marked |M are available only when Soar has
            been compiled with the MEMORY_POOL_STATS option.
  
            <rtype> ::= -total-nodes
                        -dummy-top-nodes
                        -pos-nodes
                        -unhashed-pos-nodes
                        -neg-nodes
                        -unhashed-neg-nodes
                        -cn-nodes
                        -cn-partner-nodes
                        -production-nodes
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is unmodified.
 *
 * \par    Side effects:
 *           Prints the selected statistics
 *
 *
 */

extern int soar_Stats(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Stop --
 *
 * \brief  This is the command procedure for the "stop-soar" command, 
 *         halts the Soar agents.
 * 
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 * \par    Syntax:
   \verbatim
           stop-soar [-s[elf] [reason-string]]
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           All agents are halted.
 *
 *
 */

extern int soar_Stop(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Verbose --
 *
 * \brief  This is the command procedure for the "verbose" command
 *
 *         With no arguments, this command prints out the current
 *         verbose state.  Any of the following arguments may be
 *         given:
 * \arg \c on         - turns verbose on
 * \arg \c off        - turns verbose off
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           verbose arg*
             arg  ::=  -on | -off 
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           none.  it just sets soar_verbose_flag.
 *
 * */
extern int soar_Verbose(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Warnings --
 *
 * \brief  This is the command procedure for the "warnings" command, 
 *         which enables/disables the printing of warning messages.
 *
 *         warnings -on enables the printing of warning messages.
 *         This is the default.  warnings -off turns off most warning
 *         messages.  warnings prints an indication of whether warning
 *         messages are enabled or not.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Synax:
   \verbatim
           warnings [-on | -off]
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Contains the state of the warnings.
 *
 *
 * \par    Side effects:
 *           Sets the warnings option.
 *
 * */

extern int soar_Warnings(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Log --
 *
 * \brief  This is the command procedure for the "log" command which 
 *         records session information to a log file.
 *
 *         This command may be used to open and close log files.  With
 *         the -add argument, specific strings can also be added to
 *         the log file.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           log <action>
            <action> ::= [-new | -existing] pathname 
            <action> ::= -add string
            <action> ::= -query
            <action> ::= -off
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Contains the state of the log file
 *              
 *
 * \par    Side effects:
 *           Opens and/or closes log files.
 *
 * */
extern int soar_Log(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_AttributePreferencesMode --
 *
 * \brief  This is the command procedure for the "attribute-preferences-mode"
 *         command, which controls how (certain) preferences are handled.
 *
 *         This command affects the handling of preferences (other
 *         than acceptable and reject preferences) for non-context
 *         slots.  It takes a single numeric argument:
 *
 * \arg \c 0  -  Handle them the normal (Soar 6) way.
 * \arg \c 1  -  Handle them the normal (Soar 6) way, but print a warning
 *                 message whenever a preference other than + or - is created
 *                 for a non-context slot.
 * \arg \c 2  -  Whenever a preferences other than + or - is created for a 
 *                 non-context slot, print an error message and ignore
 *                 (discard) that preference.  For non-context slots, the set
 *                 of values installed in working memory is always equal to
 *                 the set of acceptable values minus the set of rejected
 *                 values.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           attribute-preferences-mode {0 | 1 | 2}
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Contains the current attribute preferences mode.
 *              
 *
 * \par    Side effects:
 *	     Sets current_agent(attribute_preferences_mode).
 *
 * */

extern int soar_AttributePreferencesMode(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_Watch --
 *
 * \brief  This is the command procedure for the "watch" command, which 
 *         controls run-time tracing.
 *
 *         This command controls what information gets printed in the
 *         run-time trace.  With no arguments, it just prints out the
 *         current watch status.  The numeric arguments are different
 *         from the semantics in Soar 5 and 6; for details, see help
 *         watch.  Setting either the -on or -off switch selectively
 *         turns on or off only that setting.  Setting the
 *         -inclusive switch (which can be abbreviated as -inc) or
 *         setting no flag at all has the effect of setting all
 *         levels up to and including the level specified.  For
 *         example, watch productions -on selectively turns on the
 *         tracing of production firings/retractions; watch
 *         productions -off selectively turns it off again.  watch
 *         productions [-inc] turns on the tracing of productions
 *         and also turns on tracing of all levels below
 *         productions: decisions and phases, too.  Individual
 *         watch parameters may be used to modify the inclusive
 *         settings as well, selectively turning on or off and levels
 *         outside or inside the inclusive range.
 *
 *         The following keyword arguments may be given to the 'watch'
 *         command:
 *
 * \arg \c 0|none   -- turns off all printing about Soar's internals
 * \arg \c 1|decisions -- controls whether state and operator decisions
 *                are printed as they are made
 * \arg \c 2|phases -- controls whether phase names are printed as Soar 
 *                executes
 * \arg \c 3|productions -- controls whether the names of productions are
 *               printed as they fire and retract.  See individual 
 *               production-type args below.
 * \arg \c 4|wmes --  controls whether changes to working memory are printed
 * \arg \c 5|preferences -- controls whether the preferences generated by the
 *               traced  productions  are printed when those productions
 *               fire or retract.  When  a  production  fires,  all  the
 *               preferences it generates are printed. When it retracts,
 *               only the ones being removed from preference memory  are
 *               printed (i.e., the i-supported ones).
 *
 * \arg \c -nowmes|-timetags|-fullwmes --  controls the level of detail given
 *               about the wmes matched by productions whose firings and 
 *               retractions are being traced.  Level 0|-nowmes means no 
 *               information about the wmes is printed.  Level 1|-timetags 
 *               means the wme timetags are printed.  Level 2|-fullwmes
 *               means the whole wmes are printed.
 *
 * \arg \c -all|-default|-chunks|-justifications|-user {-noprint|-print|-fullprint}
 *               allows user to selectively print production *types*
 *               as they fire and retract.  -noprint prints nothing,
 *               -print prints the production name, -fullprint prints the
 *               entire production.  (fullprint not yet implemented)
 *               NOTE:  these args can be abbreviated by the first
 *               char ONLY, otherwise the full arg must be specified.
 * \arg \c -prefs|-noprefs -- turns on|off the printing of preferences for
 *               the productions being watched.  These args need to be
 *               made to apply to just the given production type, instead
 *               of to all productions.
 *
 * \arg \c aliases -on|-off -- controls the echoing of aliases as they are
 *               defined.
 * \arg \c backtracing -on|-off -- controls the printing of backtracing
 *               information when a chunk or justification os created.
 * \arg \c learning -noprint|-print|-fullprint -- controls the printing of
 *               chunks/justifications as they are created. -noprint is
 *               don't print anything, -names prints just the names,
 *               -fullprint prints the entire chunk/justification.
 * \arg \c loading -on|-off  -- controls the printing of '*' for each
 *               production loaded and a '#' for each production excised.
 *
 *      Currently the following args apply to all productions being
 *      watched, but they need to be changed to apply to only the
 *      specified production type.  This requires the addition of many
 *      new kernel flags, one for each arg for each production type.
 *           -nowmes|-timetags|-fullwmes
 *           -noprint|-print|-fullprint
 *           -prefs|-noprefs
 *
 *
 * \see    soar_Learn
 * \see    soar_PWatch
 * \see    soar_Print
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           watch arg*
             arg  ::=  0 | 1 | 2 | 3 | 4 | 5
             arg  ::=  0 | none
             arg  ::=  decisions | phases | productions | wmes | preferences
                          [-on | -off | -inc[lusive]]
             arg  ::=  -nowmes | -timetags | -fullwmes
             arg  ::=  aliases  {-on|-off}
             arg  ::=  backtracing {-on|-off}
             arg  ::=  learning {-noprint|-print|-fullprint}
             arg  ::=  loading  {-on|-off}
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 * \par    Side effects:
 *           Sets booleans for printing information during Soar execution cycles.
 *
 * */

extern int soar_Watch(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_DefaultWmeDepth --
 *
 * \brief  This is the command procedure for the "default-wme-depth" 
 *         command, which sets/prints the default print depth.
 *
 *         With no arguments, this command prints the current default
 *         print depth used by the "print" command.  With an integer
 *         argument, it sets the current default print depth.  This
 *         default print depth can be overridden on any particular 
 *         invocation of the "print" command by using the -depth flag,
 *         e.g., print -depth 10 args....  The default print depth
 *         is initially 1.
 *
 * \see    soar_Print
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           default-wme-depth [integer]
   \endverbatim
 *
 * \return Returns a standard Tcl completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 *
 * \par    Side effects:
 *           Sets the default print depth for the agent.
 *
 *
 */
extern int soar_DefaultWmeDepth(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_BuildInfo --
 *
 * \brief  This is the command procedure for the "build-info" 
 *         command, which yields information about Soar.
 *
 *         This function indicates compile time setting which affect
 *         the current instantiation of Soar.  For example, the
 *         \c DETAILED_TIMERS options must be set at compile time.
 *         This function prints such compile time options that were
 *         defined for the particular build. 
 *
 *         Note that the options which are printed using this function
 *         are filtered.  They are expected to be the most pertanant
 *         of all the compile time options.
 *
 * \see    soar_ExcludedBuildInfo
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           build-info
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 *
 *
 *
 */
extern int soar_BuildInfo(int argc, const char *argv[], soarResult * res);

/**
 *
 *
 * soar_ExcludedBuildInfo --
 *
 * \brief  This is the command procedure for the "ex-build-info" 
 *         command, which yields information about Soar.
 *
 *         This function indicates compile time setting which affect
 *         the current instantiation of Soar.  
 *         
 *         Note that the options which are printed using this function
 *         are filtered.  The options presented using this command are
 *         those which are expected to be of lesser importance.  However,
 *         using this command in conjunction with \c soar_BuildInfo
 *         will list \b all compile time options for the current build.
 *
 * \see    soar_BuildInfo
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the 
 *
 *
 * \par    Syntax:
   \verbatim
           ex-build-info
   \endverbatim
 *
 * \return Returns a standard Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 *
 *
 *
 */
extern int soar_ExcludedBuildInfo(int argc, const char *argv[], soarResult * res);

/*
 *----------------------------------------------------------------------
 *
 * soar_Interrupt --
 *
 * \brief  This command sets and queries information regarding
 *         interrupts on productions.  It can turn interrupts on single
 *         productions on or off, list the current setting for a
 *         production, or list all productions which currently have
 *         interrupts on or off.
 *
 * \param  "-> argc" The number of arguments is the \c argv block
 * \param  "-> argv" An array of strings, each of which is a word in the
 *                   argument list to this function
 * \param  "<- res"  A SoarResult structure which will be filled in by the
 *                   function.
 *
 * \par    Syntax:
   \verbatim
           interrupt [-on|-off] [production name]
   \endverbatim
 *
 * \return Returns a Soar completion code.
 *
 * \par    SoarResult:
 *           \arg On \c SOAR_ERROR:  Contains details about error.
 *           \arg On \c SOAR_OK:     Is empty.
 *
 *
 * \par    Side effects:
 *           Sets the interrupt byte for a production or prints
 *           current interrupt settings
 *
 *
 *----------------------------------------------------------------------
 */

extern int soar_Interrupt(int argc, const char *argv[], soarResult * res);

/*@}*/

#endif
