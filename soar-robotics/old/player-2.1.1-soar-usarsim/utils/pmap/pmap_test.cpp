/*
  pmap: simple mapping utilities
  Copyright (C) 2004 Andrew Howard  ahoward@usc.edu

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*
 * Desc: Simple PF mapper
 * Author: Andrew Howard
 * Date: 24 Nov 2004
 * CVS: $Id: pmap_test.cpp 4359 2008-02-15 09:14:18Z thjc $
 */

/** @defgroup pmap_test PF mapping utility (pmap_test)

The pmap_test utility demonstrates the basic functionality of the pmap
library and serves a handy mapping utility in its own right.  Given a
Player logfile containing odometry and laser data, pmap_test will
produce an occupancy grid map of the environment.

@image html pmap_test.gif "pmap_test screenshot"

Note that there are two distinct phases to the mapping process.  In
the first or @e coarse phase, data is read from a Player logfile and
processed through the lodo and pmap libraries to produce a rough map.
In the second or @e fine phase, this map is refined using the rmap
library.  The stopping conditions for the second phase are entirely
arbitrary (the quality of the map will improve asymptotically over
time).  This phase can be quite time consuming, so please be patient.


@section pmap_test_usage Usage

Basic usage is as follows:

@verbatim
$ pmap_test [options] <logfilename>
@endverbatim

where the supported options are:

- -g : disable the GUI (run in console mode only).
- --range_max : maximum effective range for the laser (m).
- --position_index : index of odometry device in logfile (e.g., 0 for device @c position:0).
- --laser_index : index of laser device in logfile.
- --num_samples : number of samples in particle filter.
- --resample_interval : number of scans between resampling steps.
- --resample_sigma : width of resampling gaussian.
- --num_cycles : number of optimization cycles in the fine phase.
- --robot_x, --robot_y, --robot_rot : initial robot pose.
- --grid_width, --grid_height : grid dimensions (m).
- --grid_scale : grid scale (m/cell).


@section pmap_test_keyboard Keyboard/mouse controls

The pmap_test utility has a simple GLUT-based GUI for monitoring and
controlling the map generation process.  The following keyboard/mouse
controls are supported:

- Left-mouse drag: pan map in window.
- T : show/hide sample trajectories.
- G : show/hide the current best map.
- +/- : zoom in/out.
- S : single step through the logfile.
- SPACE : pause/unpause logfile playback.
- W : save the current best map.
- Control-C : save the current best map and exit.

Note that hiding the sample trajectories and current best map can
result in significant speed-ups on some GLUT implementations.  The
entire GUI can be disabled using the @c -g option on the command line.


@section pmap_test_data Tips for generating logfiles

The key principle to remember when generating maps is: garbage in,
garbage out.  Here are some tips for generating good data sets:

- The laser must be @b firmly afixed (i.e., bolted) to the robot;
  gaffer tape and velcro will simply not cut it for this application.
  Ideally, the laser will be mounted and aligned such that it
  coincides with the the nominal odometric "center" of the robot.  If
  the laser is offset or oriented, the relative pose of the laser
  @b must be supplied to the program (see @ref pmap_test_usage).

- For SICK LMS200 lasers, the laser must be set to 1 degree, 10 mm,
  10Hz mode.  In this mode, the laser generates 181 range readings per
  scan, and can theoretically detect obstacles out to 80m.  @b "Do
  not" use the default 0.5 degree mode, as this is really two 1 degree
  scans that have been interlaced and offet by 0.5 degrees (this
  produces horrible artifacts and provids no additional data).

- The rotational speed of the robot should be limited to less than 30
  degree/sec; rapid turns are more likely to cause scan mis-alignment.

- When exploring, recover the gross toplogy of the environment first;
  i.e., traverse the major corridors and intersections before
  exploring rooms.  Loops should be closed with a significant overlap
  (e.g., travel at least one-and-a-quarter times around the loop).


@section pmap_test_tips Tips for generating maps

The pmap_test utility is generally used in a iterative fashion to find
the best settings for the map generation process.

- To check the validity of the data set, run pmap_test with the
number of particles set to one:
@verbatim
$ pmap_test --num_samples 1 <logfile>
@endverbatim
The map produced by this test should be locally correct (i.e., no
individually mis-aligned scans), but globaly inconsistent (i.e.,
exhibiting cummulative drift).

- Based on the result of first test, set the size of the desired map
and re-run with a larger sample set; e.g.:
@verbatim
$ pmap_test --num_samples 100 --grid_width 50 --grid_height 25 <logfile>
@endverbatim
This command specifies a map that is 50 by 25 m in size.  The initial
robot pose within the map can also be specified (see @ref
pmap_test_usage).  If the map contains gross topological errors, try
re-running with more samples (but be careful not to exceed the
available physical memory; things will get very, very slow if you do).

- The program can also be run in console mode (no GUI) for faster
mapping and/or batch processing.
@verbatim
$ pmap_test -g --num_samples 500 --grid_width 50 --grid_height 25 <logfile>
@endverbatim

*/
#if HAVE_CONFIG_H
  #include "config.h"
#endif

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef HAVE_LIBGLUT
#include <GL/glut.h>
#endif

#include "logfile.h"
#include "lodo.h"
#include "pmap.h"
#include "rmap.h"
#include "omap.h"


// Command line options
static const char *opt_filename;
static int opt_gui = 1;
static int opt_max_scans = -1;
static double opt_skip = 0.0;
static int opt_position_index = 0;
static int opt_laser_index = 0;
static double opt_scale = 0.025;
static double opt_range_max = -1.0;
static double opt_range_res = 0.10;
static pose2_t opt_robot_pose;
static pose2_t opt_laser_pose;
static int opt_num_samples = 200;
static int opt_resample_interval = -1;
static double opt_resample_sigma = -1;
static double opt_grid_width = 64.0;
static double opt_grid_height = 48.0;
static double opt_grid_scale = 0.10;
static int opt_fine_max = 100;

// configuration data, read from logfile
static int laser_range_count;
static double laser_range_max;
static double laser_angle_min;
static double laser_angle_max;
static double laser_angle_step;


// GUI settings
static int win;
static int gui_pause = 0;
static int viewport_width = 0;
static int viewport_height = 0;
static int show_samples = 1;
static int show_grid = 1;
static vector2_t mouse_offset;
static vector2_t viewport_offset;
static double start_time = 0.0;

// Object handles
static logfile_t *logfile;
static lodo_t *lodo;
static pmap_t *pmap;
static rmap_t *rmap;
static omap_t *omap;

// Mapping phases
enum {phase_coarse, phase_fine, phase_done};
static int phase = phase_coarse;

// Process variables
static pose2_t odom_pose;
static int fine_count = 0;

// Local functions
static void process();
static void save();
static void process_init();
static int process_coarse();
static void save_coarse();
static int process_fine();
static void save_fine();

// Shows help
void show_help ()
{
  fprintf (stdout,
	   "The pmap_test utility demonstrates the basic functionality of\n"
	   "the pmap library and serves a handy mapping utility in its own\n"
	   "right.  Given a Player logfile containing odometry and laser\n"
	   "data, pmap_test will produce an occupancy grid map of the\n"
	   "environment.\n\n"
	   "Basic usage is as follows:\n"
	   "\t$ pmap_test [options] <logfilename>\n\n"
	   "where the supported options are:\n"
"   -g                                 disable the GUI (run in console mode\n"
"                                      only).\n"
"   --range_max                        maximum effective range for the laser\n"
"                                      (m).\n"
"   --position_index                   index of odometry device in logfile.\n"
"   --laser_index                      index of laser device in logfile.\n"
"   --num_samples                      number of samples in particle filter.\n"
"   --resample_interval                number of scans between resampling\n"
"                                      steps.\n"
"   --resample_sigma                   width of resampling gaussian.\n"
"   --num_cycles                       number of optimization cycles in the\n"
"                                      fine phase.\n"
"   --robot_x, --robot_y, --robot_rot  initial robot pose.\n"
"   --grid_width, --grid_height        grid dimensions (m).\n"
"   --grid_scale                       grid scale (m/cell).\n"
"   --laser_x, --laser_rot             laser position offset.\n"
"   --robot_hostname                   the name of the robot to verify in\n"
"                                      the log.\n"
"   --skip                             amount of time to skip between log\n"
"                                      entries.\n"
"   --range_res                        Resolution of the laser (only used in\n"
"                                      lodo, not lodo2 which is currently\n"
"                                      used.\n"
"   --action_model_xx,\n"
"   --action_model_rx,\n"
"   --action_model_rr                  believe factors in the change of the\n"
"                                      robot's pose.\n"
);
  
  exit (EXIT_SUCCESS);
}

// Key callback
void win_key(unsigned char key, int x, int y)
{
#ifdef HAVE_LIBGLUT
  // Show the samples
  if (key == 'T' || key == 't')
  {
    show_samples = !show_samples;
    glutPostRedisplay();
  }

  // Show the grid
  else if (key == 'G' || key == 'g')
  {
    show_grid = !show_grid;
    glutPostRedisplay();
  }

  // Change scale
  else if (key == '=' || key == '+')
  {
    opt_scale /= 2;
    glutPostRedisplay();
  }
  
  // Change scale
  else if (key == '-' || key == '_')
  {
    opt_scale *= 2;
    glutPostRedisplay();
  }

  // Single step
  else if (key == 'S' || key == 's')
  {
    gui_pause = 1;
    fprintf(stderr, "single step\n");
    process();
    glutPostRedisplay();
  }

  // Pause
  else if (key == ' ')
  {
    if (!gui_pause)
      fprintf(stderr, "paused\n");
    else
      fprintf(stderr, "running\n");
    gui_pause = !gui_pause;
  }

  // Save the current map
  else if (key == 'W' || key == 'w')
  {
    save();
  }
#endif  
  return;
}



// Mouse callback
void win_mouse(int button, int state, int x, int y)
{
#ifdef HAVE_LIBGLUT

  if (state == GLUT_DOWN)
  {
    mouse_offset.x = 2 * x * opt_scale;
    mouse_offset.y = 2 * y * opt_scale;      
  }
  else if (state == GLUT_UP)
  {
    viewport_offset.x += 2 * x * opt_scale - mouse_offset.x;
    viewport_offset.y -= 2 * y * opt_scale - mouse_offset.y;
    glutPostRedisplay();
  }
#endif
  return;
}


// Handle window reshape events
void win_reshape(int width, int height)
{
#ifdef HAVE_LIBGLUT

  glViewport(0, 0, width, height);
  viewport_width = width;
  viewport_height = height;
#endif
  return;
}


// Redraw the window
void win_redraw()
{
#ifdef HAVE_LIBGLUT

  double left, right, top, bot;

  glClearColor(0.7, 0.7, 0.7, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  left = -opt_scale * viewport_width;
  right = +opt_scale * viewport_width;
  top = -opt_scale * viewport_height;
  bot = +opt_scale * viewport_height;
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(left, right, top, bot, -1, +10);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(viewport_offset.x, viewport_offset.y, 0.0);

  // Draw the grid map
  if (show_grid && opt_num_samples > 0)
    pmap_draw_map(pmap, opt_scale);

  // Draw the origin marker
  glColor3f(0, 0, 0);
  glBegin(GL_LINE_LOOP);
  glVertex3f(-1, -1, 0);
  glVertex3f(+1, -1, 0);
  glVertex3f(+1, +1, 0);
  glVertex3f(-1, +1, 0);
  glEnd();

  if (phase == phase_coarse)
  {
    // Draw samples
    if (show_samples && opt_num_samples > 0)
      pmap_draw_samples(pmap);

    // Draw the current laser scan
    if (opt_num_samples > 0)
      pmap_draw_scan(pmap, logfile->laser_ranges);
  }
  else if (phase == phase_fine)
  {
    // Draw the refinement map
    rmap_draw_cons(rmap);    
    rmap_draw_map(rmap);
  }
  else if (phase == phase_done)
  {
    // Draw the final map (we assmume it has been generated)
    omap_draw_map(omap, opt_scale);
  }

  glutSwapBuffers();
#endif    
  return;
}


// Idle callback
void win_idle()
{
  if (!gui_pause)
  {
    process();
	#ifdef HAVE_LIBGLUT
    glutPostRedisplay();      
	#endif
  }
  else
    usleep(100000);
  
  return;
}


// Run the GUI
int win_run(int *argc, char **argv)
{
#ifdef HAVE_LIBGLUT

  glutInit(argc, argv);
    
  // Create a window
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

  glutInitWindowSize(static_cast<int> (2 * opt_grid_width / opt_grid_scale), static_cast<int> (2 * opt_grid_height / opt_grid_scale));
  opt_scale = opt_grid_scale / 4;
    
  win = glutCreateWindow("PF Mapping Utility");
  glutReshapeFunc(win_reshape);
  glutDisplayFunc(win_redraw);
  glutKeyboardFunc(win_key);
  glutMouseFunc(win_mouse);
  
  // Idle loop callback
  glutIdleFunc(win_idle);

  glutMainLoop();
#endif
  return 0;
}

// recover configuration data from logfile
void process_init()
{ 
  while (1)
  {
    int logresult = logfile_read(logfile); 
    if (logresult < 0)
    {
      fprintf(stderr, "\n");
      return;
    }
    else if(logresult > 0)
      continue;
    
    // wait for first laser scan
    if (strcmp(logfile->interface, "laser") == 0 &&
        logfile->index == opt_laser_index)
    {
      // save data
      laser_range_count = logfile->laser_range_count;
      laser_range_max = logfile->laser_range_max;
      laser_angle_min = logfile->laser_angle_min;
      laser_angle_max = logfile->laser_angle_max;
      laser_angle_step = logfile->laser_angle_step;
    
      fprintf(stderr, "range_count = %d\n", laser_range_count);
      fprintf(stderr, "range_max = %f\n", laser_range_max);
      fprintf(stderr, "angle_min = %f\n", laser_angle_min);
      fprintf(stderr, "angle_max = %f\n", laser_angle_max);
      fprintf(stderr, "angle_step = %f\n", laser_angle_step);
    
      return;
    }
  }
}

// Coarse map generation (process log file)
int process_coarse()
{
  pose2_t lodo_pose;

  // Fake an EOF based on the command line options
  if (opt_max_scans > 0 && pmap->step_count >= opt_max_scans)
  {
    fprintf(stderr, "\n");
    return -1;
  }
  
  while (1)
  {
    int logresult = logfile_read(logfile); 
    if (logresult < 0)
    {
      fprintf(stderr, "\n");
      return -1;
    }
    else if(logresult > 0)
      continue;
    
    if (start_time == 0.0 && logfile->dtime > 0.0)
      start_time = logfile->dtime;
    if (logfile->dtime - start_time < opt_skip)
      continue;

    if ((strcmp(logfile->interface, "position2d") == 0 ||
         strcmp(logfile->interface, "position3d") == 0) &&
        logfile->index == opt_position_index)
    {
      odom_pose.pos.x = logfile->position_pose[0];
      odom_pose.pos.y = logfile->position_pose[1];
      odom_pose.rot = logfile->position_pose[2];
    }

    if (strcmp(logfile->interface, "laser") == 0 &&
        logfile->index == opt_laser_index)
    {
      // Update the local pose estimate (corrected odometry)
      lodo_pose = lodo_add_scan(lodo, odom_pose,
                                logfile->laser_range_count, logfile->laser_ranges);

      // Update the global pose estimate (corrected laser pose)
      if (opt_num_samples > 0)
      {
        lodo_pose = pose2_add(opt_laser_pose, lodo_pose);
        pmap_update(pmap, lodo_pose, logfile->laser_range_count, logfile->laser_ranges);
      }

      // Show progress
      fprintf(stderr, "%.3f %.3f %.3f %d %d \r", logfile->dtime, logfile->dtime - start_time,
              lodo->odom_dist, lodo->scan_count, pmap->step_count);
      fflush(stderr);
      
      break;
    }
  }
  return 0;  
}


// Save the coarse map
void save_coarse()
{
  int i;
  char filename[256];
  pmap_sample_t *sample;
  pmap_scan_t *scan;
  pose2_t pose;
  FILE *file;

  sample = PMAP_GET_SAMPLE(pmap, pmap->best_sample);

  snprintf(filename, sizeof(filename), "coarse.out");
  fprintf(stderr, "saving %s\n", filename);

  file = fopen(filename, "w+");
  if (file == NULL)
  {
    fprintf(stderr, "error writing %s : %s", filename, strerror(errno));
    return;
  }

  fprintf(file, "# Coarse trajectory for log file [%s]\n", opt_filename);
  fprintf(file, "# Format is: scan_index pos_x pos_y rot\n");

  omap_clear(omap);

  // Create map and write trajectory
  for (i = 0; i < pmap->step_count; i++)
  {
    scan = pmap->scans + i;
    pose = sample->poses[i];

    fprintf(file, "%d %f %f %f\n", i, pose.pos.x, pose.pos.y, pose.rot);    
    omap_add(omap, pose, pmap->num_ranges, scan->ranges);
  }

  fclose(file);

  // Write map
  snprintf(filename, sizeof(filename), "coarse.pgm");
  fprintf(stderr, "saving %s\n", filename);  
  omap_save_pgm(omap, filename);
  
  return;
}


// Fine map generation (relax in-memory map)
int process_fine()
{
  int i;
  pose2_t pose;
  pmap_scan_t *scan;
  pmap_sample_t *sample;

  if (rmap->num_scans == 0)
  {
    show_grid = 0;
    show_samples = 0;
    
    // Get the current best sample
    sample = PMAP_GET_SAMPLE(pmap, pmap->best_sample);

    // If the map is empty, initialize it
    for (i = 0; i < pmap->step_count; i++)
    {
      scan = pmap->scans + i;
      pose = sample->poses[i];
      rmap_add(rmap, pose, pmap->num_ranges, scan->ranges);
    }

    return 0;
  }

  else if (fine_count < opt_fine_max)
  {
    // Match points periodically; HACK
    if (fine_count % 10 == 0)
      rmap_match(rmap);

    // Relax map
    rmap_relax(rmap, 10);

    // Interpolate
    rmap_interpolate(rmap);

    fine_count++;
        
    // Show progress
    fprintf(stderr, "%d %d %d %d %f \r", fine_count,
            rmap->num_scans, rmap->num_key_scans, rmap->num_cons, rmap->relax_err);
    fflush(stderr);
    
    return 0;
  }

  else
  {
    fprintf(stderr, "\n");
    fflush(stderr);
  }
  
  return -1;
}


// Save the fine map
void save_fine()
{
  int i;
  char filename[256];
  rmap_scan_t *scan;
  pose2_t pose;
  FILE *file;
  
  snprintf(filename, sizeof(filename), "fine.out");
  fprintf(stderr, "saving %s\n", filename);
  
  file = fopen(filename, "w+");
  if (file == NULL)
  {
    fprintf(stderr, "error writing %s : %s", filename, strerror(errno));
    return;
  }

  fprintf(file, "# Fine trajectory for log file [%s]\n", opt_filename);
  fprintf(file, "# Format is: scan_index pos_x pos_y rot\n");

  omap_clear(omap);
    
  // Create map and write trajectory
  for (i = 0; i < rmap->num_scans; i++)
  {
    scan = rmap->scans + i;
    pose = scan->pose;
    fprintf(file, "%d %f %f %f\n", i, pose.pos.x, pose.pos.y, pose.rot);    
    omap_add(omap, pose, rmap->num_ranges, scan->ranges);
  }

  fclose(file);

  // Write map
  snprintf(filename, sizeof(filename), "fine.pgm");
  fprintf(stderr, "saving %s\n", filename);
  omap_save_pgm(omap, filename);
  
  return;
}


// Do some appropriate form of processing
void process()
{
  if (phase == phase_coarse)
  {
    if (process_coarse() != 0)
    {
      // Save the coarse map
      save_coarse();

      // Switch to second-phase processing
      phase = phase_fine;

      fprintf(stderr, "coarse phase complete; switching to fine\n");
    }
  }
  else if (phase == phase_fine)
  {
    if (process_fine() != 0)
    {
      // Save the fine map
      save_fine();

      // We are done
      phase = phase_done;

      fprintf(stderr, "fine phase complete; map is ready\n");
    }
  }
  else
  {
    // Do nothing
    usleep(100000);
  }
  
  return;
}


// Use an appropriate save routine
void save()
{
  if (phase == phase_coarse)
    save_coarse();
  else
    save_fine();
  return;
}


// Trap SIGINTS
void signal_handle(int arg)
{
  // Save current state and exit
  if (phase != phase_done)
    save();
  exit(-1);
  return;
}


int main(int argc, char **argv)
{
  struct timeval tv_a, tv_b;
  int opt;
  const char *opts = "gh";
  const struct option lopts[] =
    {
      {"nogui", 0, 0, 'g'},
      {"max_scans", 1, 0, 1},
      {"range_max", 1, 0, 2},
      {"laser_x", 1, 0, 3},
      {"laser_rot", 1, 0, 4},
      {"position_index", 1, 0, 5},
      {"laser_index", 1, 0, 6},
      {"num_samples", 1, 0, 7},
      {"robot_x", 1, 0, 8},
      {"robot_y", 1, 0, 9},
      {"robot_rot", 1, 0, 10},
      {"grid_width", 1, 0, 20},
      {"grid_height", 1, 0, 21},
      {"grid_scale", 1, 0, 22},
      {"skip", 1, 0, 100},
      {"range_res", 1, 0, 110},
      {"resample_interval", 1, 0, 120},
      {"resample_sigma", 1, 0, 121},
      {"num_cycles", 1, 0, 122},
      {"help", 0, 0, 'h'},
      {0, 0, 0, 0}
    };
  
  // Get our arguments
  while ((opt = getopt_long(argc, argv, opts, lopts, NULL)) >= 0)
  {
    if (opt == 'g')
      opt_gui = 0;
    else if (opt == 'h')
      show_help();
    else if (opt == 1)
      opt_max_scans = atoi(optarg);
    else if (opt == 2)
      opt_range_max = atof(optarg);
    else if (opt == 110)
      opt_range_res = atof(optarg);
    else if (opt == 3)
      opt_laser_pose.pos.x = atof(optarg);
    else if (opt == 4)
      opt_laser_pose.rot = atof(optarg) * M_PI / 180;
    else if (opt == 5)
      opt_position_index = atoi(optarg);
    else if (opt == 6)
      opt_laser_index = atoi(optarg);
    else if (opt == 7)
      opt_num_samples = atoi(optarg);
    else if (opt == 8)
      opt_robot_pose.pos.x = atof(optarg);
    else if (opt == 9)
      opt_robot_pose.pos.y = atof(optarg);
    else if (opt == 10)
      opt_robot_pose.rot = atof(optarg) * M_PI / 180;
    else if (opt == 20)
      opt_grid_width = atof(optarg);
    else if (opt == 21)
      opt_grid_height = atof(optarg);
    else if (opt == 22)
      opt_grid_scale = atof(optarg);
    else if (opt == 100)
      opt_skip = atof(optarg);
    else if (opt == 120)
      opt_resample_interval = atoi(optarg);
    else if (opt == 121)
      opt_resample_sigma = atof(optarg);
    else if (opt == 122)
      opt_fine_max = atoi(optarg);
  }

  // Get filename to process
  if (optind < 0 || optind >= argc)
  {
    //fprintf(stderr, "usage: pmap_test [options] <logfile>\n");
    show_help ();
    exit(-1);
  }
  opt_filename = argv[optind];

  // Register signal handlers
  assert(signal(SIGINT, signal_handle) != SIG_ERR);
  
  // Create logfile reader
  logfile = logfile_alloc(opt_filename);
  if (!logfile)
    return -1;
    
  //aquire laser configuration data
  process_init();
  
  if (opt_range_max > 0)
    laser_range_max = opt_range_max;  // override maximum laser range if specified in command line
  
  // restart logreader
  logfile_free(logfile);
  logfile = logfile_alloc(opt_filename);

  // Create lodo handle
  lodo = lodo_alloc(laser_range_count, laser_range_max, opt_range_res, laser_angle_min, laser_angle_step);
  if (!lodo)
    return -1;

  // Set the laser offset
  lodo->laser_pose = opt_laser_pose;

  // Create pmap handle
  pmap = pmap_alloc(laser_range_count, laser_range_max, laser_angle_min, laser_angle_step,
                      opt_num_samples, opt_grid_width, opt_grid_height, opt_grid_scale);  
  if (!pmap)
    return -1;

  // Set the initial parameters
  if (opt_resample_interval >= 0)
    pmap->resample_interval = opt_resample_interval;
  if (opt_resample_sigma >= 0)
    pmap->resample_s = opt_resample_sigma;
  pmap_set_pose(pmap, opt_robot_pose);

  // Create rmap handle
  rmap = rmap_alloc(laser_range_count, laser_range_max, laser_angle_min, laser_angle_step,
                      opt_grid_width, opt_grid_height);
  if (!rmap)
    return -1;

  // Create omap handle
  omap = omap_alloc(laser_range_count, laser_range_max, laser_angle_min, laser_angle_step,
                      opt_grid_width, opt_grid_height, opt_grid_scale); 
  if (!rmap)
    return -1;

  gettimeofday(&tv_a, NULL);

  // Run the gui
  if (opt_gui)
  {
    win_run(&argc, argv);
  }

  // Run in console mode
  else
  {
    while (phase != phase_done)
      process();
  }

  gettimeofday(&tv_b, NULL);

  // Print stats
  fprintf(stderr, "%.3f m %.3f rotations %d scans %d steps in %d seconds\n"
          "%.0f msec/scan %.0f msec/step\n",
          lodo->odom_dist, lodo->odom_turn / M_PI / 2,
          lodo->scan_count, pmap->step_count, (int) (tv_b.tv_sec - tv_a.tv_sec),
          1000 * (float) (tv_b.tv_sec - tv_a.tv_sec) / lodo->scan_count,
          1000 * (float) (tv_b.tv_sec - tv_a.tv_sec) / pmap->step_count);

  return 0;
}


