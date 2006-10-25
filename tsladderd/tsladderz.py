#!/usr/bin/python
#create the rete

import socket
import cPickle
import time
import bz2
import urllib
import tempfile
import shutil
import os

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
	message = {}
	message['command'] = 'stop'
	message['tournament_name'] = tournament_name
	
	response = send(message)
	return response
	
def start_tournament(tournament_name, max_duration):
	message = {}
	message['command'] = 'start'
	message['tournament_name'] = tournament_name
	message['max_duration'] = max_duration
	
	response = send(message)
	return response

def tsladderd_status():
	message = {}
	message['command'] = 'status'
	
	response = send(message)
	return response

def decompress(url):
	match_file = urllib.urlopen('http://tsladder:cdQdpjG@localhost:54424/%s' % url)
	content = match_file.read()
	match_file.close()
	return bz2.decompress(content)

def check_productions(zip_data, primary_file):
	result = None
	
	# create a temporary folder and change to it
	topdir = tempfile.mkdtemp()
	oldcwd = os.getcwd()
	os.chdir(topdir)

	# save the file
	zip_file = open("tank.zip", 'w')
	zip_file.write(zip_data)
	zip_file.close()

	# unzip the file
	os.system("/usr/bin/unzip -q tank.zip -d tank")

	# make everything readable
	os.system("/bin/chmod -R u+w tank")

	try:
		# Make sure the file exists
		tank_source_file_name = os.path.join("tank", primary_file)
		if not os.path.exists(tank_source_file_name):
			raise ValueError("Failed to find primary file")

		# verify no bad commands
		grepresult = os.system("/bin/grep -Rq cmd tank/*")
		if grepresult == 0:
			raise ValueError("Found illegal commands")
	except ValueError, v:
		result = str(v)

	# change out of the temp dir
	os.chdir(oldcwd)

	# remove the temporary files
	shutil.rmtree(topdir)

	# a result of none indicates success, otherwise it is an error message
	return result

if __name__ == '__main__':
	print 'sending start'
	print start_tournament('elo', 600)
	print 'sleeping'
	time.sleep(5)
	print 'sending stop'
	print stop_tournament('elo')
