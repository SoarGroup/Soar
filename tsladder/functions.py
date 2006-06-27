#!/usr/bin/python

import sys
import os
import shutil
from pathutils import *
from cgiutils import *
import MySQLdb

thisscript = os.environ['SCRIPT_NAME']

def get_username(userid, cursor):
	cursor.execute("SELECT username FROM users WHERE userid=%s", (userid,))
	result = cursor.fetchone()
	return result[0]

def get_tank(tankid, cursor):
        cursor.execute("SELECT * FROM tanks WHERE tankid=%s", (tankid,))
        result = cursor.fetchone()
        return result

def account_links(username):
	ret = "<p>" + username + ": <a href='" + thisscript + "'>Home</a> | <a href='" + thisscript + "?action=managetanks'>Manage Tanks</a> | <a href='" + thisscript + "?login=logout'>Log out</a> | <a href='" + thisscript + "?login=editaccount'>Edit account</a>"
	if username == "admin":
        	ret += " | <a href='" + thisscript + "?login=admin'>Admin</a>"
	ret += "</p>\n"
	return ret
	
def welcome_page(action, userid, cursor):
	username = get_username(userid, cursor)
	
	page = readfile('templates/header.html')
	page += account_links(username)
	page += readfile('templates/menu.html')
	page += readfile('templates/footer.html')

	print "Content-type: text/html\n"
	print page
	sys.exit()

def managetanks_page(action, userid, cursor):
	username = get_username(userid, cursor)

	page = readfile('templates/header.html')
	page += account_links(username)
	page += readfile('templates/managetanks.html')
	page += readfile('templates/footer.html')

	cursor.execute("SELECT * FROM tanks WHERE userid=%s", (userid,))
	number_of_tanks = int(cursor.rowcount)

	tanktable = "<table border='1'>\n"
	if number_of_tanks < 1:
		tanktable += "<tr><td>No tanks.</td></tr>\n"
	else:
		for tank in cursor.fetchall():
			tanktable += "<tr><td>"
			tanktable += tank[2] + " (" + tank[7] + ")"
			tanktable += "</td><td><a href='" + thisscript + "?action=mirrormatch&tankid=" + str(tank[0]) + "'>MirrorMatch</a></td><td><a href='" + thisscript + "?action=viewtank&tankid=" + str(tank[0]) + "'>View</a></td><td><a href='" + thisscript + "?action=deletetank&tankid=" + str(tank[0]) + "'>Delete</a></td></tr>\n"
	tanktable += "\n</table>"

	page = page.replace('**tank table**', tanktable)
	page = page.replace('**upload form**', readfile('templates/uploadform.html'))
	page = page.replace('**this script**', thisscript)

	print "Content-type: text/html\n"
	print page
	sys.exit()
	
def delete_tank(action, userid, cursor, tankid):
	tankname = get_tank(tankid, cursor)[2]

	cursor.execute("DELETE FROM tanks WHERE tankid=%s", (tankid,))
	
	username = get_username(userid, cursor)
	
	top = "tanks/" + username + "/" + tankname
	shutil.rmtree(top)
	return

def upload_page(action, userid, cursor, tankname, tankfilename, tankfile, source):
	# Check for duplicates
	cursor.execute("SELECT tankname FROM tanks WHERE userid=%s AND tankname=%s", (userid, tankname,))
	duplicate = False
	if int(cursor.rowcount) > 0:
		duplicate = True
		
	username = get_username(userid, cursor)
	
	page = readfile('templates/header.html')
	page += account_links(username)
	
	if duplicate:
		page += "<p><font color='red'>Tank name '" + tankname + "' already exists, hit back and select a new tank name.</font></p>"
	else:
		#create the tank directory
		tankdir = "tanks/" + username + "/" + tankname
		os.makedirs(tankdir)

		#save the zip file
		file = open(tankdir + "/" + tankfilename, 'w')
		file.write(tankfile)
		file.close()

		#unzip the file
		cwd = os.getcwd()
		os.chdir(tankdir)
		os.system("/usr/bin/unzip " + tankfilename)
		os.chdir(cwd)

		# Make sure the file exists
		if not os.path.exists(tankdir + "/" + source):
			shutil.rmtree(tankdir)
			page += "<p><font color='red'>Could not find source file</font></p>"
		else:
			cursor.execute("INSERT INTO tanks (userid, tankname, sourcefile) VALUES(%s, %s, %s)", (userid, tankname, source,))
			page += "<p>Tank " + tankname + " successfully added.</p>"
			
	page += readfile('templates/footer.html')

	print "Content-type: text/html\n"
	print page
	sys.exit()

def view_tank_page(action, userid, cursor, tankid):
	print "Content-type: text/html\n"
	print readfile('templates/header.html')

	username = get_username(userid, cursor)
	
	print account_links(username)
	cwd = os.getcwd()
	tanksdir = "tanks/" + username
	os.chdir(tanksdir)

	tankname = get_tank(tankid, cursor)[2]
	
	list_files(tankid, tankname)
	os.chdir(cwd)

	print readfile('templates/footer.html')
	sys.exit()

def list_files(tankid, dir):
	for root, dirs, files in os.walk(dir):
		print "<h3>" + root + "</h3>"
		for name in files:
			if name.endswith(".soar") or name.endswith(".txt"):
				print "<a href='" + thisscript + "?action=viewfile&tankid=" + str(tankid) + "&file=" + os.path.join(root,name) + "'>" + name + "</a><br />"
			else:
				print name + "<br />"
	print
	return

def view_file_page(action, userid, cursor, tankid, file):
	print "Content-type: text/plain\n"
	if file.find("..") != -1:
		sys.exit()

	tankname = get_tank(tankid, cursor)[2]
	
	if not file.startswith(tankname):
		sys.exit()

	username = get_username(userid, cursor)
	print readfile("tanks/" + username + "/" + file)
	sys.exit()

def mirrormatch_page(action, userid, cursor, tankid):
	print "Content-type: text/plain\n"
	
	username = get_username(userid, cursor)
	tank = get_tank(tankid, cursor)
	
	settings = readfile("templates/settings.xml")
	
	settings = settings.replace('**tank1 name**', tank[2] + "1")
	settings = settings.replace('**tank2 name**', tank[2] + "2")
	settings = settings.replace('**tank1 source**', "tanks/" + username + "/" + tank[2] + "/" + tank[7])
	settings = settings.replace('**tank2 source**', "tanks/" + username + "/" + tank[2] + "/" + tank[7])

	settingsfile = open("JavaTankSoar/tanksoar-default-settings.xml", 'w')
	settingsfile.write(settings)
	settingsfile.close()
	
	cwd = os.getcwd()
	os.chdir('JavaTankSoar')
	os.system('./fight.sh')
	os.chdir(cwd)
	
	print readfile('JavaTankSoar/TankSoarLog.txt')
	sys.exit()
