/** @ingroup utils Utilities */
/** @{ */
/** @defgroup util_playerwritemap playerwritemap
 * @brief Retrieve map data and save it to a file

@par Synopsis

@todo write a synopsis

@par Usage

@todo write usage

@author Brian Gerkey

*/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <libplayerc/playerc.h>

// use gdk-pixbuf for image saving
#include <gdk-pixbuf/gdk-pixbuf.h>

#define USAGE "Usage: playerwritemap [-h <host>] [-p <port>] [-m {v|o}] [-i <mapidx>] <filename>"

char* host;
char* fname;
int port;
int omap;
int idx;

void create_map_image(playerc_map_t* map, const char* fname, const char* fmt);

int parse_args(int argc, char** argv);

int main(int argc, char **argv)
{
  int i;
  playerc_client_t *client;
  playerc_map_t *map;
  FILE* fp;

  host = "localhost";
  port = 6665;
  omap = 1;
  idx = 0;

  if(parse_args(argc,argv) < 0)
  {
    puts(USAGE);
    exit(-1);
  }

  // Create a client and connect it to the server.
  client = playerc_client_create(NULL, host, port);
  if (playerc_client_connect(client) != 0)
    return -1;

  // Create and subscribe to a map device.
  map = playerc_map_create(client, idx);
  if (playerc_map_subscribe(map, PLAYER_OPEN_MODE))
    return -1;

  if(omap)
  {
    // Get the occ grid map
    if (playerc_map_get_map(map) != 0)
      return -1;

    printf("map: %d X %d @ %.3f origin: %f %f \n", 
           map->width, map->height, 
           map->resolution,
           map->origin[0], map->origin[1]);

    create_map_image(map, fname, "png");
  }
  else
  {
    // Get the vector map
    if (playerc_map_get_vector(map) != 0)
      return -1;

    if(!(fp = fopen(fname,"w+")))
    {
      perror("fopen() failed");
      exit(-1);
    }

    fprintf(fp,"# Created by benson's SfLineScan.\n");
    fprintf(fp,"origin %.0f %.0f\n", map->vminx * 1e3, map->vminy * 1e3);
    fprintf(fp,"width %.0f\n", (map->vmaxx - map->vminx) * 1e3);
    fprintf(fp,"height %.0f\n", (map->vmaxy - map->vminy) * 1e3);
    for (i = 0; i < map->num_segments; i++)
    {
      fprintf(fp,"%.0f %.0f %.0f %.0f\n",
             1e3*map->segments[i].x0,
             1e3*map->segments[i].y0,
             1e3*map->segments[i].x1,
             1e3*map->segments[i].y1);
    }

    fclose(fp);
  }

  // Shutdown
  playerc_map_unsubscribe(map);
  playerc_map_destroy(map);
  playerc_client_disconnect(client);
  playerc_client_destroy(client);

  return 0;
}

int 
parse_args(int argc, char** argv)
{
  int i;
  int max = argc-1;

  if(argc < 2)
    return(-1);
  fname = argv[max];

  for(i=1;i<max;i++)
  {
    if(!strcmp(argv[i],"-h"))
    {
      if(++i >= max)
        return(-1);
      host = argv[i];
    }
    else if(!strcmp(argv[i],"-i"))
    {
      if(++i >= max)
        return(-1);
      idx = atoi(argv[i]);
    }
    else if(!strcmp(argv[i],"-p"))
    {
      if(++i >= max)
        return(-1);
      port = atoi(argv[i]);
    }
    else if(!strcmp(argv[i],"-m"))
    {
      if(++i >= max)
        return(-1);
      if(!strcmp(argv[i],"v"))
        omap = 0;
      else
        omap = 1;
    }
    else
      return(-1);
  }
  return(0);
}

void
create_map_image(playerc_map_t* map, const char* fname, const char* fmt)
{
  GdkPixbuf* pixbuf;
  guchar* pixels;
  int i,j;

  g_type_init();

  pixels = (guchar*)malloc(sizeof(unsigned char) * 3 *
                           map->width * map->height);
  assert(pixels);

  for(j=0; j < map->height; j++)
  {
    for(i=0; i < map->width; i++)
    {
      if(map->cells[PLAYERC_MAP_INDEX(map,i,j)] == -1)
      {
        pixels[(map->width * 
                (map->height - j-1) + i)*3] = 255;
        pixels[(map->width * 
                (map->height - j-1) + i)*3+1] = 255;
        pixels[(map->width * 
                (map->height - j-1) + i)*3+2] = 255;
      }
      else if(map->cells[PLAYERC_MAP_INDEX(map,i,j)] == 0)
      {
        pixels[(map->width * 
                (map->height - j-1) + i)*3] = 100;
        pixels[(map->width * 
                (map->height - j-1) + i)*3+1] = 100;
        pixels[(map->width * 
                (map->height - j-1) + i)*3+2] = 100;
      }
      else
      {
        pixels[(map->width * 
                (map->height - j-1) + i)*3] = 0;
        pixels[(map->width * 
                (map->height - j-1) + i)*3+1] = 0;
        pixels[(map->width * 
                (map->height - j-1) + i)*3+2] = 0;
      }
    }
  }
  
  // create the pixbuf
  g_assert((pixbuf = gdk_pixbuf_new_from_data(pixels,
                                              GDK_COLORSPACE_RGB,
                                              FALSE,
                                              8,
                                              map->width, 
                                              map->height, 
                                              3*map->width,
                                              NULL,
                                              NULL)));

  if(!gdk_pixbuf_save(pixbuf, fname, fmt, NULL, NULL))
    puts("failed to save image file; sorry.");

  g_object_unref((GObject*)pixbuf);
}
