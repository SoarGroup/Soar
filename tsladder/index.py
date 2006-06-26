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

if action == None:
	welcome_page(action, userconfig)

if action == "managetanks":
	managetanks_page(action, userconfig)

if action == "upload":
	tankname = theform['tankname'].value
	tankfile = theform['tankfile'].value
	tankfilename = theform['tankfile'].filename
	source = theform['source'].value
	upload_page(action, userconfig, tankname, tankfilename, tankfile, source)

if action == "deletetank":
	tank = theform['tank'].value
	delete_tank(action, userconfig, tank)
	managetanks_page(action, userconfig)
	
if action == "viewtank":
	tank = theform['tank'].value
	view_tank_page(action, userconfig, tank)

if action == "viewfile":
	tank = theform['tank'].value
	file = theform['file'].value
	view_file_page(action, userconfig, tank, file)

if action == "mirrormatch":
	tank = theform['tank'].value
	mirrormatch_page(action, userconfig, tank)
