#!/usr/bin/python

import cgi
import cgitb; cgitb.enable()
#import cgitb; cgitb.enable(display=0, logdir="/tmp")

import functions

print "Content-type: text/html\n\n"

title = "Java TankSoar Ladder"

print "<head><title>" + title + "</title></head>"
print "<h1>" + title + "</h1>"

form = cgi.FieldStorage()

if not (form.has_key("action")):
	functions.welcome_page()
	raise SystemExit

action = form["action"].value

if action == "login":
	pass

elif action == "logout":
	pass

elif action == "register":
	if form.has_key("confirm"):
		userid = None
		if form.has_key("userid"):
			userid = form["userid"].value
		
		email = None
		if form.has_key("email"):
			email = form["email"].value
			
		# TODO: Make sure userid is unique and legal
		# if not legal: functions.register_page(userid, email)
		# Send confirmation code to email address
		if functions.send_confirmation(userid, email) == None:
			functions.confirm_page()
		else:
			print "<p><font color='red'>Sending of confirmation email failed. Check email address.</p>"
			functions.register_page(userid, email)
	else:
		functions.register_page()
		
elif action == "confirm":
	if form.has_key("code"):
		code = form["code"].value
		if code == "abc123":
			print "<p>Confirmation successful.</p>"
			functions.welcome_page()
		else:
			print "<p><font color='red'>Confirmation unsuccessful.</p>"
	else:
		functions.welcome_page()
		
else:
	print "<p><font color='red'>Unknown action: ", action, "</font></p>"
	functions.welcome_page()

#
#if action == "upload":
#	if form["userid"] == None:
#		print "<font color='red'>You must enter a User ID</font>"
#		upload_page()
#	elif form["tankid"] == None: 
#		print "<h1>User ID:", form["userid"], "</h1>"
#		save_tank()
#		print "<h1>Saved tank: ", form["upfile"].filename, "</h1>"
