#!/usr/bin/python

import os
pipe = os.popen( 'uname -p', 'r' )

processor = pipe.read().strip()
print '\'%s\'' % ( processor, )

if processor != 'powerpc':
    print "processor is not powerpc"
