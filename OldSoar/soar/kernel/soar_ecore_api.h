/**
 * \file soar_ecore_api.h
 *   
 *                     The Extended Low Level interface to Soar
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

#ifndef _SOAR_ECORE_API_        /* excludeFromBuildInfo */
#define _SOAR_ECORE_API_

#include "soarkernel.h"
#include "soarapi_datatypes.h"
#include "soar_ecore_utils.h"
#include "soar_core_api.h"

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
 * soar_BuildInfo --
 *
 * \brief  Indicates compile time setting which affect
 *         the current instantiation of Soar.
 *
 *         For example, the \c DETAILED_TIMERS options must be set at
 *         compile time.  This function prints such compile time
 *         options that were defined for the particular build.
 *
 *         Note that the options which are printed using this function
 *         are filtered.  They are expected to be the most pertanant
 *         of all the compile time options.
 *
 * \see soar_ecExcludedBuildInfo
 *
 */
extern void soar_ecBuildInfo(void);

/**
 *
 * soar_ecExcludedBuildInfo --
 *
 * \brief  Indicates compile time setting which affect the current
 *         instantiation of Soar.
 *         
 *         Note that the options which are printed using this function
 *         are filtered.  The options presented using this command are
 *         those which are expected to be of lesser importance.  However,
 *         using this command in conjunction with \c soar_ecBuildInfo
 *         will list \b all compile time options for the current build.
 *
 * \see    soar_ecBuildInfo
 *
 *
 */
extern void soar_ecExcludedBuildInfo(void);

/**
 *
 *
 *     soar_ecSetDefaultWmeDepth --
 *
 * \brief  Set the default wme depth to a new value
 *
 *         The default wme depth indicates how much substructure
 *         should be printed when wmes are displayed.  The default
 *         wme depth can be overridden by using the \c -depth flag
 *         with the print command.
 *
 * \param "-> depth" the new default wme depth.
 *
 * \par   Side Effects:
 *           The agent's default wme depth is modified.
 *
 *
 */
extern void soar_ecSetDefaultWmeDepth(int depth);

/**
 *
 *
 *     soar_ecOpenLog --
 *
 *
 * \param "-> mode"      either "w" to write to a new file
 *                         or "a" to append to an existing file
 * \param "-> filename"  the file's name
 *       
 * \return An integer value with the following semantics:
 * \retval 0   Success
 * \retval -2  Fail, attempted to open, but a lo
 *
 * 
 * \par   Side Effects:
 *           A log file is opened to which output is echoed.
 *
 */
extern int soar_ecOpenLog(const char *filename, char *mode);

/**
 *
 *
 *     soar_ecCloseLog --
 *
 * \brief  Close the log file.
 *
 *
 * \return An integer value with the following semantics:
 * \retval 0    Success
 * \retval -1   Fail, attempted to close, but no log file was open
 *
 * \par    Side Effects:
 *           The log file (if opened) is closed.
 *
 *
 */
extern int soar_ecCloseLog();

#ifdef USE_CAPTURE_REPLAY

/**
 *
 *
 *     soar_ecCaptureInput --
 *
 * \brief  Captures the input sent to the agent during by external
 *         calls to add-wme and remove-wme.
 *
 * \param "-> filename"  the name of the capture file or NIL to 
 *                       stop capturing
 * \return An integer value with the following semantics:
 * \retval 0  Success
 * \retval -1  Tried to close the capture file, but no file was open
 * \retval -2  Tried to open the capture file, but a capture file was
 *                  already open
 * \retval -3  Could not open the specified file.
 *
 * \par   Side Effects: 
 *            A (potentially) big file will be created containing all
 *            the input which was received by the agent during
 *            execution.
 *
 * */
extern int soar_ecCaptureInput(const char *filename);

/**
 *
 *
 *     soar_ecReplayInput --
 *
 * \brief Replays the input previously captured using the CaptureInput
 *         command.
 *
 * \param "-> filename"  the name of the capture file or NIL to 
 *                       stop capturing
 *
 *
 * \return An integer value with the following semantics:
 * \retval 0   Success
 * \retval -1   Could not open the specified file.
 * \retval -2   Incorrect file header.
 *
 * \par    Side Effects:
 *           Adds (or removes) an input function corresponding to the 
 *           current agent.  Constructs an array based on the contents
 *           of the input file which is used to supply input to the agent
 *           at run-time
 *
 *
 */
extern int soar_ecReplayInput(const char *filename);

#endif

/**
 *
 *
 *     soar_ecGDSPrint --
 *
 * \brief  Debug routine for examing the GDS when necessary.  
 * 
 *         This is horribly inefficient and should not generally be
 *         used except when something is going wrong and you want to
 *         take a peak at the GDS
 *
 *
 *
 */
extern void soar_ecGDSPrint();

/**
 *    soar_ecExplainChunkTrace --
 *
 * \brief  Explain how a chunk came into being
 *
 *         This function prints information about the productions
 *         which fired to yield the specified chunk.  It is
 *         a more verbose version of soar_ecExplainChunkConditionList
 *
 * \param "chunk_name ->"  the name of the chunk to explain
 *
 */
extern void soar_ecExplainChunkTrace(char *chunk_name);

/**
 *    soar_ecExplainChunkCondition --
 *
 * \brief  Explain how a particular condition of a chunk came into being
 *
 * \param "chunk_name ->"  the name of the chunk to explain
 * \param "cond_number ->"  the number of the condition to examine
 *
 */
extern void soar_ecExplainChunkCondition(char *chunk_name, int cond_number);

/**
 *    soar_ecExplainChunkConditionList --
 *
 * \brief  Explain how a chunks conditions came into being
 *
 *         Prints the chunk, and then each condition and its 
 *         associated ground wme.  This is a less verbose version 
 *         of \c soar_ecExplainChunkTrace
 *
 * \param "chunk_name ->"  the name of the chunk to explain
 *
 * \see soar_exExplainChunkTrace
 *
 */
extern void soar_ecExplainChunkConditionList(char *chunk_name);

/**
 *
 *
 *     soar_ecPrintFiringsForProduction --
 *
 * \brief  Print the number of times a production has fired.
 *
 *         This prints the number of times that particular production
 *         has fired, or "No production named <name>" if the
 *         specified production is not defined.
 *
 * \param "-> name"  the name of the specified production
 *
 * Side Effects:
 *
 *
 *
 */
extern void soar_ecPrintFiringsForProduction(const char *name);

/**
 *
 *
 *     soar_ecPrintTopProductionFirings --
 *
 * \brief  Print the top <n> productions with respect to their frequency of
 *         firing.
 * 
 *         Prints the top firing produtions and the number of times
 *         they have fired.  If no productions are defined, then "***
 *         No productions defined ***" is printed.
 *
 * \param  "n ->"  the number of productions to print
 *
 *
 *
 */
extern void soar_ecPrintTopProductionFirings(int n);

/**
 *
 *
 *     soar_ecPrintMemoryPoolStatistics --
 *
 * \brief  Print detailed information about Soar's internal memory usage
 *
 *         Soar uses its own memory allocation scheme to store much of
 *         its data.  Memory is allocated from one of a number of memory
 *         pools, associated with a particular agent.  This function
 *         shows information about the size and free space of these pools
 *
 */
extern void soar_ecPrintMemoryPoolStatistics(void);

/**
 *
 *
 *     soar_ecPrintMemoryStatistics --
 *
 * \brief  Print information about Soar's internal memory usage
 *
 *         This function gives an overview of Soar's memory usage
 *         without examining each pool in any depth.
 *
 */
extern void soar_ecPrintMemoryStatistics(void);

/**
 *
 *
 *     soar_ecPrintReteStatistics --
 *
 * \brief  Print information about the rete
 *
 *
 */
extern void soar_ecPrintReteStatistics(void);

/**
 *
 *
 *     soar_ecPrintSystemStatistics --
 *
 * \brief  Print information about the state of Soar
 *
 *         This function prints a variety of information about Soar's
 *         current state.  Most importantly, it includes how long
 *         Soar has been running (both in seconds, and decision cycles).
 */
extern void soar_ecPrintSystemStatistics(void);

#ifdef DC_HISTOGRAM

/**
 *
 *
 *     soar_ecPrintDCHistogram --
 *
 * \brief  Print the decision cycle time histogram (if one has been kept)
 *
 */
extern int soar_ecPrintDCHistogram(void);
#endif                          /* DC_HISTOGRAM */

#ifdef KT_HISTOGRAM
/**
 *
 *
 *     soar_ecPrintKTHistogram --
 *
 * \brief  Print the kernel time histogram (if one has been kept)
 *
 */
extern int soar_ecPrintKTHistogram(void);
#endif                          /* KT_HISTOGRAM */

/**
 *
 *
 *     soar_ecPrintAllProductionsOfType --
 *
 * \brief  Print all productions of the specified type.
 *
 *
 * \param "type ->"        the type of production
 * \param "internal ->"    \c TRUE iff the internal representation 
 *                              should be used
 * \param "print_fname ->" \c TRUE iff the file from which the production 
 *                              was loaded should also be printed
 * \param "full_prod ->"   \c TRUE if the full production should be printed
 *                             \c FALSE if only the name should be printed
 *
 * \return An integer representing the status of the operation
 * \retval 0  Success
 * \retval -1 Fail, invalid type
 *
 */

extern int soar_ecPrintAllProductionsOfType(int type, bool internal, bool print_fname, bool full_prod);

/**
 *
 *
 *     soar_ecAddWmeFilter --
 *
 * \brief  Add a wme filter to control which wmes are displayed
 *
 *         Normally, when wmes are printed during execution (as
 *         when watch 5 is set) all changes to working memory
 *         are displayed.  Adding a wme filter, allows the user
 *         to select a subset of these changes for display.
 *     
 *         wmes which match the filter are printed.
 *
 *         if the parameters to this function refer to identifiers,
 *         the identifier must already be in existence when then 
 *         filter is added.
 *
 * \param "szId ->"     the id of a matching wme, or \c * for wildcard
 * \param "szAttr ->"   the attribute of a matching wme, or \c *
 * \param "szValue ->"  the value of a matching wme, or \c *
 * \param "adds ->"     \c TRUE iff the filter should be applied
 *                             to wmes which are being put into wm.
 * \param "removes ->"  \c TRUE iff the filter should be applied 
 *                             to wmes which are being taken out of wm.
 *
 * \return An integer status
 * \retval  0   Success.
 * \retval -1   Fail, the specified identifier is invalid.
 * \retval -2   Fail, the specified attribute is invalid.
 * \retval -3   Fail, the specified valus is invalid.
 * \retval -4   Fail, the filter already exists.
 */

extern int soar_ecAddWmeFilter(const char *szId, const char *szAttr, const char *szValue, bool adds, bool removes);

/**
 *
 *
 *     soar_ecRemoveWmeFilter --
 *
 * \brief  Remove a previously specified wme filter
 *
 *
 *
 * \param "szId ->"     the id string specified when the wme was added
 * \param "szAttr ->"   the attribute string specified when the wme was added
 * \param "szValue ->"  the value string specified when the wme was added
 * \param "adds ->"     the same value as specified when the filter was added
 * \param "removes ->"  the same value as specified when the filter was added
 *
 * \return An integer status
 * \retval  0   Success.
 * \retval -1   Fail, the specified identifier is invalid.
 * \retval -2   Fail, the specified attribute is invalid.
 * \retval -3   Fail, the specified valus is invalid.
 * \retval -4   Fail, the filter could not be found.
 * 
 */
extern int soar_ecRemoveWmeFilter(const char *idStr, const char *attrStr,
                                  const char *valueStr, bool adds, bool removes);

/**
 *
 *
 *     soar_ecResetWmeFilters --
 *
 * \brief  Remove all wme filters
 *
 * \return  An interger status
 * \retval  0   Success
 * \retval -1   No removes were performed
 *  
 */
extern int soar_ecResetWmeFilters(bool adds, bool removes);

/**
 *
 *
 *     soar_ecListWmeFilters --
 *
 * \brief  Print a list of the wme filters which are currently active
 *         
 *
 *
 * param "adds ->"    \c TRUE iff the filters applying to wme additions 
 *                               should be listed
 * param "removes ->"  \c TRUE iff the filters applying to wme removals
 *                               should be listed
 *
 */
extern void soar_ecListWmeFilters(bool adds, bool removes);

/**
 *
 *
 *     soar_ecSp --
 *
 * \brief  Source a production
 *
 *         Parses a production in text representation and 
 *         loads it into the rete.
 *
 * \param "rule ->"       the textual representation of the production
 * \param "sourceFile ->" the file from which the production was loaded
 *                            or NULL
 *
 * \return An integer status code
 * \retval 0   Success
 * \retval -1  Fail, production could not be parsed
 *
 */
extern int soar_ecSp(const char *rule, const char *sourceFile);

/**
 *
 *
 *     soar_ecPrintMatchSet --
 *
 * \brief  Print the current match set (the productions which are matched)
 *
 * \param "wtt ->"  specifies how much information about the wmes
 *                         which are part of the match should be printed.
 *                         Must be one of:
 *                         \arg \c NONE_WME_TRACE     don't print anything
 *                         \arg \c TIMETAG_WME_TRACE  print only the timetag
 *                         \arg \c FULL_WME_TRACE     print the entire wme
 *
 * \param "mst ->"  specifies which type of matches should be printed.
 *                         Must be onw of:
 *                         \arg \c MS_ASSERT_RETRACT  print all matches
 *                         \arg \c MS_ASSERT          print only assertions
 *                         \arg \c MS_RETRACT         print only retractions
 *
 */
extern void soar_ecPrintMatchSet(wme_trace_type wtt, ms_trace_type mst);

/**
 *
 *
 *     soar_ecPrintMatchInfoForProduction --
 *
 * \brief  Show detailed information as to why a production does
 *         or doesn't match
 *
 *
 * \param "name ->"  the name of the production being scrutenized
 * \param "wtt ->"  specifies how much information about the wmes
 *                         which are part of the match should be printed.
 *                         Must be one of:
 *                         \arg \c NONE_WME_TRACE     don't print anything
 *                         \arg \c TIMETAG_WME_TRACE  print only the timetag
 *                         \arg \c FULL_WME_TRACE     print the entire wme
 *
 * \return  An integer status
 * \retval  0  Success
 * \retval -1  Fail, production not found.
 *
 */
extern int soar_ecPrintMatchInfoForProduction(const char *name, wme_trace_type wtt);

/**
 *
 *
 *     soar_ecPrintInternalSymbols --
 *
 * \brief  Print Soar's internally allocated symbols
 *
 */
extern void soar_ecPrintInternalSymbols(void);

/**
 *
 *
 *     soar_ecPrintPreferences --
 *
 * \brief  Print the preferences for a particular (id, attribute) pair
 *
 */
extern int soar_ecPrintPreferences(char *szId, char *szAttr, bool print_prod, wme_trace_type wtt);

/**
 *
 *
 *     soar_ecPrintProductionsBeingTraced --
 *
 * \brief  Print a list of all the productions currently being traced (watched)
 *
 *
 */
extern void soar_ecPrintProductionsBeingTraced();

/**
 *
 *
 *     soar_ecStopAllProductionTracing --
 *
 * \brief  Stop tracing all productions.
 *
 */
extern void soar_ecStopAllProductionTracing();

/**
 *
 *
 *     soar_ecBeginTracingProductions --
 *
 * \brief  Begin tracing a set of productions
 *
 * \param "n ->"     the number of productions to begin tracing
 * \param "names ->" an array (length n ) of production names corrseponding
 *                      to those which will be traced.
 *
 * \return An integer status value
 * \retval 0  Success
 * \retval -n The (n-1)th entry of the name array holds an invalid
 *                production name.  No productions after this index were
 *                added to the watch list.
 */
extern int soar_ecBeginTracingProductions(int n, const char **names);

/**
 *
 *
 *     soar_ecStopTracingProductions --
 *
 * \brief  Stop tracing a set of productions
 *
 * \param "n ->"     the number of productions to stop tracing
 * \param "names ->" an array (length n ) of production names corrseponding
 *                      to those which will no longer be traced .
 *
 * \return An integer status value
 * \retval 0  Success
 * \retval -n The (n-1)th entry of the name array holds an invalid
 *                production name.  No productions after this index were
 *                added to the watch list.
 *
 *
 */
extern int soar_ecStopTracingProductions(int n, const char **names);

/**
 *
 *
 *     soar_ecPrintMemories --
 *
 * \brief Prints information about the memory usage of different
 *               production types  
 *
 * 
 * \param "num ->"   the number of productions in each category to print
 *                            or \c -1 to print all of them
 *
 * \param "to_print ->" An array of size NUM_PRODUCTIONS_TYPES
 *                      in which  each slot:
 *                         \arg \c USER _PRODUCTION_TYPE
 *                         \arg \c DEFAULT_PRODUCTION_TYPE
 *                         \arg \c CHUNK_PRODUCTION_TYPE
 *                         \arg \c JUSTIFICATION_PRODUCTION_TYPE
 *                      is set to \c TRUE if it is to be printed
 *                        and \c FALSE otherwise.
 *
 */
extern void soar_ecPrintMemories(int num, int to_print[]);

/**
 *
 *
 * soar_ecWatchLevel --
 *
 * \brief  Set the watch level (the verbosity of output)
 *
 * \param "-> level"  0 - 5
 *
 * \return   An integer value with the following semantics:
 * \retval   0    Success
 * \retval   -1   Invalid Level
 *
 *
 *
 */
extern int soar_ecWatchLevel(int level);

/**
 *
 *
 * soar_ecPrintAllProductionsWithInterruptSetting --
 *
 * \brief  Prints all productions with the specified interrupt setting
 *
 * \param "-> interrupt_setting"  INTERRUPT_ON or INTERRUPT_OFF
 *                                (INTERRUPT_PRINT will never match)
 *
 */
extern void soar_ecPrintAllProductionsWithInterruptSetting(enum soar_apiInterruptSetting interrupt_setting);

/*@}*/
#endif                          /* _SOAR_ECORE_API_ */
