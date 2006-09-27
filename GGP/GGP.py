#!/usr/bin/env python

import BaseHTTPServer
import re
import sys
import os
import signal
import imp
import Python_sml_ClientInterface
sml = Python_sml_ClientInterface

class Element:
	children = None

	def create(self, string):
		# Strip parens
		if len(string) < 2 or string[0] != "(" or string[-1] != ")":
			print "failed paren match from string", string[0], string[-1]
			return False
		string = string[1:-1]

		self.children = []
		# remove surrounding whitespace
		string = string.strip()
		depth = 0
		start = -1
		current = None
		for x in range(len(string)):
			if string[x] == "(":
				if depth == 0:
					start = x
				depth = depth + 1
			elif string[x] == ")":
				depth = depth - 1
				if depth < 0:
					print "depth fell below zero"
					self.children = None
					return False
				if depth == 0:
					element = Element()
					element.create(string[start:x+1])
					if element.children == None:
						print "failed to create sub-child from", string[start:x+1]
						self.children = None
						return False
					self.children.append(element)
					start = -1
			elif depth > 0:
				continue
			elif string[x] == ' ':
				if current != None:
					self.children.append(current)
					current = None
			else:
				if current == None:
					current = string[x]
				else:
					current = current + string[x]
		if current != None:
			self.children.append(current)
			current = None
		return True
	
	def __str__(self):
		if self.children == None:
			return "Invalid element"
		
		if len(self.children) < 1:
			return "()"
		
		str = "("
		for element in self.children:
			if isinstance(element, Element):
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
			self.bad_request("badly formed START command")
			return False
		
		description_element = Element()
		if not description_element.create(match.group("description")):
			return False

		print "role", match.group("role")
		#print "description", match.group("description")
		print "description", description_element
		print "startclock", match.group("startclock")
		print "playclock", match.group("playclock")

		self.reply(200, 'READY')
		return True

	def get_moves(self, rest):
		match = re.match(r"^(?P<moves>.+)$", rest)
		if match == None:
			print "didn't match moves"
			return None

		moves_string = match.group("moves")
		
		# Return empty list on no moves
		if moves_string == "NIL":
			moves_element = Element()
			if not moves_element.create("()"):
				return None
			else:
				print "moves_element:", moves_element
				return moves_element

		moves_element = Element()
		if not moves_element.create(moves_string):
			return None
		print "moves_element:", moves_element
		return moves_element

	def handle_play(self, matchid, rest):
		moves = self.get_moves(rest)
		if moves == None:
			self.bad_request("badly formed PLAY command")
			return
		
		if Responder.move_count < len(self.move_responses):
			this_move = self.move_responses[Responder.move_count]
			Responder.move_count = Responder.move_count + 1
			self.reply(200, this_move)
		else:
			self.reply(200, 'NOOP')

	def handle_stop(self, matchid, rest):
		moves = self.get_moves(rest)
		if moves == None:
			self.bad_request("badly formed STOP command")
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
			self.bad_request("didn't match command/matchid")
			return
		
		command = match.group('command')
		matchid = match.group("matchid")
		print "command:", command, "matchid:", matchid
		
		if command == 'START':
			self.handle_start(matchid, match.group('rest'))
		elif command == 'PLAY':
			self.handle_play(matchid, match.group('rest'))
		elif command == 'STOP':
			self.handle_stop(matchid, match.group('rest'))
		else:
			self.bad_request("invalid command")
		
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
