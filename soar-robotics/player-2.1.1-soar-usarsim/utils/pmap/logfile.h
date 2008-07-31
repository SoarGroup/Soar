#ifndef LOGFILE_H
#define LOGFILE_H

#include <stdio.h>


/// Logfile reader
typedef struct 
{
  /// File pointer
  FILE *file;

  /// Flag set if we have reached the end-of-file
  int eof;
  
  /// Current line number
  int linenum;

  /// Line buffer
  int line_size;
  char *line;

  /// Token buffer
  int token_count;
  char *tokens[4096];

  /// Current interface
  const char *interface;

  /// Current index
  int index;

  /// Data timestamp
  double dtime;
  
  /// Position data
  double position_pose[3];

  /// GPS data (UTM position)
  double gps_pos[2];
  
  /// Laser data
  int laser_range_count;
  double laser_angle_min;
  double laser_angle_max;
  double laser_angle_step;
  double laser_range_max;
  double laser_ranges[2048];
  
} logfile_t;



/// @brief Create a logfile reader
logfile_t *logfile_alloc(const char *filename);

/// @brief Read a line from the log file
int logfile_read(logfile_t *self);

/// @brief Close Logfile and Delete logfile reader
void logfile_free(logfile_t *self);

#endif
