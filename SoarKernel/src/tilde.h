/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* tilde.h */

#ifndef TILDE_H
#define TILDE_H

/*#ifdef __cplusplus
extern "C"
{
#endif*/

typedef struct agent_struct agent;

typedef int Function ();
#if !defined (NULL)
#  define NULL 0x0
#endif

/* Changed the order of the following test from the GDB source! */

// 2/24/05: removed static keyword from function declarations below to quell gcc compiler warning

#if defined (TEST)
extern char *xmalloc (), *xrealloc ();
#else
/*static*/ char *xmalloc (agent*, int), *xrealloc (agent*, char *, int);
#endif /* TEST */

/*static*/ int tilde_find_prefix (char *string, int *len);
/*static*/ int tilde_find_suffix (char *string);
char *tilde_expand (agent* thisAgent, char *filename); 
char *tilde_expand_word (agent* thisAgent, char *filename); 

/*#ifdef __cplusplus
}
#endif*/

#endif

