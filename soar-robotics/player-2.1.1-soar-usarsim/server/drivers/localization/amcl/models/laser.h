
/**************************************************************************
 * Desc: Sensor models for the laser sensor.
 * Author: Andrew Howard
 * Date: 15 Dec 2002
 * CVS: $Id: laser.h 1713 2003-08-23 04:03:43Z inspectorg $
 *************************************************************************/

#ifndef LASER_H
#define LASER_H

#include "../pf/pf.h"
#include "../map/map.h"

#ifdef __cplusplus
extern "C" {
#endif

  
// Info for a single range measurement
typedef struct
{
  double range, bearing;
  
} laser_range_t;


// Model information
typedef struct
{
  // Pointer to the map
  map_t *map;

  // Laser pose relative to robot
  pf_vector_t laser_pose;

  // Covariance in the range reading
  double range_cov;

  // Probability of spurious range readings
  double range_bad;

  // Pre-computed laser sensor model
  int lut_size;
  double lut_res;
  double *lut_probs;
  
  // Laser (range, bearing) values
  int range_count;
  laser_range_t *ranges;

} laser_t;


// Create an sensor model
laser_t *laser_alloc(map_t *map);

// Free an sensor model
void laser_free(laser_t *sensor);

// Clear all existing range readings
void laser_clear_ranges(laser_t *sensor);

// Set the laser range readings that will be used.
void laser_add_range(laser_t *sensor, double range, double bearing);

// The sensor model function
double laser_sensor_model(laser_t *sensor, pf_vector_t pose);


#ifdef __cplusplus
}
#endif

#endif

