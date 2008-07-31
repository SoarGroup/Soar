
/**************************************************************************
 * Desc: Sensor models for the sonar sensor.
 * Author: Andrew Howard
 * Date: 15 Dec 2002
 * CVS: $Id: sonar.h 1359 2003-05-08 17:22:19Z inspectorg $
 *************************************************************************/

#ifndef SONAR_H
#define SONAR_H

#include "../pf/pf.h"
#include "../map/map.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SONAR_MAX_RANGES 32

// Model information
typedef struct
{
  // Pointer to the map
  map_t *map;

  // Pose of sonars relative to robot
  int pose_count;
  pf_vector_t poses[SONAR_MAX_RANGES];

  // Covariance in the range reading
  double range_cov;

  // Probability of spurious range readings
  double range_bad;

  // Maximum valid range value
  double range_max;

  // Pre-computed sonar sensor model
  int lut_size;
  double lut_res;
  double *lut_probs;
  
  // Sonar range values
  int range_count;
  double ranges[SONAR_MAX_RANGES];

} sonar_t;


// Create an sensor model
sonar_t *sonar_alloc(map_t *map, int pose_count, pf_vector_t *poses);

// Free an sensor model
void sonar_free(sonar_t *sensor);

// Clear all existing range readings
void sonar_clear_ranges(sonar_t *sensor);

// Set the sonar range readings that will be used.
void sonar_add_range(sonar_t *sensor, double range);

// The sensor model function
double sonar_sensor_model(sonar_t *sensor, pf_vector_t pose);


#ifdef __cplusplus
}
#endif

#endif

