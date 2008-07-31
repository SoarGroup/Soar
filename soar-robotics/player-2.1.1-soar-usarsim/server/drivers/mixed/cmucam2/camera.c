#include <replace/replace.h>
#include "camera.h"

color_config range;

/************************************************************************
			    *** WRITE CHECK ***
**************************************************************************/
/* Description: This function writes a command to the camera and checks if the
                write was done successfully by checking camera's response.
   Parameters:  fd: serial port file descriptor
                msg: the command to be send to the camera
   Returns:     1: if the write was successful
                0: otherwise
*/
int write_check(int fd, char *msg, int respond_size)
{
	char respond[5];

	// write the command to the camera
	if( write(fd, msg, strlen(msg)) != (int)strlen(msg) )
	{
		printf( "Cmucam2: writing to serial device failed.\n" );
		return 0;
	}

	if( get_bytes(fd, respond, respond_size) < 1 )
	{
		printf( "Cmucam2: get bytes failed\n" );
		return 0;
	}

	// If NCK is returned, there was an error in writing
	if(respond[0] == 'N')
	{
		printf("Cmucam2: received NCK!\n");
		return 0;
	}
	return 1;
} 

int power(int fd, int on)
{
	if(on)
		return write_check(fd, "CP 1\r", 5);
	return write_check(fd, "CP 0\r", 5);
}

/**************************************************************************
			                      *** SET IMAGER CONFIG ***
**************************************************************************/
/* Description: This function sets the camera's internal resiger values
                for controlling image qualities.
   Parameters:  fd: serial port file descriptor
                player_blobfinder_imager_config: a Player packet containing
                information for camera's internal register:
                contrast, brightness, color mode, Exposure
   Returns:     1: If the command was successfully sent to the camera
                0: Otherwise
*/

int set_imager_config(int fd, imager_config ic)
{
   int value[8], size = 0;                 // The numbers used in the command:
   char command[26];                       // ex. CR 5 255 19 33
   if(ic.contrast != -1)                   // If ther is a change set the values
   {
	   value[size++] = CONTRAST;
	   value[size++] = ic.contrast;
   }
   if(ic.brightness != -1)
   {
	   value[size++] = BRIGHTNESS;
	   value[size++] = ic.brightness;
   }
   if(ic.colormode != -1)
   {
	   value[size++] = COLORMODE;
	   if(ic.colormode == 0)
		   value[size++] = RGB_AWT_OFF;
	   if(ic.colormode == 1)
		   value[size++] = RGB_AWT_ON;
	   if(ic.colormode == 2)
		   value[size++] = YCRCB_AWT_OFF;
	   if(ic.colormode == 3)
		   value[size++] = YCRCB_AWT_ON;
   }
   if(ic.autogain != -1)
   {
	   value[size++] = AUTOGAIN;
	   if(ic.autogain == 0)
		   value[size++] = AUTOGAIN_OFF;
	   if(ic.autogain == 1)
		   value[size++] = AUTOGAIN_ON;
   }
   // Put the values into camera's command format:
   // ex. CR 6 105 18 44
   make_command("CR ", value, size, command);
   return write_check(fd, command, 5);         // send the command to the camera
}

/**************************************************************************
		number[i] = c;	                      *** GET T PACKET ***
**************************************************************************/
/* Description: This function puts the camera's output during tracking into
                a T packet, which contrains information about the blob.
   Parameters:  fd: serial port file descriptor
                tpacket: the packet that will contain the blob info
   Returns:     void
*/
int get_t_packet(int fd, packet_t *tpacket)
{
	char tpack_chars[T_PACKET_LENGTH];
	read_t_packet(fd, tpack_chars);             // read the output of the camera
	return set_t_packet(tpacket, tpack_chars);  // convert it into T packet
}

/**************************************************************************
			                      *** POLL MODE ***
**************************************************************************/
/* Description: This functions determines whether the camera should send a
                continuous stream of packets or just one packet.
   Parameters:  fd: serial port file descriptor
                on: if on == 1, only one packet is send
                    if on == 0, a continuous stream of packets is send
   Returns:     1: If the command was successfully sent to the camera
                0: Otherwise
*/
int poll_mode(int fd, int on)
{
	if(on)
		return write_check(fd, "PM 1\r", 5);
	else
		return write_check(fd, "PM 0\r", 5);
}
   
/**************************************************************************
			                      *** SET SERVO POSITION ***
**************************************************************************/
/* Description: This functions sets the servo position given the servo number
                and the angle (note: angle = 0 denotes servo position = 128
                in terms of camera's values)
   Parameters:  number[i] = c;fd: serial port file descriptor
                servo_num: the servo which we are setting the position
                I am using 0:pan  1:tilt
   Returns:     1: If the command was successfully sent to the camera
                0: Otherwise
*/
int set_servo_position(int fd, int servo_num, int angle)
{
	// change the angle into camera's format
	int position = ZERO_POSITION + angle;
	// for servo position. I am using angle 0
	char comm[10];
	// corresponding to the default servo pos. 128
	int value[] = {servo_num, position};
	// generate the command using the values
	make_command("SV ", value, sizeof(value)/sizeof(int), comm);
	printf("servo %d new position: %d\n", servo_num, angle);
	return write_check(fd, comm, 5);      // write the command to the camera
}

/**************************************************************************
			                      *** MAKE COMMAND ***
**************************************************************************/
/* Description: This function gets a sets of values and a camera command header
                number[i] = c;to generate the command for the camera.
   Parameters:  cmd: the command header, for example SF or CR (see CMUcam 2 user
                     guide)
                n: the set of values to be used in the command
                size: the number of values used
                full_command: the final command in characters to be send to the 
                              camera
   Returns:     void
*/
void make_command(char *cmd, int *n, size_t size, char *full_command)
{
	char value[3];                 // the values are all withing 3 digits
	int length, i;
	for(i = 0; i < 10; i++)        // set all to null so that if there are
		full_command[i] = '\0';    // unsed characters at the end, the camera
                                   // does not complain about the command.
                                   // there is probably a better way to do this!
	strcat(full_command, cmd);     // attach the command header, ex. SF
	for(i = 0; i < (int)size; i++) // for all the values, convert them into char
	{                              // and attach them to the end of the command
		length = sprintf(value, "%d", n[i]);    // plus a space
		strcat(full_command, value);
		strcat(full_command, " ");
	}
	strcat(full_command, "\r");    // attach the return character to the end
}

/**************************************************************************
			                      *** OPEN PORT ***
**************************************************************************/
/* Description: This function opens the serial port for communication with the 
                camera.
   Parameters:  NONE
   Returns:     the file descriptor
*/
int open_port(char *devicepath)
{
	int fd = open( devicepath, O_RDWR );             // open the serial port
	struct termios term;
	struct pollfd fds[1];
	char cam_response[5];
 
	if( tcgetattr( fd, &term ) < 0 )                 // get device attributes
	{
		puts( "Cmucam2: unable to get device attributes.");
		return -1;
	}
  
	cfmakeraw( &term );
	cfsetispeed( &term, B115200 );                   // set baudrate to 115200
	cfsetospeed( &term, B115200 );
  
	if( tcsetattr( fd, TCSAFLUSH, &term ) < 0 )
	{
		puts( "Cmucam2: unable to set device attributes");
		return -1;
	}

  // Make sure queue is empty
	tcflush(fd, TCIOFLUSH);
  
	write(fd, "\r", 1);
	fds[0].fd = fd;
	fds[0].events = 1;
	poll(fds, 1, 100);
	if(!fds[0].revents)
	{
		printf("ERROR: CMUCAM2 IS OFF!\n");
		return -1;
	}
	get_bytes(fd, cam_response, 5);

	return fd;
}

/**************************************************************************
			        *** CLOSE PORT ***
**************************************************************************/
/* Description: This function closes the serial port.
   Parameters:  fd: serial port handler
   Returns:     void
*/
void close_port(int fd)
{
	close(fd);
}

/**************************************************************************
			        *** GET BYTES  ***
**************************************************************************/
/* Description: This function reads a specified number of bytes from the serial 
                port
   Parameters:  fd: serial port handler, buf: bytes read, len: bytes to read
   Returns:     0: if could not read the number of bytes specified  1: otherwise
*/
int get_bytes(int fd, char *buf, size_t len)
{
	int bytes_read = 0, ret_val;
	while(bytes_read < (int)len)
	{
		ret_val = read(fd, buf+bytes_read, len-bytes_read);
		if(ret_val < 0)
		{
			perror("Cmucam2: getting bytes failed.\n");
			return 0;
		}
		else if(ret_val > 0)
			bytes_read += ret_val;
	}
	return bytes_read;
}
    
/**************************************************************************
		        *** GET SERVO POSITION  ***
**************************************************************************/
/* Description: This function gets the position of the specified servo.
   Parameters:  fd: serial port handler, servo_num: the servo number whose size 
                    is required
   Returns:     the position of the servo
*/
int get_servo_position(int fd, int servo_num)
{  
	int servo_position;
	int i;
	char number[3];
	char c = 0;

	if(servo_num)               // set position of servo 1
		write_check(fd, "GS 1\r", 4);
	else		               // set position of servo 0
		write_check(fd, "GS 0\r", 4);

	for(i = 0; 1; i++)
	{
		read(fd, &c, 1);
		if(c == '\r')
			break;   
		number[i] = c;
	}
	read(fd, &c, 1);           // read the : at the end
	servo_position = atoi(number);
	return servo_position - ZERO_POSITION;
}

/**************************************************************************
		        *** TRACK BLOB  ***
**************************************************************************/
/* Description: This functions starts to Track a Color. It takes in the minimum 
                and maximum RGB values and outputs a type T packet. This packet 
                by default returns the middle mass x and y coordinates, the 
                bounding box, the number of pixles tracked, and a confidence 
                values.
   cc:          the min & max RGB values of the blob to be tracked.
*/
void track_blob( int fd, color_config cc )
{
	char cmd[28];
	int value[] = {cc.rmin, cc.rmax, cc.gmin, cc.gmax, cc.bmin, cc.bmax};
	range = cc;

	make_command("TC ", value, sizeof(value)/sizeof(int), cmd); 
	if(!write_check(fd, cmd, 4))
	{
		printf("ERROR; track color failed.\n");
		return;
	}
}

/**************************************************************************
		        *** GET SERVO POSITION  ***
**************************************************************************/
/* Description: The function enables/disables automatic servoing
   Parameters:  fd: serial port handler, on: determines whethere to enable(on=1)
                    or disable(on=0)
   Returns:    0: if the camera fails to write command  1: otherwise
*/
int auto_servoing(int fd, int on)
{
  // Enabling Auto Servoing Mode for both servos
	if(on)
	{
		if(!write_check(fd, "SM 15\r", 5))
		{
			printf("CMUCAM II ERROR: Enabling auto-servo failed.\n");
			return 0;
		}
		return 1;
	}
	else
	{
		if(!write_check(fd, "SM 0\r", 5))
		{
			printf("CMUCAM II ERROR: Disabling auto-servo faild.\n");
			return 0;
		}
		return 1;
	}
}

/**************************************************************************
		        *** STOP TRACKING  ***
**************************************************************************/
/* Description: The function stops the camera from tracking blobs and sending 
                data
   Parameters:  fd: serial port handler
   Returns:     none
*/
void stop_tracking(int fd)
{
	char c = 0;
	write(fd, "\r", 1);
	while(c != ':')
		read(fd, &c, 1);
}

/**************************************************************************
		        *** READ T PACKET  ***
**************************************************************************/
/* Description: The function reads a t-packet from camera, ex. when camera is 
                tracking
   Parameters:  fd: serial port handler, tpack_chars: the characters read
   Returns:     none
*/
void read_t_packet(int fd, char *tpack_chars)
{
	char c = 0;
	int k = 0;
	while(1)
	{
		read(fd, &c, 1);     
		tpack_chars[k++] = c;
		if(c == '\r')
			break;
	}
	if(tpack_chars[k-1] != '\r')
		printf("ERROR: reading T packet failed.\n");   
	tpack_chars[k] = '\0';
}

/**************************************************************************
		        *** READ T PACKET  ***
**************************************************************************/
/* Description: The function reads a t-packet from camera, ex. when camera is 
                tracking
   Parameters:  fd: serial port handler, output: 
   Returns:     none
*/
int set_t_packet( packet_t *tpacket, char tpack_chars[] )
{ 
	char packet_type;
	sscanf(tpack_chars, "%c %d %d %d %d %d %d %d %d", &packet_type, 
		   &tpacket->middle_x, &tpacket->middle_y,
		   &tpacket->left_x, &tpacket->left_y, &tpacket->right_x, 
		   &tpacket->right_y,
		   &tpacket->blob_area, &tpacket->confidence);
	if(packet_type != 'T')
	{
		printf("ERROR: cmucam2 failed to transmit t packet.\n");
		return 0;
	}
	return 1;
}

/**************************************************************************
		        *** READ F PACKET  ***
**************************************************************************/
/* Description: The function reads a f-packet from camera
   Parameters:  fd: serial port handler, fpack_chars: the characters read
   Returns:     none
*/
int read_f_packet (int fd, char *fpack_chars)
{
	char c = 0;
	int k = 0;
   
	while ((c != 0) || (c != 1))
	{
		read(fd, &c, 1);

		if (c == 0) {
			printf ("Cmucam2: frame grab failed.\n");
			return -1;
		}
    
		if (c == 1) {
			fpack_chars[k++] = c;
			read(fd, &c, 1);
			fpack_chars[k++] = c;
      //char xsize = c;
			read(fd, &c, 1);     
			fpack_chars[k++] = c;
      //char ysize = c;
      //printf ("Cmucam2: getting a frame of X=%d and Y=%d pixels.\n",
        //(uint8_t)xsize*2, 
        //(uint8_t)ysize);
			break;
		}
	}

	while(1)
	{
		read (fd, &c, 1);
		fpack_chars[k++] = c;
		if (c == 3) {
      //printf ("Cmucam2: got a frame of %d bytes.\n", k);
			break;
		}
	}
  
	if (fpack_chars[k-1] != 3) {
		printf ("ERROR: reading F packet failed.\n");   
		return -1;
	}
	fpack_chars[k] = '\0';
	return 0;
}

/**************************************************************************
		        *** READ IMAGE ***
**************************************************************************/
/* Description: This function gets an image from the camera using the 
                specified channel as a filter.
   Parameters:  fd: serial port handler, chan_num: the channel number
   Returns:     the image as an F packet
*/
int read_image (int fd, int chan_num, packet_f *fpacket)
{  
	char fpack_chars [F_PACKET_LENGTH];
	switch (chan_num)
	{
		case 0:
		{
			write_check (fd, "SF 0\r", 4);
			break;
		}
		case 1:
		{
			write_check (fd, "SF 1\r", 4);
			break;
		}
		case 2:
		{
			write_check (fd, "SF 2\r", 4);
			break;
		}
		case -1:
		{
			write_check (fd, "SF \r", 4);
			break;
		}
		default:
		{
			printf ("Cmucam2: invalid channel number!\n");
			break;
		}
	}
   
  
	if (read_f_packet (fd, fpack_chars) != 0)
		return -1;

	return set_f_packet (fpacket, fpack_chars, chan_num);
}

/**************************************************************************
		        *** SET F PACKET  ***
**************************************************************************/
int set_f_packet (packet_f *fpacket, char fpack_chars[], int chan_num)
{ 
	fpacket->first = (uint8_t)fpack_chars[0];
	fpacket->xsize = (uint8_t)fpack_chars[1];
	fpacket->ysize = (uint8_t)fpack_chars[2];
  
	switch (chan_num)
	{
		case -1:
		{
			int i = 0;
			int j = 0;
       
			for (i = 0; i < IMAGE_HEIGHT; i++)
			{
				fpacket->rows[i].rowbyte = 
						(uint8_t)fpack_chars[3 + (i*IMAGE_WIDTH/2)];
				for (j = 0; j < IMAGE_WIDTH/2; j++)
				{
					fpacket->rows[i].rgb[j].r = (uint8_t)fpack_chars[4 + i*j];
					fpacket->rows[i].rgb[j].g = (uint8_t)fpack_chars[5 + i*j];
					fpacket->rows[i].rgb[j].b = (uint8_t)fpack_chars[6 + i*j];
				}
			}
  
			fpacket->last = (uint8_t)
					fpack_chars[3 + IMAGE_HEIGHT*(IMAGE_WIDTH/2*3 + 1)];

			break;
		}
		default:
		{
			int i = 0;
			int j = 0;
      
			for (i = 0; i < IMAGE_HEIGHT; i++)
			{
				fpacket->rows[i].rowbyte = 
						(uint8_t)fpack_chars[3 + (i*IMAGE_WIDTH/2)];
				for (j = 0; j < IMAGE_WIDTH/2; j++)
				{
					fpacket->rows[i].rgb[j].r = (uint8_t)fpack_chars[4 + i*j];
					fpacket->rows[i].rgb[j].g = (uint8_t)fpack_chars[4 + i*j];
					fpacket->rows[i].rgb[j].b = (uint8_t)fpack_chars[4 + i*j];
				}
			}
  
			fpacket->last = (uint8_t)
					fpack_chars[3 + IMAGE_HEIGHT*(IMAGE_WIDTH/2 + 1)];
			break;
		}
	}
	return 0;
}
