/**
 * \file soar_core_api.h
 *   
 *                     The Low Level interface to Soar
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
 *
 *
 *  \b Conventions: 
 *      Above each function is a short description of
 *      what it does.  The arguments and return values are also
 *      explained as well as side effects of calling the function.
 *
 *  Each argument is described an arrow of some sort is placed between
 *  the argument's name and its description.  This arrow indicates
 *  what role the argument plays, and may be one of:
 *   \arg  ->   the argument is read (but not written to) by the function
 *   \arg  <-   the argument is written to by the function 
 *             (the function may check to see if its value is NULL)
 *   \arg <->  the argument is expected to contain a valid entry which is
 *            first read by the function, and then modified before
 *            the function returns
 *
 *
 * 
 */

#ifndef _SOAR_CORE_API_H_       /* excludeFromBuildInfo */
#define _SOAR_CORE_API_H_

#include "soarkernel.h"
#include "callback.h"
#include "soarapi_datatypes.h"

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

/**
 *
 * soar_cInitializeSoar --
 *
 * \brief  Initialize Soar for the very first time. 
 *
 *         This should be the first Soar related function called, 
 *         and it should only be called once.
 *
 *
 */
extern void soar_cInitializeSoar(void);

/**
 *
 *
 * soar_cReInitSoar --
 *
 * \brief  Reset Soar (and agents) to the initial state.
 *
 *         This function reinitializes Soar by clearing the working
 *         memory of all agents and preparing them for a "new"
 *         execution. In essence, they are put back into the same
 *         state as if Soar were just started, and the agents were
 *         newly loaded.
 *
 *
 * \return  Error values that may be bitwise ORed
 * \retval 0     Success
 * \retval 0x1   Error reseting id counters    (memory leak)
 * \retval 0x2   Error reseting wme timetags   (memory leak)
 *
 *
 *
 *
 */
extern int soar_cReInitSoar(void);

/**
 *
 *
 * soar_cCreateAgent --
 *
 * \brief  Create a new soar agent with the specified name.
 *
 * \param  "-> name" the name of the new agent 
 *
 * \return  Nothing
 *  
 * \par     Side Effects:
 *          Modifies the global agent list
 *
 *
 */
extern void soar_cCreateAgent(const char *agent_name);

/**
 *
 *
 * soar_cRun --
 *
 * \brief  Run the current agent, or all agents for a specified
 *         period ...
 *
 * \param  "-> n"    the number of cycles to be run ( -1 --> forever )
 * \param  "-> all"  TRUE iff all agents should be run.
 * \param  "-> type" the type of cycle:
 *                     GO_PHASE, GO_ELABORATION, GO_DECISION
 *                     GO_STATE, GO_OPERATOR, GO_OUTPUT, GO_SLOT
 *                    
 * \param  "-> slot" the slot type:
 *            \arg     if \b type \c == \c GO_SLOT, use one of:
 *                        \c STATE_SLOT, \c OPERATOR_SLOT,
 *                        \c SUPERSTATE_SLOT, \c SUPEROPERATOR_SLOT,
 *                        \c SUPERSUPERSTATE_SLOT, \c SUPERSUPEROPERATOR_SLOT
 *            \arg     otherwise, use:
 *                       \c  NO_SLOT
 *                      
 * 
 * \return  An integer value, with the following semantics
 * \retval  0   Success
 * \retval -1   Fail, slot specifier is incompatible w/ type specifier
 * \retval -2   Fail, invalid slot specifier
 * \retval -3   Fail, no state or operator at the specified level
 * \retval -4   Fail, agent halted, cannot run specified number of cycles
 *
 *
 *
 */
extern int soar_cRun(long n, bool all, enum go_type_enum type, enum soar_apiSlotType slot);

/**
 *
 *
 * soar_cStopAllAgents --
 *
 * \brief  Stops all agents.
 *
 */
extern void soar_cStopAllAgents(void);

/**
 *
 *
 * soar_cStopCurrentAgent --
 *
 * \brief  Stops the current agent.
 *
 * \param  "-> reason"   a string indicating why the agent is being stopped
 *                       prematurely.
 *
 */
extern void soar_cStopCurrentAgent(const char *reason);

/**
 *
 *
 * soar_cDestroyAgentByName --
 *
 * \brief  Destroy an agent, given its name
 *
 * \param  "-> name"  the name of the agent to be destroyed
 *
 * \return  An integer value with the following semantics:
 * \retval  0    Success
 * \retval  -1   Fail, ambigous name
 * \retval  -2   Fail, no such agent
 *
 * \par     Side Effects:
 *           An agent may be destroyed and removed from the agent list
 *           The current agent may also be changed
 *
 *
 */
extern int soar_cDestroyAgentByName(const char *name);

/**
 *
 *
 * soar_cDestroyAllAgentsWithName --
 *
 * \brief Destroy a set of agents, given a name.
 *
 * \param  "-> name"  the name of the agent to be destroyed
 *
 * \return  An integer value with the following semantics:
 * \retval  0    Success
 * \retval  -1   Fail, no such agents
 *
 * \par     Side Effects:
 *           One or more agents may be destroyed and removed from 
 *           the agent list. The current agent may also be changed
 *
 *
 */
extern int soar_cDestroyAllAgentsWithName(char *name);

/**
 *
 *
 * soar_cDestroyAgentByAddress --
 *
 * \brief  Destroy an agent, given a pointer to it.
 *
 * \param "-> delete_agent" a pointer to the agent to destroy
 * 
 *
 * \par     Side Effects:
 *            Note: this is probably too drastic
 *            aborts with fatal error if the specified agent does not
 *            exist in the global data structure (e.g. it has already
 *            been deleted)
 *
 *
 */
extern void soar_cDestroyAgentByAddress(psoar_agent delete_agent);

/**
 *
 *
 * soar_cDestroyAgentById --
 *
 * \brief  Destroy an agent, given its unique id.
 * 
 *         There is a one to one correspondence between agents and ids.
 *         Ids are assigned when an agent is created, in increasing
 *         (but cyclical) order.  The id of a particular agent can be 
 *         retrieved using soar_cGetIdForAgentByName().  Although
 *         slightly less efficient than using a pointer to the psoar_agent
 *         structure itself, the agent id gives increased security by
 *         encapsulating all the sensitive agent data from the user of
 *         the api. 
 * 
 * \param "-> agent_id" the agent's unique identifier          
 *
 * \return  An integer value with the following semantics:
 * \retval  0    Success
 * \retval  -1   Fail, no such id
 *
 * \par     Side Effects:
 *           An agent is destroyed and removed from the agent list
 *           The current agent may also be changed
 *
 * \see     soar_cGetIdForAgentByName
 *
 */
extern int soar_cDestroyAgentById(int agent_id);

/**
 *
 *
 * soar_cQuit --
 *
 * \brief  Quit Soar.
 *
 *         This function should be called when Soar (or the application it
 *         is embedded within is just about to exit). 
 *
 * \par     Side Effects:
 *           Pops the top function from the Log Callback stack 
 *           (in an effort to stop logging)
 *
 *
 */
extern void soar_cQuit(void);

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
 * soar_cLoadReteNet --
 *
 * \brief  Load a Rete Network into the agent from a specified file.
 *
 * \param  "-> filename" the file to be loaded
 *
 * \return  An integer value with the following semantics:
 * \retval 0    Success
 * \retval -1   Fail, invalid (no) filename specified
 * \retval -2   Fail, working memory is not empty
 * \retval -3   Fail, production Memory is not empty
 * \retval -4   Fail, unable to open specified file
 *
 *
 *
 *
 */
extern int soar_cLoadReteNet(const char *filename);

/**
 *
 *
 * soar_cSaveReteNet --
 *
 * \brief  Save a Rete Network from the agent into a specified file.
 *
 * \param  "-> filename" the file to be loaded 
 *
 * \return  An integer value with the following semantics
 * \retval 0    Success
 * \retval -1   Fail, justifications exist
 * \retval -2   Fail, unable to open file
 *
 *
 *
 *
 */

extern int soar_cSaveReteNet(const char *filename);

/**
 *
 *
 * soar_cAddWme --
 *
 * \brief  Add a working memory element to the current agent's working 
 *         memory.
 *
 * \param "-> szId"      the identifier on which the wme should be added
 * \param "-> szAttr"    the attribute of the wme.  This value may or may
 *                        not start with a '^'.  If a new identifier
 *                        is requested, this argument should be "*"
 * \param "-> szValue"   the value of the wme.  Use "*" to indicate that
 *                        a new identifier should be used as the wme's
 *                        value
 * \param "-> accept"    TRUE if the wme should receive an acceptable 
 *                        preference
 * \param "<-  new_wme"  a pointer to a psoar_wme structure which is 
 *                        set to point to the new wme during the
 *                        execution of this function
 *
 * \return  An interger value with the following semantics:
 * \retval  timetag   Success (an integer > 0)
 * \retval  -1        Fail, invalid ID
 * \retval  -2        Fail, invalid Attribute
 * \retval  -3        Fail, invalid value
 * \retval  -4        Fail, unspecified
 *
 * \par     Side Effects:
 *           new_wme points to the wme which has just been added into
 *           the agent's memory
 *
 *
 */
/*extern unsigned long soar_cAddWme( const char *szId, const char *szAttr, const char *szValue,*/
extern long soar_cAddWme(const char *szId, const char *szAttr, const char *szValue, bool accept, psoar_wme * new_wme);

/**
 *
 *
 * soar_cRemoveWmeUsingTimetag --
 *
 * \brief  Remove a working memory element, given its timetag.
 *
 * \param "-> num"  the timetag of the wme which should be removed
 *
 * \return  An interger value with the following semantics
 * \retval  0    Success
 * \retval  -1   Fail, timetag does not exist
 * \retval  -2   Fail, unspecified
 *
 *
 *
 *
 */
extern int soar_cRemoveWmeUsingTimetag(int num);

/**
 *
 *
 * soar_cRemoveWme --
 *
 * \brief  Remove a working memory element.
 *
 *         This is not as safe as removing a wme using its timetag.
 *         Typically this should be called only during the input
 *         phase.
 *
 * \param  "-> psoar_wme"  the wme which should be removed.  This 
 *                          object is typically obtained using one 
 *                          of the AddWme functions.
 *
 * \return  An integer value with the following semantics:
 * \retval  0    Success
 * \retval  -1   Fail
 *
 * \par     Side Effects:
 *            A wme is removed from memory.
 *
 * 
 */
extern int soar_cRemoveWme(psoar_wme wme);

/**
 *
 *
 * soar_cExciseAllProductions --
 *
 * \brief  Remove all productions from the agents memory and 
 *         ReInitialize the agent.
 *
 *
 *
 * \par     Side Effects:
 *            Production memory is emptied
 *
 *
 */

extern void soar_cExciseAllProductions(void);

/**
 *
 *
 * soar_cExciseAllTaskProductions --
 *
 * \brief  Remove all but default productions from the agents memory and 
 *         ReInitialize the agent.
 *
 *
 *
 * \par     Side Effects:
 *           Production memory is modified
 *
 *
 */
extern void soar_cExciseAllTaskProductions(void);

/**
 *
 *
 * soar_cExciseAllProductionsOfType --
 *
 * \brief  Remove all productions of a specific type from the agents
 *         memory and ReInitialize the agent.
 *
 *
 * \param "-> type"  One of:
 *                   \arg \c  DEFAULT_PRODUCTION_TYPE
 *                   \arg \c  CHUNK_PRODUCTION_TYPE
 *                   \arg \c  JUSTIFICATION_PRODUCTION_TYPE
 *                   \arg \c  USER_PRODUCTION_TYPE
 *
 *
 * \par     Side Effects:
 *           Production memory is modified
 *
 *
 */
extern void soar_cExciseAllProductionsOfType(byte type);

/**
 *
 *
 * soar_cExciseProductionByName --
 *
 * \brief  Remove the production with the specified name.
 *
 * \param "-> name"  the name of the production to be removed
 *
 * \return  An integer value with the following semantics:
 * \retval  0    Success
 * \retval  -1   Fail, production not found
 *
 * \par     Side Effects:
 *           A production is removed from long term memory.
 *
 *
 */
extern int soar_cExciseProductionByName(const char *name);

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

/**
 *
 *
 * soar_cSetSystemParameter --
 *
 * \brief  Set a system parameter for the current agent
 *
 * \param  "-> param" the parameter to be set
 * \param  "-> value" the new value
 *
 * 
 *
 */
extern void soar_cSetSystemParameter(int param, long value);

/*#define soar_cGetInputPeriod() (soar_agent->input_period)*/
#define soar_cGetInputPeriod() (current_agent(input_period))

/*#define soar_cSetInputPeriod(p) ((p >= 0)?(soar_agent->input_period = p) : -1)*/
#define soar_cSetInputPeriod(p) ((p >= 0)?(current_agent(input_period) = p) : -1)

/*#define soar_cGetVerbosity() (soar_agent->soar_verbose_flag)*/
#define soar_cGetVerbosity() (current_agent(soar_verbose_flag))

/*#define soar_cSetVerbosity(x) ((soar_agent->soar_verbose_flag) = (x))*/
#define soar_cSetVerbosity(x) ((current_agent(soar_verbose_flag)) = (x))

#ifndef NO_TIMING_STUFF
/**
 *
 *
 * soar_cDetermineTimerResolution --
 *
 * \brief  Determine the resolution of the system timers Soar uses 
 *         to gather statistics.
 *
 * \param "<- min"  if non NULL, the long pointed to by min is 
 *                    filled with the minimum timer interval
 *                    encountered during the test or -1 if
 *                    this value is belived to be corrupt.
 * \param "<- max"  if non NULL, the long pointed to by min is 
 *                    filled with the maximum timer interval
 *                    encountered during the test or -1 if
 *                    this value is belived to be corrupt.
 *         
 * \return  An interger value with the following semantics:
 * \retval  n    the minimum non-zero value (in microseconds) 
 *                             reported by the system timers during the test.
 * \retval  -1   Fail
 *
 *
 *
 *
 */
extern double soar_cDetermineTimerResolution(double *min, double *max);
#endif

#ifdef DC_HISTOGRAM
/**
 *
 *
 * soar_cInitializeDCHistogram --
 *
 * \brief  Prepare an array of time structures which will be used to store
 *         the time executing a sets of decision cycles. 
 *
 *         Note that a a seperate timer is used for this.  This may be
 *         useful when the resolution of the timers is too rough to be
 *         useful at the level of a single decision cycle
 *
 * \param "-> size"  the number of slots (timeslices) in the array
 * \param "-> freq"  the number of decision cycles which are 
 *                     encapsulated by a slot in the array   
 *
 *
 * \par     Side Effects:
 *            data will be stored in the histogram.
 *
 *
 */
extern void soar_cInitializeDCHistogram(int size, int freq);
#endif                          /* DC_HISTOGRAM */

#ifdef KT_HISTOGRAM
/**
 *
 *
 * soar_cInitializeKTHistogram --
 *
 * \brief  Prepare an array of time structures which will be used to store
 *         the kernel time executing a each successive decision cycle.
 *
 *         Unlike the DCHistogram, this one uses the kernel timer, and
 *         updates onces per decision cycle
 *
 * \param "-> size"  the number of slots (timeslices) in the array
 *                     this is also the number of decision cycles
 *                     to be timed
 *
 *
 * \par     Side Effects:
 *           Memory is allocated for the KTHistogram
 *
 *
 */
extern void soar_cInitializeKTHistogram(int size);
#endif                          /* KT_HISTOGRAM */

/**
 *
 *
 * soar_cSetChunkNameLong --
 *
 * \brief  Set long or short chunk names according to the specified
 *         parameter
 *
 * \param "-> truly"  \c TRUE if the chunk name should be long
 *                    \c FALSE if it should be short
 *
 * \par     Side Effects:
 *           Modifies the chunk name
 *
 *
 */
extern void soar_cSetChunkNameLong(bool truly);

/**
 *
 *
 * soar_cSetChunkNameCount --
 *
 * \brief  Set the chunk count.
 *
 *         This must be greater than zero, less than max chunks and
 *         greater than the current chunk count.
 *
 * \param "-> count"  the new value for the current chunk count
 *
 * \return  An integer value with the following semantics:
 * \retval  0    Success
 * \retval  -1   Fail, specified count is < 0
 * \retval  -2   Fail, specified count is > max chunks
 * \retval  -3   Fail, specified count is < current chunk count
 *
 *
 * \par     Side Effects:
 *           Future chunks will have a count value relative
 *           to that which was set here
 *
 *
 */
extern int soar_cSetChunkNameCount(long count);

/**
 *
 *
 * soar_cSetChunkNamePrefix --
 *
 * \brief  Set the chunk name prefix. 
 *
 *         This is a string which is used to descriminate chunks
 *         from user productions.
 *
 * \param "-> prefix"  the prefix which should begin a chunk's name
 *
 * \return  An integer value with the following semantics:
 * \retval  0     Success
 * \retval  -1    Fail, desired prefix is not allowed
 *
 *
 *
 *
 */
extern int soar_cSetChunkNamePrefix(const char *prefix);

/**
 *
 *
 * soar_cSetLearning --
 *
 * \brief  Adjust the learning settings.
 *
 * \param "-> setting"  one of: 
 *                        \arg \c ON
 *                        \arg \c OFF
 *                        \arg \c ONLY
 *                        \arg \c EXCEPT
 *                        \arg \c ALL_LEVELS
 *                        \arg \c BOTTOM_UP
 *
 *
 */
extern void soar_cSetLearning(enum soar_apiLearningSetting setting);

/**
 *
 *
 * soar_cSetOperand2 --
 *
 * \brief  Toggles from Soar7 to Soar8 execution mode.
 *
 * \param "-> turnOn"  TRUE to run is Soar8 mode
 *                     FLASE to run is Soar7 mode
 *
 * \return  An integer value with the following semantics:
 * \retval  0    Success
 * \retval  -1   Fail, working memory is not empty
 * \retval  -2   Fail, production memory is not empty
 *
 *
 *
 */
extern int soar_cSetOperand2(bool turnOn);

/**
 *
 *
 * soar_cSetWaitSNC --
 *
 * \brief  Determine whether Soar should generate explict state-no-change
 *         impasses.
 *
 * \param "-> wait"  \c TRUE if Soar should wait 
 *                   \c FALSE if Soar should generate SNCs
 *
 *
 */
extern void soar_cSetWaitSNC(bool wait);

/**
 *
 *
 * soar_cMultiAttributes --
 *
 * \brief Modify the multi-attributes setting.
 *
 * \param "->  attr"  the attribute to be set
 * \param "->  value"  its new matching priority ( must be > 1 )
 *
 * \return  An integer value with the following semantics:
 * \retval  0    Success
 * \retval  -1   Fail, specified attribute is not a symbolic constant
 * \retval  -2   Fail, value was <= 1
 *
 *
 *
 */
extern int soar_cMultiAttributes(const char *attr, int value);

/** 
 *
 *
 * soar_cAttributePreferencesMode --
 *
 * \brief  Determine how preferences for non-context slots should be 
 *         handled.  
 *
 * \param "-> mode"  either 0,1 or 2
 *             \arg \c 0  -  Handle the normal (Soar 6) way
 *             \arg \c 1  -  Handle the normal (Soar 6) way but warn
 *                            when preferences other than + or - are 
 *                            used for non-context slots
 *             \arg \c 2  -  Warn when preferences other than + or -
 *                            are found and ingore their semantics.
 *                            NOTE: this is the only available mode
 *                            when using Soar8 (operand2)
 *
 * \return  An integer value with the following semantics:
 * \retval  0      Success
 * \retval  -1     Fail, invalid mode ( < 0 or > 2 )
 * \retval  -2     Fail, cannot switch modes in Soar8
 * 
 *
 */
extern int soar_cAttributePreferencesMode(int mode);

/*@}*/

/* *************************************************************************
 * *************************************************************************/

/**
 *   
 *  @name Callbacks
 *
 *
 */

/* *************************************************************************
 * *************************************************************************/
/*@{*/

/**
 *
 *
 * soar_cAddInputFunction --
 *
 * \brief  Adds an input function to the specified agent.
 *
 *         This is called during each input phase.  This function is
 *         really just a wrapper for the more general
 *         soar_cAddCallback() using the \c INPUT_PHASE_CALLBACK
 *         specifier
 *
 * \param "-> a"       the agent which will utilize the input function
 * \param "-> f"       the function to be called during the input phase
 * \param "-> cb_data" a pointer to a data structure known at the time of
 *                       registration, which will be passed to the callback
 *                       function when it is invoked (e.g. a filehandle)
 * \param "-> free_fn" a function to free the cb_data
 * \param "-> name"    a registeration name (this should be unique)
 *
 *
 * \see soar_cAddCallback
 *
 */
extern void soar_cAddInputFunction(agent * a, soar_callback_fn f,
                                   soar_callback_data cb_data, soar_callback_free_fn free_fn, const char *name);

/**
 *
 *
 * soar_cRemoveInputFunction --
 *
 * \brief  Remove an input function with the specified name. 
 *
 *         Since the name is used to do the removal, it is a good idea
 *         if these functions have unique names to begin with!
 *
 * \param "-> a"     the agent from which the input function should be removed
 * \param "-> name"  the registeration name of the function 
 *
 *
 * \see soar_cRemoveCallback
 *
 */
extern void soar_cRemoveInputFunction(agent * a, const char *name);

/**
 *
 *
 * soar_cAddOutputFunction --
 *
 * \brief  Similar to soar_cAddInputFunction(), but adds a function which
 *         is called durnig each output phase.
 *
 *         This function is really just a wrapper for the more general
 *         soar_cAddCallback() using the \c OUTPUT_PHASE_CALLBACK
 *         specifier which the important expection that it also
 *         checks to ensure that the output_link_name (i.e. the
 *         registration name) is unique.  This registration /must/
 *         correspond to the symbol used to represet the output-link
 *         (e.g. I3)
 *
 * \param "-> a"       the agent which will utilize the output function
 * \param "-> f"       the function to be called during the output phase
 * \param "-> cb_data" a pointer to a data structure known at the time of
 *                        registration, which will be passed to the callback
 *                        function when it is invoked (e.g. a filehandle)
 * \param "-> free_fn" a function to free the cb_data
 * \param "-> name"    a registeration name (must correspond to the 
 *                        output-link's symbol)
 *
 *
 *
 *
 */
extern void soar_cAddOutputFunction(agent * a, soar_callback_fn f,
                                    soar_callback_data cb_data,
                                    soar_callback_free_fn free_fn, const char *output_link_name);

/**
 *
 *
 * soar_cRemoveOutputFunction --
 *
 * \brief  Remove an output function with the specified name.
 *
 *         It is critical that the specified name corresponds to the
 *         output-link's symbol.
 *
 * \param "-> a"     the agent from which the output function should be removed
 * \param "-> name"  the registeration name (must correspond to the 
 *                       output-link's symbol)
 *
 *
 *
 *
 */
extern void soar_cRemoveOutputFunction(agent * a, const char *name);

/**
 *
 *
 * soar_cPushCallback --
 *
 * \brief  Push a callback onto the specified callback stack.
 *
 *         This is designed for callbacks that will only be
 *         regisstered temporarily becuase they do not have a
 *         registration name.  As a result, the only way to remove
 *         these callbacks is using the soar_cPopCallback() function 
 *         which can be problematic if other callbacks have been added
 *         in the meantime.
 *
 * \param "-> a"       the agent which will utilize the function
 * \param "-> type"    the destination stack for this callback function
 * \param "-> fn"      the function to be called 
 * \param "-> cb_data" a pointer to a data structure known at the time of
 *                        registration, which will be passed to the callback
 *                        function when it is invoked (e.g. a filehandle)
 * \param "-> free_fn" a function to free the cb_data
 *
 *
 *
 *
 */
extern void soar_cPushCallback(soar_callback_agent a,
                               SOAR_CALLBACK_TYPE type,
                               soar_callback_fn fn, soar_callback_data data, soar_callback_free_fn free_fn);

/**
 *
 *
 * soar_cAddCallback --
 *
 * \brief  Add a callback onto the specified callback stack.
 *
 *  Unlike soar_cPushCallback(), this function is also designed for
 *  callback which will be registered for a longer term.  Each
 *  callback must have an id string which is assumed to be unique
 *  within the specified callback stack.  This string is used later,
 *  for removal.
 *
 * \param "-> a"       the agent which will utilize the function
 * \param "-> type"    the destination stack for this callback function
 * \param "-> fn"      the function to be called during the input phase
 * \param "-> cb_data" a pointer to a data structure known at the time of
 *                       registration, which will be passed to the callback
 *                       function when it is invoked (e.g. a filehandle)
 * \param "-> free_fn" a function to free the cb_data
 * \param "-> id"      a registeration name 
 *
 *
 *
 * 
 */
extern void soar_cAddCallback(soar_callback_agent a,
                              SOAR_CALLBACK_TYPE type,
                              soar_callback_fn fn,
                              soar_callback_data data, soar_callback_free_fn free_fn, soar_callback_id id);

/**
 *
 *
 * soar_cPopCallback --
 *
 * \brief  Pops off the last callback to be added to the specified
 *         callback stack.
 *
 *         This function is used to remove temporary callbacks added
 *         with soar_cPushCallback().
 *
 * \param "-> a"     the agent to which the function was registered
 * \param "-> type"  the destination stack for this callback function
 *
 *
 * \par     Side Effects:
 *            removes the callback which was added to the specified stack
 *            most recently.
 *
 *
 */
extern void soar_cPopCallback(soar_callback_agent a, SOAR_CALLBACK_TYPE type);

/**
 *
 *
 * soar_cRemoveCallback --
 *
 * \brief  Remove the callback from the specified callback stack which
 *         has the specified id.
 *
 * \param "-> a"     the agent to which the function was registered
 * \param "-> type"  the destination stack for this callback function
 * \param "-> id"    the registration name
 *
 *
 * \par     Side Effects:
 *            removes specified the callback
 *
 *
 */
extern void soar_cRemoveCallback(soar_callback_agent a, SOAR_CALLBACK_TYPE type, soar_callback_id id);

/**
 *
 *
 * soar_cAddGlobalCallback --
 *
 * \brief  Add a callback onto the specified \b global callback stack.
 * 
 *         Unlike soar_cAddCallback(), this function adds to an agent
 *         independent callback stack.  Note that this does not add
 *         the callback to \b each \b agent's callback stack, but
 *         rather specifies a new set of callback stacks which are
 *         agent-independent.
 *
 * \param "-> type"    the global destination stack for this callback
 *                        function
 * \param "-> fn"      the function to be called 
 * \param "-> data"    a pointer to a data structure known at the time of
 *                        registration, which will be passed to the callback
 *                        function when it is invoked (e.g. a filehandle)
 * \param "-> free_fn" a function to free the cb_data
 * \param "-> id"      a registeration name 
 *
 *
 *
 *
 */
extern void soar_cAddGlobalCallback(SOAR_GLOBAL_CALLBACK_TYPE type,
                                    soar_callback_fn fn,
                                    soar_callback_data data, soar_callback_free_fn free_fn, soar_callback_id id);

/**
 *
 *
 * soar_cRemoveGlobalCallback --
 *
 * \brief  Remove a callback onto the specified /global/ callback stack. 
 *
 * \param "-> type"  the global destination stack for this callback function
 * \param "-> id"    the registeration name 
 *
 * \return
 *
 *
 *
 */
extern void soar_cRemoveGlobalCallback(SOAR_GLOBAL_CALLBACK_TYPE type, soar_callback_id id);

/**
 *
 *
 * soar_cListAllCallbacks --
 *
 * \brief  List the callbacks registered to the agent
 *
 * \param "-> agent"        the specified soar agent
 * \param "-> monitorable"  TRUE if only monitorable callbacks should
 *                           be listed
 *                           FALSE if all callbacks (including
 *                           PRINT and LOG) should be listed.
 *
 *
 *
 *
 */
extern void soar_cListAllCallbacks(soar_callback_agent a, bool monitorable_only);

/**
 *
 *
 * soar_cListAllCallbacksForEvent --
 *
 * \brief  List the all callbacks of a specific type registered to the agent
 *
 * \param "-> agent"  the specified soar agent
 * \param "-> type"   the specified callback type (stack) 
 *
 *
 *
 *
 */
extern void soar_cListAllCallbacksForEvent(soar_callback_agent agent, SOAR_CALLBACK_TYPE type);

/**
 *
 *
 * soar_cRemoveAllMonitorableCallbacks --
 *
 * \brief  Remove all of the callbacks (other than PRINT or LOG callbacks)
 *         registered to the specified agent
 *
 * \param "-> agent"  the specified soar agent
 *
 *
 *
 *
 */
extern void soar_cRemoveAllMonitorableCallbacks(soar_callback_agent agent);

/**
 *
 *
 * soar_cRemoveAllCallbacksForEvent --
 *
 * \brief  Remove all of the callbacks of a specified type which have
 *         been registered to the specified agent
 *
 * \param "-> agent"  the specified soar agent
 * \param "-> type"   the specified callback type (stack) 
 *
 *
 *
 *
 */
extern void soar_cRemoveAllCallbacksForEvent(soar_callback_agent agent, SOAR_CALLBACK_TYPE type);

/**
 *
 *
 * soar_cTestAllMonitorableCallbacks --
 *
 * \brief Register a simple print function on all the monitorable
 *        callback stacks (all but PRINT and LOG).
 *
 *        This is helpful to ensure that callbacks are getting issued
 *        as expected.
 *
 * \param "-> agent"  the specified soar agent
 *
 *
 *
 * 
 */
extern void soar_cTestAllMonitorableCallbacks(soar_callback_agent the_agent);

/**
 *
 *
 * soar_cCallbackNameToEnum --
 *
 * \brief  Return the enumerated type (the callback type) given an event
 *         name.
 *
 * \param "-> name"          the name of the callback type
 * \param "-> monitor_only"  TRUE if only monitorable call backs should be
 *                          searched.
 *                        FALSE otherwise
 *
 * \return  the correct callback value, or NO_CALLBACK
 *
 *
 *
 */
extern SOAR_CALLBACK_TYPE soar_cCallbackNameToEnum(const char *name, bool monitor_only);

extern void soar_cDefaultAskCallback(soar_callback_agent the_agent, soar_callback_data data, soar_call_data call_data);

/*@}*/

/* *************************************************************************
 * *************************************************************************/

/**
 *   
 *  @name Etc
 *
 *       Miscellanous controls, including:
 *               \arg Multi Agent Controls
 *               \arg Accessors for Encapsulated Data
 */

/* *************************************************************************
 * *************************************************************************/
/*@{*/

/**
 *
 *
 * soar_cGetWmeId --
 *
 * \brief  An accessor function for a psoar_wme.
 *
 *         Returns a string containing the ID of the specified wme.
 *         Note that memory to hold the string has been allocated
 *         within this function and must later be freed by the user
 *         when it is no longer in use.  
 *
 * \param "-> w"     a psoar_wme 
 * \param "<- buff"  a buffer to hold the result, or NULL
 * 
 * \return  a string the id of the wme.
 *             Note that an id is guarenteed to be an alphanumeric string
 *                  in which the first character is an uppercase letter
 *                  and the remaining characters are digits
 * 
 * \par Side Effects: 
 *           Unless \c buff is non-NULL, memory is allocated
 *           for the returned value.  This should be freed by the
 *           caller when its use is accomplished.
 *
 * \see    soar_cWmeGetAttr
 * \see    soar_cWmeGetValue
 *
 */
extern char *soar_cGetWmeId(psoar_wme w, char *buff, size_t buff_size);

/**
 *
 *
 * soar_cGetWmeAttr --
 *
 * \brief An accessor function for a psoar_wme.  
 * 
 *        Returns a string containing the Attribute of the specified
 *        wme.  Note that memory to hold the string has been allocated
 *        within this function and must later be freed by the user
 *        when it is no longer in use.
 *
 * \param "-> w"     a psoar_wme 
 * \param "<- buff"  a buffer to hold the result, or NULL
 * 
 *
 * \return  a string
 * 
 * \par     Side Effects:
 *           Unless \c buff is non-NULL, memory is allocated
 *           for the returned value.  This should be freed by the
 *           caller when its use is accomplished.
 *
 * \see    soar_cWmeGetId
 * \see    soar_cWmeGetValue
 *
 */
extern char *soar_cGetWmeAttr(psoar_wme w, char *buff, size_t buff_size);

/**
 *
 *
 * soar_cGetWmeValue --
 *
 * \brief  An accessor function for a psoar_wme.
 *
 *         Returns a string containing the Value of the specified wme.
 *         Note that memory to hold the string has been allocated
 *         within this function and must later be freed by the user
 *         when it is no longer in use.
 *
 * \param "-> w"     a psoar_wme 
 * \param "<- buff"  a buffer to hold the result, or NULL
 * 
 *
 * \return  a string
 * 
 * \par     Side Effects:
 *           Unless \c buff is non-NULL, memory is allocated
 *           for the returned value.  This should be freed by the
 *           caller when its use is accomplished.
 *
 * \see    soar_cWmeGetId
 * \see    soar_cWmeGetAttr
 *
 */
extern char *soar_cGetWmeValue(psoar_wme w, char *buff, size_t buff_size);

/**
 *
 *
 * soar_cGetWmeTimetag --
 *
 * \brief  An accessor function for a psoar_wme.
 *
 *
 * \param "-> w"  a psoar_wme 
 *
 * \return  The timetag corresponding to the wme.
 * 
 *
 */
extern unsigned long soar_cGetWmeTimetag(psoar_wme w);

/**
 *
 *
 * soar_cAddIntWme --
 *
 * \brief  A wrapper for the soar_cAddWme() function which allows
 *           easy addition of a wme whose value is an integer.
 *
 *
 * \param "-> szId"      the identifier on which the wme should be added
 * \param "-> szAttr"    the attribute of the wme.  This value may or may
 *                        not start with a '^'.  If a new identifier
 *                        is requested, this argument should be "*"
 * \param "-> value"     the value of the wme. This is an integer
 * \param "-> accept"    TRUE if the wme should receive an acceptable 
 *                        preference
 * \param "<-  new_wme"  a pointer to a psoar_wme structure which is 
 *                        set to point to the new wme during the
 *                        execution of this function
 *
 * \return  a timetag
 * 
 *
 * \see soar_cAddWme
 *
 */
extern unsigned long soar_cAddIntWme(char *szId, char *szAttr, int value,
                                     bool acceptable_preference, psoar_wme * new_wme);

/**
 *
 *
 * soar_cAddFloatWme --
 *
 * \brief  A wrapper for the soar_cAddWme() function which allows
 *         easy addition of a wme whose value is a float
 *
 *
 * \param "-> szId"      the identifier on which the wme should be added
 * \param "-> szAttr"    the attribute of the wme.  This value may or may
 *                        not start with a '^'.  If a new identifier
 *                        is requested, this argument should be "*"
 * \param "-> value"     the value of the wme. This is a float.
 * \param "-> accept"    TRUE if the wme should receive an acceptable 
 *                        preference
 * \param "<-  new_wme"  a pointer to a psoar_wme structure which is 
 *                        set to point to the new wme during the
 *                        execution of this function
 *
 *
 * \return  a timetag
 * 
 *
 *
 */
extern unsigned long soar_cAddFloatWme(char *szId, char *szAttr, float value,
                                       bool acceptable_preference, psoar_wme * new_wme);

/**
 *
 *
 * soar_cInitAgentIterator --
 *
 * \brief  Fill in a soar_apiAgentIterator structure for first use.
 *
 *         This structure is used to facilitate looping through all
 *         soar agents beginning with the currently selected agent.
 *
 *
 * \param "<- ai"  a pointer to a soar_apiAgentIterator structure
 *
 * 
 * \par     Side Effects:
 *           The ai structure is filled in such as described below:
 *             \c ai->more   -- TRUE iff there are more agents to iterate
 *                             through
 *
 *
 */
extern void soar_cInitAgentIterator(soar_apiAgentIterator * ai);

/**
 *
 *
 * soar_cStepAgentIterator --
 *
 * \brief  Increment an agent iterator srtucture. 
 *
 *         This function increments the agent iterator by one, and
 *         signals whether more agents still need to be iterated
 *         through.  Its results are based on the state of the
 *         specified agentIterator.
 *
 * \param "<-> ai"  a pointer to a previously filled in 
 *                    soar_apiAgentIterator structure
 *
 * \return  TRUE if there are more agents to iterate through
 * 
 * \par     Side Effects:
 *           the currently selected agent is changed, if there
 *           are more agents to iterate through before returning
 *           to the agent which was selected at the time 
 *           soar_cInitAgentIterator() was called, the "more" slot 
 *           in the ai structure is set to TRUE.
 *
 */

extern bool soar_cStepAgentIterator(soar_apiAgentIterator * ai);

/**
 *
 *
 * soar_cGetAgentByName --
 *
 * \brief  Get a pointer to a soar agent
 *
 *         This function locates a specific agent, and returns a generic
 *         pointer to it. Note that the current agent is \b not affected.
 *
 * \param "-> name"  the name of the agent to look for.
 *
 * \return  a \c psoar_agent pointer which references the specified agent
 *          or \c NULL if no such agent is found.
 * 
 *
 */

extern psoar_agent soar_cGetAgentByName(char *name);

/**
 *
 *
 * soar_cGetIdForAgentByName --
 *
 * \brief  Get the unique id of a particular agent
 *
 *         This function locates a specific agent, and returns its
 *         unique identifier.
 *
 * \param "-> name"  the name of the agent to look for.
 *
 * \return  a unique id or -1 if no agent is found.
 * 
 *
 */
int soar_cGetIdForAgentByName(char *name);

/**
 * soar_cSetCurrentAgentByName --
 *
 * \brief  Set the 'current agent'.
 *
 *         This function changes the agent that will be affected by
 *         subsequent API function calls
 *
 * \param "-> name"  the name of the agent come into focus
 *
 * \return  TRUE iff the operation was successful
 * 
 *
 */
bool soar_cSetCurrentAgentByName(char *name);

/**
 * soar_cSetCurrentAgent --
 *
 * \brief  Set the 'current agent'.
 *
 *         This function changes the agent that will be affected by
 *         subsequent API function calls
 *
 * \param "-> agent"  the psoar_agent structure of the agent which should
 *                      receive the focus
 *
 * 
 *
 */
void soar_cSetCurrentAgent(psoar_agent agent);

/**
 * soar_cGetCurrentAgent --
 *
 * \brief  Get the 'current agent'.
 *
 *
 * \return    the psoar_agent structure for the agent which is currently
 *               affected by API function calls
 *
 */
psoar_agent soar_cGetCurrentAgent();

/**
 * soar_cGetAgentInputLinkId --
 * *
 * \brief  An accessor function for a psoar_agent.
 *
 *         Returns a string containing the ID of agent's input-link.
 *         Note that memory to hold the string has been allocated
 *         within this function and must later be freed by the user
 *         when it is no longer in use.  
 *
 * \param "-> w"     a psoar_agent 
 * \param "<- buff"  a buffer to hold the result, or NULL
 * 
 * \return  a string representation of the input-link identifier or NULL
 *          if the input-link does not exist
 *             Note that an id is guarenteed to be an alphanumeric string
 *                  in which the first character is an uppercase letter
 *                  and the remaining characters are digits
 * 
 * \par Side Effects: 
 *           Unless \c buff is non-NULL, memory is allocated
 *           for the returned value.  This should be freed by the
 *           caller when its use is accomplished.
 *
 */
char *soar_cGetAgentInputLinkId(psoar_agent a, char *buff, size_t buff_size);

/**
 * soar_cGetAgentOutputLinkId --
 * *
 * \brief  An accessor function for a psoar_agent.
 *
 *         Returns a string containing the ID of agent's output-link.
 *         Note that memory to hold the string has been allocated
 *         within this function and must later be freed by the user
 *         when it is no longer in use.  
 *
 * \param "-> a"     a psoar_agent 
 * \param "<- buff"  a buffer to hold the result, or NULL
 * 
 * \return  a string representation of the ouput-link identifier or NULL if
 *          the output-link does not exist
 *             Note that an id is guarenteed to be an alphanumeric string
 *                  in which the first character is an uppercase letter
 *                  and the remaining characters are digits
 * 
 * \par Side Effects: 
 *           Unless \c buff is non-NULL, memory is allocated
 *           for the returned value.  This should be freed by the
 *           caller when its use is accomplished.
 *
 */
char *soar_cGetAgentOutputLinkId(psoar_agent a, char *buff, size_t buff_size);

/**
 * soar_cGetAgentId --
 * *
 * \brief  An accessor function for a psoar_agent.
 *
 *         Returns a string containing the agent's unique integer ID.
 *         This integer is in the range 0 ... MAX_SIMULTANEOUS_AGENTS
 *         and no two agents have the same identifier.
 *
 * \param "-> a"     a psoar_agent 
 * 
 * \return  the unique integer identifier of the specified agent
 * 
 *
 */
int soar_cGetAgentId(psoar_agent a);

/**
 * print --
 *
 * \brief  Print a given string using the agent's current print function
 *
 *         This function invokes the current agent's top level
 *         \c PRINT_CALLBACK & \c LOG_CALLBACK to perform an agent specific
 *         print operation.  This operation mimics the output
 *         generated internally by the soar kernel, and so using this
 *         function provides an easy way to guarentee that input
 *         produced by you interface, and by the soar kernel will be
 *         handled the same way.
 *
 * 
 * */
#ifdef USE_STDARGS
void print(char *format, ...);
#else
void print();
#endif

/*@}*/

#endif
