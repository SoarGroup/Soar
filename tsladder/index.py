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

#from functions import *

print "Content-type: text/html\n\n"

print "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
print "<html lang='en'><head><title>Java TankSoar Ladder</title></head>"
print "<body><h1>Java TankSoar Ladder</h1>"

print "<a href='/tsladder/index.cgi?login=logout'>Log out</a><br />"
print "<a href='/tsladder/index.cgi?login=editaccount'>Edit account</a><br />"
print "<a href='/tsladder/index.cgi?login=admin'>Admin</a><br />"

print "</html>"

#form = cgi.FieldStorage()
#
#if not (form.has_key("action")):
#	welcome_page()
#	sys.exit()
#
#action = form["action"].value
#
#if action == "login":
#	pass
#
#elif action == "logout":
#	pass
#
#elif action == "register":
#	if form.has_key("confirm"):
#		userid = None
#		if form.has_key("userid"):
#			userid = form["userid"].value
#		
#		email = None
#		if form.has_key("email"):
#			email = form["email"].value
#			
#		# TODO: Make sure userid is unique and legal
#		# if not legal: register_page(userid, email)
#		# Send confirmation code to email address
#		if send_confirmation(userid, email) == None:
#			confirm_page()
#		else:
#			print "<p><font color='red'>Sending of confirmation email failed. Check email address.</p>"
#			register_page(userid, email)
#	else:
#		register_page()
#		
#elif action == "confirm":
#	if form.has_key("code"):
#		code = form["code"].value
#		if code == "abc123":
#			print "<p>Confirmation successful.</p>"
#			welcome_page()
#		else:
#			print "<p><font color='red'>Confirmation unsuccessful.</p>"
#	else:
#		welcome_page()
#		
#else:
#	print "<p><font color='red'>Unknown action: ", action, "</font></p>"
#	welcome_page()
#
#if action == "upload":
#	if form["userid"] == None:
#		print "<font color='red'>You must enter a User ID</font>"
#		upload_page()
#	elif form["tankid"] == None: 
#		print "<h1>User ID:", form["userid"], "</h1>"
#		save_tank()
#		print "<h1>Saved tank: ", form["upfile"].filename, "</h1>"
