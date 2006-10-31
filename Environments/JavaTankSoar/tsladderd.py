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

def MemExceededHandler(id, self, agent, phase):
	logging.warning("Max memory usage exceeded: %s" % agent.GetAgentName())

def AgentCreatedHandler(id, self, agent):
	agent.RegisterForRunEvent(Python_sml_ClientInterface.smlEVENT_MAX_MEMORY_USAGE_EXCEEDED, MemExceededHandler, self)
	logging.info("Registered handler for %s" % agent.GetAgentName())

class TournamentThread(threading.Thread):
	running = False
	tournament_name = None
	stop = False
	event = threading.Event()
	TRIES = 3

	def __init__(self, message):
		self.message = message
		threading.Thread.__init__(self)

	def do_match(self):
		# Get new match information, try TRIES times
		tries = 0
		match_info = None
		while tries < TournamentThread.TRIES:
			match_file = urllib.urlopen('http://tsladder:cdQdpjG@localhost:54424/TankSoarLadder/tournaments/get_match?tournament_name=%s' % self.message['tournament_name'])
			match_info = eval(match_file.read())
			match_file.close()

			# A string result indicates an error
			if type(match_info) != 'str':
				break
			logging.warning("Error getting match, try number %d: %s" % (tries, match_info))
			match_info = None
			tries = tries + 1
			
		if match_info == None:
			logging.error("Couldn't get match in %d tries, giving up." % TournamentThread.TRIES)
			return False

		results = {}  # results for server's update_results call
		results['critical_error'] = 'True' # removed on success
		results['tournament_name'] = self.message['tournament_name'] 
		results['match_id'] = match_info['match_id']

		# create a temporary folder
		topdir = tempfile.mkdtemp()

		# change in to that temporary folder
		oldcwd = os.getcwd()
		os.chdir(topdir)

		# do the following process in a try clause so it is cleaned up properly
		# incase something catastrophic happens
		try:
			# extract each tank
			source_files = []
			tank_names = ""
			for tank_name, primary_file, tank_zip_data in match_info['match_tanks']:
				# save for output
				tank_names = tank_names + tank_name + " "
				
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
			match_settings_file = open("match-settings.xml", 'w')
			match_settings = match_info['settings']

			for x in range(len(source_files)):
				match_settings = match_settings.replace('name%d' % x, source_files[x][0])
				match_settings = match_settings.replace('productions%d' % x, source_files[x][1])
			
			match_settings_file.write(match_settings)
			match_settings_file.close()
			
			tank_names = tank_names.strip()
			logging.info("Starting match %d (%s)." % (match_info['match_id'], tank_names))

			# start tanksoar with the match settings
			#
			# Start tanksoar -> connect to kernel -> register for agent creation events 
			#  -> set connection info ready -> get agent created callback -> register for memory event
			devnull = open("/dev/null", 'w')

			# Start tanksoar
			p = subprocess.Popen("java -ea -jar JavaTankSoar.jar -settings match-settings.xml -quiet -log match.txt", shell = True, stdout = devnull, stderr = devnull)
			
			logging.info("Tanksoar started")
			
			# connect to kernel
			tries = 7
			kernel = None
			while tries > 0:
				kernel = Python_sml_ClientInterface.Kernel.CreateRemoteConnection()
				if not kernel.HadError():
					break
				time.sleep(2)
				if TournamentThread.event.isSet():
					if TournamentThread.stop:
						return False
				tries = tries - 1

			if tries <= 0:
				logging.warning("Failed to connect to kernel too many times, giving up")
				TournamentThread.stop = True
				os.kill(p.pid, 9)
				return False
			
			logging.info("Connected to kernel")
				
			# register for agent creation events
			callback_id = kernel.RegisterForAgentEvent(Python_sml_ClientInterface.smlEVENT_AFTER_AGENT_CREATED, AgentCreatedHandler, self)

			logging.info("Registered for agent creation: %d" % callback_id)

			# set connection info to ready
			kernel.SetConnectionInfo("tsladderd", "ready", "ready")

			logging.info("Connection set to ready")

			start_time = time.time()

			try:
				p.wait()
			except OSError, e:
				logging.warning("Wait interrupted, shutting down.")
				TournamentThread.stop = True
				os.kill(p.pid, 9)
				return False
				
			elapsed = time.time() - start_time
			devnull.close()
			
			logging.info("Match %d ended (elapsed: %f)." % (match_info['match_id'], elapsed))

			# parse results
			match_log = open("match.txt", 'r')

			results['elapsed_time'] = elapsed
			results['tanknames'] = ""
			results['tankscores'] = ""
			results['tankstatuses'] = ""
			results['interrupted_tanks'] = ""
			for line in match_log.readlines():
				match = re.match(r".+ INFO (.+): (-?\d+) \((\w+)\)", line)
				if match == None:
					match = re.match(r".+ WARNING (.+): agent interrupted", line)
					if match == None:
						continue
					results['interrupted_tanks'] = results['interrupted_tanks'] + match.group(1) + " "
					logging.info("%s was interrupted." % match.group(1))
					continue

				tankname, score, status = match.groups()
				logging.info("%s: %s points (%s)" % (tankname, score, status))
				
				results['tanknames'] = results['tanknames'] + tankname + " "
				results['tankscores'] = results['tankscores'] + score + " "
				results['tankstatuses'] = results['tankstatuses'] + status + " "

			match_log.close()
			os.system("bzip2 --best -f match.txt")

			results['tanknames'] = results['tanknames'].strip()
			results['tankscores'] = results['tankscores'].strip()
			results['tankstatuses'] = results['tankstatuses'].strip()
			results['interrupted_tanks'] = results['interrupted_tanks'].strip()
			
			match_log_zipped = open("match.txt.bz2", 'r')
			results['log'] = match_log_zipped.read()
			match_log_zipped.close()

			results['critical_error'] = 'False'

		finally:
			# clean up our mess
			os.chdir(oldcwd)
			shutil.rmtree(topdir)

		# post results
		urllib.urlopen('http://tsladder:cdQdpjG@localhost:54424/TankSoarLadder/tournaments/update_results', urllib.urlencode(results))
		logging.info('Posted results')

		# check for stop
		if TournamentThread.event.isSet():
			if TournamentThread.stop:
				return False
		return True

	def run(self):
		if TournamentThread.running:
			logging.warning('Tried to start tournament but tournament already running.')
			return
		TournamentThread.tournament_name = self.message['tournament_name']
		TournamentThread.running = True
		TournamentThread.event.clear()
		logging.info('Tournament started: %s' % self.message['tournament_name'])
		while self.do_match():
			pass

		TournamentThread.running = False
		TournamentThread.tournament_name = None
		TournamentThread.event.clear()

		logging.info('Tournament stopped.')

class ListenerThread(threading.Thread):
	SUCCESS = {'result' : True}
	ALREADY_RUNNING = {'result' : False, 'message' : 'Tournament is already running'}
	NOT_RUNNING = {'result' : False, 'message' : 'Tournament is not running'}
	UNKNOWN_COMMAND = {'result' : False, 'message' : 'Unknown command'}
	DEMARSHALLING_FAILED = {'result' : False, 'message' : 'Demarshalling failed'}

	def __init__(self, channel, details):
		self.channel = channel
		self.details = details
		threading.Thread.__init__(self)

	def ts_start(self, message):
		logging.info('Start message received from %s' % self.details[0])
		if TournamentThread.running:
			logging.info('Tournament already running.')
			return ListenerThread.ALREADY_RUNNING
		else:
			tournament = TournamentThread(message)
			tournament.start()
			return ListenerThread.SUCCESS

	def ts_stop(self, message):
		logging.info('Stop message received from %s' % self.details[0])
		if not TournamentThread.running:
			logging.info('Tournament not running.')
			return ListenerThread.NOT_RUNNING
		else:
			TournamentThread.stop = True
			TournamentThread.event.set()
			return ListenerThread.SUCCESS

	def ts_status(self, message):
		logging.info('Status message received from %s' % self.details[0])
		response = {}
		response['running'] = TournamentThread.running
		if TournamentThread.running:
			response['tournament_name'] = TournamentThread.tournament_name
		return response

	def run(self):
		pickled_message = ""
		while True:
			data = self.channel.recv(1024)
			if not data:
				break
			pickled_message += data
			
		message = cPickle.loads(pickled_message)
		response = None
		if message == None:
			logging.error("Message demarshalling failed from %s" % self.details[0])
			response = DEMARSHALLING_FAILED
		else:
			if message['command'] == 'start':
				response = self.ts_start(message)
			elif message['command'] == 'stop':
				response = self.ts_stop(message)
			elif message['command'] == 'status':
				response = self.ts_status(message)
			else:
				logging.error('Unknown message from %s: %s' % (self.details[0], message))
				response = UNKNOWN_COMMAND
		
		self.channel.sendall(cPickle.dumps(response))
		self.channel.close()

if __name__ == '__main__':

	logging.basicConfig(level=logging.INFO,format='%(asctime)s %(levelname)s %(message)s')
	
	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.bind(('', 54423))
	server.listen(5)
	
	try:
		logging.info("tsladderd running.")
		while True:
			channel, details = server.accept()
			ListenerThread(channel, details).start()
	except KeyboardInterrupt:
		logging.warning('Keyboard interrupt.')
		TournamentThread.stop = True
		TournamentThread.event.set()

	server.close()
