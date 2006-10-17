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

def print_callback(id, userData, agent, message):
	print message

class TournamentThread(threading.Thread):
	running = False
	stop = False
	event = threading.Event()

	def __init__(self, message):
		self.message = message
		threading.Thread.__init__(self)

	def run(self):
		if TournamentThread.running:
			print 'Tried to start tournament but tournament already running.'
			return
		TournamentThread.running = True
		TournamentThread.event.clear()
		print 'tournament started:', self.message
		while True:
			# Get new match information
			match_file = urllib.urlopen('http://tsladder:cdQdpjG@localhost:54424/TankSoarLadder/tournaments/get_match?tournament_name=%s' % self.message[1])
			match_info = eval(match_file.read())
			if type(match_info) == 'str':
				print "match info is string:", match_info
				print 'tournament stopped'
				TournamentThread.running = False
				match_file.close()
				return
			match_file.close()

			topdir = None
			oldcwd = None
			results = {}

			# do the following process in a try clause so it is cleaned up properly
			try:
				source_files = []
				try:
					# create a temporary folder
					topdir = tempfile.mkdtemp()
	
					# change in to that temporary folder
					oldcwd = os.getcwd()
					os.chdir(topdir)
			
					# extract each tank
					for tank_name, primary_file, tank_zip_data in match_info['match_tanks']:
						# save the file
						tank_zip_file = open("%s.zip" % tank_name, 'w')
						tank_zip_file.write(tank_zip_data)
						tank_zip_file.close()
	
						# unzip the file
						os.system("/usr/bin/unzip %s.zip -d %s" % (tank_name, tank_name))
	
						# make everything readable
						os.system("/bin/chmod -R u+w %s" % topdir)
	
						# Make sure the file exists
						tank_source_file_name = os.path.join(tank_name, primary_file)
						if not os.path.exists(tank_source_file_name):
							raise ValueError("Failed to find primary file")
				
						# verify no bad commands
						grepresult = os.system("/bin/grep -Rq cmd %s*" % tank_name)
						if grepresult == 0:
							raise ValueError("Found illegal commands")

						# append the source file
						source_files.append((tank_name, os.path.join(topdir, tank_source_file_name)))
		
				except ValueError, v:
					print v
					print 'tournament stopped'
					TournamentThread.running = False
					return

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
				
				print "starting match %d" % match_info['match_id']

				# start tanksoar with the match settings
				devnull = open("/dev/null", 'w')
				start_time = time.time()
				p = subprocess.Popen("java -ea -jar JavaTankSoar.jar -settings match-settings.xml -quiet -log match.txt", shell = True, stdout = devnull, stderr = devnull)
				p.wait()
				elapsed = time.time() - start_time
				devnull.close()
				print "match %d ended" % match_info['match_id']

				# parse results
				match_log = open("match.txt", 'r')

				results['match_id'] = match_info['match_id']
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
						print "%s was interrupted" % match.group(1)
						continue

					tankname, score, status = match.groups()
					print "%s: %s points (%s)" % (tankname, score, status)
					results['tanknames'] = results['tanknames'] + tankname + " "
					results['tankscores'] = results['tankscores'] + score + " "
					results['tankstatuses'] = results['tankstatuses'] + status + " "

				results['tanknames'] = results['tanknames'].strip()
				results['tankscores'] = results['tankscores'].strip()
				results['tankstatuses'] = results['tankstatuses'].strip()
				results['interrupted_tanks'] = results['interrupted_tanks'].strip()
				
				match_log.close()
				
				os.system("bzip2 --best -f match.txt")

				match_log_zipped = open("match.txt.bz2", 'r')
				results['log'] = match_log_zipped.read()
				match_log_zipped.close()

				results['tournament_name'] = self.message[1]

			finally:
				# make sure we change back to original directory
				os.chdir(oldcwd)
					
				# remove the temporary files
				shutil.rmtree(topdir)

			urllib.urlopen('http://tsladder:cdQdpjG@localhost:54424/TankSoarLadder/tournaments/update_results', urllib.urlencode(results))
			
			print 'results sent'

			if TournamentThread.event.isSet():
				if TournamentThread.stop:
					break

		print 'tournament stopped'
		TournamentThread.running = False

class ListenerThread(threading.Thread):
	responses = {'success': ('success', 'success'),
		'tournament_running': ('error', 'tournament_running'),
		'tournament_not_running': ('error', 'tournament_not_running'),
		'sml_error': ('error', 'sml_error'),
		'no_primary_file': ('error', 'no_primary_file'),
		'illegal_commands': ('error', 'illegal_commands'),
		'rete_net_save_failed': ('error', 'rete_net_save_failed'),}
	
	def __init__(self, channel, details):
		self.channel = channel
		self.details = details
		threading.Thread.__init__(self)

	def ts_start(self, message):
		print 'start message received'
		if TournamentThread.running:
			print 'tournament already running'
			self.channel.sendall(cPickle.dumps(self.responses['tournament_running']))
		else:
			print 'starting tournament'
			tournament = TournamentThread(message)
			tournament.start()
			self.channel.sendall(cPickle.dumps(self.responses['success']))

	def ts_stop(self, message):
		print 'stop message received'
		if not TournamentThread.running:
			print 'tournament not running'
			self.channel.sendall(cPickle.dumps(self.responses['tournament_not_running']))
		else:
			print 'stopping tournament'
			TournamentThread.stop = True
			TournamentThread.event.set()
			self.channel.sendall(cPickle.dumps(self.responses['success']))


	def run(self):
		print 'Received connection:', self.details[0]

		pickled_message = ""
		while True:
			data = self.channel.recv(1024)
			if not data:
				break
			pickled_message += data
		#print 'pickled message:', pickled_message
		message = cPickle.loads(pickled_message)
		
		if message[0] == 'start':
			self.ts_start(message)
		elif message[0] == 'stop':
			self.ts_stop(message)
		elif message[0] == 'rete':
			self.ts_rete(message)
		else:
			print 'Unknown message:', message[0]

		self.channel.close()
		print 'Closed connection:', self.details[0]

if __name__ == '__main__':

	server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server.bind(('', 54423))
	server.listen(5)
	
	try:
		while True:
			channel, details = server.accept()
			ListenerThread(channel, details).start()
	except KeyboardInterrupt:
		print 'Keyboard interrupt.'
		TournamentThread.stop = True
		TournamentThread.event.set()

	server.close()
