#!/usr/bin/python
#create the rete

import socket
import cPickle
import time

def send(message):
	client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	
	response = None
	try:
		client.connect(('localhost', 54423))
		client.sendall(cPickle.dumps(message))
		client.shutdown(1) #socket.SHUT_WR)

		pickled_response = ""
		while True:
			data = client.recv(1024)
			if not data:
				break
			pickled_response += data
		response = cPickle.loads(pickled_response)

	except socket.error, e:
		print 'Error:', e
		raise

	client.close()
	return response

def stop_tournament(tournament_name):
	response = send(('stop', tournament_name))
	return response
	
def start_tournament(tournament_name, max_duration):
	response = send(('start', tournament_name, max_duration))
	return response

import bz2
import urllib
def decompress(url):
	match_file = urllib.urlopen('http://tsladder:cdQdpjG@localhost:54424/%s' % url)
	content = match_file.read()
	match_file.close()
	return bz2.decompress(content)

if __name__ == '__main__':
	print 'sending start'
	print start_tournament('elo', 600)
	print 'sleeping'
	time.sleep(5)
	print 'sending stop'
	print stop_tournament('elo')
