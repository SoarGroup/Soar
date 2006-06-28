#!/usr/bin/python

import os
import sys
sys.path.append(os.path.abspath('modules'))

import time
import MySQLdb
import logging
from pathutils import *
import re

pid = os.fork()
if pid != 0:
	pidfile = file('tsladderd.pid', 'w')
	pidfile.write(str(pid))
	pidfile.close()
	sys.exit(0)

logging.basicConfig(level=logging.DEBUG, 
		    format='%(asctime)s %(levelname)s %(message)s',
		    filename='tsladderd.log',
		    filemode='w')

logging.info('tsladderd started (%d).', pid)

db = MySQLdb.connect(host="localhost", user="tsladder", passwd="75EbRL", db="tsladder")
cursor = db.cursor()
logging.info('Connected to database.')

sleepytime = 5

while True:
	cursor.execute("SELECT * FROM tanks ORDER BY RAND(NOW()) LIMIT 2")
	if int(cursor.rowcount) < 2:
		logging.warning('There is only one tank in the database.')
		logging.info("Sleeping for a bit.")
		time.sleep(sleepytime)
		continue

	redtank = None
	reduser = None
	bluetank = None
	blueuser = None

	for tank in cursor.fetchall():
		cursor.execute("SELECT username FROM users WHERE userid=%s", (tank[1],))
		username = cursor.fetchone()[0]
		status = "active"
		color = None
		if not tank[8]:
			status = "inactive"
	
		if redtank == None:
			color = "Red"
			redtank = tank
			reduser = username
		else:
			color = "Blue"
			bluetank = tank
			blueuser = username

		logging.info("%s: %s.%s: (%d-%d-%d) (%s)", 
			     color, username,
			     tank[2], tank[3], tank[4], tank[5], status) 

	settings = readfile("templates/settings.xml")

	settings = settings.replace('**red tank name**', reduser + "." + redtank[2])
	settings = settings.replace('**blue tank name**', blueuser + "." + bluetank[2])
	settings = settings.replace('**red tank source**', "tanks/" + reduser + "/" + redtank[2] + "/" + redtank[7])
	settings = settings.replace('**blue tank source**', "tanks/" + blueuser + "/" + bluetank[2] + "/" + bluetank[7])

	settingsfile = open("JavaTankSoar/tanksoar-default-settings.xml", 'w')
	settingsfile.write(settings)
	settingsfile.close()

	logging.info("Starting match.")
	
	cwd = os.getcwd()
	os.chdir('JavaTankSoar')
	if os.path.exists("TankSoarLog.txt"):
		os.unlink("TankSoarLog.txt")
	os.system('java -jar JavaTankSoar.jar -quiet')

	logging.info("Match finished.")

	output = file("TankSoarLog.txt");
	matched_something = False
	for line in output.readlines():
		match = re.match(r"(.+): (-?\d+) \((\w+)\)", line)
		if match == None:
			continue
		matched_something = True
		#logging.debug("Matched this line: %s", line)
		tankname, score, status = match.groups()
		score = int(score)
		if status == "winner":
			sql = "UPDATE tanks SET wins=wins+1 WHERE tankid=%s";
		elif status == "loser":
			sql = "UPDATE tanks SET losses=losses+1 WHERE tankid=%s";
		elif status == "draw":
			sql = "UPDATE tanks SET draws=draws+1 WHERE tankid=%s";
		else:
			logging.warning("Unknown status %s for %s!", status, tankname)
			continue

		tankid = None
		if tankname == reduser + "." + redtank[2]:
			tankid = redtank[0]
		else:
			tankid = bluetank[0]
		cursor.execute(sql, (tankid,))
		cursor.execute("SELECT pointspermatch, wins, losses, draws FROM tanks WHERE tankid=%s", (tankid,))
		pointspermatch, wins, losses, draws = cursor.fetchone()
		matches = int(wins) + int(losses) + int(draws)
		pointspermatch = ((pointspermatch*(matches-1)) + score) / matches
		cursor.execute("UPDATE tanks SET pointspermatch=%s WHERE tankid=%s", (pointspermatch, tankid))
		logging.info("%s (%s) scored %d points (%s), %f pointspermatch in %d matches.", tankname, tankid, score, status, pointspermatch, matches)
		
		
	output.close()
	os.chdir(cwd)

	if not matched_something:
		logging.warning("Failed to match status lines! Ignoring match!")

	
	
	logging.info("Sleeping for a bit.")
	time.sleep(sleepytime)

logging.info('Shutting down.')
logging.shutdown()
sys.exit(1)

