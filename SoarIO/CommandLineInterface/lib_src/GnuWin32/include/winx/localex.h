#ifndef __WINX_LOCALEX_H__
#define __WINX_LOCALEX_H__
#ifdef __GW32__

#include <features.h>
#define _LOCALE_H
#include <bits/locale.h>

#undef	LC_ALL
#undef LC_COLLATE
#undef LC_CTYPE
#undef	LC_MONETARY
#undef	LC_NUMERIC
#undef	LC_TIME
#undef	LC_MIN
#undef	LC_MAX

#define LC_CTYPE          0
#define LC_NUMERIC        1
#define LC_TIME           2
#define LC_COLLATE        3
#define LC_MONETARY       4
#define LC_MESSAGES       5
#define	LC_ALL		  6
#define LC_PAPER	  7
#define LC_NAME		  8
#define LC_ADDRESS	  9
#define LC_TELEPHONE	  10
#define LC_MEASUREMENT	  11
#define LC_IDENTIFICATION 12
/* This has to be changed whenever a new locale is defined.  */
#define __LC_LAST   13

/* These are the bits that can be set in the CATEGORY_MASK argument to
   `newlocale'.  In the GNU implementation, LC_FOO_MASK has the value
   of (1 << LC_FOO), but this is not a part of the interface that
   callers can assume will be true.  */
# define LC_CTYPE_MASK        (1 << __LC_CTYPE)
# define LC_NUMERIC_MASK (1 << __LC_NUMERIC)
# define LC_TIME_MASK         (1 << __LC_TIME)
# define LC_COLLATE_MASK (1 << __LC_COLLATE)
# define LC_MONETARY_MASK     (1 << __LC_MONETARY)
# define LC_MESSAGES_MASK     (1 << __LC_MESSAGES)
# define LC_PAPER_MASK        (1 << __LC_PAPER)
# define LC_NAME_MASK         (1 << __LC_NAME)
# define LC_ADDRESS_MASK (1 << __LC_ADDRESS)
# define LC_TELEPHONE_MASK    (1 << __LC_TELEPHONE)
# define LC_MEASUREMENT_MASK  (1 << __LC_MEASUREMENT)
# define LC_IDENTIFICATION_MASK    (1 << __LC_IDENTIFICATION)
# define LC_ALL_MASK          (LC_CTYPE_MASK \
                     | LC_NUMERIC_MASK \
                     | LC_TIME_MASK \
                     | LC_COLLATE_MASK \
                     | LC_MONETARY_MASK \
                     | LC_MESSAGES_MASK \
                     | LC_PAPER_MASK \
                     | LC_NAME_MASK \
                     | LC_ADDRESS_MASK \
                     | LC_TELEPHONE_MASK \
                     | LC_MEASUREMENT_MASK \
                     | LC_IDENTIFICATION_MASK \
                     )

#include <xlocale.h>

typedef __locale_t locale_t;

/* Locale object for C locale.  */
extern struct __locale_struct _nl_C_locobj;

#endif /* __GW32__ */

#endif /* __WINX_LOCALEX_H__ */
