#ifndef _WINX_SYS_PARAMX_H_
# define _WINX_SYS_PARAMX_H_

#define MAXPATHLEN	PATH_MAX
#define MAXSYMLINKS  255
#define MAXNAMLEN	(PATH_MAX)
#define PATHSIZE	PATH_MAX
#define NOFILE		64
#define HZ		1000
#define MAXHOSTNAMELEN	64
#define BIG_ENDIAN	4321
#define LITTLE_ENDIAN	1234
#define BYTE_ORDER	LITTLE_ENDIAN
#ifndef NOMINMAX
# ifndef MAX
#  define MAX(a,b) ((a)>(b)?(a):(b))
# endif
# ifndef MIN
#  define MIN(a,b) ((a)<(b)?(a):(b))
# endif
#endif
#ifndef MAX_ARGS
#define MAX_ARGS	4096
#endif
#ifndef NCARGS
#define NCARGS		MAX_ARGS
#endif

#endif /* _WINX_SYS_PARAMX_H_ */
