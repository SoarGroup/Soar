// This header contains the sum of prof.h, prof_win32.h and prof_gather.h
// so we can just include this header without changing the include paths
// for the project we are profiling.

#ifndef Prof_INC_PROF_H
#define Prof_INC_PROF_H


// comment out next line to entirely disable profiler
#define Prof_ENABLED


#ifndef INC_PROFILER_LOWLEVEL_H
#define INC_PROFILER_LOWLEVEL_H

#ifdef __cplusplus
  #define Prof_C               "C"
  #define Prof_extern_C        extern "C"
  #define Prof_dummy_declare
#else
  #define Prof_C
  #define Prof_extern_C
  #define Prof_dummy_declare   int Prof_dummy_dec =
#endif

#ifdef WIN32

#ifndef Prof_INC_PROF_WIN32_H
#define Prof_INC_PROF_WIN32_H

typedef __int64 Prof_Int64;

#ifdef __cplusplus
  inline
#elif _MSC_VER >= 1200
  __forceinline
#else
  static
#endif
      void Prof_get_timestamp(Prof_Int64 *result)
      {
         __asm {
            rdtsc;
            mov    ebx, result
            mov    [ebx], eax
            mov    [ebx+4], edx
         }
      }

#endif

#else
  #error "need to define Prof_get_timestamp() and Prof_Int64"
#endif


typedef struct
{
   char * name;
   void * highlevel;
   char   initialized;
   char   visited;
   char   pad0,pad1;
} Prof_Zone;

typedef struct Prof_Zone_Stack
{
   Prof_Int64               t_self_start;

   Prof_Int64               total_self_ticks;
   Prof_Int64               total_hier_ticks;

   unsigned int             total_entry_count;

   struct Prof_Zone_Stack * parent;
   Prof_Zone              * zone;
   int                      recursion_depth;

   void                   * highlevel;      // used arbitrarily by the analysis pass
} Prof_Zone_Stack;


extern Prof_C Prof_Zone_Stack * Prof_stack; // current Zone stack
extern Prof_C Prof_Zone_Stack   Prof_dummy; // parent never matches

extern Prof_C Prof_Zone_Stack * Prof_StackAppend(Prof_Zone *zone);
// return the zone stack created by pushing 'zone' on the current


#ifdef Prof_ENABLED

static Prof_Int64 Prof_time;

#define Prof_Begin_Cache(z)                                     \
      /* declare a static cache of the zone stack */            \
   static Prof_Zone_Stack *Prof_cache = &Prof_dummy

#define Prof_Begin_Raw(z)                                       \
   Prof_Begin_Cache(z);                                         \
   Prof_Begin_Code(z)

#define Prof_Begin_Code(z)                                      \
   Prof_dummy_declare (                                         \
                                                                \
      /* check the cached Zone_Stack and update if needed */    \
    (Prof_cache->parent != Prof_stack                           \
        ? Prof_cache = Prof_StackAppend(&z)                     \
        : 0),                                                   \
                                                                \
    ++Prof_cache->total_entry_count,                            \
    Prof_get_timestamp(&Prof_time),                             \
                                                                \
      /* stop the timer on the parent zone stack */             \
    (Prof_stack->total_self_ticks +=                            \
       Prof_time - Prof_stack->t_self_start),                   \
                                                                \
      /* make cached stack current */                           \
    Prof_stack = Prof_cache,                                    \
                                                                \
      /* start the timer on this stack */                       \
    Prof_stack->t_self_start = Prof_time,                       \
    0)

#define Prof_End_Raw()                          \
                                                \
   (Prof_get_timestamp(&Prof_time),             \
                                                \
      /* stop timer for current zone stack */   \
    Prof_stack->total_self_ticks +=             \
       Prof_time - Prof_stack->t_self_start,    \
                                                \
      /* make parent chain current */           \
    Prof_stack = Prof_stack->parent,            \
                                                \
      /* start timer for parent zone stack */   \
    Prof_stack->t_self_start = Prof_time)


#define Prof_Declare(z)  Prof_Zone Prof_region_##z
#define Prof_Define(z)   Prof_Declare(z) = { #z }
#define Prof_Region(z)   Prof_Begin_Raw(Prof_region_##z);
#define Prof_End         Prof_End_Raw();

#define Prof_Begin(z)    static Prof_Define(z); Prof_Region(z)
#define Prof_Counter(z)  Prof_Begin(z) Prof_End

#ifdef __cplusplus

   #define Prof(x)        static Prof_Define(x); Prof_Scope(x)
 
   #define Prof_Scope(x)   \
      Prof_Begin_Cache(x); \
      Prof_Scope_Var Prof_scope_var(Prof_region_ ## x, Prof_cache)

   struct Prof_Scope_Var {
      inline Prof_Scope_Var(Prof_Zone &zone, Prof_Zone_Stack * &Prof_cache);
      inline ~Prof_Scope_Var();
   };

   inline Prof_Scope_Var::Prof_Scope_Var(Prof_Zone &zone, Prof_Zone_Stack * &Prof_cache) {
      Prof_Begin_Code(zone);
   }

   inline Prof_Scope_Var::~Prof_Scope_Var() {
      Prof_End_Raw();
   }

#endif



#else  // ifdef Prof_ENABLED

#ifdef __cplusplus
#define Prof(x)
#define Prof_Scope(x)
#endif

#define Prof_Define(name)
#define Prof_Begin(z)
#define Prof_End
#define Prof_Region(z)
#define Prof_Counter(z)

#endif

#endif // INC_PROFILER_LOWLEVEL_H

/*
 *  Prof_update
 *
 *  Pass in true (1) to accumulate history info; pass
 *  in false (0) to throw away the current frame's data
 */
typedef enum
{
   Prof_discard   =0,
   Prof_accumulate=1,
} ProfUpdateMode;

extern Prof_C void Prof_update(ProfUpdateMode accumulate);

/*
 *  Prof_report_writefile
 *
 *  This writes a report for the current frame out to a
 *  text file. This is useful since (a) you can't cut and
 *  paste from the graphical version and (b) to give a
 *  demonstration of how to parse the report (which might
 *  make a graphical D3D version a little easier to write).
 */
extern Prof_C void Prof_report_writefile(char *filename);

/*
 *  Prof_draw_gl -- display the current report via OpenGL
 *
 *  You must provide a callable text-printing function.
 *  Put the opengl state into a 2d rendering mode.
 *
 *  Parameters:
 *    <sx,sy>         --  location where top line is drawn
 *    <width, height> --  total size of display (if too small, text will overprint)
 *    line_spacing    --  how much to move sy by after each line; use a
 *                        negative value if y decreases down the screen
 *    precision       --  decimal places of precision for time data, 1..4 (try 2)
 *    print_text      --  function to display a line of text starting at the
 *                        given coordinate; best if 0,1,2...9 are fixed-width
 *    text_width      --  a function that computes the pixel-width of
 *                        a given string before printing. you can fake with a
 *                        simple approximation of width('0')*strlen(str)
 *
 *  to avoid overprinting, you can make print_text truncate long strings
 */
extern Prof_C void Prof_draw_gl(float sx, float sy,
                         float width, float height,
                         float line_spacing,
                         int precision,
                         void (*print_text)(float x, float y, char *str),
                         float (*text_width)(char *str));

/*
 *  Prof_draw_graph_gl -- display a graph of per-frame times
 *
 *  Parameters
 *    <sx, sy>      --  origin of the graph--location of (0,0)
 *    x_spacing     --  screenspace size of each history sample; e.g.
 *                         2.0 pixels
 *    y_spacing     --  screenspace size of one millisecond of time;
 *                         for an app with max of 20ms in any one zone,
 *                         8.0 would produce a 160-pixel tall display,
 *                         assuming screenspace is in pixels
 */
extern Prof_C void Prof_draw_graph_gl(float sx, float sy,
                               float x_spacing, float y_spacing);

typedef enum
{
   Prof_SORT_SELF_TIME,
   Prof_SORT_HIERARCHICAL_TIME,
   Prof_CALL_GRAPH,
   Prof_CALL_GRAPH_DESCENDENTS,
} Prof_Report_Mode;

extern Prof_C void Prof_set_report_mode(Prof_Report_Mode e);
extern Prof_C void Prof_move_cursor(int delta);
extern Prof_C void Prof_select(void);
extern Prof_C void Prof_select_parent(void);
extern Prof_C void Prof_move_frame(int delta);

extern Prof_C void Prof_set_smoothing(int smoothing_mode);
extern Prof_C void Prof_set_frame(int frame);
extern Prof_C void Prof_set_cursor(int line);

/*
 * This doesn't work yet.
 */
typedef enum
{
   Prof_FLATTEN_RECURSION,
   Prof_SPREAD_RECURSION
} Prof_Recursion_Mode;

extern Prof_C void Prof_set_recursion(Prof_Recursion_Mode e);

#endif // Prof_INC_PROF_H

