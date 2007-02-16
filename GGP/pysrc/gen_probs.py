import os, kif, tempfile, subprocess

solver_path = '/home/jzxu/workspace/GGP/solver'
log_file = 'log.txt'
kif_dir = '/home/jzxu/workspace/GGP/kif/R02_mod'
kifs = [ 'mummymaze1p-horiz.kif' ]

def self_cross_product(array):
	return reduce(lambda a1,a2: a1+a2, map(lambda x: [(x, y) for y in array], array))

def run_search(explorer_loc, mummy_loc, exit_loc, mummytype, file, depth):
	cmd = list((solver_path,) + explorer_loc + mummy_loc + exit_loc + \
			  (mummytype, file, depth))
	cmd = map(lambda x: str(x), cmd)
	sp = subprocess.Popen(cmd, stdout = subprocess.PIPE)
	res = sp.stdout.read().strip()
	return res

log = open(log_file, 'w')

for f in ['%s/%s' % (kif_dir, k) for k in kifs]:
	orig_locs = kif.GetLocations(f)
	exit_loc = orig_locs['exit']
	objects = orig_locs.keys()[:]
	objects.remove('exit')
	assert len(objects) == 2, 'Can only deal with two variable locations'
	indexes = range(1,kif.GetMapSize(f)+1)
	all_locs = self_cross_product(indexes)
	all_loc_pairs = filter(lambda x: x[0] != x[1] and x[0] != exit_loc, \
			                   self_cross_product(all_locs))
	
	for loc_pair in all_loc_pairs:
		print loc_pair[0], loc_pair[1]
		res = 'depth'
		depth = 50
		res = run_search(loc_pair[0], loc_pair[1], exit_loc, 'h', f, depth)
		
		if res == 'eaten':
			log.write('%s %d %d %d %d - unsolvable\n' % ((f,) + loc_pair[0] + loc_pair[1]))
		elif res == 'depth':
			log.write('%s %d %d %d %d - depth\n' % ((f,) + loc_pair[0] + loc_pair[1]))
		else:
			log.write('%s %d %d %d %d - %d %s\n' % ((f,) + loc_pair[0] + loc_pair[1] + (len(res.split()), res)))

log.close()
