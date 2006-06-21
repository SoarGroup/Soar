#!/usr/bin/python

import cgi

# Dumps readable error output
import cgitb; cgitb.enable()
# Dump to file instead:
#import cgitb; cgitb.enable(display=0, logdir="/tmp")

print "Content-type: text/html\n\n"

form = cgi.FieldStorage()

def upload_page():
	print "<form method='POST' enctype='multipart/form-data' action='index.cgi'>"
	print "File to upload: <input type='file' name='upfile'><br>"
	print "<input type='submit' value='Upload'>"
	print "<input type='hidden' name='action' value='upload'>"
	print "</form>"
	return

def display_page():
	fileitem = form["upfile"]
	if fileitem.file:
		while True:
			line = fileitem.file.readline()
			if not line:
				break
			print line
	return

if not (form.has_key("action")):
	upload_page()
else:
	display_page()
raise SystemExit

