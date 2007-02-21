from PyRunSoar import RunSoar
import tempfile, re, sys, random, os

base_dir = '/home/swinterm/GGP'

kif_dir = os.path.join(base_dir, 'kif','R02_mod')
agent_dir = os.path.join(base_dir, 'agents')

output_dir_n = 0
while os.path.exists(os.path.join(base_dir, 'output', str(output_dir_n))):
	output_dir_n += 1
output_dir = os.path.join(base_dir, 'output', str(output_dir_n))
os.mkdir(output_dir)

solution_data = os.path.join(base_dir, 'pysrc', 'sorted2.txt')

# define the transfer levels and the problems in each level
TransferLevels = dict()
TransferLevels[1] = { 'source':['mummymaze1p-horiz'], 'target':'mummymaze1p-horiz' }
#TransferLevels[6] = { 'source':['mummymaze1p-horiz','mummymaze1p-vert'], 'target':'mummymaze1p-hv' }
#TransferLevels[7] = { 'source':['mummymaze1p-horiz'], 'target':'mummymaze1p-unreachable' }
#TransferLevels[8] = { 'source':['mummymaze1p-nosouth'], 'target':'mummymaze1p-horiz' }
#TransferLevels[9] = { 'source':'mummymaze1p-horiz', 'target':'mummymaze1p-quad' }
#TransferLevels[10] = { 'source':'mummymaze1p-horiz', 'target':'mummymaze1p-hhunter' }

# the different solution depths we want to try
depths = [5, 10, 15, 20, 25, 30, 35, 40, 43]

# number of instances to run for each solution depth
# each instance should have a different starting location
# for both the explorer and the mummy
instances = 5

def MakeSoarScript(file, sources, chunking, chunk_output_file, stat_file):
	f = open(file, 'w')
	for src in sources:
		f.write("source %s\n" % src)
	
	if chunking:
		f.write("learn --on\n")
	
	f.write("indifferent-selection --first\nwatch 0\nrun\n")
	
	if chunk_output_file != None:
		f.write("command-to-file -a %s print --chunks --full\n" % chunk_output_file)
	
	if stat_file != None:
		f.write("command-to-file %s stats\n" % stat_file)
	
	f.close()

def ModifyAgentInitLoc(infile, outfile, ex, ey, mx, my):
	loc_pat = re.compile('\\(<([-\\w\\*]+)>\\s+\\^p1\\s+([-\\w\\*]+)\\s+\\^p2\\s+\\d+\\s+\\^p3\\s+\\d+\\s*\\)')

	ifile = open(infile, 'r')
	ofile = open(outfile, 'w')
	lines = ifile.readlines()
	n = 0
	while n < len(lines):
		if lines[n].rstrip() in ['#XXX', '#XXXX']:
			m = loc_pat.match(lines[n+1].strip())
			assert m != None, "regex doesn't match '%s'" % lines[n+1].strip()
			var = m.group(1)
			obj = m.group(2)
			if lines[n].rstrip() == '#XXX':
				ofile.write('(<%s> ^p1 %s ^p2 %d ^p3 %d)\n' % (var, obj, mx, my))
			else:
				ofile.write('(<%s> ^p1 %s ^p2 %d ^p3 %d)\n' % (var, obj, ex, ey))

			n += 2
		else:
			ofile.write(lines[n])
			n += 1

def GetInitLocations(datafile, kif, steps, n):
	data = open(datafile, 'r')
	possible_locs = []
	kif_file = os.path.split(kif)[1]
	for line in data:
		if line.startswith(kif_file):
			tokens = line.split()
			if int(tokens[6]) == steps:
				possible_locs.append([int(t) for t in tokens[1:5]])
	
	random.shuffle(possible_locs)
	return possible_locs[0:n+1]

def RunSource(level, src, chunk_file, kif, depth, instances):
	fd, temp_chunk = tempfile.mkstemp(); os.close(fd)
	init_locs = GetInitLocations(solution_data, kif, depth, instances)

	for n, loc in zip(range(len(init_locs)), init_locs):
		print "Initial locations", loc
		log_file_prefix = os.path.join(output_dir, \
									   'level-%d-depth-%d-source-%s-inst-%d' % (level, depth, src, n))
		stats_file = log_file_prefix + '-stats'
		log_file = log_file_prefix + '-log'
		base_agent = os.path.join(agent_dir, '%s.soar' % src)
		fd, temp_agent = tempfile.mkstemp(); os.close(fd)
		ModifyAgentInitLoc(base_agent, temp_agent, loc[0], loc[1], loc[2], loc[3])
		fd, script_file = tempfile.mkstemp(); os.close(fd)
		MakeSoarScript(script_file, [temp_agent], True, temp_chunk, stats_file)
		RunSoar(script_file, True, log_file)
		os.remove(script_file)

		# append data from temporary chunk file to real chunk file
		tcf = open(temp_chunk, 'r')
		cf = open(chunk_file, 'a')
		for line in tcf:
			if not line.startswith('Log file'):
				cf.write(line)
		tcf.close()
		cf.close()
		os.remove(temp_chunk)

def RunTarget(level, tgt, chunk_file, kif, depth, init_locs, use_chunks):
	for n, loc in zip(range(len(init_locs)), init_locs):
		print "Initial locations", loc
		if use_chunks:
			log_file_prefix = os.path.join(output_dir, \
					"level-%d-depth-%d-target-chunk-inst-%d" % (level, depth, n))
		else:
			log_file_prefix = os.path.join(output_dir, \
					"level-%d-depth-%d-target-nochunk-inst-%d" % (level, depth, n))

		stats_file = log_file_prefix + '-stats'
		log_file = log_file_prefix + '-log'

		base_agent = os.path.join(agent_dir, "%s.soar" % tgt)
		fd, temp_agent = tempfile.mkstemp(); os.close(fd)
		ModifyAgentInitLoc(base_agent, temp_agent, loc[0], loc[1], loc[2], loc[3])
		fd, script_file = tempfile.mkstemp(); os.close(fd)
		if use_chunks:
			MakeSoarScript(script_file, [temp_agent, chunk_file], True, None, stats_file)
		else:
			MakeSoarScript(script_file, [temp_agent], True, None, stats_file)
			
		RunSoar(script_file, True, log_file)
		os.remove(script_file)


def RunBenchmark():

	for level in TransferLevels:
		print "Running level %d" % level
		tgt = TransferLevels[level]['target']
		tgt_kif = os.path.join(kif_dir, '%s.kif' % tgt)

		# for each solution depth, we evaluate how much transfer we get
		for d in depths:
			print "Running depth %d" % d
			# run all source agents
			chunk_file = os.path.join(output_dir, 'level-%d-depth-%d-chunks.soar' % (level, d))
			for src_prob in TransferLevels[level]['source']:
				src_kif = os.path.join(kif_dir, '%s.kif' % src_prob)
				print "Running source problem %s" % src_prob
				RunSource(level, src_prob, chunk_file, src_kif, d, instances)

			# run the target agent with chunking and without
			init_locs = GetInitLocations(solution_data, tgt_kif, d, instances)
			print "Running target %s with chunking" % tgt
			RunTarget(level, tgt, chunk_file, tgt_kif, d, init_locs, True)
			print "Running target %s without chunking" % tgt
			RunTarget(level, tgt, chunk_file, tgt_kif, d, init_locs, False)

if __name__ == '__main__':
	RunBenchmark()
