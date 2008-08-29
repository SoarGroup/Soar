#!/usr/bin/python

import os
pipe = os.popen( 'uname -p', 'r' )

processor = pipe.read().strip()
print '\'%s\'' % ( processor, )
