#ifndef _SOAR_BUILD_OPTIONS__          /* excludeFromBuildInfo */
#define _SOAR_BUILD_OPTIONS__


/* ExcludeFromBuildInfo __NeXT__ */
/* ExcludeFromBuildInfo __linux__ */
/* ExcludeFromBuildInfo __ultrix__ */
/* ExcludeFromBuildInfo __hpux__ */

/* 
 * See the file README.BuildOptions
 */


#define MAX_SIMULTANEOUS_AGENTS 128
#define USE_STDARGS
#define USE_CAPTURE_REPLAY

#define WARN_IF_TIMERS_REPORT_ZERO


/* Define the Build Here... */
#define STD


















/***************************************************************************
 *
 *         Build Descriptions
 *
 ***************************************************************************/

#ifdef HEAVY

  #define DETAILED_TIMING_STATS

#endif /* HEAVY SOAR */




#ifdef STD
#endif /* STD_SOAR */



#ifdef LITE

  #define NO_TOP_JUST
  #define THIN_JUSTIFICATIONS
  #define SINGLE_THIN_JUSTIFICATION
  #define OPTIMIZE_TOP_LEVEL_RESULTS
  #define FEW_CALLBACKS

#endif /* LITE */






#endif


