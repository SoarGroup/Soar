#!/usr/bin/env python

import BaseHTTPServer
import re

class Responder(BaseHTTPServer.BaseHTTPRequestHandler):
	player_name = 'voigt'
	
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
			return

		print "role", match.group("role")
		print "description", match.group("description")
		print "startclock", match.group("startclock")
		print "playclock", match.group("playclock")

		self.reply(200, 'READY')

	def get_moves(self, rest):
		match = re.match(r"^(?P<moves>.+)$", rest)
		if match == None:
			return None

		print "moves", match.group("moves")
		return match.group("moves")

	def handle_play(self, matchid, rest):
		moves = get_moves(rest)
		if moves == None:
			self.bad_request("badly formed PLAY command")
			return

		self.reply(200, 'MOVE')

	def handle_stop(self, matchid, rest):
		moves = get_moves(rest)
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

if __name__ == '__main__':
	server_address = ('', 41414)
	httpd = BaseHTTPServer.HTTPServer(server_address, Responder)
	httpd.serve_forever()

