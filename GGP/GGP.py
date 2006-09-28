#!/usr/bin/env python

import BaseHTTPServer
import re
import sys
import os
import signal
import imp
import Python_sml_ClientInterface
sml = Python_sml_ClientInterface

global kernel
global agent

kernel = None
agent = None

class ElementGGP:
	"""Used to represent the lisp-like code of GGP."""

	__children = None

	def __init__(self, string = None):
		"""Create using an optional string of code. If no string
		is given, an empty, valid element "()" is created. Raises
		ValueError if there is a problem with the passed string."""
		
		if string == None:
			return

		depth = 0
		start = -1
		current = None
		for x in range(len(string)):
			if depth == 0:
				# At the top level
				if string[x] == '(':
					# Start of top level element
					depth += 1
					self.__children = []
				else:
					# Paren must start top level element
					raise ValueError("No opening paren: %s" % string)
			elif depth == 1:
				# Inside the top level
				if string[x] == '(':
					# Start of sub element
					start = x
					depth += 1

				elif string[x] == ')':
					if current != None:
						# Currently parsing a string, ends the string
						self.__children.append(current)
						current = None
						
					# End of top level element
					if not x == (len(string) - 1):
						self.__children = None
						raise ValueError("Extra characters at end: %s" % string)

				elif string[x] == ' ':
					# Space inside top level
					if current != None:
						# Currently parsing a string, ends the string
						self.__children.append(current)
						current = None
						
				else:
					# Some other character inside top level
					if current != None:
						# Currently parsing a string, append it
						current += string[x]
					else:
						# Not currently parsing a string, start one
						current = string[x]

			else:
				# Inside sub element, keep track of parens
				if string[x] == '(':
					depth += 1
				elif string[x] == ')':
					depth -= 1
					if depth == 1:
						# End of sub element, append it
						try:
							self.__children.append(ElementGGP(string[start:x+1]))
						except ValueError:
							self.__children = None
							raise
						start = -1
	
	def get(self, index):
		"""Retreives the child at index.
		
		Raises IndexError if the index is invalid.
		Raises ValueError if the element is invalid."""
		
		if self.__children == None:
			raise ValueError("Invalid element")
			
		if index < 0 or index >= len(self.__children):
			raise IndexError
		
		return self.__children[index]
	
	def remove(self, index):
		"""Removes the child at index.
		
		Raises IndexError if the index is invalid.
		Raises ValueError if the element is invalid."""
		
		if self.__children == None:
			raise ValueError("Invalid element")
			
		if index < 0 or index >= len(self.__children):
			raise IndexError

		self.__children[index:1] = []
	
	def num_children(self):
		"""Returns the number of children.
		
		Raises ValueError if the element is invalid."""
		
		if self.__children == None:
			raise ValueError("Invalid element")

		return len(self.__children)
			
	def __str__(self):
		if self.__children == None:
			raise ValueError("Invalid element")
		
		if len(self.__children) < 1:
			return "()"
		
		str = "("
		for element in self.__children:
			if isinstance(element, ElementGGP):
				str = str + element.__str__() + " "
			else:
				str = str + element + " "
		
		str = str[:-1]
		str = str + ")"
		return str

class Responder(BaseHTTPServer.BaseHTTPRequestHandler):
	player_name = 'voigt'
	move_responses = ['(u c a)', '(s b c)', '(s a b)', ]
	move_count = 0
	
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
			role = rest_element.get(0)
			description_element = rest_element.get(1)
			startclock = rest_element.get(2)
			playclock = rest_element.get(3)
		except ValueError, IndexError:
			self.bad_request("Malformed START command")
			return False
			
		print "role:", role
		print "description:", description_element
		print "startclock:", startclock
		print "playclock:", playclock

		if isinstance(role, ElementGGP):
			self.bad_request("Malformed START command (role is element)")
			return False
			
		if not isinstance(description_element, ElementGGP):
			self.bad_request("Malformed START command (description not element)")
			return False
			
		if isinstance(startclock, ElementGGP):
			self.bad_request("Malformed START command (startclock is element)")
			return False
			
		if isinstance(playclock, ElementGGP):
			self.bad_request("Malformed START command (playclock is element)")
			return False
			
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
			moves_element = rest_element.get(0)
		except IndexError:
			self.bad_request("Malformed %s command (no moves)" % command)
			return
		
		print "moves_element:", moves_element

		if not isinstance(moves_element, ElementGGP):
			if moves_element == "NIL":
				moves_element = ElementGGP("()")
			else:
				self.bad_request("Malformed %s command (moves string and not NIL)" % command)
				return

		if play:
			# Put the last move on the IL

			# Run soar 1 till output
			agent.RunSelfTilOutput()

			# Translate ol to command

			# send the command
			self.reply(200, 'NOOP')
      
			# Scripted responses
			#if Responder.move_count < len(self.move_responses):
			#	this_move = self.move_responses[Responder.move_count]
			#	Responder.move_count = Responder.move_count + 1
			#	self.reply(200, this_move)
			#else:
			#	self.reply(200, 'NOOP')
		else:
			self.reply(200, 'DONE')

	def do_POST(self):
		if self.headers.has_key('receiver'):
			receiver = self.headers.get('receiver')
			if receiver.lower() != self.player_name:
				self.bad_request('receiver name mismatch')
				return
				
		if not self.headers.has_key('content-length'):
			self.bad_request('no content-length')
			return

		length = int(self.headers.get('content-length'))
		content = self.rfile.read(length)
		self.rfile.close()
		content = content.replace('\r', '')
		content = content.replace('\n', '')
		print len(content), repr(content)

		message_element = ElementGGP(content)
		
		command = message_element.get(0)
		matchid = message_element.get(1)
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
