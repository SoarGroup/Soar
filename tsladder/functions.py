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
        	ret += " | <a href='" + thisscript + "?login=admin'>Admin</a> | <a href='" + thisscript + "?action=resetstats'>Reset Stats</a>"
	ret += "</p>\n"
	return ret
	
def welcome_page(action, userid, cursor):
	username = get_username(userid, cursor)
	
	page = readfile('templates/header.html')
	page += account_links(username)
	page += readfile('templates/menu.html')
	page += readfile('templates/footer.html')

	stats = "<table border='1'>\n"
	stats += "<tr><td colspan='7'><center>Max updates: 10000, winning score: 100</center></td></tr>\n"
	stats += "<tr><td>Tank</td><td>Wins</td><td>Losses</td><td>Draws</td><td>Unfinished</td><td>Points per Match</td><td>Ladder Points</td></tr>\n"
	cursor.execute("SELECT * FROM tanks ORDER BY ladderpoints DESC")
	for tank in cursor.fetchall():
		username = get_username(tank[1], cursor)
		stats += "<tr><td>"
		stats += username + "." + tank[2]
		stats += "</td><td>"
		stats += str(tank[3])
		stats += "</td><td>"
		stats += str(tank[4])
		stats += "</td><td>"
		stats += str(tank[5])
		stats += "</td><td>"
		stats += str(tank[6])
		stats += "</td><td>"
		stats += str(tank[7])
		stats += "</td><td>"
		stats += str(tank[11])
		stats += "</td></tr>\n"
	stats += "</table>"
	
	page = page.replace('**stats table**', stats)
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
			tanktable += tank[2] + " (" + tank[8] + ")"
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
	username = get_username(userid, cursor)

	cursor.execute("SELECT fighting FROM tanks WHERE tankid=%s", (tankid,))
	fighting, = cursor.fetchone()
	fighting = int(fighting)
	if fighting:
		print "Content-type: text/html\n"
		print readfile('templates/header.html')
		print account_links(username)
		print "<font color='red'>" + tankname + " is currently fighting, try again in a little while.</font>"
		sys.exit()

	cursor.execute("DELETE FROM tanks WHERE tankid=%s", (tankid,))
	
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

import time

def mirrormatch_page(action, userid, cursor, tankid):
	print "Content-type: text/plain\n"
	
	username = get_username(userid, cursor)
	tank = get_tank(tankid, cursor)
	
	settings = readfile("templates/settings-mirrormatch.xml")
	
	settings = settings.replace('**red tank name**', tank[2] + "-red")
	settings = settings.replace('**blue tank name**', tank[2] + "-blue")
	settings = settings.replace('**red tank source**', "tanks/" + username + "/" + tank[2] + "/" + tank[8])
	settings = settings.replace('**blue tank source**', "tanks/" + username + "/" + tank[2] + "/" + tank[8])

	settingsfile = open("JavaTankSoar/tanksoar-default-settings.xml", 'w')
	settingsfile.write(settings)
	settingsfile.close()
	
	cwd = os.getcwd()
	os.chdir('JavaTankSoar')
	os.unlink("TankSoarLog.txt")
	#output = os.popen('./fight.sh')
	os.spawnlp(os.P_NOWAIT, './fight.sh', './fight.sh')

	while not os.path.exists("TankSoarLog.txt"):
		time.sleep(1)
		
	output = open("TankSoarLog.txt")
	
	breakout = False
	while True:
		for line in output.readlines():
			sys.stdout.write(line)
			if line.startswith("Shutdown called."):
				breakout = True
				break
		if breakout:
			break;
		time.sleep(1)
	
	output.close()
	os.chdir(cwd)
	sys.exit()

def resetstats(cursor):
	cursor.execute("UPDATE tanks SET wins=0,losses=0,draws=0,pointspermatch=0,fighting=0,ladderpoints=0,unfinished=0")
	cursor.execute("DELETE FROM matches")
	return
