
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "rflex-info.h"
#include "rflex_commands.h"
#include "rflex-io.h"
#include "rflex_configs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include <pthread.h>
#include <unistd.h>
#include <rflex.h>


//holds data until someone wants to read it
typedef struct {
  int distance;
  int bearing;
  int t_vel;
  int r_vel;
  int num_sonars;
  int *ranges;
  int *oldranges;
  int num_bumpers;
  char *bumpers;
  int voltage;
  bool brake;
  int lcd_x;
  int lcd_y;
  unsigned char * lcd_data;
  int num_ir;
  unsigned char * ir_ranges;
  int home_bearing;
  int home_bearing_found;
  
} rflex_status_t;


static int clear_incoming_data(int fd);


static rflex_status_t status;

//finds the sign of a value
static int sgn( long val )
{
  if (val<0) {
    return(0);
  } else {
    return(1);
  }
}

/* COMPUTE CRC CODE */

static int computeCRC( unsigned char *buf, int nChars )
{ 
  int i, crc;
  if (nChars==0) {
    crc = 0;
  } else {
    crc = buf[0];
    for (i=1; i<nChars; i++) {
      crc ^= buf[i];
    }
  }
  return(crc);
}

/* CONVERSION BYTES -> NUM */

#if 0 

static unsigned int convertBytes2UInt8( unsigned char *bytes )
{
  unsigned int i;
  memcpy( &i, bytes, 1 );
  return(i);
}

#endif

static unsigned int convertBytes2UInt16( unsigned char *bytes )
{
  unsigned int i;
  memcpy( &i, bytes, 2 );
  return(htons(i));
}


static unsigned long convertBytes2UInt32( unsigned char *bytes )
{
  unsigned long i;
  memcpy( &i, bytes, 4 );
  return(htonl(i));
}

/* CONVERSION NUM -> BYTES */

static void convertUInt8( unsigned int i, unsigned char *bytes )
{
  memcpy( bytes, &i, 1 );
}

#if 0 

static void convertUInt16( unsigned int i, unsigned char *bytes )
{
  uint16_t conv;
  conv = htonl( i );
  memcpy( bytes, &conv, 2 );
}

#endif

static void convertUInt32( unsigned long l, unsigned char *bytes )
{
  uint32_t conv;
  conv = htonl( l );
  memcpy( bytes, &conv, 4 );
}

//sends a command to the rflex
static void cmdSend( int fd, int port, int id, int opcode, int len, unsigned char *data )
{
  int i;
  static unsigned char cmd[MAX_COMMAND_LENGTH];
  /* START CODE */
  cmd[0] = 0x1b;
  cmd[1] = 0x02;
  /* PORT */
  cmd[2] = (unsigned char) port;
  /* ID */
  cmd[3] = (unsigned char) id;
  /* OPCODE */
  cmd[4] = (unsigned char) opcode;
  /* LENGTH */
  cmd[5] = (unsigned char) len;
  /* DATA */
  for (i=0; i<len; i++) {
    cmd[6+i] = data[i];
  }
  /* CRC */
  cmd[6+len] = computeCRC( &(cmd[2]), len+4 );    /* END CODE */
  cmd[6+len+1] = 0x1b;
  cmd[6+len+2] = 0x03;

  //pthread_testcancel();
  writeData( fd, cmd, 9+len );

	// Some issues with commands not being recognised if sent too rapidly
	// (too small a buffer on recieving end?
	// So we delay for a bit, specifically we wait until specified amount of
	// time has passed without recieving a packet. This roughtly approximates
	// to waiting till the command has finshed being executed on the robot
	int count;
	timeval now;
	timeval start = {0,0};
	do  
	{
		count = clear_incoming_data(fd);
  		gettimeofday(&now,NULL);
		if (count > 0 ) 
			start = now;
		count = (now.tv_sec - start.tv_sec) * 1000000 + (now.tv_usec - start.tv_usec);

               // release somewhat so other threads can run.
               usleep(500);
	} while (count < 10000);

  
  //pthread_testcancel();
}

void rflex_sonars_on( int fd )
{
  unsigned char data[MAX_COMMAND_LENGTH];
  convertUInt32( (unsigned long) rflex_configs.sonar_echo_delay/*30000*/, &(data[0]) );
  convertUInt32( (unsigned long) rflex_configs.sonar_ping_delay/*0*/, &(data[4]) );
  convertUInt32( (unsigned long) rflex_configs.sonar_set_delay/*0*/, &(data[8]) );
  convertUInt8(  (unsigned int) 2, &(data[12]) );
  cmdSend( fd, SONAR_PORT, 4, SONAR_RUN, 13, data );
}

void rflex_sonars_off( int fd )
{
  unsigned char data[MAX_COMMAND_LENGTH];
  convertUInt32( (long) 0, &(data[0]) );
  convertUInt32( (long) 0, &(data[4]) );
  convertUInt32( (long) 0, &(data[8]) );
  convertUInt8(  (int) 0, &(data[12]) );
  cmdSend( fd, SONAR_PORT, 4, SONAR_RUN, 13, data );
}

void rflex_digital_io_on( int fd, int period )
{
  unsigned char data[MAX_COMMAND_LENGTH];
  convertUInt32( (long) period, &(data[0]) );
  cmdSend( fd, DIO_PORT, 0, DIO_REPORTS_REQ, 4, data );
}

void rflex_digital_io_off( int fd )
{
  unsigned char data[MAX_COMMAND_LENGTH];
  convertUInt32( (long) 0, &(data[0]) );
  cmdSend( fd, DIO_PORT, 4, DIO_REPORTS_REQ, 4, data );
}

void rflex_ir_on( int fd )
{
  unsigned char data[MAX_COMMAND_LENGTH];
  convertUInt32( (long) 0, &(data[0]) );
  convertUInt32( (long) 70, &(data[4]) );
  convertUInt32( (long) 10, &(data[8]) );
  convertUInt32( (long) 20, &(data[12]) );
  convertUInt32( (long) 150, &(data[16]) );
  convertUInt8( (unsigned char) 2, &(data[20]) );
  cmdSend( fd, IR_PORT, 0, IR_RUN, 21, data );
}

void rflex_ir_off( int fd )
{
  unsigned char data[MAX_COMMAND_LENGTH];
  convertUInt32( (long) 0, &(data[0]) );
  convertUInt32( (long) 0, &(data[4]) );
  convertUInt32( (long) 0, &(data[8]) );
  convertUInt32( (long) 0, &(data[12]) );
  convertUInt32( (long) 0, &(data[16]) );
  convertUInt8( (unsigned char) 0, &(data[20]) );
  cmdSend( fd, IR_PORT, 0, IR_RUN, 21, data );
}
void rflex_brake_on( int fd )
{
  cmdSend( fd, MOT_PORT, 0, MOT_BRAKE_SET, 0, NULL );
}

void rflex_brake_off( int fd )
{
  cmdSend( fd, MOT_PORT, 0, MOT_BRAKE_RELEASE, 0, NULL );
}

void rflex_motion_set_defaults( int fd )
{
  cmdSend( fd, MOT_PORT, 0, MOT_SET_DEFAULTS, 0, NULL );
}

void rflex_odometry_on( int fd, long period )
{ 
  unsigned char data[MAX_COMMAND_LENGTH];
  convertUInt32( period, &(data[0]) );         /* period in ms */
  convertUInt32( (long) 3, &(data[4]) );       /* mask */
  cmdSend( fd, MOT_PORT, 0, MOT_SYSTEM_REPORT_REQ, 8, data );
}

void rflex_odometry_off( int fd )
{ 
  unsigned char data[MAX_COMMAND_LENGTH];
  convertUInt32( (long) 0, &(data[0]) );       /* period in ms */
  convertUInt32( (long) 0, &(data[4]) );       /* mask */
  cmdSend( fd, MOT_PORT, 0, MOT_SYSTEM_REPORT_REQ, 8, data );
}


void rflex_set_velocity( int fd, long tvel, long rvel,
			   long acceleration )
{
  unsigned char data[MAX_COMMAND_LENGTH];

  long utvel;long urvel;

  utvel=labs(tvel);
  urvel=labs(rvel);

  // ** workaround for stupid hardware bug, cause unknown, but this works
  // ** with minimal detriment to control
  // ** avoids all values with 1b in highest or 3'rd highest order byte
  
  // 0x1b is part of the packet terminating string
  // which is most likely what causes the bug

  // ** if 2'nd order byte is 1b, round to nearest 1c, or 1a
  if((urvel&0xff00)==0x1b00){
    // ** if lowest order byte is>127 round up, otherwise round down 
    urvel=urvel&0xffff0000|(urvel&0xff>127?0x1c00:0x1aff);
  }

  // ** if highest order byte is 1b, round to 1c, otherwise round to 1a
  if((urvel&0xff000000)==0x1b000000){
    // ** if 3'rd order byte is>127 round to 1c, otherwise round to 1a
    urvel=urvel&0x00ffffff|(urvel&0xff0000>127?0x1c000000:0x1aff0000);
  }

  convertUInt8( (long) 0,                 &(data[0]) );       /* forward motion */
  convertUInt32( (long) labs(utvel),        &(data[1]) );       /* abs trans velocity*/
  convertUInt32( (long) acceleration,    &(data[5]) );       /* trans acc */
  convertUInt32( (long) STD_TRANS_TORQUE, &(data[9]) );       /* trans torque */
  convertUInt8( (long) sgn(tvel),         &(data[13]) );      /* trans direction */

  cmdSend( fd, MOT_PORT, 0, MOT_AXIS_SET_DIR, 14, data );

  convertUInt8( (long) 1,                 &(data[0]) );       /* rotational motion */
  convertUInt32( (long) labs(urvel),        &(data[1]) );       /* abs rot velocity  */
  /* 0.275 rad/sec * 10000 */ 
  convertUInt32( (long) STD_ROT_ACC,      &(data[5]) );       /* rot acc */
  convertUInt32( (long) STD_ROT_TORQUE,   &(data[9]) );       /* rot torque */
  convertUInt8( (long) sgn(rvel),         &(data[13]) );      /* rot direction */

  cmdSend( fd, MOT_PORT, 0, MOT_AXIS_SET_DIR, 14, data );
}

void rflex_stop_robot( int fd, int deceleration)
{
  rflex_set_velocity( fd, 0, 0, deceleration);
}

int rflex_open_connection(char *device_name, int *fd)
{
  RFLEX_Device   rdev;

  strncpy( rdev.ttyport, device_name, MAX_NAME_LENGTH);
  rdev.baud           = 115200;
  rdev.databits       = 8;
  rdev.parity         = N;
  rdev.stopbits       = 1;
  rdev.hwf            = 0;
  rdev.swf            = 0;
  
  printf("trying port %s\n",rdev.ttyport); 
  if (DEVICE_connect_port( &rdev )<0) {
    fprintf(stderr,"Can't open device %s\n",rdev.ttyport);
    return -1;
  } 

  *fd = rdev.fd;
  rflex_odometry_on(*fd, 100000);
  rflex_digital_io_on(*fd, 100000);
  rflex_motion_set_defaults(*fd);
  return 0;
}

int rflex_close_connection(int *fd)
{
  assert(fd);
  if (*fd < 0)
  	return -1;
  rflex_motion_set_defaults(*fd);
  rflex_odometry_off(*fd);
  rflex_digital_io_off(*fd);
  rflex_sonars_off(*fd);
  rflex_ir_off(*fd);
  
  printf("Closing rflex serial port\n");
  close(*fd);
  *fd=-1;

  return 0;
}


//processes a motor packet from the rflex - and saves the data in the
//struct for later use
static void parseMotReport( unsigned char *buffer )
{
  int rv, timeStamp, acc, trq;
  unsigned char axis, opcode;
    
  opcode = buffer[4];
  switch(opcode) {
  case MOT_SYSTEM_REPORT:
    rv        = convertBytes2UInt32(&(buffer[6]));
    timeStamp = convertBytes2UInt32(&(buffer[10]));
    axis      = buffer[14];
    if (axis == 0) {
      status.distance = convertBytes2UInt32(&(buffer[15]));
      status.t_vel = convertBytes2UInt32(&(buffer[19]));
    } else if (axis == 1) {
      status.bearing = convertBytes2UInt32(&(buffer[15]));
      status.r_vel = convertBytes2UInt32(&(buffer[19]));
    }
    acc       = convertBytes2UInt32(&(buffer[23]));
    trq       = convertBytes2UInt32(&(buffer[27]));
    break;
  default:
    break;
  }
}

//processes a dio packet from the rflex - and saves the data in the
//struct for later use, dio report includes bump sensors...
static void parseDioReport( unsigned char *buffer )
{
	unsigned long timeStamp;
	unsigned char opcode, length, address;
	unsigned short data;

	opcode = buffer[4];
	length = buffer[5];


	switch(opcode) 
	{
		case DIO_REPORT:
			if (length < 6)
   			{
       			fprintf(stderr, "DIO Data Packet too small\n");
       			break;
   			}
   			timeStamp = convertBytes2UInt32(&(buffer[6]));
   			address = buffer[10];
   			data = convertBytes2UInt16(&(buffer[11]));

			// Check for the heading home event;
			if(rflex_configs.heading_home_address == address)
			{
				if(status.home_bearing_found)
					break;
				static bool found_first = false;
				static int first_home_bearing = 0;
				if (found_first)
				{
					if ((first_home_bearing - status.bearing) > 0.785* rflex_configs.odo_angle_conversion)
					{
						first_home_bearing=static_cast<int> (first_home_bearing-rflex_configs.odo_angle_conversion*2*M_PI);
					}
					else if ((first_home_bearing - status.bearing) < 0.785* rflex_configs.odo_angle_conversion)
					{
						first_home_bearing=static_cast<int> (first_home_bearing+rflex_configs.odo_angle_conversion*2*M_PI);
					}
					if (abs(first_home_bearing - status.bearing) > 0.01 * rflex_configs.odo_angle_conversion)
					{
						rflex_configs.home_on_start = false;
						status.home_bearing=status.bearing > first_home_bearing? status.bearing:first_home_bearing;
						status.home_bearing_found = true;
						printf("Home bearing found %d\n",status.home_bearing);
					}
				}
				else
				{
						first_home_bearing=status.bearing;
						found_first = true;		
				}
				break;
			}


			if(BUMPER_ADDR == rflex_configs.bumper_style)
			{
			   // on the b21r the bump packets are address 0x40 -> 0x4D, there are some other dio packets
			   // but dont know what they do so we throw them away
			   
			   // check if the dio packet came from a bumper packet
			   if ((address < rflex_configs.bumper_address) || (address >= (rflex_configs.bumper_address+status.num_bumpers)))
			   {
			      // not bumper
			      fprintf(stderr,"(dio) address = 0x%02x ",address);
			      break;
			   }
			   else
			   {
			      // is bumper
			      //fprintf(stderr,"(bump) address = 0x%02x ",address);
			      // assign low data byte to the bumpers (16 bit DIO data, low 4 bits give which corners or the panel are 'bumped')
			      status.bumpers[address - rflex_configs.bumper_address] = data & 0x0F;
			   }
			}
			else
			{
			   // on the magellan pro the bump packets are address 0x40 and 0x41. Each bits of these address
			   // match one bumper
			   
			   // Check if the dio paquet came from a bumper packet
			   if ((address < rflex_configs.bumper_address) || (address >= (rflex_configs.bumper_address+(status.num_bumpers/8))))
			   {
			      // not bumper
			      fprintf(stderr,"(dio) address = 0x%02x ",address);
			      break;
			   }
			   else
			   {
			      // is bumper
			      fprintf(stderr,"(bump) address = 0x%02x ",address);

			      // Loop for each bit
			      for (int i=0; i<8; i++)
			      {
				 // assign each bit of the data to a bumper. 
				 status.bumpers[((address - rflex_configs.bumper_address) * 8 )+ i] = data & (0x01 << i);
			      }
			   }
			}
    		break;
   		default:
     		break;
   	}
}

// Processes the IR sensor report
 static void parseIrReport( unsigned char *buffer )
 {
  // unsigned long timeStamp;
   unsigned char opcode, length;//, address;
   //unsigned short data;

   opcode = buffer[4];
   length = buffer[5];

//	for (int i = 0; i < length; ++i)
	//	printf("%02x",buffer[i+6]);
		//printf("\n");


	// allocate ir storage if we havent already
	// must be a better place to do this but it works
   if (status.num_ir== 0 && rflex_configs.ir_poses.poses_count > 0)
   {
       status.ir_ranges = new unsigned char[rflex_configs.ir_poses.poses_count];
       if (status.ir_ranges != NULL)
           status.num_ir = rflex_configs.ir_poses.poses_count;
	else
		fprintf(stderr,"Error allocating ir range storage in rflex status\n");
   }

   switch(opcode) 
   {
   case IR_REPORT:
   {
	   if (length < 1)
	   {
    	   fprintf(stderr, "IR Data Packet too small\n");
	       break;
	   }
   
		// get number of IR readings to make
	   	unsigned char pckt_ir_count = buffer[6];  
	   	if (pckt_ir_count < rflex_configs.ir_base_bank)
   			pckt_ir_count = 0;
		else
			pckt_ir_count -= rflex_configs.ir_base_bank;
   
		if (pckt_ir_count > rflex_configs.ir_bank_count)
   			pckt_ir_count = rflex_configs.ir_bank_count;
		

		// now actually read the ir data
		int ir_data_index = 0;
		for (int i=0; i < rflex_configs.ir_bank_count && ir_data_index < status.num_ir; ++i)
		{
			for (int j = 0; j < rflex_configs.ir_count[i] && ir_data_index < status.num_ir; ++j,++ir_data_index)
			{
				// work out the actual offset in teh packet 
				//(current bank + bank offfset) * bank data block size + current ir sensor + constant offset
				int data_index = (rflex_configs.ir_base_bank + i) * 13 + j + 11;
				status.ir_ranges[ir_data_index] = buffer[data_index];
			}
		}

    	break;
	}
   default:
	    break;
   }
   
   // debug print
/*   for (int i = 0; i < status.num_ir ; ++i)
   	printf("%02x ", status.ir_ranges[i]);
	printf("\n");*/
}


//processes a sys packet from the rflex - and saves the data in the
//struct for later use, sys is primarily used for bat voltage & brake status
static void parseSysReport( unsigned char *buffer )
{
   unsigned long timeStamp, voltage;
   unsigned char opcode, length, brake;


   opcode = buffer[4];
   length = buffer[5];
   

	switch (opcode)
	{
		case SYS_LCD_DUMP:
			// currently designed for 320x240 screen on b21r
			// stored in packed format
			if (length < 6)
			{
				fprintf(stderr, "Got bad Sys packet (lcd)\n");
				break;
			}
			
			unsigned char lcd_length, row;
			timeStamp=convertBytes2UInt32(&(buffer[6]));
			row = buffer[10];
			lcd_length = buffer[11];
			if (row > status.lcd_y || lcd_length > status.lcd_x)
			{
				fprintf(stderr,"LCD Data Overflow\n");
				break;	
			}
			
			memcpy(&status.lcd_data[row*status.lcd_x],&buffer[12],lcd_length);
			
			// if we got whole lcd dump to file
/*			if (row == 239)
			{
				FILE * fout;
				if ((fout = fopen("test.raw","w"))!=0)
				{
					for (int y=0; y<status.lcd_y; ++y)
					{
						for (int x=0; x<status.lcd_x;++x)					
						{
							unsigned char Temp = status.lcd_data[y*status.lcd_x + x];
							for (int i = 0; i < 8; ++i)
							{
								if ((Temp >> i) & 0x01)

									fprintf(fout,"%c",0x0);
								else
									fprintf(fout,"%c",0xFF);
							}
						}
						fprintf(fout,"\n");
					}
				}
				fclose(fout);
			}*/
									
						
			break;
			
		case SYS_STATUS:
			if (length < 9)
			{
				fprintf(stderr, "Got bad Sys packet (status)\n");
				break;
			}
			timeStamp=convertBytes2UInt32(&(buffer[6]));
			// raw voltage measurement...needs calibration offset added
			voltage=convertBytes2UInt32(&(buffer[10]));
			brake=buffer[14];
			
			status.voltage = voltage;
			status.brake = brake;
			break;
			
		default:
			fprintf(stderr,"Unknown sys opcode recieved\n");
	}
}

//processes a sonar packet fromt the rflex, and saves the data in the
//struct for later use
//HACK - also buffers data and filters for smallest in last AGE readings
static void parseSonarReport( unsigned char *buffer )
{

  unsigned int sid;
 
  int x,smallest;

  int count, retval, timeStamp;
  unsigned char opcode, dlen;
    
  opcode = buffer[4];
  dlen   = buffer[5];

  status.num_sonars=rflex_configs.max_num_sonars;
  switch(opcode) {
  case SONAR_REPORT:
	if (status.ranges == NULL || status.oldranges == NULL)
		return;
    retval    = convertBytes2UInt32(&(buffer[6]));
    timeStamp = convertBytes2UInt32(&(buffer[10]));
    count = 0;
    while ((8+count*3<dlen) && (count<256) && (count < rflex_configs.num_sonars)) {
      sid = buffer[14+count*3];
      //shift buffer
      for(x=0;x<rflex_configs.sonar_age-1;x++)
	status.oldranges[x+1+sid*rflex_configs.sonar_age]=status.oldranges[x+sid*rflex_configs.sonar_age];
      //add value to buffer
      smallest=status.oldranges[0+sid*rflex_configs.sonar_age]=convertBytes2UInt16( &(buffer[14+count*3+1]));
      //find the smallest
      for(x=1;x<rflex_configs.sonar_age;x++)
	if(smallest>status.oldranges[x+sid*rflex_configs.sonar_age])
	  smallest=status.oldranges[x+sid*rflex_configs.sonar_age];
      //set the smallest in last sonar_age as our value
      status.ranges[sid] = smallest;
      count++;
    }
    break;
  default:
    break;
  }
}

//processes a joystick packet fromt the rflex, and sets as command if 
// joystick command enabled
static void parseJoyReport( int fd, unsigned char *buffer )
{
	static bool JoystickWasOn = false;

	int x,y;
  unsigned char opcode, dlen, buttons;
    
  opcode = buffer[4];
  dlen   = buffer[5];

  switch(opcode) {
  case JSTK_GET_STATE:
  	if (dlen < 13)
	{	
		fprintf(stderr,"Joystick Packet too small\n");
		break;
	}
	x = convertBytes2UInt32(&buffer[10]);
	y = convertBytes2UInt32(&buffer[14]);
	buttons = buffer[18];
	
	if ((buttons & 1) == 1)
	{
		JoystickWasOn = true;
		rflex_set_velocity(fd,(long) M2ARB_ODO_CONV(y * rflex_configs.joy_pos_ratio),(long) RAD2ARB_ODO_CONV(x * rflex_configs.joy_ang_ratio),(long) M2ARB_ODO_CONV(rflex_configs.mPsec2_trans_acceleration));    
		RFLEX::joy_control = 5;
	}
	else if (JoystickWasOn)
	{
		JoystickWasOn = false;
		rflex_set_velocity(fd,0,0,(long) M2ARB_ODO_CONV(rflex_configs.mPsec2_trans_acceleration));    
		RFLEX::joy_control = 5;	
	}

    break;
  default:
    break;
  }
}


//parses a packet from the rflex, and decides what to do with it
static int parseBuffer( int fd,unsigned char *buffer, unsigned int len )
{
  unsigned int port, dlen, crc;

  port   = buffer[2];
  dlen   = buffer[5];

  if (dlen+8>len) {
    return(0);
  } else {
    crc    = computeCRC( &(buffer[2]), dlen+4 );
    if (crc != buffer[len-3])
      return(0);
    switch(port) {
    case SYS_PORT:
//      fprintf( stderr, "(sys)" );
      parseSysReport( buffer );
      break;
    case MOT_PORT:
      parseMotReport( buffer );
      break;
    case JSTK_PORT:
		parseJoyReport( fd, buffer );
      break;
    case SONAR_PORT:
      parseSonarReport( buffer );
      break;
    case DIO_PORT:
	    //fprintf( stderr, "(dio)" );
		parseDioReport( buffer );
        break;
    case IR_PORT:
		parseIrReport( buffer);
//      fprintf( stderr, "(ir)" );
      break;
    default:
      break;
    }
  }
  return(1);
}

// returns number of commands parsed
static int clear_incoming_data(int fd)
{
  unsigned char buffer[4096];
  int len;
  int bytes;
	int count = 0;

  // 32 bytes here because the motion packet is 34. No sense in waiting for a
  // complete packet -- we can starve because the last 2 bytes might always
  // arrive with the next packet also, and we never leave the loop. 

  while ((bytes = bytesWaiting(fd)) > 32) {
  	count ++;
    //pthread_testcancel();
    waitForAnswer(fd, buffer, &len);
    //pthread_testcancel();
    parseBuffer(fd, buffer, len);
  }
  return count;
}

//returns the odometry data saved in the struct
void rflex_update_status(int fd, int *distance,  int *bearing, 
			   int *t_vel, int *r_vel)
{
  clear_incoming_data(fd);

  *distance = status.distance;
  if(status.home_bearing_found)
	  *bearing = status.bearing-status.home_bearing;
  else
	  *bearing = status.bearing;
  *t_vel = status.t_vel;
  *r_vel = status.r_vel;
}

/* gets actual data and returns it in ranges
 * NOTE - actual mappings are strange
 * each module's sonar are numbered 0-15
 * thus for 4 sonar modules id's are 0-64
 * even if only say 5 or 8 sonar are connected to a given module
 * thus, we record values as they come in 
 * (data comes in in sets of n modules, 3 if you have 3 modules for example)
 *
 * note the remmapping done using the config parameters into 0-n, this mapping
 * should come out the same as the mapping adverstised on your robot
 * this if you put the poses in that order in the config file - everything
 * will line up nicely
 *
 * -1 is returned if we get back fewer sonar than requested 
 * (meaning your parameters are probobly incorrect) otherwise we return 0 
 */
int rflex_update_sonar(int fd,int num_sonars, int * ranges){
  int x;
  int y;
  int i;

  clear_incoming_data(fd);

  //copy all data
  i=0;
  y=0;
  for(x=0;x<rflex_configs.num_sonar_banks;x++)
    for(y=0;y<rflex_configs.num_sonars_in_bank[x];y++)
	{
      ranges[i]=status.ranges[x*rflex_configs.num_sonars_possible_per_bank+y];
	  if (ranges[i] > rflex_configs.sonar_max_range)
	  	ranges[i] = rflex_configs.sonar_max_range;
	  i++;
	 }
  if (i<num_sonars){
    fprintf(stderr,"Requested %d sonar only %d supported\n",num_sonars,y);
    num_sonars = y;
    return -1;
  }
 return 0;
}

// copies data from internal bumper list to the proper rflex bumper list
void rflex_update_bumpers(int fd, int num_bumpers, char *values)
{
	clear_incoming_data(fd);
	// allocate bumper storage if we havent already
	// must be a better place to do this but it works
	// *** watch out this is duplicated ***
/*	if (status.num_bumpers != rflex_configs.bumper_count)
	{
   		delete status.bumpers;
		status.bumpers = new char[rflex_configs.bumper_count];
		if (status.bumpers != NULL)
			status.num_bumpers = rflex_configs.bumper_count;
	}*/

	if (num_bumpers > status.num_bumpers) 
	{
    	fprintf(stderr,"Requested more bumpers than available.\n");
    	num_bumpers = status.num_bumpers;
  	}

	memcpy(values, status.bumpers, num_bumpers*sizeof(char));
}


// copies data from internal bumper list to the proper rflex bumper list
void rflex_update_ir(int fd, int num_irs, 
			    unsigned char *values)
{
  clear_incoming_data(fd);

  if (num_irs > status.num_ir) {
    //fprintf(stderr,"Requested more ir readings than available. %d of %d\n", num_irs,status.num_ir );
    num_irs = status.num_ir;
  }

  memcpy(values, status.ir_ranges, num_irs*sizeof(char));
}


//returns the last battery, timestamp and brake information
void rflex_update_system( int fd , int *battery, 
			     int *brake)
{
  //cmdSend( fd, SYS_PORT, 0, SYS_LCD_DUMP, 0, NULL );
  cmdSend( fd, SYS_PORT, 0, SYS_STATUS, 0, NULL );
  if (rflex_configs.use_joystick)
  {
  	cmdSend( fd, JSTK_PORT, 0, JSTK_GET_STATE, 0, NULL);
  }
  
 
  clear_incoming_data(fd);
  
  *battery = status.voltage;
  //timestamp = timestamp;
  *brake = status.brake;
}



/* TODO - trans_pos rot_pos unused... AGAIN, fix this 
 * same effects are emulated at a higher level - is it possible to
 * do it here?
 */
void rflex_initialize(int fd, int trans_acceleration,
		      int rot_acceleration,
		      int trans_pos,
		      int rot_pos)
{
  unsigned char data[MAX_COMMAND_LENGTH];
  int x;

  data[0] = 0;
  convertUInt32( (long) 0, &(data[1]) );          /* velocity */ 
  convertUInt32( (long) trans_acceleration, &(data[5]) );       
                                                  /* acceleration */ 
  convertUInt32( (long) 0, &(data[9]) );      /* torque */ 
  data[13] = 0;

  cmdSend( fd, MOT_PORT, 0, MOT_AXIS_SET_DIR, 14, data );

  data[0] = 1;
  convertUInt32( (long) 0, &(data[1]) );          /* velocity */ 
  convertUInt32( (long) rot_acceleration, &(data[5]) );       
                                                  /* acceleration */ 
  convertUInt32( (long) 0, &(data[9]) );      /* torque */ 
  data[13] = 0;

  cmdSend( fd, MOT_PORT, 0, MOT_AXIS_SET_DIR, 14, data );

  //mark all non-existant (or no-data) sonar as such
  //note - this varies from MAX_INT set when sonar fail to get a valid echo.
  status.ranges=(int*) malloc(rflex_configs.max_num_sonars*sizeof(int));
  status.oldranges=(int*) malloc(rflex_configs.max_num_sonars*rflex_configs.sonar_age*sizeof(int));
  for(x=0;x<rflex_configs.max_num_sonars;x++)
    status.ranges[x]=-1;
	
	// initialise the LCD dump array
	status.lcd_data=new unsigned char[320*240/8];
	if (status.lcd_data != NULL)
	{
		status.lcd_x=320/8;
		status.lcd_y=240;
	}
	
	// allocate bumper storage if we havent already
	// must be a better place to do this but it works
	// *** watch out this is duplicated ***
	if (status.num_bumpers != rflex_configs.bumper_count)
	{
   		delete status.bumpers;
		status.bumpers = new char[rflex_configs.bumper_count];
		if (status.bumpers != NULL)
			status.num_bumpers = rflex_configs.bumper_count;
		for (int i = 0; i < status.num_bumpers; ++i)
			status.bumpers[i] = 0;
	}
	status.home_bearing_found=false;
	
}












