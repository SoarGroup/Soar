

#ifndef RTKPRIVATE_H
#define RTKPRIVATE_H

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#if HAVE_STDINT_H
  #include <stdint.h>
#endif
#include <sys/types.h>

// Boolean macros
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Error macros
#define PRINT_ERR(m)         printf("\rrtk error : %s %s\n  "m"\n", \
                                    __FILE__, __FUNCTION__)
#define PRINT_ERR1(m, a)     printf("\rrtk error : %s %s\n  "m"\n", \
                                    __FILE__, __FUNCTION__, a)
#define PRINT_ERR2(m, a, b)  printf("\rrtk error : %s %s\n  "m"\n", \
                                    __FILE__, __FUNCTION__, a, b)

// Warning macros
#define PRINT_WARN(m)         printf("\rrtk warning : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_WARN1(m, a)     printf("\rrtk warning : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_WARN2(m, a, b)  printf("\rrtk warning : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_WARN3(m, a, b, c) printf("\rrtk warning : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c)

// Message macros
#define PRINT_MSG(m) printf("rtk msg : %s :\n  "m"\n", __FILE__)
#define PRINT_MSG1(m, a) printf("rtk msg : %s :\n  "m"\n", __FILE__, a)
#define PRINT_MSG2(m, a, b) printf("rtk msg : %s :\n  "m"\n", __FILE__, a, b)

// DEBUG macros
#ifdef DEBUG
#define PRINT_DEBUG(m)         printf("\rrtk debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_DEBUG1(m, a)     printf("\rrtk debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_DEBUG2(m, a, b)  printf("\rrtk debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_DEBUG3(m, a, b, c) printf("\rrtk debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c)
#else
#define PRINT_DEBUG(m)
#define PRINT_DEBUG1(m, a)
#define PRINT_DEBUG2(m, a, b)
#define PRINT_DEBUG3(m, a, b, c)
#endif

// Figures use these functions to add/remove themself from the canvas.
extern void rtk_canvas_add_fig(rtk_canvas_t *canvas, rtk_fig_t *fig);
extern void rtk_canvas_del_fig(rtk_canvas_t *canvas, rtk_fig_t *fig);

// Re-calculate all the figures in a canvas (private).
extern void rtk_canvas_calc(rtk_canvas_t *canvas);

// Grab an image from the server
extern uint16_t *rtk_canvas_get_image_rgb16(rtk_canvas_t *canvas, int sizex, int sizey);

// Grab an image from the server
extern uint8_t *rtk_canvas_get_image_rgb24(rtk_canvas_t *canvas);

// Private figure functions
extern void rtk_fig_dirty(rtk_fig_t *fig);
extern void rtk_fig_calc(rtk_fig_t *fig);
extern void rtk_fig_render(rtk_fig_t *fig);
extern void rtk_fig_render_xfig(rtk_fig_t *fig);
extern int rtk_fig_hittest(rtk_fig_t *fig, int dx, int dy);
extern void rtk_fig_on_mouse(rtk_fig_t *fig, int event, int mode);


// Append an item to a linked list
#define RTK_LIST_APPEND(head, item) \
item->prev = head;\
item->next = NULL;\
if (head == NULL)\
    head = item;\
else\
{\
    while (item->prev->next)\
       item->prev = item->prev->next;\
    item->prev->next = item;\
}

// Remove an item from a linked list
#define RTK_LIST_REMOVE(head, item) \
if (item->prev)\
    item->prev->next = item->next;\
if (item->next)\
    item->next->prev = item->prev;\
if (item == head)\
    head = item->next;

// Append an item to a linked list
#define RTK_LIST_APPENDX(head, list, item) \
item->list##_##prev = head;\
item->list##_##next = NULL;\
if (head == NULL)\
    head = item;\
else\
{\
    while (item->list##_##prev->list##_##next)\
       item->list##_##prev = item->list##_##prev->list##_##next;\
    item->list##_##prev->list##_##next = item;\
}

// Remove an item from a linked list
#define RTK_LIST_REMOVEX(head, list, item) \
if (item->list##_##prev)\
    item->list##_##prev->list##_##next = item->list##_##next;\
if (item->list##_##next)\
    item->list##_##next->list##_##prev = item->list##_##prev;\
if (item == head)\
    head = item->list##_##next;

#endif
