#!/usr/bin/python

import os
import sys
sys.path.append(os.path.abspath('modules'))

import time
import MySQLdb
import logging
from pathutils import *

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

while True:
	cursor.execute("SELECT * FROM tanks ORDER BY RAND(NOW()) LIMIT 2")
	if int(cursor.rowcount) < 2:
		logging.warning('There is only one tank in the database.')
		logging.info("Sleeping for a bit.")
		time.sleep(10)
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

	settings = settings.replace('**red tank name**', redtank[2])
	settings = settings.replace('**blue tank name**', bluetank[2])
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
	os.chdir(cwd)

	logging.info("Sleeping for a bit.")
	time.sleep(10)

logging.info('Shutting down.')
logging.shutdown()
sys.exit(1)

