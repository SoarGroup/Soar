#!/usr/bin/python

import os
import sys
import time
import MySQLdb
import logging

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
	cursor.execute("SELECT * FROM tanks")
	if int(cursor.rowcount) < 2:
		logging.warning('There is only one tank in the database. Sleeping.')
		time.sleep(10)
	else:
		break

for tank in cursor.fetchall():
	cursor.execute("SELECT username FROM users WHERE userid=%s", (tank[1],))
	status = "active"
	if not tank[8]:
		status = "inactive"
	logging.info("Tank: %s: %s: (%d-%d-%d) (%s)", 
		    cursor.fetchone()[0], 
		    tank[2], tank[3], tank[4], tank[5], status) 

logging.info('Shutting down.')
logging.shutdown()
sys.exit(1)

