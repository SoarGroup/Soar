#!/usr/bin/python

import os
import sys
sys.path.append(os.path.abspath('modules'))

import time
import MySQLdb
import logging
from pathutils import *
import re
import getopt
import os.path
import tsladder
import random

class Ladder:
	"A class representing the TankSoar ladder scheduling."
	
	id = None
	matchcount = None
	laddername = None
	winningscore = None
	maxupdates = None
	db = None
	cursor = None
	
	def __init__(self, id):
		self.id = id
		self.db = MySQLdb.connect(host="localhost", user="tsladder", passwd="75EbRL", db="tsladder")
		self.cursor = self.db.cursor()
		logging.info('Connected to database tsladder')
		self.cursor.execute("SELECT name,matchcount,winningscore,maxupdates FROM meta WHERE ladderid=%s", (self.id,))
		(self.laddername, self.matchcount, self.winningscore, self.maxupdates) = self.cursor.fetchone()
		logging.info("Starting ladder %s", self.laddername)

	def select_tanks(self):
		#self.cursor.execute("SELECT * FROM tanks ORDER BY RAND(NOW()) LIMIT 2")
		self.cursor.execute("SELECT tankid, wins, losses, draws FROM tanks")
		alltanks = []
		max = 0
		for tankid, wins, losses, draws in self.cursor.fetchall():
			matches = wins + losses + draws
			if matches > max:
				max = matches
			alltanks.append([matches, tankid])
		total = 0
		slottanks = []
		for matches, tankid in alltanks:
			slots = max - matches
			slots += 1
			total += slots
			slottanks.append([slots, tankid])
		selection = random.randint(1,total)
		firsttankid = None
		currentslot = 0
		for slots, tankid in slottanks:
			currentslot += slots
			if selection <= currentslot:
				firsttankid = tankid
				break
		if firsttankid == None:
			raise TypeError
		logging.debug("Selected first tank using bias, tank id %d", firsttankid) 
		sqltanks = []
		self.cursor.execute("SELECT * FROM tanks WHERE tankid=%s", (firsttankid,))
		sqltanks.append(self.cursor.fetchone())
		self.cursor.execute("SELECT * FROM tanks WHERE tankid!=%s ORDER BY RAND(NOW()) LIMIT 1", (firsttankid))
		sqltanks.append(self.cursor.fetchone())
		return sqltanks

	def run(self):
		while True:
			self.cursor.execute('show index from tanks')
			index = self.cursor.fetchone()
			if index[6] < 2:
				logging.warning('There are not enough tanks in the database')
				self.sleep()
				continue
				
			logging.debug("Selecting 2 tanks from %d in database", index[6])
			sqltanks = self.select_tanks()
			self.fight(sqltanks)

			self.matchcount += 1
			self.cursor.execute("UPDATE meta SET matchcount=%s WHERE ladderid=%s", (self.matchcount, self.id))
			#self.sleep()
	
	def sleep(self):
		logging.debug("Sleeping for 10 seconds")
		time.sleep(10)

	RED = 0
	BLUE = 1

	def fight(self, sqltanks):
		tanks = []
		for t in sqltanks:
			tank = tsladder.Tank(self.cursor, t)
			tank.set_fighting(True)
			tanks.append(tank)
		
		logging.info("red: %s", tanks[self.RED].get_info_string())
		logging.info("blue: %s", tanks[self.BLUE].get_info_string())

		settings = readfile("templates/%s.xml" % self.laddername)

		settings = settings.replace('**red tank name**', tanks[self.RED].get_full_name())
		settings = settings.replace('**blue tank name**', tanks[self.BLUE].get_full_name())
		settings = settings.replace('**red tank source**', tanks[self.RED].get_full_source())
		settings = settings.replace('**blue tank source**', tanks[self.BLUE].get_full_source())

		settingsfile = open("JavaTankSoar/tanksoar-default-settings.xml", 'w')
		settingsfile.write(settings)
		settingsfile.close()

		logging.info("Starting match %d", self.matchcount)
		cwd = os.getcwd()
		os.chdir('JavaTankSoar')
		if not os.path.exists("tsladder-logs/%s" % self.laddername):
			os.mkdir("tsladder-logs/%s" % self.laddername)
		os.system('java -jar JavaTankSoar.jar -quiet -notrandom -log tsladder-logs/%s/%d.txt' % (self.laddername, self.matchcount))
		
		logging.info("Match %d complete", self.matchcount)
		
		for tank in tanks:
			tank.set_fighting(False)

		output = file("tsladder-logs/%s/%d.txt" % (self.laddername, self.matchcount))
		matched_something = False
		finished = False
		winningTank = None
		for line in output.readlines():
			match = re.match(r".+ INFO (.+): (-?\d+) \((\w+)\)", line)
			if match == None:
				continue
			#logging.debug("Matched this line: %s", line)
			tankname, score, status = match.groups()

			tank = None
			opponent = None
			if tankname == tanks[self.RED].get_full_name():
				tank = tanks[self.RED]
				opponent = tanks[self.BLUE]
			elif tankname == tanks[self.BLUE].get_full_name():
				tank = tanks[self.BLUE]
				opponent = tanks[self.RED]
			else:
				logging.error("Did not match tankname: %s", tankname)
				continue

			matched_something = True

			score = int(score)
			draw = False
			winner = False
			if status == "winner":
				winningTank = tank
				winner = True
			elif status == "draw":
				draw = True

			iFinished = False
			if score >= self.winningscore:
				finished = True
				iFinished = True

			tank.update_record(winner=winner, draw=draw, finished=iFinished, score=score, opponentrating=opponent.rating)
			
			logging.info("%s scored %d points (%s)", tankname, score, status)
			
		output.close()
		os.chdir("tsladder-logs/%s" % self.laddername)
		os.system("gzip -f %d.txt" % self.matchcount)
		os.chdir(cwd)

		if not matched_something:
			logging.error("Failed to match status lines! Ignoring match!")
		else:
			winningid = 0
			if winningTank != None:
				winningid = winningTank.id
			self.cursor.execute("INSERT INTO matches (matchid, red, blue, winner) VALUES(%s, %s, %s, %s)", (self.matchcount, tanks[self.RED].id, tanks[self.BLUE].id, winningid))
			logging.info("Added match record %s", self.matchcount)
			if not finished:
				for tank in tanks:
					tank.increment_unfinished()

def usage():
	print "tsladderd.py usage:"
	print "\t-h, --help: Print this message."
	print "\t-c, --console: Log to console."
	print "\t-d, --daemon: Fork to background."
	print "\t-i #, --id=#: Ladder id to run (see meta table)."
	print "\t-q, --quiet: Decrease logger verbosity, use multiple times for greater effect."
	print "\t-v, --verbose: Increase logger verbosity, use multiple times for greater effect."

def main():
	loglevel = logging.INFO
	console = False
	id = 1

	try:
		opts, args = getopt.getopt(sys.argv[1:], "cdhi:qv", ["console", "daemon", "help", "id=", "quiet", "verbose",])
	except getopt.GetoptError:
		print "Unrecognized option."
		usage()
		sys.exit(1)

	for o, a in opts:
		if o in ("-c", "--console"):
			console = True
		if o in ("-d", "--daemon"):
			pid = os.fork()
			if pid != 0:
				print "tsladderd.py pid", pid
				pidfile = file('tsladderd.pid', 'w')
				pidfile.write(str(pid))
				pidfile.close()
				sys.exit(0)
		if o in ("-h", "--help"):
			usage()
			sys.exit()
		if o in ("-i", "--id"):
			id = int(a)
		if o in ("-q", "--quiet"):
			loglevel += 10
		if o in ("-v", "--verbose"):
			loglevel -= 10

	if loglevel < logging.DEBUG:
		loglevel = logging.DEBUG
	if loglevel > logging.CRITICAL:
		loglevel = logging.CRITICAL

	if not console:
		logging.basicConfig(level=loglevel,format='%(asctime)s %(levelname)s %(message)s',filename='tsladderd.log',filemode='w')
	else:
		logging.basicConfig(level=loglevel,format='%(asctime)s %(levelname)s %(message)s')

	ladder = Ladder(id)

	logging.info('Staring match loop')
	ladder.run()
	logging.shutdown()
	sys.exit(0)

if __name__ == "__main__":
	main()
