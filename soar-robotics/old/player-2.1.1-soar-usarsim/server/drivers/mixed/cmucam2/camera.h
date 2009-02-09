#ifndef CAMERA_H_
#define CAMERA_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* These should NOT be redefined. */
//typedef unsigned int     uint32_t;
//typedef unsigned short   uint16_t;
//typedef unsigned char    uint8_t;

/* Instead #include playerconfig.h, which gets them in a portable way */
#include "playerconfig.h"


/**************************************************************************
			    *** CONSTANST ***
**************************************************************************/
#define IMAGE_WIDTH      174    // the width of the frame camera sends
#define IMAGE_HEIGHT     143    // the height of the frame camera sends
#define CONTRAST         5      // camera's contrast register #
#define BRIGHTNESS       6      // camera's brightness register #
#define COLORMODE        18     // camera's colormode register #
#define RGB_AWT_ON       44     // camera's RGB auto white balance on
#define RGB_AWT_OFF      40     // camera'sRGB auto white balance off
#define YCRCB_AWT_ON     36     // camera'sYCrCb auto white balance on
#define YCRCB_AWT_OFF    32     // camera'sYCrCb auto white balance off
#define AUTOGAIN         19     // camera's autogain register #
#define AUTOGAIN_ON      33     // camera's autogain on
#define AUTOGAIN_OFF     32     // camera's autogain off
#define ZERO_POSITION    128    // servos' middle position as defiend by camera
#define MIN_RGB          16     // camera's min rgb value
#define MAX_RGB          240    // camera's max rgb value
#define T_PACKET_LENGTH  50     // max length of T packet that camera returns
#define F_PACKET_LENGTH  37474  // max length of F packet that camera returns


/**************************************************************************
			                      *** T PACKET ***
**************************************************************************/
typedef struct                      // camera's output packet for tracking blobs
{
	int middle_x, middle_y;         // the blob entroid (image coords)
	int left_x;                     // the left most corner's x value
	int left_y;                     // the left msot corner's y value
	int right_x;                    // the right most corner's x vlaue
	int right_y;                    // the right most corner's y value
	int blob_area;                  // number of pixles int he tracked regtion,
                                    // scaled and capped at 255:(pixles+4)/8
	int confidence;                 // the (# of pixles/area)*256 of the bounded
                                    // rectangle and capped at 255
}packet_t; 

/**************************************************************************
			                      *** F PACKET ***
**************************************************************************/
typedef struct
{
	int r, g, b;
} rgb_type;

typedef struct
{
	int rowbyte;
	rgb_type rgb[IMAGE_WIDTH];
} row_type;

typedef struct
{
	int first;
	int xsize, ysize;
	row_type rows[IMAGE_HEIGHT];
	int last;
} packet_f; 

/**************************************************************************
			                    *** IMAGER CONFIG ***
**************************************************************************/
typedef struct           // camera's internal register controlling image quality
{ 
	uint8_t subtype;     // must be PLAYER_BLOBFINDER_REQ_SET_IMAGER_PARAMS.
	int16_t brightness;  // contrast:      -1 = no change.  (0-255)
	int16_t contrast;    // brightness:    -1 = no change.  (0-255)
	int8_t  colormode;   // color mode:    -1 = no change.
                         //                0  = RGB/auto white balance Off,
                         //                1  = RGB/AutoWhiteBalance On,
                         //                2  = YCrCB/AutoWhiteBalance Off,
                         //                3  = YCrCb/AWB On)
	int8_t  autogain;    // auto gain:     -1 = no change.
		                 //                0  = off, o
                         //                1  = on.
} imager_config;

/**************************************************************************
			                    *** CONFIG CONFIG ***
**************************************************************************/
typedef struct
{ 
	uint8_t subtype;                 // must be PLAYER_BLOBFINDER_REQ_SET_COLOR.
	int16_t rmin, rmax;              // RGB minimum and max values (0-255)
	int16_t gmin, gmax;
	int16_t bmin, bmax;
} color_config;

/**************************************************************************
			                        *** RGB ***
**************************************************************************/
typedef struct                          // RGB values
{
	int red;
	int green;
	int blue;
} rgb;

/**************************************************************************
			                    *** CONFIG CONFIG ***
**************************************************************************/
typedef struct                          // camera's image
{
	int width;
	int height;
	rgb **pixel;
} image;


/**************************************************************************
			                      *** FUNCTION PROTOTYPES ***
**************************************************************************/
int  get_t_packet(int fd, packet_t *tpacket);
int  set_imager_config(int fd, imager_config ic);
int  get_bytes(int fd, char *buf, size_t len);
int  open_port(char *devicepath);
void close_port(int fd);
void read_t_packet(int fd, char *tpackChars);
int  read_f_packet (int fd, char *fpackChars);
int  set_t_packet( packet_t *tpacket, char tpack_chars[] );
int  set_f_packet (packet_f *fpacket, char fpack_chars[], int chan_num);
int  set_servo_position(int fd, int servo_num, int angle);
int  get_servo_position(int fd, int servo_num);
void stop_tracking(int fd);
int  write_check(int fd, char *msg, int  respond_size);
int  poll_mode(int fd, int on);
void make_command(char *cmd, int *n, size_t size, char *fullCommand);
int  auto_servoing(int fd, int on);
void track_blob(int fd, color_config cc);
int  read_image (int fd, int chan_num, packet_f *fpacket);

#ifdef __cplusplus
}
#endif

#endif
