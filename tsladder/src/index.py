#!/usr/bin/python

import cgi

# Dumps readable error output
import cgitb; cgitb.enable()
#import cgitb; cgitb.enable(display=0, logdir="/tmp")

print "Content-type: text/html\n\n"
print "<h1>Java TankSoar Ladder</h1>"


def upload_page():
	print "<form method='POST' enctype='multipart/form-data' action='index.cgi'>"
	print "User ID: <input type='text' name='userid' /><br />"
	print "Tank ID: <input type='text' name='tankid' /><br />"
	print "Tank zip: <input type='file' name='upfile' /><br />"
	print "<input type='submit' value='Upload Tank' />"
	print "<input type='hidden' name='action' value='upload' />"
	print "</form>"
	return

def save_tank():
	f = open("tanks/" + form["upfile"].filename, 'w')
	f.write(form["upfile"].value)
	f.close()

form = cgi.FieldStorage()

if not (form.has_key("action")):
	upload_page()
	
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

