#ifndef _EMOTION_H_
#define _EMOTION_H_

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

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
    
    float sum_negative_object_frame_intensity;
    float average_negative_object_frame_intensity;
    float max_negative_object_frame_intensity;

    float sum_negative_no_object_frame_intensity;
    float average_negative_no_object_frame_intensity;
    float max_negative_no_object_frame_intensity;

    float sum_positive_object_frame_intensity;
    float average_positive_object_frame_intensity;
    float max_positive_object_frame_intensity;

    float sum_positive_no_object_frame_intensity;
    float average_positive_no_object_frame_intensity;
    float max_positive_no_object_frame_intensity;

    float sum_object_frame_intensity;
    float average_object_frame_intensity;
    float max_object_frame_intensity;

    float sum_no_object_frame_intensity;
    float average_no_object_frame_intensity;
    float max_no_object_frame_intensity;


    wme * sum_negative_object_frame_intensity_wme;
    wme * average_negative_object_frame_intensity_wme;
    wme * max_negative_object_frame_intensity_wme;

    wme * sum_negative_no_object_frame_intensity_wme;
    wme * average_negative_no_object_frame_intensity_wme;
    wme * max_negative_no_object_frame_intensity_wme;

    wme * sum_positive_object_frame_intensity_wme;
    wme * average_positive_object_frame_intensity_wme;
    wme * max_positive_object_frame_intensity_wme;

    wme * sum_positive_no_object_frame_intensity_wme;
    wme * average_positive_no_object_frame_intensity_wme;
    wme * max_positive_no_object_frame_intensity_wme;

    wme * sum_object_frame_intensity_wme;
    wme * average_object_frame_intensity_wme;
    wme * max_object_frame_intensity_wme;

    wme * sum_no_object_frame_intensity_wme;
    wme * average_no_object_frame_intensity_wme;
    wme * max_no_object_frame_intensity_wme;

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