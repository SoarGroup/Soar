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
			# Get tanks
			match_filename, headers = urllib.urlretrieve('http://tsladder:cdQdpjG@localhost:54424/TankSoarLadder/tournaments/%s/get_match' % self.message[1])
			match_file = open(match_filename, 'r')
			match_id = None
			match_tanks = []
			for line in match_file:
				if match_id == None:
					match_id = int(line)
				else:
					match_tanks.append(line.strip())

			match_file.close()

			

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

	def ts_rete(self, message):
		print 'rete message received'

		tankdir = tempfile.mkdtemp()

		#save the zip file
		tankzip, tankzipfilename = tempfile.mkstemp(suffix=".zip", dir=tankdir)
		os.write(tankzip, message[1])
		os.close(tankzip)

		#unzip the file
		os.system("/usr/bin/unzip %s -d %s " % (tankzipfilename, tankdir))
		os.system("/bin/chmod -R u+w %s" % tankdir)
		grepresult = os.system("/bin/grep -Rq cmd %s*" % tankdir)
	
		# Make sure the file exists
		tanksourcefilename = os.path.join(tankdir, message[2])
		if not os.path.exists(tanksourcefilename):
			shutil.rmtree(tankdir)
			self.channel.sendall(cPickle.dumps(self.responses['no_primary_file']))
			return
			
		# Make sure there are no illegal commands
		if grepresult == 0:
			shutil.rmtree(tankdir)
			self.channel.sendall(cPickle.dumps(self.responses['illegal_commands']))
			return
			
		sml = Python_sml_ClientInterface

		kernel = sml.Kernel.CreateKernelInNewThread()

		if kernel == None:
			print 'kernel creation failed'
			shutil.rmtree(tankdir)
			self.channel.sendall(cPickle.dumps(self.responses['sml_error']))
			return
			
		print 'kernel created'

		agent = kernel.CreateAgent('ggp')
		if agent == None:
			print "agent creation failed: %s" % kernel.GetLastErrorDescription()
			kernel.Shutdown()
			del kernel
			shutil.rmtree(tankdir)
			self.channel.sendall(cPickle.dumps(self.responses['sml_error']))
			return

		print 'agent created'
	
		agent.RegisterForPrintEvent(sml.smlEVENT_PRINT, print_callback, None)

		agent.ExecuteCommandLine("pushd %s" % tankdir)
		agent.ExecuteCommandLine("source %s" % tanksourcefilename)
		source_result = agent.GetLastCommandLineResult()
		agent.ExecuteCommandLine("popd")
		if not source_result:
			print 'source failed:', tanksourcefilename
			kernel.DestroyAgent(agent)
			kernel.Shutdown()
			del kernel
			shutil.rmtree(tankdir)
			self.channel.sendall(cPickle.dumps(self.responses['sml_error']))
			return

		retenetfile, retenetfilename = tempfile.mkstemp()
		os.close(retenetfile)
		agent.ExecuteCommandLine("rete-net -s %s" % retenetfilename)
		if not agent.GetLastCommandLineResult():
			print 'rete-net save failed'
			kernel.DestroyAgent(agent)
			kernel.Shutdown()
			del kernel
			shutil.rmtree(tankdir)
			self.channel.sendall(cPickle.dumps(self.responses['rete_net_save_failed']))
			return
		
		retenetfile = open(retenetfilename, 'rb')
		response = ('success', retenetfile.read())
		retenetfile.close()
		shutil.rmtree(tankdir)
		self.channel.sendall(cPickle.dumps(response))

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
