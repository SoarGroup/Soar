#!/usr/bin/env python

# Simple parser to convert Player-format logfiles to CARMEN-format
# logfiles.
#    Brian Gerkey <brian@...>
#    Hauke Strasdat

# NOTES:
#   - So far, this parser handles only position (ODOM) and laser (FLASER)
#     messages.  This is sufficient for most mapping tasks.
#  
#   - Both 1-degree and 0.5-degree laser scans are handled (downsampled to
#     CARMEN's 1-degree format).
#
#   - Only a 180-degree FOV is allowed.
#
#   - I have guessed at some details of the CARMEN log format, but I'm
#     pretty sure that I got the data correct
#
#   - x-offset of the laser is handled correctly now. A warning appears if
#     a y/yaw offset is uded in the player logfile


import string
import sys
import fileinput
import math

USAGE = 'USAGE: playerlog2carmenlog.y <player.log> <carmen.log>'

if __name__ == '__main__':

  if len(sys.argv) != 3:
    print USAGE
    sys.exit(-1)

  infilename = sys.argv[1]
  outfilename = sys.argv[2]

  # Read in the entire file
  instream = fileinput.input(infilename)

  outfile = open(outfilename, 'w+')

  x = 0.000000
  y = 0.000000
  theta = 0.000000

  offset_x = 0.0 

  host = 'nohost'

  for line in instream:
    if line[0] == '#':
      continue
    lsplit = string.split(line)
    if len(lsplit) < 6:
      print 'Discarding invalid line: ' + line
      continue

    type = lsplit[3]
    msgtype = int(lsplit[5])
    msgsubtype = int(lsplit[6])

    # only want data messages
    if type == 'laser' and msgtype == 4:

      offset_x = float(lsplit[7])
      offset_y = float(lsplit[8])
      offset_theta = float(lsplit[9])
      print "laser offset:", offset_x, offset_y, offset_theta
      if offset_y != 0 or offset_theta != 0:
        print "WARNING: Can not deal with y or theta offset! Values ignored!"

      continue
        

    elif msgtype != 1:
      print 'Skipping message with type ' + `msgtype`
      continue


    # we're interested in odometry
    if type == 'position2d':
      if msgsubtype != 1:
        print 'Skipping position2d message with subtype ' + `msgsubtype`
        continue
      header = 'ODOM'
      time = lsplit[0]
      x = float(lsplit[7])
      y = float(lsplit[8])
      theta = float(lsplit[9])
      tv = lsplit[10]
      rv = lsplit[11]
      accel = '0.000'
      outfile.write(header + ' ' + str(x) + ' ' + str(y) + ' ' + str(theta) + ' ' + tv + ' ' + rv + ' ' + accel + ' ' + time + ' ' + host + ' ' + time + '\n')
    # we're also interested in laser scans
    elif type == 'laser':
        
      if msgsubtype != 1:
        print 'Skipping laser message with subtype ' + `msgsubtype`
        continue
      header = 'FLASER'
      time = lsplit[0]
      min_angle = lsplit[8]
      max_angle = lsplit[9]
      if (min_angle != '-1.5708') or (max_angle != '+1.5708'):
        print 'Sorry, CARMEN requires 180-degree FOV for laser'
        sys.exit(-1)
      num_readings = int(lsplit[12])
      if (num_readings != 181) and (num_readings != 361):
        print 'Sorry, I can only convert 1-deg and 0.5-deg resolution logs'
        sys.exit(-1)
      i = 0
      scanstr = ''
      while i < (num_readings - 1):
        range = lsplit[13 + 2*i]
        scanstr += range + ' '
        i += num_readings/180



      l_x = x + offset_x*math.cos(theta)
      l_y = y + offset_x*math.sin(theta)

      outfile.write(header + ' 180 ' + scanstr +
                    str(l_x) + ' ' + str(l_y) + ' ' + str(theta) + ' ' +
                    str(x) + ' ' + str(y) + ' ' + str(theta) + ' ' +
                    time + ' ' + host + ' ' + time + '\n')
    else:
      #print 'Ignoring line of type ' + type
      pass 

