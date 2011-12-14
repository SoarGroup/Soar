from __future__ import print_function
import sys, os, socket, time

TERMSTRING='\n***\n'

totalsent = 0

class Sock(object):
	def __init__(self):
		self.sock = None
		self.recvbuf = ''

	def connect(self, path_or_host, port=None):
		if port == None:
			self.sock = socket.socket(socket.AF_UNIX)
			print('waiting for domain socket {} to be created'.format(path_or_host), file=sys.stderr)
			while not os.path.exists(path_or_host):
				time.sleep(0.1)
				
			while True:
				try:
					self.sock.connect(path_or_host)
					break
				except socket.error:
					time.sleep(0.5)
			
			print('connected to {}'.format(path_or_host), file=sys.stderr)
		else:
			self.sock = socket.socket(socket.AF_INET)
			self.sock.connect((path_or_host, port))
	
	def serve(self, path):
		listener = socket.socket(socket.AF_UNIX)
		if os.path.exists(path):
			os.unlink(path)
		listener.bind(path)
		listener.listen(1)
		self.sock, addr = listener.accept()
		
	def receive(self, raw=False):
		while not TERMSTRING in self.recvbuf:
			r = self.sock.recv(1024)
			if len(r) == 0:
				return None
			self.recvbuf += r
		
		msg, self.recvbuf = self.recvbuf.split(TERMSTRING, 1)
		return msg.decode('ascii')
	
	def send(self, msg):
		global totalsent
		
		self.sock.sendall(msg + TERMSTRING)
		totalsent += len(msg + TERMSTRING)
		#print('Sending:\n{}\nTotal Sent: {}'.format(msg, totalsent))

	def close(self):
		self.sock.close()

	def has_buffered(self):
		return TERMSTRING in self.recvbuf
