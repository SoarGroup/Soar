/**
 *  \file soarapi_datatypes.h
 *
 *  \brief This file contains datatypes used by the Soar API
 *
 *
 */

#ifndef _SOARAPI_DATATYPES_H_   /* excludeFromBuildInfo */
#define _SOARAPI_DATATYPES_H_

#include "soarkernel.h"
#include "sysdep.h"
#include <stdarg.h>

#define SOARRESULT_RESULT_LENGTH 256

/**
 * The soarResult structure contains a string message, containing
 * information about the last high-level api function invocation.
 */
typedef struct soarResult_struct {

    char result[SOARRESULT_RESULT_LENGTH];
    int resultLength;

} soarResult;

#define init_soarResult(r) { \
 (r).resultLength = SOARRESULT_RESULT_LENGTH; \
 (*((r).result)) = '\0'; \
}

#define clearSoarResultResult(r) (*((r)->result)) = '\0';

#define setSoarResultResultStdError(r) {strncpy( ((r)->result),  "Error", SOARRESULT_RESULT_LENGTH); (r)->result[SOARRESULT_RESULT_LENGTH-1]=0;}

#ifdef USE_STDARGS
extern void setSoarResultResult(soarResult * res, const char *format, ...);
#else
extern void setSoarResultResult();
#endif

#ifdef USE_STDARGS
extern void appendSoarResultResult(soarResult * res, const char *format, ...);
#else
extern void appendSoarResultResult();
#endif

#ifdef USE_STDARGS
extern void appendSymbolsToSoarResultResult(soarResult * res, const char *format, ...);
#else
extern void appendSymbolsToSoarResultResult();
#endif

/**
 * \brief   A generic pointer to a soar agent
 *
 *          this pointer should be used outside of the Soar kernel
 *          proper.  It encapsulates the agent's internal data structure
 *          and provides some degree of safety.
 *
 */
typedef void *psoar_agent;

/**
 * \brief   A generic pointer to a soar agent
 *
 *          this pointer should be used outside of the Soar kernel
 *          proper.  It encapsulates the agent's internal data structure
 *          and provides some degree of safety.
 *
 */
typedef void *psoar_wme;

/*
 * NOTE Documenting this struct seems to result in a who code section
 *  being generated... :(
 */

/**
 *    An iteration structure
 *
 *          This structure is used to iterate though the agent list.
 *          All fields should be considered private expect for the
 *          \c more field.
 *
 */
typedef struct soar_apiAgentIter_struct {

    cons *_begin;  /**< Private Field */
    cons *_current;/**< Private Field */
    bool more;     /**< \c TRUE iff there are more agents to iterate through */

} soar_apiAgentIterator;

/**
 * \brief  A soar api function return code
 *
 */
enum soar_apiResult {
    SOAR_OK,
    SOAR_ERROR
};

/*
 * \brief  A production type
 *
 */
enum soar_apiProductionType {
    CHUNKS,
    DEFAULT,
    TASK,
    USER,
    ALL
};

/*
 * \brief  The agent's learning setting
 *
 */
enum soar_apiLearningSetting {
    ON,
    OFF,
    EXCEPT,
    ONLY,
    ALL_LEVELS,
    BOTTOM_UP
};

/*
 * \brief  The slot type
 *
 */
enum soar_apiSlotType {
    NO_SLOT,
    STATE_SLOT,
    OPERATOR_SLOT,
    SUPERSTATE_SLOT,
    SUPEROPERATOR_SLOT,
    SUPERSUPERSTATE_SLOT,
    SUPERSUPEROPERATOR_SLOT
};

/*
 * \brief The interrupt setting
 *
 */
enum soar_apiInterruptSetting {
    INTERRUPT_OFF,
    INTERRUPT_ON,
    INTERRUPT_PRINT
};

/**
 *  The ASK Callback sends the following datastructure
 *  to the function which is registered with the Soar kernel
 */
typedef struct ask_cb_data_st {

    preference **selection;
    preference *candidates;

} soar_apiAskCallbackData;

typedef struct sapiwme_st {
    const char *id;
    const char *attr;
    const char *value;
    long timetag;
} soarapi_wme;

#endif
