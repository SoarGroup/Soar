#ifndef _WINX_UNISTDX_H_
#define	_WINX_UNISTDX_H_
#ifdef __GW32__

#define _UNISTD_H

#include <features.h>

/* These may be used to determine what facilities are present at compile time.
   Their values can be obtained at run time from `sysconf'.  */

/* POSIX Standard approved as ISO/IEC 9945-1 as of August, 1988 and
   extended by POSIX-1b (aka POSIX-4) and POSIX-1c (aka POSIX threads).  */
#define   _POSIX_VERSION 199506L

/* These are not #ifdef __USE_POSIX2 because they are
   in the theoretically application-owned namespace.  */

/* POSIX Standard approved as ISO/IEC 9945-2 as of December, 1993.  */
#define   _POSIX2_C_VERSION   199209L

/* The utilities on GNU systems also correspond to this version.  */
#define _POSIX2_VERSION  199209L

/* If defined, the implementation supports the
   C Language Bindings Option.  */
#define   _POSIX2_C_BIND 1

/* If defined, the implementation supports the
   C Language Development Utilities Option.  */
#define   _POSIX2_C_DEV  1

/* If defined, the implementation supports the
   Software Development Utilities Option.  */
#define   _POSIX2_SW_DEV 1

/* If defined, the implementation supports the
   creation of locales with the localedef utility.  */
#define _POSIX2_LOCALEDEF       1

/* X/Open version number to which the library conforms.  It is selectable.  */
#ifdef __USE_UNIX98
# define _XOPEN_VERSION  500
#else
# define _XOPEN_VERSION  4
#endif

/* Commands and utilities from XPG4 are available.  */
#define _XOPEN_XCU_VERSION    4

/* We are compatible with the old published standards as well.  */
#define _XOPEN_XPG2 1
#define _XOPEN_XPG3 1
#define _XOPEN_XPG4 1

/* The X/Open Unix extensions are available.  */
#define _XOPEN_UNIX 1

/* Encryption is present.  */
#define   _XOPEN_CRYPT   1

/* The enhanced internationalization capabilities according to XPG4.2
   are present.  */
#define   _XOPEN_ENH_I18N     1

/* The legacy interfaces are also available.  */
#define _XOPEN_LEGACY    1



//#undef ALWAYS_LARGE_FILE_SUPPORT
#ifdef ALWAYS_LARGE_FILE_SUPPORT
/* always use large-file support */
# ifndef _LARGEFILE_SOURCE
#  define _LARGEFILE_SOURCE 1
# endif
# ifndef _LARGEFILE64_SOURCE
#  define _LARGEFILE64_SOURCE 1
# endif
# ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
#  define _INTEGRAL_MAX_BITS 64
# endif
#endif/* ALWAYS_LARGE_FILE_SUPPORT */

#include <sys/types.h>
#include <process.h>
#include <features.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __REDIRECT
extern int __REDIRECT (__close, (int __fd) __THROW,
                 _close);
#else
# define __close _close
#endif

extern __inline__ ssize_t __read (int fd, void *buf, size_t nbytes) __THROW
{
  return _read (fd, buf, nbytes);
}

/* This variable is set nonzero at startup if the process's effective
   IDs differ from its real IDs, or it is otherwise indicated that
   extra security should be used.  When this is set the dynamic linker
   and some functions contained in the C library ignore various
   environment variables that normally affect them.  */
extern int __libc_enable_secure;
#ifdef IS_IN_rtld
/* XXX The #ifdef should go.  */
extern int __libc_enable_secure_internal attribute_hidden;
#endif


/* Get the real user ID of the calling process.  */
extern __uid_t getuid (void) __THROW;

/* Get the effective user ID of the calling process.  */
extern __uid_t geteuid (void) __THROW;

/* Get the real group ID of the calling process.  */
extern __gid_t getgid (void) __THROW;

/* Get the effective group ID of the calling process.  */
extern __gid_t getegid (void) __THROW;

/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
extern int getgroups (int __size, __gid_t __list[]) __THROW;

#ifdef    __USE_GNU
/* Return nonzero iff the calling process is in group GID.  */
extern int group_member (__gid_t __gid) __THROW;
#endif

/* Set the user ID of the calling process to UID.
   If the calling process is the super-user, set the real
   and effective user IDs, and the saved set-user-ID to UID;
   if not, the effective user ID is set to UID.  */
extern int setuid (__uid_t __uid) __THROW;

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Set the real user ID of the calling process to RUID,
   and the effective user ID of the calling process to EUID.  */
extern int setreuid (__uid_t __ruid, __uid_t __euid) __THROW;
#endif

#if defined __USE_BSD || defined __USE_XOPEN2K
/* Set the effective user ID of the calling process to UID.  */
extern int seteuid (__uid_t __uid) __THROW;
#endif /* Use BSD.  */

/* Set the group ID of the calling process to GID.
   If the calling process is the super-user, set the real
   and effective group IDs, and the saved set-group-ID to GID;
   if not, the effective group ID is set to GID.  */
extern int setgid (__gid_t __gid) __THROW;

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Set the real group ID of the calling process to RGID,
   and the effective group ID of the calling process to EGID.  */
extern int setregid (__gid_t __rgid, __gid_t __egid) __THROW;
#endif

#if defined __USE_BSD || defined __USE_XOPEN2K
/* Set the effective group ID of the calling process to GID.  */
extern int setegid (__gid_t __gid) __THROW;
#endif /* Use BSD.  */

/* Get the real user ID of the calling process.  */
extern __uid_t getuid (void) __THROW;

/* Get the effective user ID of the calling process.  */
extern __uid_t geteuid (void) __THROW;

/* Get the real group ID of the calling process.  */
extern __gid_t getgid (void) __THROW;

/* Get the effective group ID of the calling process.  */
extern __gid_t getegid (void) __THROW;

/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
extern int getgroups (int __size, __gid_t __list[]) __THROW;

#ifdef    __USE_GNU
/* Return nonzero iff the calling process is in group GID.  */
extern int group_member (__gid_t __gid) __THROW;
#endif

/* Set the user ID of the calling process to UID.
   If the calling process is the super-user, set the real
   and effective user IDs, and the saved set-user-ID to UID;
   if not, the effective user ID is set to UID.  */
extern int setuid (__uid_t __uid) __THROW;

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Set the real user ID of the calling process to RUID,
   and the effective user ID of the calling process to EUID.  */
extern int setreuid (__uid_t __ruid, __uid_t __euid) __THROW;
#endif

#if defined __USE_BSD || defined __USE_XOPEN2K
/* Set the effective user ID of the calling process to UID.  */
extern int seteuid (__uid_t __uid) __THROW;
#endif /* Use BSD.  */

/* Set the group ID of the calling process to GID.
   If the calling process is the super-user, set the real
   and effective group IDs, and the saved set-group-ID to GID;
   if not, the effective group ID is set to GID.  */
extern int setgid (__gid_t __gid) __THROW;

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Set the real group ID of the calling process to RGID,
   and the effective group ID of the calling process to EGID.  */
extern int setregid (__gid_t __rgid, __gid_t __egid) __THROW;
#endif

#if defined __USE_BSD || defined __USE_XOPEN2K
/* Set the effective group ID of the calling process to GID.  */
extern int setegid (__gid_t __gid) __THROW;
#endif /* Use BSD.  */

#define __getpid getpid


/* Return the number of bytes in a page.  This is the system's page size,
   which is not necessarily the same as the hardware page size.  */
extern int getpagesize (void)  __THROW __attribute__ ((__const__));


/* Get the `_PC_*' symbols for the NAME argument to `pathconf' and `fpathconf';
   the `_SC_*' symbols for the NAME argument to `sysconf';
   and the `_CS_*' symbols for the NAME argument to `confstr'.  */
#include <bits/confname.h>

/* Get file-specific configuration information about PATH.  */
extern long int pathconf (__const char *__path, int __name) __THROW;

/* Get file-specific configuration about descriptor FD.  */
extern long int fpathconf (int __fd, int __name) __THROW;

/* Get the value of the system variable NAME.  */
extern long int sysconf (int __name) __THROW __attribute__ ((__const__));

#ifdef __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* _WINX_UNISTDX_H_ */
