/*
* This file holds data structures etc. necessary for the UsLaser3d, but
* not yet included in the standard player header file.
*/

#ifndef TEMP_LASER3D_STRUCTURES_H
#define TEMP_LASER3D_STRUCTURES_H

#include "libplayercore/player.h"


#define PLAYER_POINTCLOUD3D_CODE   61
#define PLAYER_POINTCLOUD3D_STRING    "pointcloud3d"

// /////////////////////////////////////////////////////////////////////////////
/** @ingroup interfaces
 * @defgroup interface_pointcloud3d pointcloud3d
 * @brief A 3-D point cloud

The @p pointcloud3d interface is used to transmit 3-D point cloud data
(e.g., from a 3-D range sensor).
*/

/** @ingroup interface_pointcloud3d
 * @{ */

/** Maximum number of points that can be included in a data packet */
#define PLAYER_POINTCLOUD3D_MAX_POINTS 8192

/** Data subtype: state */
#define PLAYER_POINTCLOUD3D_DATA_STATE 1

/** @} */

typedef struct player_laser3d_config
{
  float min_angles[2];
  float resolutions[2]; 
} player_laser3d_config_t; 

typedef struct player_laser3d_geom
{
  /** Laser pose, in robot cs (m, m, rad). */
  player_pose2d_t pose;
  /** Laser dimensions (m, m). */
  player_bbox2d_t size;
} player_laser3d_geom_t;

#endif
