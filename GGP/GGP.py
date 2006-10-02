#!/usr/bin/env python

import BaseHTTPServer
import re
import sys
import os
import signal
import imp
import Python_sml_ClientInterface
sml = Python_sml_ClientInterface
import ElementGGP

global kernel
global agent

kernel = None
agent = None

class Responder(BaseHTTPServer.BaseHTTPRequestHandler):
	#player_name = 'jzxu'

	my_role = None
	roles = None
	input_link = None
	role_ids = None

	last_moves_id = None
	
	def bad_request(self, message):
		print "Warning:", message
		self.reply(400)
	
	def reply(self, code, message = None):
		self.wfile.write('%s %d %s\n' % (self.protocol_version, code, self.responses[code][0]))
		if message != None:
			self.wfile.write('Content-type: text/acl\n')
			self.wfile.write('Content-length: %d\n' % len(message))
			self.wfile.write('\n')
			self.wfile.write('%s\n' % message)
		else:
			self.wfile.write('\n')
		self.wfile.write('\n')

	def handle_start(self, matchid, rest_element):
		role = None
		description_element = None
		startclock = None
		playclock = None
		try:
			role = rest_element[0]
			description_element = rest_element[1]
			startclock = rest_element[2]
			playclock = rest_element[3]
		except ValueError, IndexError:
			self.bad_request("Malformed START command")
			return False
			
		print "role:", role
		print "description:", description_element
		print "startclock:", startclock
		print "playclock:", playclock

		if not isinstance(role, str):
			self.bad_request("Malformed START command (role not string)")
			return False
			
		if not isinstance(description_element, ElementGGP.ElementGGP):
			self.bad_request("Malformed START command (description not element)")
			return False
			
		if not isinstance(startclock, str):
			self.bad_request("Malformed START command (startclock not string)")
			return False
			
		if not isinstance(playclock, str):
			self.bad_request("Malformed START command (playclock not string)")
			return False

		try:
			startclock = int(startclock)
			playclock = int(playclock)
		except ValueError:
			self.bad_request("Malformed START command (startclock or playclock not integer)")
			return False
		
		Responder.my_role = role
		Responder.roles = []
		for x in range(len(description_element)):
			command = description_element[x][0]
			if not isinstance(command, ElementGGP.ElementGGP):
				if command == "ROLE":
					print "adding role:", description_element[x][1]
					Responder.roles.append(description_element[x][1])
		print "my role:", Responder.my_role

		if Responder.my_role not in Responder.roles:
			self.bad_request("Malformed START command (my role not in given roles)")
			return False

		Responder.input_link = agent.GetInputLink()
		Responder.last_moves_id = agent.CreateIdWME(Responder.input_link, "last-moves")
		Responder.role_ids = {}
		for a_role in Responder.roles:
			Responder.role_ids[a_role] = None

		self.reply(200, 'READY')
		return True

	def handle_play_stop(self, matchid, play, rest_element):
		command = None
		if play:
			command = "PLAY"
		else:
			command = "STOP"
			
		if rest_element == None:
			self.bad_request("Malformed %s command (no rest)" % command)
			return

		moves_element = None
		try:
			moves_element = rest_element[0]
		except IndexError:
			self.bad_request("Malformed %s command (no moves)" % command)
			return
		
		print "moves_element:", moves_element

		if not isinstance(moves_element, ElementGGP.ElementGGP):
			if moves_element == "NIL":
				moves_element = ElementGGP.ElementGGP()
			else:
				self.bad_request("Malformed %s command (moves string and not NIL)" % command)
				return

		if play:
			if len(moves_element) > 0:
				# Put the last move on the il
				# ^io.input-link.last-moves.role.action-name.p1
				# ^io.input-link.last-moves.role.action-name.p2
				# ...
				# ^io.input-link.last-moves.role.action-name.pN
				for x in range(len(Responder.roles)):
					role = Responder.roles[x]
					if Responder.role_ids[role] != None:
						agent.DestroyWME(Responder.role_ids[role])
					Responder.role_ids[role] = agent.CreateIdWME(Responder.last_moves_id, role)
					command = moves_element[x]
					if isinstance(command, ElementGGP.ElementGGP):
						action_id = agent.CreateIdWME(Responder.role_ids[role], command[0])
						for y in range(1, len(command)):
							agent.CreateStringWME(action_id, "p%d" % y, command[y])
					else:
						action_id = agent.CreateIdWME(Responder.role_ids[role], command)

			# Run soar 1 till output
			agent.RunSelfTilOutput()

			# Translate ol to command
			# ^io.output-link.action-name.pi
			# ^io.output-link.action-name.p2
			# ...
			# ^io.output-link.action-name.pN
			output_link = agent.GetOutputLink()
			command_string = ""
			for x in range(agent.GetNumberCommands()):
				command_id = agent.GetCommand(x)
				command_string += command_id.GetAttribute()
				for y in range(command_id.GetNumberChildren()):
					value = command_id.GetParameterValue("p%d" % (y + 1))
					command_string += " %s" % value

			# send the command
			self.reply(200, "(%s)" % command_string)
		else:
			# Should keep running until the agent halts itself
			agent.RunSelfForever()
			self.reply(200, 'DONE')

	def do_POST(self):
		# Removed, not really necessary
		#if self.headers.has_key('receiver'):
		#	receiver = self.headers.get('receiver')
		#	if receiver.lower() != self.player_name:
		#		self.bad_request('receiver name mismatch')
		#		return
				
		if not self.headers.has_key('content-length'):
			self.bad_request('no content-length')
			return

		length = int(self.headers.get('content-length'))
		content = self.rfile.read(length)
		self.rfile.close()
		content = content.replace('\r', '')
		content = content.replace('\n', '')
		print len(content), repr(content)

		message_element = ElementGGP.ElementGGP(content)
		
		command = message_element[0]
		matchid = message_element[1]
		print "command:", command
		print "matchid:", matchid
		
		message_element.remove(0)
		message_element.remove(0)
		
		if command == 'START':
			self.handle_start(matchid, message_element)
		elif command == 'PLAY':
			self.handle_play_stop(matchid, True, message_element)
		elif command == 'STOP':
			self.handle_play_stop(matchid, False, message_element)
		else:
			self.bad_request("Invalid command: %s" % command)
		
	def do_GET(self):
		self.do_POST()

def print_callback(id, userData, agent, message):
	print message

def shutdown():
	global kernel
	global agent

	if agent != None:
		kernel.DestroyAgent(agent)
		agent = None
	kernel.Shutdown()
	del kernel

if __name__ == '__main__':
	kernel = sml.Kernel.CreateKernelInNewThread()
	if kernel == None:
		print "Kernel creation failed."
		sys.exit(1)
	
	agent = kernel.CreateAgent('ggp')
	if agent == None:
		print "Agent creation failed: %s" % kernel.GetLastErrorDescription()
		shutdown()
		sys.exit(1)

	agent.RegisterForPrintEvent(sml.smlEVENT_PRINT, print_callback, None)
	
	print agent.ExecuteCommandLine("source blocksworld_noframe.soar")
	if not agent.GetLastCommandLineResult():
		print "Production load failed"
		shutdown()
		sys.exit(1)
	
	server_address = ('', 41414)
	httpd = BaseHTTPServer.HTTPServer(server_address, Responder)
	try:
		httpd.serve_forever()
	except KeyboardInterrupt:
		shutdown()
		sys.exit(0)
