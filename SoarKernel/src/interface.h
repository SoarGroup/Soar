/* =================================================================
                             interface.h                             

  This portion of the code is normally replaced by the Tcl interface
  and interface support routines, but some maintenance continues, in
  the event someone needs to build a version without Tcl.  However some
  user interface commands are likely to break or give unusual output.

  User Interface Command Routines:

     Each user interface command has a corresponding function
     (user_interface_routine) to handle it.  These commands/functions
     should be installed at system startup time via add_command().  The
     command name string passed to add_command() must be permanently
     available (e.g., a constant in global data memory).

     When a user interface routine is called, the current lexeme is the
     command name.  The function should call the lexer to read its arguments,
     etc.  If successful, the function should return TRUE and should exit
     with the current lexeme being the closing right parenthesis (otherwise
     the dispatching function will print an error message about extra
     arguments being given).  If unsuccessful, the function should
     return FALSE.

  Dispatching commands:

     Dispatch_command() dispatches the appropriate user interface routine
     for the current command (i.e., the command named by the current lexeme).
     It calls set_lexer_allow_ids(TRUE) before dispatching the command,
     so if the command doesn't allow id's, it should call 
     set_lexer_allow_ids(FALSE) immediately.  Dispatch_command() returns 
     TRUE if the command was successful, FALSE if any error occurred.
   
     Repeatedly_read_and_dispatch_commands() keeps calling dispatch_command()
     until end-of-file is reached on the current input file.
     
     Load_file() sets up the lexer to read from a given open file, executes
     all the commands in that file, and then restore the lexer to reading
     the previous file.

  Help Information:

     Add_help() should be called at system startup time to specify to the
     "help" command what help info is available.  It takes a topic name and
     an array of lines of text for the helpscreen.  All these strings
     should be permanently available (e.g., constants in global data memory).
================================================================= */

#ifndef INTERFACE_H
#define INTERFACE_H

#include "lexer.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* These must be declared before the macro getrusage is defined (under hp-unix)
   to avoid a conflict with the getrusage function prototyped in sys/resource.h.
   -ajc (5/28/02)
*/
#if !defined(__SC__) && !defined(THINK_C) && !defined(WIN32) && !defined(MACINTOSH)
#include <sys/time.h>       /* used for timing stuff */
#include <sys/resource.h>   /* used for timing stuff */
#endif /* !__SC__ && !THINK_C && !WIN32 */

#if defined(__hpux) || defined(UNIX)
#include <sys/syscall.h>
#include <unistd.h>

/* I'm not sure why this macro is in here, but it causes problems in 
   Linux, since SYS_GETRUSAGE doesn't seem to be defined anywhere.
   I suspect that it is only needed under hp-unix. For the time being,
   I am not using it, since Linux has a getrusage function and I want to 
   avoid conflicts.
*/
#ifdef __hpux 
#define getrusage(a, b) syscall(SYS_GETRUSAGE, a, b)
#endif

#ifdef USE_MACROS
#define getwd(arg) getcwd(arg, (size_t) 9999)
#else

inline char * getwd(char * arg)
{
#ifdef WIN32
	return _getcwd(arg, (size_t) 9999);
#else
        return getcwd(arg, (size_t) 9999);
#endif
}

#endif /* USE_MACROS */

#endif /* __hpux */

#if defined(WIN32)
#include <direct.h>

#ifdef USE_MACROS
#define getwd(arg) _getcwd(arg, (size_t) 9999)
#else

inline char * getwd(char * arg)
{
	return _getcwd(arg, (size_t) 9999);
}

#endif /* USE_MACROS */

#endif /* WIN32 */

#if defined(MACINTOSH)

#ifdef USE_MACROS
#define getwd(arg) getcwd(arg, (size_t) 9999)
#else

inline char * getwd(char * arg)
{
	return getcwd(arg, (size_t) 9999);
}

#endif /* USE_MACROS */

#endif /* MACINTOSH */

typedef char Bool;
typedef struct agent_struct agent;
typedef struct kernel_struct Kernel;

#ifdef WIN32
typedef struct _iobuf FILE;
#endif

typedef Bool (*user_interface_routine)(agent* thisAgent);
typedef Bool (*user_interface_routine2)(Kernel* thisKernel, agent* thisAgent);
extern void add_command (agent* thisAgent, char *command_name, user_interface_routine2 f);

extern Bool dispatch_command (Kernel* thisKernel, agent* thisAgent);

extern void repeatedly_read_and_dispatch_commands (Kernel* thisKernel, agent* thisAgent);

extern void load_file (Kernel* thisKernel, agent* thisAgent, char *file_name, FILE *already_open_file);

extern void add_help (agent* thisAgent, char *topic, char **lines_of_text);

//extern void init_built_in_commands (agent* thisAgent);

extern void init_multi_agent_built_in_commands(agent* thisAgent);

extern Bool old_parse_go_command (agent* thisAgent);
extern void old_execute_go_selection (agent* thisAgent);

/*  this routine is defined in interface.c, but the Symbol struct
    hasn't been defined yet, so we can't declare it yet.  So kjc
    moved this prototype to interface.c  */
/*extern Symbol *read_identifier_or_context_variable (void); */

//extern void respond_to_load_errors (agent* thisAgent);

/* AGR 568 begin */
typedef struct expansion_node {
  struct lexeme_info lexeme;
  struct expansion_node *next;
} expansion_node;

typedef struct alias_struct {
  char *alias;
  struct expansion_node *expansion;
  struct alias_struct *next;
} alias_struct;

typedef struct dir_stack_struct {
  char *directory;
  struct dir_stack_struct *next;
} dir_stack_struct;
/* AGR 568 end */

#ifdef ATTENTION_LAPSE
//#ifndef USE_TCL
long init_lapse_duration(struct timeval *tv);
//#endif
#endif

/* AGR 568  This bug fix concerned an alias command.  But I've expanded
   it a little to also include the pushd and popd commands, which are
   all being implemented for the release of 6.2.  11-May-94 */

#ifdef __cplusplus
}
#endif

#endif
