 
/**
 *  \file BuildOptions
 *
 *  \brief Soar's compile time options.
 *
 *
 *   All compile time options described in this document should
 *   be defined prior to compilation in a file called
 *   "soarBuildOptions.h".  This file is included by all of 
 *   soar's kernel files.  Once a build is completed, you may
 *   determine the options it was built with using the two core api
 *   commands "soar_ecBuildInfo" and "soar_ecExcludedBuildInfo"
 *   it is expected that some compile time flags will be more informative
 *   than others.  These are listed in using the first command.  All
 *   remaining compile time flags have been explicitly hidden using 
 *   comments in the source code.  It is expected that they will be of
 *   no particular use, but for completeness, they can be view using the
 *   second command (soar_ecExcludedBuildInfo)
 *   
 *
 *
 *
 * \par About the Options:
 *   
 *   The compile time options described below can all be used to modify
 *   the resulting Soar application.  Some of these options have only
 *   subtle effects, while others are much more overt.  To those getting
 *   started down this path, you might want to consider using one of
 *   the high level build options:
 *
 *      STD        - make a "normal" version of Soar.
 *                     do not include Detailed Timers, or 
 *                     debugging facilities
 *      
 *     
 *      HEAVY      - make a "heavy" version of Soar. 
 *                     include Detailed Timers
 *
 *
 *      LITE       - make a "lite" version of Soar.
 *                     do not include many of the callbacks
 *                     do not include support for learning
 *                     perform special optimizations when possible
 *
 * \see STD
 * \see HEAVY
 * \see LITE
 *
 */

/**
 *
 *
 *
 *  \def ALLOW_I_SUPPORTED_SUBGOAL_RESULTS_WITH_THIN_JUSTS
 *       Requires: THIN_JUSTIFICATIONS 
 * 
 *	 When used with THIN_JUSTIFICATIONS
 *	 (or SINGLE_THIN_JUSTIFICATION), this option overrides the 
 *	 default behvaior which is to force O-support for subgoal 
 *	 results.  With this option, support for preferences from 
 *	 subgoal results is calculated in the normal fashion. NOTE:
 *	 using this option will often result in memory leaks, because
 *	 instantiations which support subgoal results will never be
 *	 deallocated.
 *
 *	 \see WARN_IF_RESULT_IS_I_SUPPORTED
 */

/**
 *  \def COUNT_KERNEL_TIMER_STOPS
 *       Requires:  KERNEL_TIME_ONLY
 * 
 *       Setting this option instructs Soar
 *       to keep track of how many times the timers have been toggled.
 *       This is especially useful when highly accurate benchmarking data
 *       is needed and the cost of system timing calls must be estimated.
 */

/**
 *  \def DC_HISTOGRAM	
 *        
 *	 With this option defined, Soar will keep track of the time
 *	 spent on sets of decision cycles.  In systems with poor timer
 *	 resolution, this can potentially be used to provide more accurate
 *	 timing information.  Unlike Soar's normal timers which are turned
 *	 on many times per decision cycle (even when DETAILED_TIMERS is not
 *	 defined, and even if KERNEL_TIME_ONLY is defined), DC_HISTOGRAM
 *	 uses its own timer which is started every nth decision
 *	 cycle.  The timer continues throughout all operation and measures
 *	 the USER time (as do all Soar timers) spent in each section of
 *	 n consecutive decision cycles.  This means that the timer is turned
 *	 on and off only infrequently (and the rate can be set dynamcally)
 *	 so that it can be guarenteed that this rate is much larger than
 *	 the timer's granularity. (See also KT_HISTOGRAM)
 */

/** 
 *  \def DEBUG_CHUNK_NAMES
 * 
 *       Setting this option spams some marginally useful messages about
 *       creating chunk names.  It may come in handy for certain debugging
 *       situations, but is not recommended for normal users.
 */



/**
 *  \def DEBUG_MEMORY
 *
 *	 Enables extended debugging of Soar's internal memory allocation
 *       system.  This is not recomende except for debugging the Soar
 *       kernel itself.
 */



/**
 *  \def DETAILED_TIMING_STATS
 *	 Requires: not( NO_TIMING_STUFF )
 *
 *       Without this defined, Soar will keep track of time spent in
 *	 only a small number of areas.  Most important among them are
 *	 the time spent in the kernel itself; in input and output
 *	 functions and the total cput time of the entire operation.
 *	 With this defined, Soar keeps track of time at a much more
 *	 refined level of detail. Note that this is not always a great
 *	 idea.  First of all, this option will slow down the overall
 *	 operating speed of Soar. Secondly the times reported by the
 *	 timers (especially the subtimers) may be misleading becuase
 *	 the timers are turned on and off within a time frame that is
 *	 on par with the resolution of the timer itself.
 *
 */

/**
 *  \def DONT_ALLOW_VARIABLIZATION     - highly suggested for soar-lite
 *
 *       this option prevents chunks/justifications from being 
 *       variablized, and in doing so removes a small amount of code
 *       from being executed.  As a result, however, learning is not 
 *       effective, so this option should only be used in those 
 *       situations in which learning is profitable. (As in the
 *       situations which soar-lite is targeted for).
 */

/**
 *  \def DONT_CALC_GDS_OR_BT	    
 *     
 *       This option prevents backtracing as well as refrains from
 *       calculating any goal dependency sets.  Note that depending
 *	 on the productions, the behavior of Soar using this build
 *	 option may differ from the original Soar8 version.  Initial
 *	 tests suggest that using this option does not result in
 *	 a significantly faster kernel.
 */

/**
 *  \def DONT_DO_IO_CYCLES
 *	  
 *       This options prevents Soar from performing the input and
 *	 output cycle.  Although, Soar will still enter both
 *	 phases, calls to do_input_cycle are removed as well as some
 *	 minor additional streamlining.
 *
 */

/**
 *  \def FEW_CALLBACKS             - suggested
 *
 *       strips out many of the callbacks which are routinely
 *       invoked during execution.  Some critical callbacks
 *       remain, as do those which are executed only before
 *       and after execution.  A complete list of the 
 *       callbacks which remain usable if this option is invoked
 *       follows:
 *
 *         \arg AFTER_INIT_AGENT_CALLBACK (may be removed)
 *         \arg SYSTEM_STARTUP_CALLBACK
 *         \arg AFTER_DECISION_CYCLE_CALLBACK
 *         \arg SYSTEM_TERMINATION_CALLBACK
 *         \arg SYSTEM_PARAMETER_CHANGED_CALLBACK
 *         \arg BEFORE_INIT_SOAR_CALLBACK
 *         \arg AFTER_INIT_SOAR_CALLBACK
 *         \arg AFTER_HALT_SOAR_CALLBACK
 *         \arg PRINT_CALLBACK
 *         \arg LOG_CALLBACK
 *         \arg RECORD_CALLBACK
 *         \arg INPUT_CYCLE_CALLBACK
 *         \arg OUTPUT_CYCLE_CALLBACK
 *         \arg all global callbacks
 */

/**
 *  \def HEAVY
 *
 *       This is not a real build option in itself, but an agglomerate
 *	 of other build options.  It is specified only in the file
 *	 soarBuildOptions.h, and defines a \b heavy build of Soar
 *	 which includes a large amount of functionality.
 *
 *	 \see LITE
 *	 \see STD
 */


/**
 *  \def KERNEL_TIME_ONLY
 * 
 *       with this option set, soar only keeps track of the kernel time
 *       which has been used.  This is in contrast to building without
 *       any flags or with the \c DETAILED_TIMING_STATS flag.  In
 *       either of those cases, more timers are used for tracking Soar's
 *       performance.  In some systems, setting this option may improve
 *       performance significantly.
 */

/**
 *   \def KT_HISTOGRAM
 *	
 *        Like the \c DC_HISTOGRAM option, this option keeps track of the
 *	  time spent in individual decision cycles.  Unlike
 *	  \c DC_HISTOGRAM, however, \c KT_HISTOGRAM does not use its own
 *	  timer.  Instead, it relys on Soar's decision cycle timer.
 *	  Whenever the kernel timer is stopped, the acrued time is
 *	  added not only to the total kernel time, but also to the slot
 *	  in the kt_histogram for the current decision cycle.  This means
 *	  that the sum of the bins in the kt_histogram will sum up to
 *	  exactly the kernel time.  But it also means that these results
 *	  will be misleading if the granulairty of the timers is relatively
 *	  high.  In such cases it is better to use the \c DC_HISTOGRAM with
 *	  a frequency of many decision cycles.  When the timers have low
 *	  granularity, however, this function will produce good results, and
 *	  may be more interesting than using the \c DC_HISTOGRAM with frequency
 *	  of 1 decision cycle since these timers do not include time spent
 *	  in the input/output functions.
 *
 */


/**
 *  \def LITE
 * 
 *       This is not a real build option in itself, but an agglomerate
 *	 of other build options.  It is specified only in the file
 *	 soarBuildOptions.h, and defines a \b lite build of Soar
 *	 which reduces Soar's functionality to a realitve minimum
 *
 *	 \see HEAVY
 *	 \see STD
 */


/**
 *  \def NO_TOP_LEVEL_REFS              - suggested (buggy?)
 *
 *       without this options, reference counts on data structures 
 *       which reside in the top state (i.e. level 1) are incremented
 *       just as those which correspond to any other data structure.
 *       However, this method of reference counting results in many
 *       (perhaps all?) of these top level data structures to remain
 *       in the memory pools, never to be deallocated.  Programs which
 *       operate mainly on the top state (such as a flattened version of
 *       towers of hanoi) suffer huge memory leaks. It is possible that
 *       without these references, your program could behave differently,
 *       or possibly crash, but if you are intending to run for long 
 *       periods of time, where memory may become an issue, it is worth
 *       testing your system with this option
 */

/**
 * \def MEMORY_POOL_STATS 
 * 
 *      Use this build option if you want Soar to keep statistics about
 *      its usage of the interal memory pools.  Do not define this option
 *      if you would prefer to avoid this overhead.
 */


/**
 *   \def MAKE_PRODUCTION_FOR_THIN_JUSTS
 *        Requires: THIN_JUSTIFICATIONS
 *	 
 *	  This option, when used in conjuction with THIN_JUSTIFICATIONS
 *	  (and possibly SINGLE_THIN_JUSTIFICATION) allows the justification
 *	  to be created, but still refrains from adding it to the rete.
 *	  The only reason this is at all useful is for debugging purposes
 *	  where it can be useful to see such structures as they are created
 *	  and examine their conditions and actions.
 */

/**
 *  \def MAX_SIMULTANEOUS_AGENTS
 *	
 *	 This options specifies the number of maximum simultaneous
 *	 agents for any particular individual soar instantiation.
 *	 Typically, this is set at 128, much greater than required
 *	 for the typical user.  Agent ids are assigned from the range
 *	 (\c 0, \c MAX_SIMULATANEOUS_AGENTS \c - \c 1).  This ensures
 *	 that external code which invokes Soar functionality (such as
 *	 the Tcl interface) can quickly associate externally defined 
 *	 data (such as a Tcl interpreter) with an agent by placing
 *	 such data into an array of size \c MAX_SIMULTANEOUS_AGENTS.
 *	 Agent ids are assigned in pseudo increasing order, so that
 *	 an id will not be reused until all other ids have been used at 
 *	 least once. 
 *
 */
	 
/**
 *
 *  \def MHZ
 *
 *	This option is used to specify the clock speed (in megahertz)
 *	of the platform on which soar is both compiled and used.  It
 *	only needs to be defined if you are also using \c PII_TIMERS, an
 *	option which may be important in some research situations, 
 *	but will be unwarrented for most users.
 *
 *	\see PII_TIMERS
 *
 */

/**
 *
 *  \def NO_ADC_CALLBACK
 *      
 *	 this option removes the AFTER_DECISION_CYCLE_CALLBACK which
 *	 is probably one of the most useful callbacks. It doesn't make
 *	 much sense to use this without FEW_CALLBACKS 
 *
 */


/**
 *  \def NO_TOP_JUST		 - highly suggested
 *
 *       this option prevents justifications from being built at the top 
 *       level.  Since they serve no purpose there anyway, this provides
 *       a performance gain with no negative tradeoffs, thus it is 
 *       suggested for all builds.
 * 
 */


/**
 *  
 *  \def OPTIMIZE_TOP_LEVEL_RESULTS
 *       Requires: NO_TOP_JUST
 *
 *       results which are retured solely to the top level can be 
 *       optimized even  beyond the point which SINGLE_THIN_JUSTIFICAITON
 *       provides.  This is due largely to the fact that there is on 
 *       GDS for the top state and so no backtracing needs to be done.  
 *       Although it seems this option avoids a large portion of code 
 *       which is necessary for learning.  Therefore, it is probably most
 *       useful when used in conjunction with SINGLE_THIN_JUSTIFICATION.
 *
 *       \see NO_TOP_JUST
 *
 */

/**
 *  \def PII_TIMERS
 *	 Requires:  MHZ
 *
 *       This option is only valid on machines which use an Intel 
 *	 Pentium II or higher processor.  Builds with this option
 *	 defined do not use the timers provided by the system, but
 *	 instead use a timer on-board the PII itself to perform 
 *	 timing operations.  Using this option, there is can be no
 *	 distinction between different processes which may be sharing
 *	 the CPU, this is in contrast to the system timers which measure
 *	 only the time used by the Soar process itself.  However, in
 *	 a lightly loaded machine, Soar may well be the dominating process
 *	 if this is true, then the advantage of these timers is their
 *	 exteremly fine granularity which is unmatched.  System timers
 *	 operate at the rate of the scheduler (typically 10ms) and thus
 *	 have a relatively large granularity. The PII timers update once
 *	 a clock cycle, thus providing a granularity of well under 1us.
 *
 *	\see MHZ
 */

/**
 *  \def REMOVE_INSTS_WITH_O_PREFS 
 *   
 *       Instantiations with all O-supported results don't really need
 *       to stick around in memory after their preferences have been
 *       asserted so long as no learning (or backtracing) will take
 *       place.  (At least as far as I understand). In a normal build
 *       of Soar, however, they do stick around and after a long
 *       period of time (sometimes forever!), can take up a lot of
 *       space.  This build option removes such instantiations after
 *       their preferences have been asserted.
 *
 */

/** 
 *  \def SINGLE_THIN_JUSTIFICATION 
 *       Requires:  THIN_JUSTIFICATIONS	 	 
 *
 *       This option makes returning from a subgoal even less expensive
 *       than THIN_JUSTIFICATIONS alone.  The reason is that only a single
 *	 instantiation is created (as opposed to one for each level in the
 *	 subgoal stack).  As with THIN_JUSTIFICATIONS, all results are
 *	 forced to become O-supported.
 * 
 *       \see THIN_JUSTIFICATIONS
 *	 \see WARN_IF_RESULT_IS_I_SUPPORTED
 *
 *  \par Note:
 *
 *      When using \c THIN_JUSTIFICATIONS or \c SINGLE_THIN_JUSTIFICATION,
 *      instantiations are built to support the results of subgoals,
 *      but there is no p node in the rete associated with these 
 *      instantiations.  This means that Soar's native mechanism for 
 *      garbage collecting won't work on these things, and they will 
 *      stick around forever (which they shouldn't do).  Although this
 *      has no effect on the system's behavior, it is a memory leak 
 *      that we would like to avoid.  Therefore, a work around has been
 *      created.  The basis for this change is the fact that O-supported
 *      preferences stick around regardless of whether or not their
 *      associated instantiation is there.  This means that instantiations
 *      created for subgoal results can be removed as soon as their 
 *      preferences have been asserted so long as they have only
 *      O-supported results.  So, after these preferences are asserted,
 *      we check each instaniation to see if it has all O-supported
 *      results.  If this is the case, and if the production pointer of
 *      this instantiation is NULL, we know we are looking at a \b Thin 
 *      \b Justification (and that \c MAKE_PRODUCTION_FOR_THIN_JUSTS is
 *      not defined) and we can remove the inst. If
 *      \c MAKE_PRODUCTION_FOR_THIN_JUSTS
 *      is defined, an additional boolean value in the instantiation
 *      structure allows us to determine if this is a \c THIN JUSTIFICATION
 *      or not.  Finally, if 
 *	\c ALLOW_I_SUPPORTED_SUBGOAL_RESULTS_WITH_THIN_JUSTS
 *      is defined, there may exist some \c THIN JUSTIFICATIONS with I supported
 *      results.  In such cases these instantiations cannot be removed, 
 *      and a memory leak will ensue.
 *
 *
 *
 *
 */

/**
 *  \def STD
 * 
 *       This is not a real build option in itself, but an agglomerate
 *	 of other build options.  It is specified only in the file
 *	 soarBuildOptions.h, and defines a \b standard build of Soar
 *	 which includes similar build options to previously released
 *	 Soar kernels.
 *
 *	 \see LITE
 *	 \see HEAVY
 */

/**
 *  \def THIN_JUSTIFICATIONS 
 *      
 *       In a normal version of Soar, chunks/justifications are built in
 *       a recursive manner.  When a subgoal returns results to the top
 *       level, for example, a chunk/justification is built for every
 *       intermediate level in the subgoal stack.  Adding the chunk
 *       or justification to the rete is a potential time sink, and when
 *       learning is off, there is no need for any of these productions
 *       to be added at all.  This build option circumvents this problem
 *       by refraining from building any production at all.  At the same
 *       time, it forces all subgoal results to beome O-supported, so that
 *       memory leaks (caused by having lingering instantiations with no 
 *       associated production {Note: 1}) will not occur. 
 *
 *       \see WARN_IF_RESULT_IS_I_SUPPORTED
 */

/**
 *
 * \def SOAR_8_ONLY
 *
 *       make soar 8 a compile time option as opposed to a run time option.
 */

/**
 *
 * \def TRACE_CONTEXT_DECISIONS_ONLY
 *
 *       allows only minimal support for tracing, such as the effects which
 *       would generally be provided by issuing a "watch 1" command in the 
 *       TSI.
 */


/**
 * \def USE_STDARGS
 *
 *      This build option should be used when you compile with the 
 *      ANSI stdarg facility.  (This will be most users). 
 */

/**
 *
 *
 * \def WARN_IF_RESULT_IS_I_SUPPORTED
 *
 *      this option prints a warning if the result of a subgoal is 
 *      I-supported.  It is mainly useful to help determine whether
 *      using \c THIN_JUSTIFICATIONS (and possbiy \c 
 *      SINGLE_THIN_JUSTIFICATIONS) will change the agent's behavior.
 *
 */

/**
 * 
 * \def WARN_IF_TIMERS_REPORT_ZERO
 *
 *       this option will print a warning if any of Soar's timers report
 *	 zero time.  In such cases, there is a high likely hood that the
 *	 timing data will misrepresent the truth because of abnormally high
 *	 granularity.  Using this option may slow the system down slightly
 *	 (only slightly it adds approx 10 lines of code to the timing 
 *	 functions), however, two builds which differ only in their use
 *	 of this option should similarly have equally fast (or slow) timer
 *	 responses.  Thus, if consistently no warnings are generated, it
 *	 should be safe to run the version withou this option and have full
 *	 confidence of the results.
 *
 */












