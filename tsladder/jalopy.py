#! /usr/bin/python -u

# jalopy.py
# A wrapper script for makepage.

# This script calls makepage with all the details set for running the 'jalopy' website.
# This allows it to be called without a 'site' parameter.
# These details can also be changed and this script renamed to allow multiple sites from one 'makepage'.

# Copyright Michael Foord 2004
# Part of jalopy
# http://www.voidspace.org.uk/python/jalopy.html

# Released subject to the BSD License
# Please see http://www.voidspace.org.uk/documents/BSD-LICENSE.txt

# For information about bugfixes, updates and support, please join the Pythonutils mailing list.
# http://voidspace.org.uk/mailman/listinfo/pythonutils_voidspace.org.uk
# Comments, suggestions and bug reports welcome.
# Scripts maintained at http://www.voidspace.org.uk/python/index.shtml
# E-mail fuzzyman@voidspace.org.uk

import cgi
import sys
import os
try:
    import cgitb
    cgitb.enable()
except ImportError:
    sys.stderr = sys.stdout

sys.path.append('../jalopy')
from makepage import *

#####################################################################
# This is the site specific stuff

thesite = 'jalopy'

#####################################################################
thisscript = os.environ.get('SCRIPT_NAME','')

if __name__ == '__main__':
    readconfig(thesite, thisscript)
    theform = cgi.FieldStorage()
    main(theform)
