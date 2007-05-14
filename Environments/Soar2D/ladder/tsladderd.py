#!/usr/bin/env python

import time
import socket
import threading
import sys
import cPickle
import Python_sml_ClientInterface
import tempfile
import os
import shutil
import urllib
import urllib2
import subprocess
import re
import logging
import Queue
import types

# old disable command

class MatchStatus:
	def __init__(self, name):
		self.error = False
		self.error_message = "No error."

		self.tournament_name = name
		self.clear_match()
	
	def clear_match(self):
		self.match_id = None
		self.settings = None
		self.tank_names = None
		self.tank_filenames = None
		self.tank_data_zips = None
		self.tank_scores = None
		self.tank_statuses = None
		self.interrupted_tanks = None
		self.mem_killed_tanks = None
		self.log = None
		self.start_time = None
		self.elapsed_time = None
		self.process = None
	
	def new_match(self, match_info):
		self.match_id = match_info["match_id"]
		self.settings = match_info["settings"]
		
		# these are all lined up records
		
		self.tank_names = []			# names of tanks
		self.tank_filenames = []		# primary files in zips
		self.tank_data_zips = []		# zips
		self.tank_scores = []			# scores of tanks
		self.tank_statuses = []			# statuses of tanks (win/lose/draw)
		
		for tank_name, primary_file, tank_zip_data in match_info['match_tanks']:
			self.tank_names.append(tank_name)
			self.tank_filenames.append(primary_file)
			self.tank_data_zips.append(tank_zip_data)

		self.interrupted_tanks = []		# list of tanks interruped for stop-soar
		self.mem_killed_tanks = []		# list of tanks killed because of mem exceeded handler
		self.log = None			# zipped log, valid after completion
		self.start_time = None		# time started
		self.elapsed_time = None		# time elapsed after completion
		self.process = None			# The tanksoar process

	def results(self, scores, statuses, log):
		for tank_name in self.tank_names:
			self.tank_scores.append(scores[tank_name])
			self.tank_statuses.append(statuses[tank_name])
		self.log = log

	def convert(self, the_list):
		new_list = ""
		if the_list == None or len(the_list) == 0:
			return new_list
		else:
			for item in the_list:
				new_list = new_list + item + " "
			return new_list.strip()

	def post(self):
		# All lists must be converted in to space-delimited strings
		results = {}
		
		results['tournament_name'] = self.tournament_name
		results['match_id'] = self.match_id
		results['critical_error'] = self.error
		results['critical_error_message'] = self.error_message
		results['tank_names'] = self.convert(self.tank_names)
		results['tank_scores'] = self.convert(self.tank_scores)
		results['tank_statuses'] = self.convert(self.tank_statuses)
		results['interrupted_tanks'] = self.convert(self.interrupted_tanks)
		results['mem_killed_tanks'] = self.convert(self.mem_killed_tanks)
		results['log'] = self.log
		results['elapsed_time'] = self.elapsed_time
		
		urllib.urlopen('http://tsladder:vx0beeHH@localhost:54424/TankSoarLadder/tournaments/update_results', urllib.urlencode(results))
		logging.info('Posted results')	
	
class MatchFiles:
	def __init__(self, status):
		self.status = status
		self.index = -1

	def __iter__(self):
		return self
	
	def next(self):
		self.index += 1
		if self.index >= len(self.status.tank_names):
			raise StopIteration
		return self.status.tank_names[self.index], self.status.tank_filenames[self.index], self.status.tank_data_zips[self.index]

class Tourney(threading.Thread):
	# Locks the status member
	# Only need to lock if the following changes:
	#  * Presence of status member
	#  * tournament_name
	#  * tank_names
	#  * match_id
	#  * start_time
	#  * process
	#  * Tourney.shutdown
	status_lock = threading.Lock()

	# None == not running
	status = None

	# Signaled to start tournament
	start_condition = threading.Condition(status_lock)

	# Signaled to stop tournament
	stop_event = threading.Event()

	# True to end thread
	shutdown = False

	# True when killed
	killed = True
	
	def __init__(self):
		threading.Thread.__init__(self)
	
	def bootstrap_match(self):
		match_info = None
		match_file = None
		
		while True:
			failed = False
			
			match_file = urllib.urlopen('http://tsladder:vx0beeHH@localhost:54424/TankSoarLadder/tournaments/get_match?tournament_name=%s' % self.status.tournament_name)
			try:
				match_info = eval(match_file.read())
			except SyntaxError:
				failed = True
			if not failed:
				break
			logging.warning("Got document instead of match, trying again.")
			time.sleep(2)
			
		match_file.close()

		if type(match_info) == types.StringType:
			match_info = match_info.strip()
			logging.warning("Error getting match: %s" % (match_info))
			return False

		self.status_lock.acquire()
		self.status.new_match(match_info)
		self.status_lock.release()
		
		return True
		
	def set_up_tanks(self, topdir, oldcwd):
		# extract each tank
		source_files = []
		for tank_name, primary_file, tank_zip_data in MatchFiles(self.status):
			logging.info("Processing %s" % tank_name)
			# save the file
			tank_zip_file = open("%s.zip" % tank_name, 'w')
			tank_zip_file.write(tank_zip_data)
			tank_zip_file.close()

			# unzip the file
			os.system("/usr/bin/unzip -q %s.zip -d %s" % (tank_name, tank_name))

			# make everything readable
			os.system("/bin/chmod -R u+w %s" % topdir)

			# append the source file
			tank_source_file_name = os.path.join(tank_name, primary_file)
			source_files.append((tank_name, os.path.join(topdir, tank_source_file_name)))

		# back to the original (tanksoar) dir
		os.chdir(oldcwd)

		# save the match settings
		match_settings_file = open("ladder.xml", 'w')

		for x in range(len(source_files)):
			self.status.settings = self.status.settings.replace('name%d' % x, source_files[x][0])
			self.status.settings = self.status.settings.replace('productions%d' % x, source_files[x][1])
		
		match_settings_file.write(self.status.settings)
		match_settings_file.close()
		
	def fight(self):
		devnull = open("/dev/null", 'w')

		# Start tanksoar
		self.status_lock.acquire()
		self.status.process = subprocess.Popen(
				"java -ea -jar Soar2D.jar ladder.xml", 
				shell = True, 
				stdout = devnull, 
				stderr = devnull)
		self.status_lock.release()
		
		logging.info("Soar2D TankSoar started, pid %d" % self.status.process.pid)
		
		logging.info("Match %d started." % self.status.match_id)
		
		self.status_lock.acquire()
		self.status.start_time = time.time()
		self.status_lock.release()

		logging.info("Waiting for TankSoar to finish.")
		try:
			self.status.process.wait()
		except OSError, e:
			error_message = "Wait interrupted, throwing out match."
			logging.warning(error_message)
			self.status.error = True
			self.status.error_message = error_message
			try:
				os.kill(self.status.process.pid, 9)
			except OSError, e:
				logging.warning("Failed to kill process:" % e)
			return False

		if self.killed:
			logging.info("TankSoar killed.")
			return False
		
		logging.info("TankSoar finished.")
			
		self.status.elapsed_time = time.time() - self.status.start_time
		
		logging.info("Match %d ended (elapsed: %.1fs)." % (self.status.match_id, self.status.elapsed_time))
		#kernel.Shutdown()
		devnull.close()
		return True

	def parse_results(self):
		# parse results
		match_log = open("ladder.log", 'r')

		scores = {}
		statuses = {}
		for line in match_log.readlines():
			match = re.match(r"\d+ INFO (.+): (-?\d+) \((\w+)\)", line)
			if match == None:
				match = re.match(r"\d+ WARNING (.+): agent interrupted", line)
				if match == None:
					match = re.match(r"\d+ WARNING (.+): agent exceeded maximum memory usage", line)
					if match == None:
						continue
					self.status.mem_killed_tanks.append(match.group(1))
					logging.info("%s exceeded maximum memory usage." % match.group(1))
					urllib.urlopen('http://tsladder:vx0beeHH@localhost:54424/TankSoarLadder/disable_participant?tournament_name=%s&tank_name=%s&reason=mem_exceeded' % (Tourney.status.tournament_name, tank_name))
					continue
				self.status.interrupted_tanks.append(match.group(1))
				logging.info("%s was interrupted." % match.group(1))
				continue

			tankname, score, status = match.groups()
			logging.info("%s: %s points (%s)" % (tankname, score, status))
			
			scores[tankname] = score
			statuses[tankname] = status
		
		match_log.close()
		os.system("bzip2 --best -f ladder.log")

		match_log_zipped = open("ladder.log.bz2", 'r')
		self.status.results(scores, statuses, match_log_zipped.read())
		match_log_zipped.close()

	def do_match(self):

		if not self.bootstrap_match():
			return False
		
		# We have a match which means we have to post a result.

		# create a temporary folder, change to it
		topdir = tempfile.mkdtemp()
		oldcwd = os.getcwd()
		os.chdir(topdir)

		try:
			self.set_up_tanks(topdir, oldcwd)
			if self.fight():
				try:
					self.parse_results()
				except KeyError, err:
					logging.warning("Parse failed, disabling: %s"  % str(err).strip("'"))
					urllib.urlopen('http://tsladder:vx0beeHH@localhost:54424/TankSoarLadder/disable_participant?tournament_name=%s&tank_name=%s&reason=parse_results_failed' % (self.status.tournament_name, str(err).strip("'")))
		finally:
			# clean up our mess no matter what
			os.chdir(oldcwd)
			shutil.rmtree(topdir)

		self.status.post()
		
		self.status_lock.acquire()
		self.status.clear_match()
		self.status_lock.release()

		# check for stop
		if self.stop_event.isSet():
			return False
		return True

	def run(self):

		self.start_condition.acquire()

		while not self.shutdown:
			self.start_condition.wait()
			
			if self.shutdown:
				break
			
			self.stop_event.clear()
			self.killed = False
			
			self.status = MatchStatus(self.message['tournament_name'])
			self.start_condition.release()
		
			logging.info('Tournament started: %s' % self.message['tournament_name'])
			
			while self.do_match():
				pass
			
			logging.info('Tournament stopped.')

			self.start_condition.acquire()
			self.status = None

		self.start_condition.release()
		logging.info('Tournament shutdown.')
	
	def start_tournament(self, message):
		self.message = message
		self.start_condition.notify()

	def stop_tournament(self):
		self.stop_event.set()
		
	def kill_tournament(self):
		self.killed = True
		self.stop_event.set()
		if self.status != None:
			if self.status.process == None:
				return True
			try:
				os.kill(self.status.process.pid, 9)
			except OSError, e:
				logging.warning("Failed to kill process:" % e)
				return False
		return True
		
	def shutdown_tournament(self):
		self.shutdown = True
		self.start_condition.notify()
		self.stop_event.set()
		if self.status != None:
			if self.status.process == None:
				return True
			try:
				os.kill(self.status.process.pid, 9)
			except OSError, e:
				logging.warning("Failed to kill process:" % e)
				return False
		return True
		
SUCCESS = {'result' : True, 'message' : 'Success'}
ALREADY_RUNNING = {'result' : False, 'message' : 'Tournament is already running'}
NOT_RUNNING = {'result' : False, 'message' : 'Tournament is not running'}
UNKNOWN_COMMAND = {'result' : False, 'message' : 'Unknown command'}
FAILED_TO_KILL = {'result' : False, 'message' : 'Failed to kill process, it may already be dead.'}
DEMARSHALLING_FAILED = {'result' : False, 'message' : 'Demarshalling failed'}

def ts_start(tournament, message):
	logging.info("Start message received.")
	if tournament.status != None:
		return ALREADY_RUNNING
	else:
		tournament.start_tournament(message)
		return SUCCESS

def ts_stop(tournament, message):
	logging.info("Stop message received.")
	if tournament.status == None:
		return NOT_RUNNING
	else:
		tournament.stop_tournament()
		return SUCCESS

def ts_status(tournament, message):
	#logging.info("Status message received.")
	response = {}

	if tournament.status == None:
		response['running'] = False
	else:
		response['running'] = True
		response['tournament_name'] = tournament.status.tournament_name
		if tournament.status.match_id == None or tournament.status.start_time == None:
			response['start_time'] = 0
			response['tank_names'] = ""
		else:
			response['start_time'] = tournament.status.start_time
			response['tank_names'] = tournament.status.convert(tournament.status.tank_names)
	
	return response
	
def ts_score(tournament, message):
	#logging.info("Score message received.")

	os.system('grep score ladder.log > grep_score.txt')
	response = {}

	log = open("grep_score.txt", 'r')
	response['log'] = log.read()
	log.close()
	
	return response
	
def ts_kill(tournament, message):
	logging.info("Kill message received.")
	if tournament.status == None:
		return NOT_RUNNING
	else:
		if tournament.kill_tournament():
			return SUCCESS
		else:
			return FAILED_TO_KILL

if __name__ == '__main__':

	logging.basicConfig(level=logging.INFO,format='%(asctime)s %(levelname)s %(message)s')

	os.chdir("..")
	
	tournament = Tourney()
	tournament.start()
	
	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.bind(('', 54423))
	server.listen(5)
	
	try:
		logging.info("tsladderd running.")
		while True:
			channel, details = server.accept()
			pickled_message = ""
			while True:
				data = channel.recv(1024)
				if not data:
					break
				pickled_message += data
				
			message = cPickle.loads(pickled_message)
			response = None
			if message == None:
				logging.error("Message demarshalling failed from %s" % details[0])
				response = DEMARSHALLING_FAILED
			else:
				tournament.status_lock.acquire()
				if message['command'] == 'start':
					response = ts_start(tournament, message)
				elif message['command'] == 'stop':
					response = ts_stop(tournament, message)
				elif message['command'] == 'status':
					response = ts_status(tournament, message)
				elif message['command'] == 'score':
					response = ts_score(tournament, message)
				elif message['command'] == 'kill':
					response = ts_kill(tournament, message)
				else:
					logging.error('Unknown message from %s: %s' % (details[0], message))
					response = UNKNOWN_COMMAND
				tournament.status_lock.release()
			
			channel.sendall(cPickle.dumps(response))
			channel.close()

	except KeyboardInterrupt:
		logging.warning('Keyboard interrupt.')
		tournament.status_lock.acquire()
		tournament.shutdown_tournament()
		tournament.status_lock.release()

	server.close()
