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
	stats += "<tr><td colspan='10'><center>Max updates: 10000, winning score: 100</center></td></tr>\n"
	stats += "<tr align='center'><td>Tank</td><td>Total<br>Matches</td><td>Wins</td><td>Losses</td><td>Draws</td><td>Unfinished<br>Matches</td><td>Unfinished<br>%</td><td>Average<br>Points</td><td>Average<br>Ladder<br>Points</td><td>Rating</td></tr>\n"
	cursor.execute("SELECT * FROM tanks ORDER BY rating DESC")
	for tank in cursor.fetchall():
		username = get_username(tank[1], cursor)
		totalMatches = tank[3] + tank[4] + tank[5]
		avgLadderPoints = 0
		unfinishedPercent = 0
		if totalMatches > 0:
			avgLadderPoints = float(tank[11]) / totalMatches
			unfinishedPercent = (float(tank[6]) / totalMatches) * 100
		line = "<tr align='right'><td align='left'>%s</td><td><a href='%s?action=viewmatches&tankid=%d'>%d</a></td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%.1f%%</td><td>%.3f</td><td>%.3f</td><td>%d</td></tr>\n" % (username + "." + tank[2], thisscript, tank[0], totalMatches, tank[3], tank[4], tank[5], tank[6], unfinishedPercent, tank[7], avgLadderPoints, tank[12])
		stats += line
	stats += "</table>"
	
	page = page.replace('**stats table**', stats)
	print "Content-type: text/html\n"
	print page
	sys.exit()

def view_matches_page(action, userid, cursor, tankid):
	username = get_username(userid, cursor)
	
	page = readfile('templates/header.html')
	page += account_links(username)
	page += readfile('templates/menu.html')
	page += readfile('templates/footer.html')

	cursor.execute("SELECT tankname FROM tanks WHERE tankid=%s", (tankid,))
	(tankname,) = cursor.fetchone()
	selfname = username + "." + tankname

	stats = "<table border='1'>\n"
	stats += "<tr align='center'><td colspan='2'>Tank: %s</td></tr><tr align='center'><td>Opponent</td><td>Result</td></tr>\n" % (selfname,)
	haveMatch = False

	cursor.execute("SELECT * FROM matches WHERE red=%s", (tankid,))
	for match in cursor.fetchall():
		haveMatch = True
		opponentname = None
		try:
			opponenttank = get_tank(match[2], cursor)
			opponentname = get_username(opponenttank[1], cursor) + "." + opponenttank[2]
		except TypeError:
			opponentname = "(deleted)"

		result = "Draw"
		if match[3] != 0:
			if match[3] == tankid:
				result = "Win"
			else:
				result = "Loss"
		line = "<tr align='left'><td>%s</td><td align='center'><a href='?action=viewmatchlog&matchid=%d'>%s</a></td></tr>\n" % (opponentname, match[0], result)
		stats += line
        
	cursor.execute("SELECT * FROM matches WHERE blue=%s", (tankid,))
	for match in cursor.fetchall():
		haveMatch = True
		opponentname = None
		try:
			opponenttank = get_tank(match[1], cursor)
			opponentname = get_username(opponenttank[1], cursor) + "." + opponenttank[2]
		except TypeError:
			opponentname = "(deleted)"
			
		result = "Draw"
		if match[3] != 0:
			if match[3] == tankid:
				result = "Win"
			else:
				result = "Loss"
		line = "<tr align='left'><td>%s</td><td align='center'><a href='?action=viewmatchlog&matchid=%d'>%s</a></td></tr>\n" % (opponentname, match[0], result)
		stats += line
	
	if not haveMatch:
		stats += "<tr align='center'><td colspan='2'>No matches</td></tr>\n"

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
			#tanktable += "</td><td><a href='" + thisscript + "?action=mirrormatch&tankid=" + str(tank[0]) + "'>MirrorMatch</a></td><td><a href='" + thisscript + "?action=viewtank&tankid=" + str(tank[0]) + "'>View</a></td><td><a href='" + thisscript + "?action=deletetank&tankid=" + str(tank[0]) + "'>Delete</a></td></tr>\n"
			tanktable += "</td><td><a href='" + thisscript + "?action=viewtank&tankid=" + str(tank[0]) + "'>View</a></td><td><a href='" + thisscript + "?action=deletetank&tankid=" + str(tank[0]) + "'>Delete</a></td></tr>\n"
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
		os.system("/bin/chmod -R u+w *")
		grepresult = os.system("/bin/grep -Rq cmd *")
		os.chdir(cwd)

		# Make sure the file exists
		if not os.path.exists(tankdir + "/" + source):
			shutil.rmtree(tankdir)
			page += "<p><font color='red'>Could not find source file</font></p>"
		else:
			# Make sure there are no illegal commands
			if grepresult == 0:
				shutil.rmtree(tankdir)
				page += "<p><font color='red'>Detected 'cmd' directive in source file, please remove and re-upload</font></p>"
				
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

import gzip
def view_match_log_page(action, userid, cursor, matchid):
	print "Content-type: text/plain\n"
	# TODO: verify this is a number
	matchlog = gzip.open("tsladder-logs/elo/" + str(matchid) + ".txt.gz")
	for line in matchlog.readlines():
		sys.stdout.write(line)
	matchlog.close()
	sys.exit()

def resetstats(cursor):
	cursor.execute("UPDATE tanks SET wins=0,losses=0,draws=0,pointspermatch=0,fighting=0,ladderpoints=0,unfinished=0,rating=1000")
	cursor.execute("DELETE FROM matches")
	return
