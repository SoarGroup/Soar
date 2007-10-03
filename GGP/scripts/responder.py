 #!/usr/bin/env python

import BaseHTTPServer
import sys
import os
import signal
import imp
import threading

sys.path.append(os.path.join(os.environ['GGP_PATH'], 'old_translator'))
import ElementGGP

port = 41414

class StoppableHTTPServer (BaseHTTPServer.HTTPServer):
	"""http server that reacts to self.stop flag"""

	def serve_forever (self):
		"""Handle one request at a time until stopped."""
		self.stop = False
		while not self.stop:
			self.handle_request()

class Responder(BaseHTTPServer.BaseHTTPRequestHandler):
	
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

	def do_POST(self):
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
			self.reply(200, 'READY')
		elif command == 'PLAY':
			assert len(moves) > 0
			m = moves.pop(0)
			print "The move is %s" % m
			self.reply(200, m)
		elif command == 'STOP':
			self.reply(200, 'DONE')
			self.server.stop = True
		else:
			self.bad_request("Invalid command: %s" % command)
		
	def do_GET(self):
		self.do_POST()

class ResponderThread(threading.Thread):
	def run(self):
		global port
		server_address = ('', port)
		httpd = StoppableHTTPServer(server_address, Responder)
		try:
			httpd.serve_forever()
		except KeyboardInterrupt:
			return

if __name__ == '__main__':
	global moves

	moves = [l.strip() for l in sys.stdin.readlines()]
	print moves

	t = ResponderThread()
	t.start()
