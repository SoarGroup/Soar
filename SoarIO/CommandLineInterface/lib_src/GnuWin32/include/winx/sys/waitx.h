#ifndef __WINX_SYS_WAITX_H__
#define __WINX_SYS_WAITX_H__

#define WEXITSTATUS(stat_val) (  (stat_val) & 0x000ff)
#define WIFEXITED(stat_val)   (!((stat_val) & 0x3ff00))
#define WIFSIGNALED(stat_val) ( ((stat_val) & 0x3ff00))
#define WIFSTOPPED(stat_val)  0
#define WNOHANG               1
#define WSTOPSIG(stat_val)    0
#define WTERMSIG(stat_val)    ( ((stat_val) >> 8 ) & 0x3ff)
#define WUNTRACED        0

#endif /* __WINX_SYS_WAITX_H__ */
