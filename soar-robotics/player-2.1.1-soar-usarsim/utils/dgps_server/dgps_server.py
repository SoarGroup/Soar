#!/usr/bin/env python

# Desc: Application for collecting DGPS (RTCM) corrections from a base
# station.  Raw packets from the base station are re-transmitted over
# a multicast UDP socket.
#
# Author: Andrew Howard
# Date: 6 Aug 2003

import os, termios, select
import socket
import sys
import time
import getopt


# DGPS server class
class DGPSServer:
    """ Collect DGPS corrections from a base-station and broadcast
    them over the network using UDP.  """

    def __init__(self, serial_port, udp_ip, udp_port, options):

        self.serial_port = serial_port
        self.udp_ip = udp_ip
        self.udp_port = udp_port
        self.options = options

        self.udp_open()
        self.serial_open()
        return


    def main(self):
        """Serve up corrections forever."""

        print 'waiting for DGPS data...'

        counter = 0
        while 1:

            if '--test-udp' in self.options:
                time.sleep(1)
                data = 'test msg %d\0' % counter
            else:
                data = self.serial_read()

            self.udp_write(data)

            counter += 1
            print 'sending data on %s %d -- %d %d   \r' % \
                  (self.udp_ip, self.udp_port, counter, len(data)),
            sys.stdout.flush()

        return


    def udp_open(self):
        """Create a multicast UDP socket."""

        self.udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp_sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1)
        return


    def udp_write(self, msg):
        """Write a data packet."""

        self.udp_sock.sendto(msg, 0, (self.udp_ip, self.udp_port))
        return


    def serial_open(self):
        """Open and configure the serial port."""

        self.serial_fd = os.open(self.serial_port, os.O_RDWR)

        attr = termios.tcgetattr(self.serial_fd)
        attr[4] = termios.B4800
        attr[6][termios.VMIN] = 1
        attr[6][termios.VTIME] = 0
        termios.tcsetattr(self.serial_fd, termios.TCSAFLUSH, attr)
        return


    def serial_read(self):
        """Read packets from the serial port."""

        p = select.poll()
        p.register(self.serial_fd, select.POLLIN)

        # Block until we get one byte
        data = os.read(self.serial_fd, 1)

        # Read following bytes, allow for a 100ms gap between messages
        while p.poll(100):
            data += os.read(self.serial_fd, 1)

        return data
    


if __name__ == '__main__':

    serial_port = '/dev/ttyS0'
    udp_ip = '225.0.0.43'
    udp_port = 7778
    options = []

    # Get cmd line args
    (opts, args) = getopt.getopt(sys.argv[1:], '', ['test-udp'])
    for opt in opts:
        options.append(opt[0])

    server = DGPSServer(serial_port, udp_ip, udp_port, options)

    try:
        server.main()
    except KeyboardInterrupt:
        print 

    
    
