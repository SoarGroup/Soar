#ifndef Prof_INC_PROF_INTERNAL_H
#define Prof_INC_PROF_INTERNAL_H

// report functions

#define NUM_VALUES 4
#define NUM_TITLE 2
#define NUM_HEADER (NUM_VALUES+1)

typedef struct {
   int indent;
   char *name;
   int number;
   char prefix;
   int value_flag;
   double values[NUM_VALUES];
   double heat;

   // used internally
   void *zone;
} Prof_Report_Record;

typedef struct
{
   char *title[NUM_TITLE];
   char *header[NUM_HEADER];
   int num_record;
   int hilight;
   Prof_Report_Record *record;
} Prof_Report;

extern void         Prof_free_report(Prof_Report *z);
extern Prof_Report *Prof_create_report(void);


// really internal functions

extern void Prof_graph(int num_frames, 
                       void (*callback)(int id, int x0, int x1, float *values, void *data),
                       void *data);

#ifdef Prof_ENABLED
extern void Prof_init_highlevel();

extern double Prof_get_time(void);

// DJP: Attempting to share profiling data across DLLs
// by using a ProfilerDLL to store the data, but it doesn't seem to work.
// Probably because of the whole hash lookup method.
//#define USE_DLL
#ifdef USE_DLL

#define PROFILERDLL_API __declspec(dllimport)
extern PROFILERDLL_API int Prof_num_zones;
extern PROFILERDLL_API Prof_Zone *Prof_zones[512];
PROFILERDLL_API void SetZone(int i, Prof_Zone* pZone);
PROFILERDLL_API Prof_Zone* GetZone(int i) ;

#else
extern int        Prof_num_zones;
extern Prof_Zone *Prof_zones[];
#endif

extern Prof_C Prof_Declare(_global);
#endif


#endif
