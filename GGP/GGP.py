#!/usr/bin/env python

import BaseHTTPServer
import re
import sys
import os
import signal
import imp
import Python_sml_ClientInterface
sml = Python_sml_ClientInterface

class ElementGGP:
	"""Used to represent the lisp-like code of GGP."""

	children = None

	def __init__(self, string = None):
		"""Create using an optional string of code. If no string
		is given, an empty, valid element "()" is created. Throws
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
					self.children = []
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
						self.children.append(current)
						current = None
						
					# End of top level element
					if not x == (len(string) - 1):
						self.children = None
						raise ValueError("Extra characters at end: %s" % string)

				elif string[x] == ' ':
					# Space inside top level
					if current != None:
						# Currently parsing a string, ends the string
						self.children.append(current)
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
							self.children.append(ElementGGP(string[start:x+1]))
						except ValueError:
							self.children = None
							raise
						start = -1
	
	def __str__(self):
		if self.children == None:
			raise ValueError("Invalid element")
		
		if len(self.children) < 1:
			return "()"
		
		str = "("
		for element in self.children:
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

	def handle_start(self, matchid, rest):
		match = re.match(r"^(?P<role>\w+)\s+(?P<description>\(.*\))\s+(?P<startclock>\d+)\s+(?P<playclock>\d+)$", rest)
		if match == None:
			self.bad_request("Malformed START command")
			return False
		
		description_element = None
		try:
			description_element = ElementGGP(match.group("description"))
		except ValueError, v:
			print "Failed to create description element:", v
			return False

		print "role:", match.group("role")
		print "description:", description_element
		print "startclock:", match.group("startclock")
		print "playclock:", match.group("playclock")

		self.reply(200, 'READY')
		return True

	def get_moves(self, rest):
		if rest == None or len(rest) < 1:
			raise ValueError("No moves string")
			
		# Return empty list on no moves
		if rest == "NIL":
			moves_element = ElementGGP()
			print "moves_element:", moves_element
			return moves_element

		moves_element = None
		try:
			moves_element = ElementGGP(rest)
		except ValueError, v:
			print "Failed to create moves element:", v
			return None
			
		print "moves_element:", moves_element
		return moves_element

	def handle_play(self, matchid, rest):
		if rest == None or len(rest) < 1:
			raise ValueError("No rest on PLAY commmand")
		
		moves_element = self.get_moves(rest)
		if moves_element == None:
			self.bad_request("Malformed PLAY command")
			return
		
		# Scripted responses
		if Responder.move_count < len(self.move_responses):
			this_move = self.move_responses[Responder.move_count]
			Responder.move_count = Responder.move_count + 1
			self.reply(200, this_move)
		else:
			self.reply(200, 'NOOP')

	def handle_stop(self, matchid, rest):
		if rest == None or len(rest) < 1:
			raise ValueError("No rest on STOP command")
			
		moves_element = self.get_moves(rest)
		if moves_element == None:
			self.bad_request("Malformed STOP command")
			return

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
		
		match = re.match(r"^\((?P<command>[A-Z]+)\s+?(?P<matchid>[^\s]+)\s+(?P<rest>.+)\)\s*$", content)
		if match == None:
			self.bad_request("Didn't match command/matchid")
			return
		
		command = match.group('command')
		matchid = match.group("matchid")
		print "command:", command
		print "matchid:", matchid
		
		if command == 'START':
			self.handle_start(matchid, match.group('rest'))
		elif command == 'PLAY':
			self.handle_play(matchid, match.group('rest'))
		elif command == 'STOP':
			self.handle_stop(matchid, match.group('rest'))
		else:
			self.bad_request("Invalid command: %s" % command)
		
	def do_GET(self):
		self.do_POST()

def print_callback(id, userData, agent, message):
	print "soar>", message

if __name__ == '__main__':
	#kernel = sml.Kernel.CreateKernelInNewThread()
	#agent = kernel.CreateAgent('ggp')
	#agent.RegisterForPrintEvent(sml.smlEVENT_PRINT, print_callback, None)
	#agent.LoadProductions('blocksworld.soar')
	
	server_address = ('', 41414)
	httpd = BaseHTTPServer.HTTPServer(server_address, Responder)
	try:
		httpd.serve_forever()
	except KeyboardInterrupt:
		#kernel.DestroyAgent(agent)
		#agent = None
		#kernel.Shutdown()
		#del kernel
		sys.exit(0)
