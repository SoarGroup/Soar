import os, sys
import urllib, urllib2, base64, re

def check_moves(game, moves):
	base_dir = os.path.join(os.environ['GGP_PATH'], 'kif')
	
	if game[3] == 's':
		if len(game) == 5:
			cond = 'source-%s' % game[4]
		else:
			cond = 'source'
	else:
		cond = 'target'

	if game[0] == 'e':
		kif = os.path.join(base_dir, 'escape', 'escape-%s-%s-%s.kif' % (game[1], game[2], cond))
	elif game[0] == 'r':
		kif = os.path.join(base_dir, 'rogue', 'mrogue-%s-%s-%s.kif' % (game[1], game[2], cond))
	elif game[0] == 'w':
		kif = os.path.join(base_dir, 'mm', 'wargame-%s-%s-%s.kif' % (game[1], game[2], cond))
	elif game[0] == '10':
		kif = os.path.join(base_dir, 'level10', 'differing-%s-%s-%s.kif' % (game[1], game[2], cond))

	rules = open(kif).read()

	begin_url = 'http://games.stanford.edu:5999/director/debuggame?'
	add_url = 'http://games.stanford.edu:5999/director/addaxioms?'
	dbg_url = 'http://games.stanford.edu:5999/director/debugwalk?'

	# this re matches the line that gives the match id
	match_id_re = re.compile(r'<INPUT TYPE="HIDDEN" NAME="matchid" VALUE="DEBUG.([A-Z][0-9]+)"/>')
	# this re matches the line that determines if the agent reached the goal
	goal_succ_re = re.compile(r'<close>Exit: \(GOAL ([A-Z]+) ([0-9]+)\)</close>')
	goal_fail_re = re.compile(r'<close>Fail: \(GOAL .*\)</close>')
	term_fail_re = re.compile(r'<close>Fail: TERMINAL</close>')

	user = 'Joseph.Xu'
	password = 'TL2007'
	auth = base64.encodestring('Basic %s:%s' % (user, password))[:-1]

	# first initiate a session and get the match id
	req = urllib2.Request(begin_url)
	req.add_header('Authorization', auth)
	page = urllib2.urlopen(req)
	for l in page:
		m = match_id_re.search(l)
		if m:
			match_id = m.groups(1)[0]
			break

	print match_id

	form_data = {'rules' : rules,
			'matchid' : 'DEBUG.%s' % match_id,
			'stylesheet' : 'GENERIC'}
	rules_post = urllib.urlencode(form_data)

	# upload the rules
	# I'm assuming there's nothing wrong with the rules
	req = urllib2.Request(add_url, rules_post)
	req.add_header('Authorization', auth)
	page = urllib2.urlopen(req)

	# now just run through all the moves
	for m in moves:
		print 'Making move %s' % m
		vals = {'kind' : 'manual',
				'matchid' : 'DEBUG.%s' % match_id,
				'moves' : '(%s)' % m}
		url = dbg_url + urllib.urlencode(vals)
		req = urllib2.Request(url)
		req.add_header('Authorization', auth)
		urllib2.urlopen(req)

	# get the goal
	vals = {'kind' : 'goals', 'matchid' : 'DEBUG.%s' % match_id }
	url = dbg_url + urllib.urlencode(vals)
	req = urllib2.Request(url)
	req.add_header('Authorization', auth)
	page = urllib2.urlopen(req).readlines()

	for l in page:
		m = goal_succ_re.search(l)
		if m:
			score = m.groups(1)
			print "Score:", score[1]
			break
		m = goal_fail_re.search(l)
		if m:
			print "No goal"
			break

if __name__ == '__main__':
	if len(sys.argv) == 1:
		input = sys.stdin
	else:
		input = open(sys.argv[1])
	start = True
	for line in input.readlines():
		if start:
			game = line.strip().split()
			moves = []
			start = False
		else:
			if len(line.strip()) == 0:
				start = True
				print game
				print moves
				check_moves(game, moves)
			else:
				moves.append(line.strip())

	if not start:
		check_moves(game, moves)
