/* definitions.h

This file contains various definitions that
are required by the project KernelPort.

*/

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#ifndef GSYSPARAM_H
#include "gsysparam.h"
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

const int SOAR_OK(0);
const int SOAR_ERROR(1);

const int UNTOUCHED(0);
const int TOUCHED(1);
const int ALLOCATED(2);

const int ON(0);
const int OFF(1);
const int EXCEPT(2);
const int ONLY(3);
const int ALL_LEVELS(4);
const int BOTTOM_UP(5);

typedef char Bool;
typedef union symbol_union Symbol;

typedef enum {
  NO_GLOBAL_CALLBACK,
  GLB_CREATE_AGENT,
  GLB_AGENT_CREATED,
  GLB_DESTROY_AGENT,
  NUMBER_OF_GLOBAL_CALLBACKS

} SOAR_GLOBAL_CALLBACK_TYPE; 

enum soar_apiSlotType {
  NO_SLOT,
  STATE_SLOT,
  OPERATOR_SLOT,
  SUPERSTATE_SLOT,
  SUPEROPERATOR_SLOT,
  SUPERSUPERSTATE_SLOT,
  SUPERSUPEROPERATOR_SLOT
};

/* various important literals that are not defined elsewhere */
#define STD

#define USE_STDARGS

#define SOARRESULT_RESULT_LENGTH 256
#define MAX_SIMULTANEOUS_AGENTS 128

#define ONE_MILLION (1000000)

#define clearSoarResultResult(r) (*((r)->result)) = '\0';
#define setSoarResultResultStdError(r) (strcpy( ((r)->result),  "Error"));
#define init_soarResult(r) { \
 (r).resultLength = SOARRESULT_RESULT_LENGTH; \
 (*((r).result)) = '\0'; \
} 

typedef struct soarResult_struct {

  char result[SOARRESULT_RESULT_LENGTH];
  int resultLength;

} soarResult;

/* This isn't the right place to define this, but I can't
   think of a better one. */
typedef struct wme_filter_struct {
  Symbol *id;
  Symbol *attr;
  Symbol *value;
  Bool   adds;
  Bool   removes;
} wme_filter;

#endif
