#ifndef _WINX_WCHARX_H_
#define	_WINX_WCHARX_H_

#ifdef __GW32__

#include <features.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __mbstate_t_defined
# define __mbstate_t_defined  1
typedef mbstate_t __mbstate_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* _WINX_WCHARX_H_ */
