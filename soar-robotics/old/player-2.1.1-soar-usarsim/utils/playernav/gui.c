#include <math.h>
#include <sys/time.h>

#include <gdk/gdkkeysyms.h>

#include <libplayercore/playercommon.h>
#include "playernav.h"

// flag and index for robot currently being moved by user (if any)
extern int robot_moving_p;
extern int robot_moving_idx;

#define ROBOT_ALPHA 128
#define PARTICLE_LENGTH 0.25
static guint32 robot_colors[] = { GNOME_CANVAS_COLOR_A(255,0,0,ROBOT_ALPHA),
                                  GNOME_CANVAS_COLOR_A(0,255,0,ROBOT_ALPHA),
                                  GNOME_CANVAS_COLOR_A(0,0,255,ROBOT_ALPHA),
                                  GNOME_CANVAS_COLOR_A(255,0,255,ROBOT_ALPHA),
                                  GNOME_CANVAS_COLOR_A(255,255,0,ROBOT_ALPHA),
                                  GNOME_CANVAS_COLOR_A(0,255,255,ROBOT_ALPHA) };
static size_t num_robot_colors = sizeof(robot_colors) / sizeof(robot_colors[0]);

static gboolean dragging=FALSE;
static gboolean setting_theta=FALSE;
static gboolean setting_goal=FALSE;

extern int dumpp;
extern int showparticlesp;
int show_robot_names;

/*
 * handle quit events, by setting a flag that will make the main loop exit
 */
static gboolean 
_quit_callback(GtkWidget *widget,
               GdkEvent *event,
               gpointer data)
{
  gtk_main_quit();
  return(TRUE);
}

static gboolean 
_toggle_dump(GtkWidget *widget,
             GdkEvent *event,
             gpointer data)
{
  dumpp = !dumpp;
  return(TRUE);
}

static gboolean 
_show_names(GtkWidget *widget,
             GdkEvent *event,
             gpointer data)
{
  int i;
  gui_data_t* gui_data = (gui_data_t*)widget; 

  show_robot_names = !show_robot_names;

  if(show_robot_names)
  {
    for(i=0;i<gui_data->num_robots;i++)
      gnome_canvas_item_show(gui_data->robot_labels[i]);
  }
  else
  {
    for(i=0;i<gui_data->num_robots;i++)
      gnome_canvas_item_hide(gui_data->robot_labels[i]);
  }

  return(TRUE);
}


static gboolean 
_refresh_map(GtkWidget *widget,
             GdkEvent *event,
             gpointer data)
{
  update_map((gui_data_t*)widget);
  return(TRUE);
}

static gboolean 
_show_particles(GtkWidget *widget,
                GdkEvent *event,
                gpointer data)
{
  int i;
  gui_data_t* gui_data = (gui_data_t*)widget; 
  gboolean onmap;
  pose_t robot_pose;

  showparticlesp = !showparticlesp;

  if(showparticlesp)
  {
    for(i=0;i<gui_data->num_robots;i++)
    {
      if(!gui_data->localizes[i] || (gui_data->localizes[i]->hypoth_count <= 0))
        continue;

      robot_pose.px = gui_data->localizes[i]->hypoths[0].mean.px;
      robot_pose.py = gui_data->localizes[i]->hypoths[0].mean.py;
      robot_pose.pa = gui_data->localizes[i]->hypoths[0].mean.pa;

      onmap = ((fabs(robot_pose.px) <
                (gui_data->mapdev->width * 
                 gui_data->mapdev->resolution / 2.0)) ||
               (fabs(robot_pose.py) <
                (gui_data->mapdev->height * 
                 gui_data->mapdev->resolution / 2.0)));

      if(onmap)
      {
        playerc_localize_get_particles(gui_data->localizes[i]);
        draw_particles(gui_data,i);
      }
    }
  }
  else
  {
    for(i=0;i<gui_data->num_robots;i++)
    {
      if(gui_data->robot_particles[i])
        gnome_canvas_item_hide(gui_data->robot_particles[i]);
    }
  }

  return(TRUE);
}


static gboolean 
_stop_all_robots(GtkWidget *widget,
                 GdkEvent *event,
                 gpointer data)
{
  int i;
  playerc_planner_t* planner;
  gui_data_t* gui_data = (gui_data_t*)widget; 

  for(i=0;i<gui_data->num_robots;i++)
  {
    planner = gui_data->planners[i];
    if(planner)
    {
      playerc_planner_enable(planner,0);
      gui_data->robot_enable_states[i] = 0;
    }
  }
  return(TRUE);
}

static gboolean 
_go_all_robots(GtkWidget *widget,
               GdkEvent *event,
               gpointer data)
{
  int i;
  playerc_planner_t* planner;
  gui_data_t* gui_data = (gui_data_t*)widget; 

  for(i=0;i<gui_data->num_robots;i++)
  {
    planner = gui_data->planners[i];
    if(planner)
    {
      playerc_planner_enable(planner,1);
      gui_data->robot_enable_states[i] = 1;
    }
  }
  return(TRUE);
}

static void
_zoom_callback(GtkAdjustment* adjustment,
               gpointer data)
{
  double newzoom;
  gui_data_t* gui_data = (gui_data_t*)data;

  newzoom = gtk_adjustment_get_value(adjustment);

  gnome_canvas_set_pixels_per_unit(gui_data->map_canvas, newzoom);
}

static gboolean
_robot_button_callback(GnomeCanvasItem *item,
                       GdkEvent *event,
                       gpointer data)
{
  static int idx;
  gboolean onrobot=FALSE;
  gboolean ongoal=FALSE;
  double theta;
  static GnomeCanvasPoints* points = NULL;
  static GnomeCanvasItem* setting_theta_line = NULL;
  pose_t pose;
  double mean[3];
  static double cov[3] = {0.5*0.5, 0.5*0.5, (M_PI/6.0)*(M_PI/6.0)};

  gui_data_t* gui_data = (gui_data_t*)data;

  pose.pa = 0.0;
  pose.px = event->button.x;
  pose.py = -event->button.y;
  
  // lookup (and store) which robot (or goal) was clicked
  if((item != (GnomeCanvasItem*)gnome_canvas_root(gui_data->map_canvas)) &&
     !setting_theta && !setting_goal)
  {
    for(idx=0;idx<gui_data->num_robots;idx++)
    {
      if(item == gui_data->robot_items[idx])
      {
        onrobot = TRUE;
        break;
      }
      else if(item == gui_data->robot_goals[idx])
      {
        ongoal = TRUE;
        break;
      }
    }
    assert(idx < gui_data->num_robots);
    if(!show_robot_names)
      gnome_canvas_item_hide(gui_data->robot_labels[idx]);
  }

  switch(event->type)
  {
    case GDK_BUTTON_PRESS:
      switch(event->button.button)
      {
        // Middle button enables/disables the robot
        case 2:
          if(onrobot && !setting_theta)
          {
            gui_data->robot_enable_states[idx] = 
                    !gui_data->robot_enable_states[idx];
            playerc_planner_enable(gui_data->planners[idx],
                                   gui_data->robot_enable_states[idx]);
          }
          break;
        case 3:
          if((onrobot || ongoal) && !setting_theta)
          {
            setting_goal=TRUE;
            move_item(gui_data->robot_goals[idx],pose,0);
            gnome_canvas_item_show(gui_data->robot_goals[idx]);
          }
        case 1:
          if(!setting_theta)
          {
            if(onrobot || (ongoal && event->button.button == 3))
            {
              gnome_canvas_item_grab(item,
                                     GDK_POINTER_MOTION_MASK | 
                                     GDK_BUTTON_RELEASE_MASK,
                                     NULL, event->button.time);
              dragging = TRUE;

              // set these so that the robot's pose won't be updated in
              // the GUI while we're dragging  the robot (that happens
              // in playernav.c)
              if(event->button.button == 1)
              {
                robot_moving_p = 1;
                robot_moving_idx = idx;
              }
            }
          }
          else
          {
            theta = atan2(-points->coords[3] + points->coords[1],
                          points->coords[2] - points->coords[0]);

            mean[0] = points->coords[0];
            mean[1] = -points->coords[1];
            mean[2] = theta;

            if(setting_goal)
            {
              if(gui_data->planners[idx])
              {
                printf("setting goal for robot %d to (%.3f, %.3f, %.3f)\n",
                       idx, mean[0], mean[1], mean[2]);
                if(playerc_planner_set_cmd_pose(gui_data->planners[idx],
                                                mean[0], mean[1], 
                                                mean[2]) < 0)
                {
                  fprintf(stderr, "error while setting goal on robot %d\n", 
                          idx);
                  //gtk_main_quit();
                  //return(TRUE);
                }
                else
                {
                  gui_data->goals[idx][0] = mean[0];
                  gui_data->goals[idx][1] = mean[1];
                  gui_data->goals[idx][2] = mean[2];
                  gui_data->planners[idx]->waypoint_count = -1;
                }
              }
              else
              {
                puts("WARNING: NOT setting goal; couldn't connect to planner\n");
              }
            }
            else
            {
              if(gui_data->localizes[idx])
              {
                printf("setting pose for robot %d to (%.3f, %.3f, %.3f)\n",
                       idx, mean[0], mean[1], mean[2]);

                if(playerc_localize_set_pose(gui_data->localizes[idx], 
                                             mean, cov) < 0)
                {
                  fprintf(stderr, "error while setting pose on robot %d\n", 
                          idx);
                  gtk_main_quit();
                  return(TRUE);
                }
              }
              else
              {
                puts("WARNING: NOT setting pose; couldn't connect to localize\n");
              }
            }

            //move_robot(gui_data->robot_items[idx],pose);
            gnome_canvas_item_hide(setting_theta_line);
            setting_theta = FALSE;
            setting_goal = FALSE;

            robot_moving_p = 0;
          }
          break;
        default:
          break;
      }
      break;
    case GDK_MOTION_NOTIFY:
      if(onrobot)
        gnome_canvas_item_show(gui_data->robot_labels[idx]);
      if(dragging)
      {
        if(setting_goal)
          move_item(gui_data->robot_goals[idx],pose,0);
        else
          move_item(item,pose,1);
      }
      else if(setting_theta)
      {
        points->coords[2] = pose.px;
        points->coords[3] = -pose.py;
        gnome_canvas_item_set(setting_theta_line,
                              "points",points,
                              NULL);
      }
      break;
    case GDK_BUTTON_RELEASE:
      if(dragging)
      {
        dragging = FALSE;
        setting_theta = TRUE;

        if(!points)
          g_assert((points = gnome_canvas_points_new(2)));
        points->coords[0] = pose.px;
        points->coords[1] = -pose.py;
        points->coords[2] = pose.px;
        points->coords[3] = -pose.py;
        if(!setting_theta_line)
        {
          g_assert((setting_theta_line = 
                  gnome_canvas_item_new(gnome_canvas_root(gui_data->map_canvas),
                                        gnome_canvas_line_get_type(),
                                        "points", points,
                                        "width_pixels", 1,
                                        "fill-color-rgba", COLOR_BLACK,
                                        NULL)));
        }
        else
        {
          gnome_canvas_item_set(setting_theta_line,
                                "points",points,
                                NULL);
          gnome_canvas_item_show(setting_theta_line);
        }

        gnome_canvas_item_ungrab(item, event->button.time);
      }
      break;
    default:
      break;
  }

  return(TRUE);
}

void
canvas_to_meters(gui_data_t* gui_data, double* dx, double* dy, int cx, int cy)
{
  gnome_canvas_c2w(gui_data->map_canvas,cx,cy,dx,dy);
  *dy = -*dy;
}

void
item_to_meters(GnomeCanvasItem* item,
               double* dx, double* dy, 
               double ix, double iy)
{
  *dx=ix;
  *dy=iy;
  gnome_canvas_item_i2w(item, dx, dy);
  *dy = -*dy;
}

void
meters_to_canvas(gui_data_t* gui_data, int* cx, int* cy, double dx, double dy)
{
  double x,y;
  x=dx;
  y=-dy;
  gnome_canvas_w2c(gui_data->map_canvas,x,y,cx,cy);
}

void
make_menu(gui_data_t* gui_data)
{
  GtkMenuBar* menu_bar;
  GtkMenu* file_menu;
  GtkMenuItem* file_item;
  GtkCheckMenuItem* dump_item;
  GtkMenuItem* quit_item;

  GtkMenu* view_menu;
  GtkMenuItem* view_item;
  GtkCheckMenuItem* show_names_item;
  GtkCheckMenuItem* show_particles_item;
  GtkCheckMenuItem* refresh_map_item;
  
  GtkMenu* stop_menu;
  GtkMenuItem* stop_item;
  GtkMenuItem* stop_all_item;
  GtkMenuItem* go_all_item;

  GtkAccelGroup *accel_group;

  /* Create a GtkAccelGroup and add it to the main window. */
  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(gui_data->main_window, accel_group);

  file_menu = (GtkMenu*)gtk_menu_new();    /* Don't need to show menus */
  view_menu = (GtkMenu*)gtk_menu_new();    /* Don't need to show menus */
  stop_menu = (GtkMenu*)gtk_menu_new();    /* Don't need to show menus */

  /* Create the menu items */
  quit_item = (GtkMenuItem*)gtk_menu_item_new_with_label("Quit");
  dump_item = (GtkCheckMenuItem*)gtk_check_menu_item_new_with_label("Capture stills");
  show_names_item = (GtkCheckMenuItem*)gtk_check_menu_item_new_with_label("Show robot names");
  show_particles_item = (GtkCheckMenuItem*)gtk_check_menu_item_new_with_label("Show particles");
  refresh_map_item = (GtkCheckMenuItem*)gtk_menu_item_new_with_label("Refresh map");
  stop_all_item = (GtkMenuItem*)gtk_menu_item_new_with_label("Stop all robots");
  go_all_item = (GtkMenuItem*)gtk_menu_item_new_with_label("Go all robots");

  /* Setup accelerators:
       Ctrl-s stops all robots (handy in an emergency)
       Ctrl-g goes all robots
       Ctrl-q quits
       Ctrl-n toggles showing robot names
       Ctrl-d toggles dumping screenshots
   */
  gtk_widget_add_accelerator((GtkWidget*)stop_all_item, "activate", 
                             accel_group, GDK_s, GDK_CONTROL_MASK, 
                             GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator((GtkWidget*)go_all_item, "activate", 
                             accel_group, GDK_g, GDK_CONTROL_MASK, 
                             GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator((GtkWidget*)quit_item, "activate", 
                             accel_group, GDK_q, GDK_CONTROL_MASK, 
                             GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator((GtkWidget*)show_names_item, "activate", 
                             accel_group, GDK_n, GDK_CONTROL_MASK, 
                             GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator((GtkWidget*)show_particles_item, "activate", 
                             accel_group, GDK_p, GDK_CONTROL_MASK, 
                             GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator((GtkWidget*)refresh_map_item, "activate", 
                             accel_group, GDK_r, GDK_CONTROL_MASK, 
                             GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator((GtkWidget*)dump_item, "activate", 
                             accel_group, GDK_d, GDK_CONTROL_MASK, 
                             GTK_ACCEL_VISIBLE);

  /* Add them to the menu */
  gtk_menu_shell_append (GTK_MENU_SHELL(file_menu), (GtkWidget*)dump_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(file_menu), (GtkWidget*)quit_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(view_menu), (GtkWidget*)show_names_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(view_menu), (GtkWidget*)show_particles_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(view_menu), (GtkWidget*)refresh_map_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(stop_menu), (GtkWidget*)stop_all_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(stop_menu), (GtkWidget*)go_all_item);

  /* We can attach the Quit menu item to
   * our exit function */
  g_signal_connect_swapped(G_OBJECT (quit_item), "activate",
                           G_CALLBACK(_quit_callback),
                           (gpointer) "file.quit");
  g_signal_connect_swapped(G_OBJECT (dump_item), "activate",
                           G_CALLBACK(_toggle_dump),
                           (gpointer) "file.dump");
  g_signal_connect_swapped(G_OBJECT (show_names_item), "activate",
                           G_CALLBACK(_show_names),
                           (gpointer) gui_data);
  g_signal_connect_swapped(G_OBJECT (show_particles_item), "activate",
                           G_CALLBACK(_show_particles),
                           (gpointer) gui_data);
  g_signal_connect_swapped(G_OBJECT (refresh_map_item), "activate",
                           G_CALLBACK(_refresh_map),
                           (gpointer) gui_data);
  g_signal_connect_swapped(G_OBJECT (stop_all_item), "activate",
                           G_CALLBACK(_stop_all_robots),
                           (gpointer) gui_data);
  g_signal_connect_swapped(G_OBJECT (go_all_item), "activate",
                           G_CALLBACK(_go_all_robots),
                           (gpointer) gui_data);

  /* We do need to show menu items */
  gtk_widget_show((GtkWidget*)dump_item);
  gtk_widget_show((GtkWidget*)quit_item);
  gtk_widget_show((GtkWidget*)show_names_item);
  gtk_widget_show((GtkWidget*)show_particles_item);
  gtk_widget_show((GtkWidget*)refresh_map_item);
  gtk_widget_show((GtkWidget*)stop_all_item);
  gtk_widget_show((GtkWidget*)go_all_item);

  menu_bar = (GtkMenuBar*)gtk_menu_bar_new ();
  gtk_box_pack_start(gui_data->vbox,
                     (GtkWidget*)(menu_bar),
                     FALSE, FALSE, 0);
  gtk_widget_show((GtkWidget*)menu_bar);

  file_item = (GtkMenuItem*)gtk_menu_item_new_with_label ("File");
  gtk_widget_show((GtkWidget*)file_item);
  view_item = (GtkMenuItem*)gtk_menu_item_new_with_label ("View");
  gtk_widget_show((GtkWidget*)view_item);
  stop_item = (GtkMenuItem*)gtk_menu_item_new_with_label ("Stop/Go");
  gtk_widget_show((GtkWidget*)stop_item);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), (GtkWidget*)file_menu);
  gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), (GtkWidget*)file_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_item), (GtkWidget*)view_menu);
  gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), (GtkWidget*)view_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(stop_item), (GtkWidget*)stop_menu);
  gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), (GtkWidget*)stop_item);
}

void
init_gui(gui_data_t* gui_data, int argc, char** argv)
{
  //double t[6];
  double initial_zoom, max_zoom;
  GtkAdjustment* adjust;

  g_type_init();
  gtk_init(&argc, &argv);

  g_assert((gui_data->main_window = 
            (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL)));
  gtk_widget_set_size_request((GtkWidget*)(gui_data->main_window),
                              MIN_DISPLAY_WIDTH,MIN_DISPLAY_WIDTH);
  // display the map "natural size" + a bit for the scrollbars
  gtk_window_resize(gui_data->main_window,
                    (gui_data->initial_zoom * gui_data->mapdev->width)+40,
                    (gui_data->initial_zoom * gui_data->mapdev->height)+40);

  /* a box to hold everything else */
  g_assert((gui_data->vbox = (GtkBox*)gtk_vbox_new(FALSE, 5)));

  /* a box to hold everything else */
  g_assert((gui_data->hbox = (GtkBox*)gtk_hbox_new(FALSE, 5)));

  g_assert((gui_data->map_window = 
            (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL)));
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gui_data->map_window),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  // Initialize horizontal scroll bars
  adjust = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(gui_data->map_window));
  adjust->step_increment = 5;
  gtk_adjustment_changed(adjust);
  gtk_adjustment_set_value(adjust, adjust->value - GTK_WIDGET(gui_data->map_window)->allocation.width / 2);

  // Initialize vertical scroll bars
  adjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(gui_data->map_window));
  adjust->step_increment = 5;
  gtk_adjustment_changed(adjust);
  gtk_adjustment_set_value(adjust, adjust->value - GTK_WIDGET(gui_data->map_window)->allocation.height / 2);

  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  if(gui_data->aa)
    g_assert((gui_data->map_canvas = (GnomeCanvas*)gnome_canvas_new_aa()));
  else
    g_assert((gui_data->map_canvas = (GnomeCanvas*)gnome_canvas_new()));
  gtk_widget_pop_colormap();
  gtk_widget_pop_visual();

  gnome_canvas_set_center_scroll_region(gui_data->map_canvas, TRUE);
  gnome_canvas_set_scroll_region(gui_data->map_canvas,
                                 -(gui_data->mapdev->width * 
                                   gui_data->mapdev->resolution)/2.0,
                                 -(gui_data->mapdev->height *
                                   gui_data->mapdev->resolution)/2.0,
                                 (gui_data->mapdev->width *
                                   gui_data->mapdev->resolution)/2.0,
                                 (gui_data->mapdev->height *
                                   gui_data->mapdev->resolution)/2.0);

  // the zoom scrollbar

  // set canvas zoom to make the map fill the window
  initial_zoom = (gui_data->initial_zoom) * 
          (1.0 / gui_data->mapdev->resolution);
  // zoom in at most 10 times
  max_zoom = 10.0 * initial_zoom;

  g_assert((gui_data->zoom_adjustment = 
            (GtkAdjustment*)gtk_adjustment_new(initial_zoom, 
                                               initial_zoom,
                                               max_zoom,
                                               (max_zoom-initial_zoom)/1e3,
                                               (max_zoom-initial_zoom)/1e2,
                                               (max_zoom-initial_zoom)/1e2)));
  g_assert((gui_data->zoom_scrollbar = 
            (GtkVScrollbar*)gtk_vscrollbar_new(gui_data->zoom_adjustment)));

  gtk_container_add(GTK_CONTAINER(gui_data->main_window),
                    (GtkWidget*)(gui_data->vbox));
  make_menu(gui_data);

  gtk_box_pack_start(gui_data->vbox,
                     (GtkWidget*)(gui_data->hbox), 
                     TRUE, TRUE, 0);
  gtk_box_pack_start(gui_data->hbox,
                     (GtkWidget*)(gui_data->zoom_scrollbar), 
                     FALSE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(gui_data->map_window),
                    (GtkWidget*)(gui_data->map_canvas));
  gtk_box_pack_start(gui_data->hbox,
                     (GtkWidget*)(gui_data->map_window), 
                     TRUE, TRUE, 0);

  gtk_widget_show((GtkWidget*)(gui_data->vbox));
  gtk_widget_show((GtkWidget*)(gui_data->hbox));
  gtk_widget_show((GtkWidget*)(gui_data->zoom_scrollbar));
  gtk_widget_show((GtkWidget*)(gui_data->map_window));
  gtk_widget_show((GtkWidget*)(gui_data->map_canvas));

  g_signal_connect(G_OBJECT(gui_data->main_window),"delete-event",
                   G_CALLBACK(_quit_callback),NULL);
  g_signal_connect(G_OBJECT(gui_data->main_window),"destroy-event",
                   G_CALLBACK(_quit_callback),NULL);
  g_signal_connect(G_OBJECT(gui_data->zoom_adjustment),"value-changed",
                   G_CALLBACK(_zoom_callback),(void*)gui_data);

  gtk_signal_connect(GTK_OBJECT(gnome_canvas_root(gui_data->map_canvas)),
                     "event",
                     (GtkSignalFunc)(_robot_button_callback),(void*)gui_data);

  gtk_adjustment_set_value(gui_data->zoom_adjustment,initial_zoom);
  gtk_adjustment_value_changed(gui_data->zoom_adjustment);
#if 0
  g_signal_connect(G_OBJECT(gui_data->main_window),"size-allocate",
                   G_CALLBACK(_resize_window_callback),(void*)gui_data);
#endif
}

void
fini_gui(gui_data_t* gui_data)
{
  assert(gui_data->main_window);
  gtk_widget_destroy((GtkWidget*)(gui_data->main_window));
}

/*
 * create the background map image and put it on the canvas
 */
void
create_map_image(gui_data_t* gui_data)
{
  GdkPixbuf* pixbuf;
  static guchar* pixels = NULL;
  int i,j;

  if(!gui_data->mapdev->width || !gui_data->mapdev->height)
  {
    gui_data->imageitem = NULL;
    return;
  }

  if(pixels)
    free(pixels);
  assert(pixels = (guchar*)malloc(sizeof(unsigned char) * 3 *
                                  gui_data->mapdev->width *
                                  gui_data->mapdev->height));

  for(j=0; j < gui_data->mapdev->height; j++)
  {
    for(i=0; i < gui_data->mapdev->width; i++)
    {
      if(gui_data->mapdev->cells[PLAYERC_MAP_INDEX(gui_data->mapdev,i,j)] == -1)
      {
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3] = 255;
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3+1] = 255;
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3+2] = 255;
        /*
        pixels[(gui_data->mapdev->width * j + i)*3] = 255;
        pixels[(gui_data->mapdev->width * j + i)*3+1] = 255;
        pixels[(gui_data->mapdev->width * j + i)*3+2] = 255;
        */
      }
      else if(gui_data->mapdev->cells[PLAYERC_MAP_INDEX(gui_data->mapdev,i,j)] == 0)
      {
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3] = 100;
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3+1] = 100;
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3+2] = 100;
        /*
        pixels[(gui_data->mapdev->width * j + i)*3] = 100;
        pixels[(gui_data->mapdev->width * j + i)*3+1] = 100;
        pixels[(gui_data->mapdev->width * j + i)*3+2] = 100;
                */
      }
      else
      {
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3] = 0;
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3+1] = 0;
        pixels[(gui_data->mapdev->width * 
                (gui_data->mapdev->height - j-1) + i)*3+2] = 0;
        /*
        pixels[(gui_data->mapdev->width * j + i)*3] = 0;
        pixels[(gui_data->mapdev->width * j + i)*3+1] = 0;
        pixels[(gui_data->mapdev->width * j + i)*3+2] = 0;
                */
      }
    }
  }
  
  // create the pixbuf
  g_assert((pixbuf = gdk_pixbuf_new_from_data(pixels,
                                              GDK_COLORSPACE_RGB,
                                              FALSE,
                                              8,
                                              gui_data->mapdev->width, 
                                              gui_data->mapdev->height, 
                                              3*gui_data->mapdev->width,
                                              NULL,
                                              NULL)));

  g_assert((gui_data->imageitem = 
            gnome_canvas_item_new(gnome_canvas_root(gui_data->map_canvas), 
                                  gnome_canvas_pixbuf_get_type(),
                                  "width-set", TRUE,
                                  "height-set", TRUE,
                                  "width", gui_data->mapdev->width *
                                  gui_data->mapdev->resolution,
                                  "height", gui_data->mapdev->height *
                                  gui_data->mapdev->resolution,
                                  "x", gui_data->mapdev->origin[0],
                                  "y", -(gui_data->mapdev->origin[1] +
                                         (gui_data->mapdev->height *
                                          gui_data->mapdev->resolution)),
                                  "pixbuf", pixbuf,
                                  NULL)));

  gnome_canvas_item_lower_to_bottom((GnomeCanvasItem*)gui_data->imageitem);
  gnome_canvas_item_show((GnomeCanvasItem*)gui_data->imageitem);

  gnome_canvas_set_scroll_region(gui_data->map_canvas,
                                 gui_data->mapdev->origin[0],
                                 -(gui_data->mapdev->origin[1] + 
                                   gui_data->mapdev->height * 
                                   gui_data->mapdev->resolution),
                                 (gui_data->mapdev->origin[0] + 
                                  gui_data->mapdev->width * 
                                  gui_data->mapdev->resolution),
                                 -gui_data->mapdev->origin[1]);

  g_object_unref((GObject*)pixbuf);
}

void
create_robot(gui_data_t* gui_data, int idx, pose_t pose)
{
  GnomeCanvasGroup* robot;
  GnomeCanvasItem* robot_circle;
  GnomeCanvasItem* robot_v;
  GnomeCanvasItem* robot_text;
  GnomeCanvasItem* robot_goal;
  GnomeCanvasPoints* points;
  char robotname[256];

  assert(idx < gui_data->num_robots);

  g_assert((robot = (GnomeCanvasGroup*)
            gnome_canvas_item_new(gnome_canvas_root(gui_data->map_canvas),
                                  gnome_canvas_group_get_type(),
                                  "x", 0.0, "y", 0.0,
                                  NULL)));

  g_assert((robot_circle = 
            gnome_canvas_item_new(robot,
                                  gnome_canvas_ellipse_get_type(),
                                  "x1", -ROBOT_RADIUS,
                                  "y1", -ROBOT_RADIUS,
                                  "x2",  ROBOT_RADIUS,
                                  "y2",  ROBOT_RADIUS,
                                  "outline_color_rgba", COLOR_BLACK,
                                  "fill_color_rgba", 
                                  robot_colors[idx % num_robot_colors],
                                  "width_pixels", 1,
                                  NULL)));
  g_assert((points = gnome_canvas_points_new(3)));
  points->coords[0] = ROBOT_RADIUS * cos(ROBOT_V_ANGLE);
  points->coords[1] = ROBOT_RADIUS * sin(ROBOT_V_ANGLE);
  points->coords[2] = 0.0;
  points->coords[3] = 0.0;
  points->coords[4] = ROBOT_RADIUS * cos(ROBOT_V_ANGLE);
  points->coords[5] = ROBOT_RADIUS * sin(-ROBOT_V_ANGLE);

  g_assert((robot_v = 
            gnome_canvas_item_new(robot,
                                  gnome_canvas_line_get_type(),
                                  "points", points,
                                  "fill_color_rgba", COLOR_BLACK,
                                  "width_pixels", 1,
                                  NULL)));

  // a triangle to mark the goal
  points->coords[0] = ROBOT_RADIUS * cos(M_PI/2.0);
  points->coords[1] = ROBOT_RADIUS * sin(M_PI/2.0);
  points->coords[2] = ROBOT_RADIUS * cos(7*M_PI/6.0);
  points->coords[3] = ROBOT_RADIUS * sin(7*M_PI/6.0);
  points->coords[4] = ROBOT_RADIUS * cos(11*M_PI/6.0);
  points->coords[5] = ROBOT_RADIUS * sin(11*M_PI/6.0);

  g_assert((robot_goal = 
            gnome_canvas_item_new(gnome_canvas_root(gui_data->map_canvas),
                                  gnome_canvas_polygon_get_type(),
                                  "points", points,
                                  "outline_color_rgba", COLOR_BLACK,
                                  "fill_color_rgba", 
                                  robot_colors[idx % num_robot_colors],
                                  "width_pixels", 1,
                                  NULL)));
  gnome_canvas_item_hide(robot_goal);


  gnome_canvas_points_unref(points);


  sprintf(robotname, "%s:%d", 
          gui_data->hostnames[idx], gui_data->ports[idx]);
  g_assert((robot_text =
            gnome_canvas_item_new(robot,
                                  gnome_canvas_text_get_type(),
                                  "text", robotname,
                                  "x", 0.0,
                                  "y", 0.0,
                                  "x-offset", 2.0*ROBOT_RADIUS,
                                  "y-offset", -2.0*ROBOT_RADIUS,
                                  "fill-color-rgba", COLOR_BLACK,
                                  NULL)));
  gnome_canvas_item_hide(robot_text);

  move_item((GnomeCanvasItem*)robot,pose,1);

  gui_data->robot_items[idx] = (GnomeCanvasItem*)robot;
  gui_data->robot_labels[idx] = robot_text;
  gui_data->robot_goals[idx] = robot_goal;
  gui_data->robot_poses[idx] = pose;

  gtk_signal_connect(GTK_OBJECT(robot_goal), "event",
                     (GtkSignalFunc)_robot_button_callback, (void*)gui_data);
  gtk_signal_connect(GTK_OBJECT(robot), "event",
                     (GtkSignalFunc)_robot_button_callback, (void*)gui_data);
}

void
move_item(GnomeCanvasItem* item, pose_t pose, int raise)
{
  double t[6];

  t[0] = cos(pose.pa);
  t[1] = -sin(pose.pa);
  t[2] = sin(pose.pa);
  t[3] = cos(pose.pa);
  t[4] = pose.px;
  t[5] = -pose.py;
  gnome_canvas_item_affine_absolute(item, t);
  if(raise)
    gnome_canvas_item_raise_to_top(item);
}

void
draw_particles(gui_data_t* gui_data, int idx)
{
  int i;
  double stddev;
  pose_t ellipse_pose;
  GnomeCanvasItem* line;
  GnomeCanvasItem* ellipse;
  GnomeCanvasPoints* linepoints;

  if(gui_data->robot_particles[idx])
  {
    gtk_object_destroy(GTK_OBJECT(gui_data->robot_particles[idx]));
    gui_data->robot_particles[idx] = NULL;
  }

  if(gui_data->localizes[idx]->num_particles)
  {
    g_assert((gui_data->robot_particles[idx] = 
              gnome_canvas_item_new(gnome_canvas_root(gui_data->map_canvas),
                                    gnome_canvas_group_get_type(),
                                    "x", 0.0, "y", 0.0,
                                    NULL)));
    g_assert((linepoints = gnome_canvas_points_new(2)));
    for(i=0;i<gui_data->localizes[idx]->num_particles;i++)
    {
      linepoints->coords[0] = 
              gui_data->localizes[idx]->particles[i].pose[0];
      linepoints->coords[1] = 
              -gui_data->localizes[idx]->particles[i].pose[1];
      linepoints->coords[2] = 
              (gui_data->localizes[idx]->particles[i].pose[0] + 
               PARTICLE_LENGTH * 
               cos(gui_data->localizes[idx]->particles[i].pose[2]));
      linepoints->coords[3] = 
              -(gui_data->localizes[idx]->particles[i].pose[1] +
                PARTICLE_LENGTH * 
                sin(gui_data->localizes[idx]->particles[i].pose[2]));

      g_assert((line = 
                gnome_canvas_item_new((GnomeCanvasGroup*)gui_data->robot_particles[idx],
                                      gnome_canvas_line_get_type(),
                                      "points", linepoints,
                                      "width_pixels", 1,
                                      // TODO: figure out how to use
                                      //       arrowheads
                                      //"last-arrowhead", TRUE,
                                      //"arrow-shape-a", particle_length/3.0,
                                      //"arrow-shape-b", particle_length/3.0,
                                      //"arrow-shape-c", particle_length/3.0,
                                      "fill-color-rgba",
                                      robot_colors[idx % num_robot_colors],
                                      NULL)));
    }

    // Draw the 3-sigma ellipse 
    stddev = sqrt(gui_data->localizes[idx]->variance);
    g_assert((ellipse = 
              gnome_canvas_item_new((GnomeCanvasGroup*)gui_data->robot_particles[idx],
                                    gnome_canvas_ellipse_get_type(),
                                    "x1", -3*stddev,
                                    "y1", -3*stddev,
                                    "x2", 3*stddev,
                                    "y2", 3*stddev,
                                    "fill-color-rgba",
                                    robot_colors[idx % num_robot_colors],
                                    NULL)));

    ellipse_pose.px = gui_data->localizes[idx]->mean[0];
    ellipse_pose.py = gui_data->localizes[idx]->mean[1];
    ellipse_pose.pa = gui_data->localizes[idx]->mean[2];
    move_item(ellipse,ellipse_pose,0);
  }
}

void
draw_waypoints(gui_data_t* gui_data, int idx)
{
  int i;
  GnomeCanvasPoints* points;
  GnomeCanvasPoints* linepoints;
  GnomeCanvasItem* waypoint = NULL;
  GnomeCanvasItem* line;
  pose_t pose;

  if(gui_data->robot_paths[idx])
  {
    //puts("destroying waypoints");
    gtk_object_destroy(GTK_OBJECT(gui_data->robot_paths[idx]));
    gui_data->robot_paths[idx] = NULL;
  }

  if(gui_data->planners[idx]->path_done)
  {
    if(!dragging && !setting_theta)
      gnome_canvas_item_hide(gui_data->robot_goals[idx]);
  }
  else if(gui_data->planners[idx]->path_valid)
  {
    g_assert((gui_data->robot_paths[idx] = 
              gnome_canvas_item_new(gnome_canvas_root(gui_data->map_canvas),
                                    gnome_canvas_group_get_type(),
                                    "x", 0.0, "y", 0.0,
                                    NULL)));

    g_assert((points = gnome_canvas_points_new(3)));
    g_assert((linepoints = gnome_canvas_points_new(2)));

    // a small triangle to mark each waypoint
    points->coords[0] = 0.5 * ROBOT_RADIUS * cos(M_PI/2.0);
    points->coords[1] = 0.5 * ROBOT_RADIUS * sin(M_PI/2.0);
    points->coords[2] = 0.5 * ROBOT_RADIUS * cos(7*M_PI/6.0);
    points->coords[3] = 0.5 * ROBOT_RADIUS * sin(7*M_PI/6.0);
    points->coords[4] = 0.5 * ROBOT_RADIUS * cos(11*M_PI/6.0);
    points->coords[5] = 0.5 * ROBOT_RADIUS * sin(11*M_PI/6.0);

    for(i=MAX(gui_data->planners[idx]->curr_waypoint-1,0);
        i<gui_data->planners[idx]->waypoint_count;
        i++)
    {
      if(i<gui_data->planners[idx]->waypoint_count-1)
      {
        g_assert((waypoint = 
                  gnome_canvas_item_new((GnomeCanvasGroup*)gui_data->robot_paths[idx],
                                        gnome_canvas_polygon_get_type(),
                                        "points", points,
                                        "outline_color_rgba", COLOR_BLACK,
                                        "fill_color_rgba", 
                                        robot_colors[idx % num_robot_colors],
                                        "width_pixels", 1,
                                        NULL)));
      }
      else
      {
        if(!dragging && !setting_theta)
        {
          gnome_canvas_item_show(gui_data->robot_goals[idx]);
          waypoint = gui_data->robot_goals[idx];
        }
      }

      if(waypoint)
      {
        pose.px =  gui_data->planners[idx]->waypoints[i][0];
        pose.py =  gui_data->planners[idx]->waypoints[i][1];
        pose.pa = 0.0;
        move_item(waypoint, pose,0);
      }

      if(i>0)
      {
        linepoints->coords[0] = gui_data->planners[idx]->waypoints[i-1][0];
        linepoints->coords[1] = -gui_data->planners[idx]->waypoints[i-1][1];
        linepoints->coords[2] = gui_data->planners[idx]->waypoints[i][0];
        linepoints->coords[3] = -gui_data->planners[idx]->waypoints[i][1];

        g_assert((line = 
                  gnome_canvas_item_new((GnomeCanvasGroup*)gui_data->robot_paths[idx],
                                        gnome_canvas_line_get_type(),
                                        "points", linepoints,
                                        "width_pixels", 3,
                                        "fill-color-rgba",
                                        robot_colors[idx % num_robot_colors],
                                        NULL)));
      }
    }

    gnome_canvas_points_unref(points);
    gnome_canvas_points_unref(linepoints);
  }
}

void
draw_goal(gui_data_t* gui_data, int idx)
{
  if(!dragging && !setting_theta)
  {
    pose_t p;
    p.px = gui_data->planners[idx]->gx;
    p.py = gui_data->planners[idx]->gy;
    p.pa = gui_data->planners[idx]->ga;
    move_item(gui_data->robot_goals[idx],p,0);
    gnome_canvas_item_show(gui_data->robot_goals[idx]);
  }
}


void
dump_screenshot(gui_data_t* gui_data)
{
  static int idx = 0;
  static GdkPixbuf* screenshot = NULL;
  GdkWindow* win;
  char fname[PATH_MAX];
  gint width, height;
  
  g_assert((win = ((GtkWidget*)gui_data->map_canvas)->window));
  if(gdk_window_is_viewable(win))
  {
    gdk_window_get_size(win, &width, &height);

    g_assert((screenshot = 
              gdk_pixbuf_get_from_drawable(screenshot,
                                           (GdkDrawable*)win,
                                           gdk_colormap_get_system(),
                                           0,0,0,0,
                                           width,height)));

    sprintf(fname,"playernav-img-%04d.png",idx);
    printf("writing screenshot to %s\n", fname);

    if(!(gdk_pixbuf_save(screenshot, fname, "png", NULL, NULL)))
      puts("FAILED");

    idx++;
  }
}

