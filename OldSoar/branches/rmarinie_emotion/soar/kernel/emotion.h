#ifndef _EMOTION_H_
#define _EMOTION_H_

// this structure is pretty useless
typedef struct appraisal_variable_value {
    float value;
} appraisal_variable_value;

typedef struct appraisal_frame_struct {
    struct appraisal_frame_struct *next, *prev;
    float variables[2];
    bool object;
} appraisal_frame;

typedef struct emotions_struct {

    /* list which stores all of the appraisals */
    appraisal_frame * appraisal_frames;

    /* aggregated values of appraisal variables currently on e-link */
    float aggs[2];

    /* WMEs which store the aggregated values of the appraisal variables */
    wme * agg_wmes[2];

    /* WMEs which store the generated emotions */
    wme * emotion_wme;
    Symbol * emotion_symbol;

    unsigned long latest_timetag;
    unsigned long num_appraisals;

} emotions_struct;

void emotion_init(emotions_struct ** e);

void emotion_update();

void emotion_destructor();

#endif