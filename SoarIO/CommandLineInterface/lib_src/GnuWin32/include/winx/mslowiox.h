/* adapted from perl/win32/win32.h & win32.c, which is GPL */

#ifndef __WINX_MSLOWIOX_H__
#define __WINX_MSLOWIOX_H__
#ifdef __GW32__

#include <_mingw.h>

/* Control structure for lowio file handles */
typedef struct {
        intptr_t osfhnd;    /* underlying OS file HANDLE */
        char osfile;        /* attributes of file (e.g., open in text mode?) */
        char pipech;        /* one char buffer for handles opened on pipes */
        int lockinitflag;
        CRITICAL_SECTION lock;
    }   ioinfo;
/* Definition of IOINFO_L2E, the log base 2 of the number of elements in each
 * array of ioinfo structs. */
#define IOINFO_L2E          5
/* Definition of IOINFO_ARRAY_ELTS, the number of elements in ioinfo array */
#define IOINFO_ARRAY_ELTS   (1 << IOINFO_L2E)
/* Definition of IOINFO_ARRAYS, maximum number of supported ioinfo arrays. */
#define IOINFO_ARRAYS       64
#define MAX_DESCRIPTOR_COUNT        (IOINFO_ARRAYS * IOINFO_ARRAY_ELTS)
/* Access macros for getting at an ioinfo struct and its fields from a file handle */
#define _pioinfo(i) ( __pioinfo[(i) >> IOINFO_L2E] + ((i) & (IOINFO_ARRAY_ELTS - 1)) )
#define _osfhnd(i)  ( _pioinfo(i)->osfhnd )
#define _osfile(i)  ( _pioinfo(i)->osfile )
#define _pipech(i)  ( _pioinfo(i)->pipech )

/* Special, static ioinfo structure used only for more graceful handling
 * of a C file handle value of -1 (results from common errors at the stdio
 * level). */
__MINGW_IMPORT ioinfo __badioinfo;
/* Array of arrays of control structures for lowio files. */
__MINGW_IMPORT ioinfo * __pioinfo[];

/* __osfile flag values for DOS file handles */
#define FOPEN           0x01    /* file handle open */
#define FEOFLAG         0x02    /* end of file has been encountered */
#define FCRLF           0x04    /* CR-LF across read buffer (in text mode) */
#define FPIPE           0x08    /* file handle refers to a pipe */
#define FNOINHERIT      0x10    /* file handle opened _O_NOINHERIT */
#define FAPPEND         0x20    /* file handle opened O_APPEND */
#define FDEV            0x40    /* file handle refers to device */
#define FTEXT           0x80    /* file handle is in text mode */

#define LF          10   /* line feed */
#define CR          13   /* carriage return */
#define CTRLZ       26      /* ctrl-z means eof for text */

#endif /* __GW32__ */

#endif /* __WINX_MSLOWIOX_H__ */
