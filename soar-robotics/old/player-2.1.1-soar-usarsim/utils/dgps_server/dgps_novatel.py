#!/usr/bin/env python
#------------------------------------------------------------------------------
# dgps_novatel.py : setup Novatel DGPS reference station
#
#	- programmed by Boyoon Jung (boyoon@robotics.usc.edu)
#------------------------------------------------------------------------------
#import uspp.uspp

import os
import sys
import termios
import time


# GPS class
class NovatelGPS:
    """
    Novatel GPS device driver.
    """
    
    def __init__(self, device, baudrate):
        """
        initialize communication.
        """

        # open a serial port
        self.serial_open(device)

        # send a command
        self.serial_write('fix none\r\n')
        self.serial_write('log com1 bestposa ontime 1\r\n')
        return


    def __del__(self):
        """
        finalize communication.
        """

        #REMOVE self.serial_write('log com1 bestposa once\r\n')
        return


    def serial_open(self, device):
        """Open and configure the serial port."""

        self.serial_fd = os.open(device, os.O_RDWR)

        attr = termios.tcgetattr(self.serial_fd)
        attr[4] = termios.B9600
        attr[5] = termios.B9600        
        termios.tcsetattr(self.serial_fd, termios.TCSAFLUSH, attr)
        return


    def serial_write(self, data):
        """Write data to serial port."""

        os.write(self.serial_fd, data);
        return


    def serial_read(self, bytes):
        """Read data from serial port."""

        return os.read(self.serial_fd, bytes);


    def update(self):
        """
        parse a single sentence.
        """

        # read out a garbage
        while 1:
            sentence = self.serial_read(1);
            if sentence == '#':
                break

        # read a sentence
        sentence = ''
        while 1:
            sentence += self.serial_read(1);
            if sentence[-1] == '\r':
                sentence = sentence[:-10]    # no checksum info
                break

        # parse the sentence
        tokens = sentence.split(';')
        if len(tokens) < 2:
            return
        tokens = tokens[1].split(',')
        
        self.status = tokens[0]
        self.latitude = tokens[2]
        self.longitude = tokens[3]
        self.altitude = tokens[4]
        self.std_lat = tokens[7]
        self.std_lon = tokens[8]
        self.std_alt = tokens[9]
        return


    def fix(self):
        """
        fix the position of the reference station.
        """

        self.serial_write('unlogall\r\n')
                
        # change the interface mode of com1
        self.serial_write('interfacemode com1 novatel rtcm\r\n')

        # fix the position
        self.serial_write('fix position ' + self.latitude +
                          ' ' + self.longitude +
                          ' ' + self.altitude + '\r\n')

        # make it generate RTCM messages
        self.serial_write('dgpsrxid auto\r\n')
        self.serial_write('log com1 rtcm1 ontime 2\r\n')
        self.serial_write('log com1 rtcm3 ontime 5\r\n')
        self.serial_write('log com1 rtcm9 ontime 2\r\n')

        return


# main function
if __name__ == '__main__':

    # load default settings
    device = '/dev/ttyS0'
    baudrate = 9600

    # process the command line arguments
    if len(sys.argv) > 2: device = sys.argv[1]
    if len(sys.argv) > 3: baudrate = int(sys.argv[2])

    # initialize communication
    gps = NovatelGPS(device, baudrate)

    # wait until the GPS receiver is initialized
    try:
        sys.stderr.write('Initializing GPS receiver ')
        while 1:
            sys.stderr.write('.')
            gps.update()
            if gps.status == 'SOL_COMPUTED':
                sys.stderr.write(' Done.\n\n');
                break;
    except:
        print "Fail to initialize GPS receiver."
        sys.exit(0)

    # print the information
    try:
        while 1:
            gps.update()
            print 'Pose = (' + gps.latitude + ',' \
                  + gps.longitude + ',' \
                  + gps.altitude \
                  + ') STD = (' + gps.std_lat + ',' \
                  + gps.std_lon + ',' \
                  + gps.std_alt + ')'

    except KeyboardInterrupt:
        pass

    #except:
    #    print "."
    #    sys.exit(0)

    # Fix the gps coord
    gps.fix()

    print
    print 'Position is fixed to'
    print '    Latitude  : ' + gps.latitude
    print '    Longitude : ' + gps.longitude
    print '    Altitude  : ' + gps.altitude
    print
