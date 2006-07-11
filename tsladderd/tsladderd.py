#!/usr/bin/python

import os
import sys
sys.path.append(os.path.abspath('modules'))

import time
import MySQLdb
import logging
from pathutils import *
import re

def new_round(cursor):
	cursor.execute("SELECT * FROM tanks ORDER BY RAND(NOW())")
	if int(cursor.rowcount) < 2:
		return None
	return cursor.fetchall()

def fight(redtank, bluetank, round, matchNumber):
	cursor.execute("UPDATE tanks SET fighting=1 WHERE tankid=%s", (redtank[0],))
	cursor.execute("UPDATE tanks SET fighting=1 WHERE tankid=%s", (bluetank[0],))
	
	cursor.execute("SELECT username FROM users WHERE userid=%s", (redtank[1],))
	reduser = cursor.fetchone()[0]
	logging.info("red: %s.%s: %d points (%d-%d-%d)", reduser, redtank[2], redtank[11], redtank[3], redtank[4], redtank[5]) 

	cursor.execute("SELECT username FROM users WHERE userid=%s", (bluetank[1],))
	blueuser = cursor.fetchone()[0]
	logging.info("blue: %s.%s: %d points (%d-%d-%d)", blueuser, bluetank[2], bluetank[11], bluetank[3], bluetank[4], bluetank[5]) 

	settings = readfile("templates/settings.xml")

	settings = settings.replace('**red tank name**', reduser + "." + redtank[2])
	settings = settings.replace('**blue tank name**', blueuser + "." + bluetank[2])
	settings = settings.replace('**red tank source**', "tanks/" + reduser + "/" + redtank[2] + "/" + redtank[8])
	settings = settings.replace('**blue tank source**', "tanks/" + blueuser + "/" + bluetank[2] + "/" + bluetank[8])

	settingsfile = open("JavaTankSoar/tanksoar-default-settings.xml", 'w')
	settingsfile.write(settings)
	settingsfile.close()

	logging.info("Starting round " + str(round) + ", match " + str(matchNumber) + ".")
	
	cwd = os.getcwd()
	os.chdir('JavaTankSoar')
	#os.system('java -jar JavaTankSoar.jar -quiet -notrandom')
	os.system('java -jar JavaTankSoar.jar -quiet -notrandom -log tsladder-logs/' + str(matchNumber) + '.txt')

	logging.info("Round " + str(round) + ", match " + str(matchNumber) + " finished.")
	
	cursor.execute("UPDATE tanks SET fighting=0 WHERE tankid=%s", (redtank[0],))
	cursor.execute("UPDATE tanks SET fighting=0 WHERE tankid=%s", (bluetank[0],))
	
	output = file("tsladder-logs/" + str(matchNumber) + ".txt");
	#output = file("TankSoarLog.txt")
	matched_something = False
	finished = False
	redid = 0
	blueid = 0
	winnerid = 0
	for line in output.readlines():
		match = re.match(r".+ INFO (.+): (-?\d+) \((\w+)\)", line)
		if match == None:
			continue
		matched_something = True
		#logging.debug("Matched this line: %s", line)
		tankname, score, status = match.groups()

		tankid = None
		if tankname == reduser + "." + redtank[2]:
			tankid = redtank[0]
			redid = tankid
		else:
			tankid = bluetank[0]
			blueid = tankid

		score = int(score)
		matchpoints = 0
		if status == "winner":
			matchpoints = 2
			sql = "UPDATE tanks SET wins=wins+1 WHERE tankid=%s"
			winnerid = tankid
		elif status == "loser":
			sql = "UPDATE tanks SET losses=losses+1 WHERE tankid=%s"
		elif status == "draw":
			matchpoints = 1
			sql = "UPDATE tanks SET draws=draws+1 WHERE tankid=%s"
		else:
			logging.warning("Unknown status %s for %s!", status, tankname)
			continue

		if score >= 100:
			finished = True
			matchpoints += 1

		cursor.execute(sql, (tankid,))
		cursor.execute("SELECT pointspermatch, wins, losses, draws, ladderpoints FROM tanks WHERE tankid=%s", (tankid,))
		pointspermatch, wins, losses, draws, ladderpoints = cursor.fetchone()
		ladderpoints += matchpoints
		matches = int(wins) + int(losses) + int(draws)
		pointspermatch = ((pointspermatch*(matches-1)) + score) / matches
		cursor.execute("UPDATE tanks SET pointspermatch=%s, ladderpoints=%s WHERE tankid=%s", (pointspermatch, ladderpoints, tankid))
		logging.info("%s (%s) scored %d points (%s), %f pointspermatch in %d matches, ladder points increase by %d to %d.", tankname, tankid, score, status, pointspermatch, matches, matchpoints, ladderpoints)
		
	output.close()
	os.chdir('tsladder-logs')
	os.system("gzip -f " + str(matchNumber) + ".txt")
	os.chdir(cwd)

	if not matched_something:
		logging.warning("Failed to match status lines! Ignoring match!")
	else:
		cursor.execute("INSERT INTO matches (matchid, red, blue, winner) VALUES(%s, %s, %s, %s)", (matchNumber, redid, blueid, winnerid))
		logging.info("Added match record %s", matchNumber)
		if finished:
			logging.info("The match was finished.")
		else:
			logging.info("The match was not finished.")
			cursor.execute("UPDATE tanks SET unfinished=unfinished+1 WHERE tankid=%s", (redid,))
			cursor.execute("UPDATE tanks SET unfinished=unfinished+1 WHERE tankid=%s", (blueid,))

	return

if __name__ == "__main__":

	if len(sys.argv) > 1 and sys.argv[1].find("daemon") != -1:
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

	db = MySQLdb.connect(host="localhost", user="tsladder", passwd="75EbRL", db="tsladder")
	cursor = db.cursor()
	logging.info('Connected to database.')

	sleepytime = 5
	round = 0
	matchNumber = 0
	
	while True:
		#Old select two at random code
		#cursor.execute("SELECT * FROM tanks ORDER BY RAND(NOW()) LIMIT 2")
		
		round += 1
		tanks = new_round(cursor)
		if tanks == None:
			logging.warning('There is only one tank in the database.')
			logging.info("Sleeping for a bit.")
			time.sleep(sleepytime)
			continue

		j = 0
		for i in range(len(tanks)):
			j = i + 1
			while j < len(tanks):
				redtank = tanks[i]
				bluetank = tanks[j]
				matchNumber += 1
				fight(tanks[i], tanks[j], round, matchNumber)
				j += 1
				logging.info("Sleeping for a bit.")
				time.sleep(sleepytime)

	logging.info('Shutting down.')
	logging.shutdown()
	sys.exit(1)


