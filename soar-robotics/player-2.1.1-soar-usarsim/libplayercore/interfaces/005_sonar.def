description {
 * @brief Array of ultrasonic rangers

The @p sonar interface provides access to a collection of fixed range
sensors, such as a sonar array.
}

/** Request/reply subtype: get geometry */
message { REQ, GET_GEOM, 1, player_sonar_geom_t };
/** Request/reply subtype: power */
message { REQ, POWER, 2, player_sonar_power_config_t };

/** Data subtype: ranges */
message { DATA, RANGES, 1, player_sonar_data_t };
/** Data subtype: geometry */
message { DATA, GEOM, 2, player_sonar_geom_t };

/** @brief Data: ranges (@ref PLAYER_SONAR_DATA_RANGES)

The @p sonar interface returns up to @ref PLAYER_SONAR_MAX_SAMPLES range
readings from a robot's sonars. */
typedef struct player_sonar_data
{
  /** The number of valid range readings. */
  uint32_t ranges_count;
  /** The range readings [m] */
  float *ranges;
} player_sonar_data_t;

/** @brief Data AND Request/reply: geometry.

To query the geometry of the sonar transducers, send a null
@ref PLAYER_SONAR_REQ_GET_GEOM request.  Depending on the underlying
driver, this message can also be sent as data with the subtype
@ref PLAYER_SONAR_DATA_GEOM. */
typedef struct player_sonar_geom
{
  /** The number of valid poses. */
  uint32_t poses_count;
  /** Pose of each sonar, in robot cs */
  player_pose3d_t *poses;
} player_sonar_geom_t;

/** @brief Request/reply: Sonar power.

On some robots, the sonars can be turned on and off from software.
To do so, send a @ref PLAYER_SONAR_REQ_POWER request.  Null response. */
typedef struct player_sonar_power_config
{
  /** Turn power off TRUE or FALSE */
  uint8_t state;
} player_sonar_power_config_t;

/** @} */
