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


print "Content-type: text/html\n\n"

print "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
print "<html lang='en'><head><title>JTSL: Home</title><link href='/tsladder/jtsl.css' rel='stylesheet' type='text/css'/></head>"
print "<body><h1>Java TankSoar Ladder</h1>"
print "<h2>Welcome " + userconfig['username'] + "</h2>"

print "<p><a href='/tsladder/index.cgi?login=logout'>Log out</a> | <a href='/tsladder/index.cgi?login=editaccount'>Edit account</a>",
if userconfig['admin'] == "3":
	print " | <a href='/tsladder/index.cgi?login=admin'>Admin</a>"
print "</p>"

form = cgi.FieldStorage()

if action == None:
	welcome_page()

print "<p>Action is '" + action + "'</p>"

if action == "upload":
	upload_page(action, userconfig)
