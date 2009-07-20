#!/usr/bin/env python

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
import time

class MatchStatus:
	def __init__(self, tournament_name, match_info):
		self.error = False
		self.error_message = "No error."

		self.tournament_name = tournament_name

		self.match_id = match_info["match_id"]
		self.map = match_info["map"]
		self.first_tank_name = match_info["first_tank_name"]
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
		self.endframe = None		# last frame, world count
		self.start_time = None		# time started
		self.elapsed_time = None		# time elapsed after completion

	def results(self, scores, statuses, endframe):
		for tank_name in self.tank_names:
			self.tank_scores.append(scores[tank_name])
			self.tank_statuses.append(statuses[tank_name])
		self.endframe = endframe

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
		results['map'] = self.map
		results['critical_error'] = self.error
		results['critical_error_message'] = self.error_message
		results['tank_names'] = self.convert(self.tank_names)
		results['tank_scores'] = self.convert(self.tank_scores)
		results['tank_statuses'] = self.convert(self.tank_statuses)
		results['interrupted_tanks'] = self.convert(self.interrupted_tanks)
		results['mem_killed_tanks'] = self.convert(self.mem_killed_tanks)
		results['endframe'] = self.endframe
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

class Ladder(threading.Thread):
	
	names = []
	names_index = 0;
	names_lock = threading.Lock()
	names_condition = threading.Condition(names_lock)

	process = None
	process_lock = threading.Lock()
	process_condition = threading.Condition(process_lock)
	
	current_tournament = None
	start_time = None
	combatants = None
	match_id = None

	killed = False
	shutdown = False
	
	def __init__(self):
		threading.Thread.__init__(self)
		
	def run(self):
		logging.info('Ladder started.')

		self.names_condition.acquire()
		
		while True:
			# Is the tournament list empty?
			if len(self.names) < 1:
				# Wait for names notification
				logging.info('Ladder waiting.')
				self.names_condition.wait()

			logging.info('Ladder awake.')

			# Break out if we're shutting down
			if self.shutdown:
				self.names_condition.release()
				break
			
			# Loop through if we don't have anything to run (shouldn't happen but could)
			if len(self.names) < 1:
				continue
			
			# Pick tournament to run
			self.names_index += 1
			self.names_index %= len(self.names)
			self.current_tournament = self.names[self.names_index]
			logging.info('Running tournament %s' % self.current_tournament)

			# Release names lock
			self.names_condition.release()
			
			# Do match
			try:
				self.do_match()
			except Exception, e:
				#raise
				logging.warning(e)

			# Break out if we're shutting down
			if self.shutdown:
				break
			
			# Re-acquire lock
			self.names_condition.acquire()
			
		logging.info('Ladder shutting down.')
				
	def do_match(self):
		status = self.bootstrap_match()
		
		# We have a match which means we have to post a result.

		# create a temporary folder, change to it
		topdir = tempfile.mkdtemp()
		oldcwd = os.getcwd()
		os.chdir(topdir)
		failure = False

		try:
			self.set_up_tanks(status, topdir, oldcwd)
			self.fight(status)
			try:
				self.parse_results(status)
			except KeyError, err:
				logging.error("Parse failed!")
				failure = True
				#urllib.urlopen('http://tsladder:vx0beeHH@localhost:54424/TankSoarLadder/disable_participant?tournament_name=%s&tank_name=%s&reason=parse_results_failed' % (self.current_tournament, str(err).strip("'")))
		finally:
			# clean up our mess no matter what
			os.chdir(oldcwd)
			shutil.rmtree(topdir)

		if not failure:
			status.post()
		
	def bootstrap_match(self):
		match_info = None
		match_file = None
		
		documentErrors = 0
		while True:
			failed = False
			
			match_file = urllib.urlopen('http://tsladder:vx0beeHH@localhost:54424/TankSoarLadder/tournaments/get_match?tournament_name=%s' % self.current_tournament)
			try:
				match_info = eval(match_file.read())
			except SyntaxError:
				failed = True
			if not failed:
				break
			documentErrors += 1
			if documentErrors >= 3:
				match_info = match_file.read()
				break;
			logging.warning("Got document instead of match, trying again.")
			time.sleep(2)
			
		match_file.close()

		if type(match_info) == types.StringType:
			match_info = match_info.strip()
			raise Exception("Error getting match: %s" % (match_info))

		return MatchStatus(self.current_tournament, match_info)
		
	def set_up_tanks(self, status, topdir, oldcwd):
		# extract each tank
		source_files = []
		first_name = None
		mirror = False
		for tank_name, primary_file, tank_zip_data in MatchFiles(status):
			if first_name == tank_name:
				mirror = True
				continue
			if tank_name == status.first_tank_name:
				logging.info("Processing %s (first tank)" % tank_name)
			else:
				logging.info("Processing %s" % tank_name)
			first_name = tank_name
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
			status.settings = status.settings.replace('name%d' % x, source_files[x][0])
			status.settings = status.settings.replace('productions%d' % x, source_files[x][1])

		#Mirror match
		if mirror:
			status.settings = status.settings.replace('name1', "%s.mirror" % source_files[0][0])
			status.settings = status.settings.replace('productions1', source_files[0][1])
		
		match_settings_file.write(status.settings)
		match_settings_file.close()

	def fight(self, status):
		devnull = open("/dev/null", 'w')

		# Start tanksoar
		self.process_condition.acquire()
		self.process = subprocess.Popen(
				"java -ea -jar Soar2D.jar ladder.xml", 
				shell = True, 
				stdout = devnull, 
				stderr = devnull)
		status.start_time = time.time()
		self.start_time = status.start_time
		self.combatants = status.convert(status.tank_names)
		self.match_id = status.match_id
		self.killed = False
		self.process_condition.release()
		
		logging.info("Soar2D TankSoar started, pid %d" % self.process.pid)
		
		logging.info("Match %d started (%s)." % (status.match_id, status.map))
		

		logging.info("Waiting for TankSoar to finish.")
		try:
			self.process.wait()
		except OSError, e:
			error_message = "Wait interrupted, throwing out match."
			logging.warning(error_message)
			status.error = True
			status.error_message = error_message
			try:
				os.kill(self.process.pid, 9)
			except OSError, e:
				raise Exception("Failed to kill process:" % e)

		self.process_condition.acquire()
		self.process = None
		try:
			if self.killed:
				raise Exception("TankSoar killed.")
		finally:
			self.process_condition.release()

		logging.info("TankSoar finished.")
			
		status.elapsed_time = time.time() - status.start_time
		
		logging.info("Match %d ended (wall clock elapsed: %.1fs)." % (status.match_id, status.elapsed_time))
		devnull.close()

	def parse_results(self, status):
		# parse results
		match_log = subprocess.Popen('tac ./%d.log' % status.match_id, shell=True, stdout=subprocess.PIPE).stdout

		endframe = None
		scores = {}
		statuses = {}
		for line in match_log.readlines():
			if endframe != None:
				match = re.match(r"^(\d+) .*", line)
				if match != None:
					if match.group(1) != endframe:
						break;

			match = re.match(r"(\d+) INFO (.+): (-?\d+) \((\w+)\)", line)
			if match == None:
				match = re.match(r"\d+ WARNING (.+): agent interrupted", line)
				if match == None:
					match = re.match(r"\d+ WARNING (.+): agent exceeded maximum memory usage", line)
					if match == None:
						continue
					status.mem_killed_tanks.append(match.group(1))
					logging.info("%s exceeded maximum memory usage." % match.group(1))
					urllib.urlopen('http://tsladder:vx0beeHH@localhost:54424/TankSoarLadder/disable_participant?tournament_name=%s&tank_name=%s&reason=mem_exceeded' % (self.current_tournament, match.group(1)))
					continue
				status.interrupted_tanks.append(match.group(1))
				logging.info("%s was interrupted." % match.group(1))
				continue

			frame, tankname, score, tank_status = match.groups()
			logging.info("%s: %s points (%s) (frame: %s)" % (tankname, score, tank_status, frame))
			
			endframe = frame
			scores[tankname] = score
			statuses[tankname] = tank_status
		
		match_log.close()
		os.system("bzip2 --best -f %d.log" % status.match_id)
		shutil.move("%d.log.bz2" % status.match_id, "ladder")

		status.results(scores, statuses, endframe)

	def start_tournament(self, name):
		self.names_condition.acquire()
		
		try:
			if name in self.names:
				logging.warning('Tournament %s already started.' % name)
				return False
			
			self.names.append(name)
			self.names_condition.notify()
		finally:
			self.names_condition.release()
		return True
	
	def stop_tournament(self, name):
		self.names_condition.acquire()

		try:
			if name not in self.names:
				logging.warning('Tournament %s already stopped.' % name)
				return False
			self.names.remove(name)
		finally:
			self.names_condition.release()
		return True
	
	def kill(self):
		self.names_condition.acquire()
		try:
			self.names = []
			self.process_condition.acquire()
			self.killed = True
			try:
				if self.process != None:
					try:
						os.kill(self.process.pid, 9)
					except OSError, e:
						logging.warning("Failed to kill process:" % e)
						return False
			finally:
				self.process_condition.release()
		finally:
			self.names_condition.release()
		return True

	def shutdown_all(self):
		self.names_condition.acquire()
		self.shutdown = True
		self.names = []
		self.names_condition.notify()
		self.process_condition.acquire()
		if self.process != None:
			try:
				os.kill(self.process.pid, 9)
			except OSError, e:
				logging.warning("Failed to kill process:" % e)
		self.process_condition.release()
		self.names_condition.release()
	
SUCCESS = {'result' : True, 'message' : 'Success'}
ALREADY_RUNNING = {'result' : False, 'message' : 'Tournament is already running'}
NOT_RUNNING = {'result' : False, 'message' : 'Tournament is not running'}
UNKNOWN_COMMAND = {'result' : False, 'message' : 'Unknown command'}
FAILED_TO_KILL = {'result' : False, 'message' : 'Failed to kill process, it may already be dead.'}
DEMARSHALLING_FAILED = {'result' : False, 'message' : 'Demarshalling failed'}

def ts_start(tournament, message):
	logging.info("Start message received.")
	if tournament.start_tournament(message['tournament_name']):
		return SUCCESS
	return ALREADY_RUNNING

def ts_stop(tournament, message):
	logging.info("Stop message received.")
	if tournament.stop_tournament(message['tournament_name']):
		return SUCCESS
	return NOT_RUNNING

def ts_status(tournament, message):
	#logging.info("Status message received.")
	response = {}
	
	tournament.names_condition.acquire()
	response['names'] = list(tournament.names)
	tournament.process_condition.acquire()
	if tournament.process == None:
		response['running'] = False
	else:
		response['running'] = True
		response['tournament_name'] = tournament.current_tournament
		response['start_time'] = tournament.start_time
		response['tank_names'] = tournament.combatants
	tournament.process_condition.release()
	tournament.names_condition.release()
	
	return response
	
def ts_score(tournament, message):
	#logging.info("Score message received.")
	response = {}
	response['log'] = ""

	tournament.process_condition.acquire()

	try:
		if tournament.process == None:
			response['log'] = "Not running."
		else:
			frame = None
			pipe = subprocess.Popen('tac ./%d.log' % tournament.match_id, shell=True, stdout=subprocess.PIPE).stdout
			for line in pipe.readlines():
				if frame == None:
					match = re.match(r"^(\d+) .*", line)
					if match != None:
						frame = match.group(1)
						break

			if frame == None:
				logging.warning("Didn't parse frame!")

			pipe.close()

			pipe = subprocess.Popen('tac ./%d.log | grep score' % tournament.match_id, shell=True, stdout=subprocess.PIPE).stdout

			found = set()

			for line in pipe.readlines():
				match = re.match(r"\d+ INFO (.+) score.*", line)
				if match != None:
					if match.group(1) not in found:
						found.add(match.group(1))
						response['log'] += line
						if len(found) > 1:
							break;
			pipe.close()

			if response['log'] == "":
				response['log'] = "No score.\n"

			if frame != None:
				response['log'] += "Current frame: %s" % frame

	finally:
		tournament.process_condition.release()
	
	return response

def ts_log(message):
	#logging.info("Log message received.")
	pipe = subprocess.Popen('bzcat ladder/%s.log.bz2' % message['match_id'], shell=True, stdout=subprocess.PIPE).stdout
	response = pipe.read()
	if len(response) == 0:
		response = "Match not found."
	return response
	
def ts_kill(tournament, message):
	logging.info("Kill message received.")
	if tournament.kill():
		return SUCCESS
	return FAILED_TO_KILL

if __name__ == '__main__':

	logging.basicConfig(level=logging.INFO,format='%(asctime)s %(levelname)s %(message)s')

	os.chdir("..")
	logging.info(os.getcwd())
	
	tournament = Ladder()
	tournament.start()
	
	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.bind(('', 54423))
	server.listen(5)
	
	try:
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
					elif message['command'] == 'log':
						response = ts_log(message)
					else:
						logging.error('Unknown message from %s: %s' % (details[0], message))
						response = UNKNOWN_COMMAND
				
				channel.sendall(cPickle.dumps(response))
				channel.close()

		except KeyboardInterrupt:
			logging.warning('Keyboard interrupt.')
		else:
			logging.warning('Unhandled exception.')
	finally:
		tournament.shutdown_all()

	server.close()
	
