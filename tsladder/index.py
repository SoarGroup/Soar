#!/usr/bin/python -u

import cgi
import cgitb; cgitb.enable()
#import cgitb; cgitb.enable(display=0, logdir="/tmp")

import sys
import os
sys.path.append(os.path.abspath('modules'))

from configobj import ConfigObj
from pathutils import *
from cgiutils import *

userdir = 'users/'
theform = cgi.FieldStorage()
from logintools import login
action, userconfig = login(theform, userdir)

from functions import *

form = cgi.FieldStorage()

if action == None:
	welcome_page(action, userconfig)

if action == "upload":
	upload_page(action, userconfig)
