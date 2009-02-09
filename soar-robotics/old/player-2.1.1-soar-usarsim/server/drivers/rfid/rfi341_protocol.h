/*
 Desc: Driver for the SICK RFI341 unit
 Author: Nico Blodow and Radu Bogdan Rusu
 Date: 9 Mar 2007
 CVS: $Id: rfi341_protocol.h 6499 2008-06-10 01:13:51Z thjc $
*/
#include <termios.h>
#include <sys/types.h>
#include <libplayercore/playercore.h>

#define BUF_SIZE 1024

#define STX 0x02
#define ETX 0x03
#define ACK 0x06
#define NAK 0x15
#define SYN 0x16
#define ESC 0x18

////////////////////////////////////////////////////////////////////////////////
class rfi341_protocol
{
  public:
    rfi341_protocol (const char* port_name, int debug_mode);

    // Creates socket, connects
    // Connects first at 'connect_speed'
    int Connect (int connect_speed);

    // but for transfer, we might want to use a different 'transfer_speed'
    int SetupSensor (int transfer_speed);

    int Disconnect ();

    // assembles a command and sends it on the wire
    int SendCommand (const char* cmd);
    // reads the result of a query from the device
    int ReadResult  ();
    player_rfid_data_t ReadTags ();

  private:
    // assembles STX's, message, checksum ready to be sent. Cool.
    int assemblecommand (unsigned char* command, int len);

    int number_of_tags;
    char **tags;

    // Initial serial port attributes
    struct termios initial_options;

    // Internal Parameters:
    int verbose;
    int fd;
    const char* port;
    int portspeed;

    // for reading:
    unsigned char buffer[4096];
    unsigned int bufferlength;
    int checksum;

    // for sending:
    unsigned char command[BUF_SIZE];
    int commandlength;
};
