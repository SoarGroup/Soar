#!/usr/bin/python

import cgi
import cgitb; cgitb.enable()
#import cgitb; cgitb.enable(display=0, logdir="/tmp")

import functions

print "Content-type: text/html\n\n"
print "<h1>Java TankSoar Ladder</h1>"

form = cgi.FieldStorage()

if not (form.has_key("action")):
	welcome_page()

elif form["action"].value == "upload":
	if form["userid"] == None:
		print "<font color='red'>You must enter a User ID</font>"
		upload_page()
	elif form["tankid"] == None: 
		print "<h1>User ID:", form["userid"], "</h1>"
		save_tank()
		print "<h1>Saved tank: ", form["upfile"].filename, "</h1>"
		
else:
	print "<font color='red'>Unknown action: ", form["action"], "</font>"

raise SystemExit

