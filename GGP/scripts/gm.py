#!/usr/bin/python

import os, sys
import urllib, urllib2, base64, re
import socket
import responder

user = 'Joseph.Xu'
password = 'TL2007'
auth = base64.encodestring('Basic %s:%s' % (user, password))[:-1]

url_base = 'http://games.stanford.edu:5999/director/'

setup_url = url_base + 'DisplayClass?'
create_url = url_base + 'CREATE?'
untimed_start_url = url_base + 'playmatch?'
timed_start_url = url_base + 'runmatch?'

match_id_re = re.compile(r'<INPUT TYPE="TEXT" NAME="Object" VALUE="Match\.(\d+)" SIZE="30"/>')

class GGPPostObj:
	def __init__(self, class_name, id):
		self.__id = id
		self.__class = class_name
		self.__fields = []
	
	def add_field(self, style, field_name, val):
		self.__fields.append((style, field_name, val))
	
	def generate_post_data(self):
		post_vals = []
		post_vals.append(('Start', ''))
		post_vals.append(('Object', '%s.%s' % (self.__class, self.__id)))
		post_vals.append(('Class', self.__class))
		for i, f in enumerate(self.__fields):
			post_vals.append(('Start', str(i)))
			post_vals.append(('Style', f[0]))
			post_vals.append(('Slot', '%s.%s' % (self.__class, f[1])))
			if f[0] == 'Selector':
				post_vals.append((str(id), f[2]))
			else:
				post_vals.append(('Value', f[2]))
			post_vals.append(('End', ''))
		post_vals.append(('End', ''))
		return post_vals

def get_match_id():
	args = urllib.urlencode({'Class' : 'Match', 'Command' : 'Create'})
	req = urllib2.Request(setup_url, args)
	req.add_header('Authorization', auth)
	page = urllib2.urlopen(req)
	for line in page.readlines():
		re_match = match_id_re.search(line)
		if re_match:
			return re_match.groups(1)[0]
	return None

def create_game(match_id, game_name, player, start_clock, play_clock, owner):
	match_obj = GGPPostObj('Match', match_id)
	match_obj.add_field('Selector', 'Game', game_name)
	match_obj.add_field('Selector', 'Player', player)
	match_obj.add_field('Selector', 'Startclock', start_clock)
	match_obj.add_field('Selector', 'Playclock', play_clock)
	match_obj.add_field('Glyph', 'Owner', owner)
	match_obj.add_field('Glyph', 'Status', 'ready')

	post_data = match_obj.generate_post_data() + [('Command', 'Create')]
	req = urllib2.Request(create_url, urllib.urlencode(post_data))
	req.add_header('Authorization', auth)
	page = urllib2.urlopen(req)

def start_game(match_id, timed):
	args = urllib.urlencode({'Match': 'MATCH.%s' % match_id})
	if timed:
		req = urllib2.Request(timed_start_url, args)
	else:
		req = urllib2.Request(untimed_start_url, args)
	
	req.add_header('Authorization', auth)
	urllib2.urlopen(req)

def port_open(p):
	sock = socket.socket()
	try:
		sock.bind(('127.0.0.1', p))
	except socket.error:
		return False
	sock.close()
	return True

if __name__ == '__main__':
	if len(sys.argv) == 1:
		input = sys.stdin.readlines()
	else:
		input = open(sys.argv[1]).readlines()

	domain_aliases = { 'w' : 'Wargame', 'r' : 'MRogue', 'e' : 'Escape', 'd' : 'Differing', 'b' : 'Build' }
	
	game_desc = input[0].split()
	domain = domain_aliases[game_desc[0]]
	level = game_desc[1]
	scenario = game_desc[2]
	st = {'s' : 'Source', 't' : 'Target'}[game_desc[3]]
	if len(game_desc) == 5:
		game_name = '-'.join([domain, level, scenario, st, game_desc[4], 'Evaluation'])
	else:
		game_name = '-'.join([domain, level, scenario, st, 'Evaluation'])
	

	hostname = socket.gethostname()
	# check if the default port is open. If not, use alternate port
	ports = [41414, 41415]
	if port_open(ports[0]):
		print 'Using primary port %d' % ports[0]
		port = ports[0]
		player = '%s_Umich_0' % (hostname[0].upper() + hostname[1:])
	else:
		# use alternate port
		print 'Primary port %d already used, using alternate %d' % tuple(ports)
		port = ports[1]
		player = '%s_Umich_1' % (hostname[0].upper() + hostname[1:])

	owner = 'Joseph.Xu'
	start_clock = '10'
	play_clock = '10'

	# set up the game
	print "Creating match for game %s" % game_name
	match_id = get_match_id()
	create_game(match_id, game_name, player, start_clock, play_clock, owner)
	print "Created match %s" % match_id

	responder.moves = input[1:]
	responder.port = port
	rt = responder.ResponderThread()
	rt.start()
	print "Started Responder"
	
	start_game(match_id, False)
	print "Started game"
