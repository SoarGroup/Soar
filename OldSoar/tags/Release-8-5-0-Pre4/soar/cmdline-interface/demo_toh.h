#define MAX_DISKS 4
#include "soarkernel.h"
#include "soar_core_api.h"

typedef struct disk_struct {
  int cOn;
  int cSz;
  struct disk_struct *cAbove;

  char *on;
  char size[10];
  char above[10];
  char wmeID[6];
  long onTT;
  long aboveTT;

} disk;

typedef struct global_info_struct {
  char inputLinkID[6];
  int nDisks;
  disk disks[MAX_DISKS+1];
  disk *upper[3];
  char pegs[3][3];
  char moveId[5];
  int  cMoveDisk;
  int  cMovePeg;
  int cycle;
  bool forceUpdate;

} glb_info;


extern void toh_input_fn( agent *a, soar_callback_data d, soar_call_data c );
extern void toh_output_fn( agent *a, soar_callback_data d, soar_call_data c );
extern void toh_initialize( int ndisks );
extern void toh_wait_cb( agent *a, soar_callback_data d, soar_call_data c );
extern glb_info glbInfo;
