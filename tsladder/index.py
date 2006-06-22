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

thisscript = os.environ.get('SCRIPT_NAME', '')
action = None
userdir = 'users/'
theform = cgi.FieldStorage()
from logintools import login
action, userconfig = login(theform, userdir, thisscript, action)

from functions import *

print "Content-type: text/html\n\n"

print "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
print "<html lang='en'><head><title>JTSL: Home</title><link href='/tsladder/jtsl.css' rel='stylesheet' type='text/css'/></head>"
print "<body><h1>Java TankSoar Ladder</h1>"
print "<h2>Welcome " + userconfig['username'] + "</h2>"

print "<a href='/tsladder/index.cgi?login=logout'>Log out</a> | <a href='/tsladder/index.cgi?login=editaccount'>Edit account</a>",
if userconfig['admin'] == "3":
	print " | <a href='/tsladder/index.cgi?login=admin'>Admin</a>"
print "<br />"

print "<p>You're early! There isn't any code here yet.</p>"

form = cgi.FieldStorage()

if not (form.has_key("action")):
	welcome_page()
else:
	print "<p>action is " + form["action"].value + "</p>"

	action = form["action"].value

#if action == "upload":
#	if form["userid"] == None:
#		print "<font color='red'>You must enter a User ID</font>"
#		upload_page()
#	elif form["tankid"] == None: 
#		print "<h1>User ID:", form["userid"], "</h1>"
#		save_tank()
#		print "<h1>Saved tank: ", form["upfile"].filename, "</h1>"

print "</body></html>"

