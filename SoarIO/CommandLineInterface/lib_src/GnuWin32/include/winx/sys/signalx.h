#ifndef __WINX_SYS_SIGNALX_H__
#define __WINX_SYS_SIGNALX_H__

#include <features.h>
#include <sys/types.h>
#include <bits/sigset.h>

/* Type of a signal handler.  */
typedef void (*__sighandler_t) (int);

#ifdef __USE_BSD
typedef __sighandler_t sig_t;
#endif

#  define SIGHUP      -1
#  define SIGQUIT     -3
#  define SIGTRAP     -5
#  define SIGIOT      -6
#  define SIGEMT      -7
#  define SIGKILL     -9
#  define SIGBUS      -10
#  define SIGSYS      -12
#  define SIGPIPE     -13
#  define SIGALRM     -14
#  define SIGURG      -16
#  define SIGSTOP     -17
#  define SIGTSTP     -18
#  define SIGCONT     -19
#  define SIGCHLD     -20
#  define SIGTTIN     -21
#  define SIGTTOU     -22
#  define SIGIO       -23
#  define SIGXCPU     -24
#  define SIGXFSZ     -25
#  define SIGVTALRM   -26
#  define SIGPROF     -27
#  define SIGWINCH    -28
#  define SIGLOST     -29
#  define SIGUSR1     -30
#  define SIGUSR2     -32

#define SIG_SETMASK 0	/* set mask with sigprocmask() */
#define SIG_BLOCK 1	/* set of signals to block */
#define SIG_UNBLOCK 2	/* set of signals to, well, unblock */

/* Type of a signal handler.  */
typedef void (*__sighandler_t) (int);

struct sigaction
  {
    /* Signal handler.  */
    __sighandler_t sa_handler;
    /* Additional set of signals to be blocked.  */
    __sigset_t sa_mask;
    /* Special flags.  */
    int sa_flags;
  };


/* Structure describing a signal stack (obsolete).  */
struct sigstack
  {
    __ptr_t ss_sp;       /* Signal stack pointer.  */
    int ss_onstack;      /* Nonzero if executing on this stack.  */
  };


/* Alternate, preferred interface.  */
typedef struct sigaltstack
  {
    __ptr_t ss_sp;
    size_t ss_size;
    int ss_flags;
  } stack_t;

/* POSIX sigsetjmp/siglongjmp macros */
#define sigjmp_buf jmp_buf

#define _SAVEMASK	_JBLEN
#define _SIGMASK	(_JBLEN+1)

/*
#define sigsetjmp(env, savemask) ((env)[_SAVEMASK] = savemask,\
               sigprocmask (SIG_SETMASK, 0, (sigset_t *) ((env) + _SIGMASK)),\
               setjmp (env))

#define siglongjmp(env, val) ((((env)[_SAVEMASK])?\
               sigprocmask (SIG_SETMASK, (sigset_t *) ((env) + _SIGMASK), 0):0),\
               longjmp (env, val))


#define sigsetjmp(env, savemask) (1)
#define siglongjmp(env, val) (1)
*/
#ifndef sigemptyset
# define sigemptyset(s) (*(s) = 0)
#endif
#ifndef sigmask
# define sigmask(sig) (1 << ((sig) - 1))
#endif
/* # define sigaddset(s, sig) (*(s) |= sigmask (sig)) */

#endif /* __WINX_SYS_SIGNALX_H__ */
