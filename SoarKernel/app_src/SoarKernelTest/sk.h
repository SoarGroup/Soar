/********************************************************************
	name:		sk.h
	created:	2001/12/15
	created:	15:12:2001   22:38
	author:		Jens Wessling
	
	purpose:	
*********************************************************************/

#ifndef SK_HEADER
#define SK_HEADER

#ifndef GSYSPARAM_H
#include "gsysparam.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef tolower
/* I can't believe Sun's ctype.h doesn't have this. */
//extern int tolower(int);
#endif


#define ABSTRACT_REPLAY 1

extern char * soar_version_string;
extern char * soar_news_string;

/* REW: begin 05.05.97 */
#define OPERAND2_MODE_NAME "Operand2/Waterfall"
/* REW: end   05.05.97 */

/* --------------------------------------------------------- */
/* Line width of terminal (used for neatly formatted output) */
/* --------------------------------------------------------- */

#define COLUMNS_PER_LINE 80

/* ------------------------------ */
/* Global type declarations, etc. */
/* ------------------------------ */

typedef unsigned char byte;

/* Some compilers define these. */
#ifndef TRUE                /* excludeFromBuildInfo */
#define TRUE (1)
#endif
#ifndef FALSE               /* excludeFromBuildInfo */
#define FALSE (0)
#endif

#define NIL (0)

#define EOF_AS_CHAR ((char)EOF)

/* ----------------- */
/* Goal Stack Levels */
/* ----------------- */

typedef signed short goal_stack_level;
#define TOP_GOAL_LEVEL 1
#define ATTRIBUTE_IMPASSE_LEVEL 32767
#define LOWEST_POSSIBLE_GOAL_LEVEL 32767

#ifdef __cplusplus
}
#endif

#endif
