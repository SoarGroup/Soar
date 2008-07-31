
/**************************************************************************
 * Desc: Sensor/action models for odometry.
 * Author: Andrew Howard
 * Date: 15 Dec 2002
 * CVS: $Id: odometry.h 1713 2003-08-23 04:03:43Z inspectorg $
 *************************************************************************/

#ifndef ODOMETRY_H
#define ODOMETRY_H

#include "../pf/pf.h"
#include "../pf/pf_pdf.h"

#ifdef __cplusplus
extern "C" {
#endif

  
// Model information
typedef struct
{
  // PDF used for initialization
  pf_pdf_gaussian_t *init_pdf;

  // PDF used to generate action samples
  pf_pdf_gaussian_t *action_pdf;

} odometry_t;


// Create an sensor model
odometry_t *odometry_alloc();

// Free an sensor model
void odometry_free(odometry_t *sensor);

// Prepare to initialize the distribution
void odometry_init_init(odometry_t *self, pf_vector_t mean, pf_matrix_t cov);

// Finish initializing the distribution
void odometry_init_term(odometry_t *self);

// Initialize the distribution
pf_vector_t odometry_init_model(odometry_t *self);

// Prepare to update the distribution using the action model.
void odometry_action_init(odometry_t *self, pf_vector_t old_pose, pf_vector_t new_pose);

// Finish updating the distrubiotn using the action model
void odometry_action_term(odometry_t *self);

// The action model function
pf_vector_t odometry_action_model(odometry_t *self, pf_vector_t pose);


#ifdef __cplusplus
}
#endif

#endif

