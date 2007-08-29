import os, sys

def make_matlab_min_func(root):
	fstr = ""
	# turning this into a list fixes the predicate order in the nlp
	preds = list(root.get_all_predicates())
	preds.sort()
	dists = {}
	root.dist_to_goal(dists)
	for p, d in dists.items():
		membership = map(lambda x: int(x), [(i in p) for i in preds])
		# to get the distance estimate for a state, we average the estimates
		# for each predicate in the state. So we have to divide each 1 in the
		# membership array by the total number of 1's.
		num_preds = float(sum(membership))
		cons = map(lambda x: x / num_preds, membership)
		blah = '+'.join(['%f*x(%d)' % (x, i+1) for i, x in enumerate(cons)])
		fstr += '(%d-(%s))^2+' % (d, blah)
	return fstr[:-1] # cut off last +

def make_weighted_matlab_min_func(root, weights):
	fstr = ""
	# turning this into a list fixes the predicate order in the nlp
	preds = list(weights.keys())
	preds.sort()
	w = [weights[p] for p in preds]
	dists = {}
	root.dist_to_goal(dists)
	for s, d in dists.items():
		# to get the distance estimate for a state, we take the weighted
		# average for each predicate in the state.
		cons = [int(preds[i] in s) * w[i] for i in range(len(preds))]
		blah = '+'.join(['%f*x(%d)' % (x, i+1) for i, x in enumerate(cons)])
		fstr += '(%d-(%s))^2+' % (d, blah)
	return fstr[:-1] # cut off last +

def make_matlab_constraints(root):
	"""This function is mostly useless right now as the problem can become
	overconstrained in this way"""
	cstr = ""
	# turning this into a list fixes the predicate order in the nlp
	preds = list(root.get_all_predicates())
	preds.sort()
	dists = {}
	root.dist_to_goal(dists)
	cstr += '['
	for p, d in dists.items():
		membership = map(lambda x: int(x), [(i in p) for i in preds])
		# to get the distance estimate for a state, we average the estimates
		# for each predicate in the state. So we have to divide each 1 in the
		# membership array by the total number of 1's.
		num_preds = float(sum(membership))
		cons = map(lambda x: x / num_preds, membership)
		blah = '+'.join(['%f*x(%d)' % (x, i+1) for i, x in enumerate(cons)])
		cstr += '%s<=%d;' % (blah, d)
	cstr += ']'
	return cstr

def make_files(root):
	objfun_file = open('objfun.m','w')
	objfun_file.write('function f = objfun(x)\n')
	objfun_file.write('f=%s;' % make_matlab_min_func(root))
	objfun_file.close()

	num_vars = len(root.get_all_predicates())
	cmd_file = open('do_opt.m', 'w')
	cmd_file.write('x0=[%s];\n' % ','.join(['0'] * num_vars))
	cmd_file.write('lb=[%s];\n' % ','.join(['0'] * num_vars))
	cmd_file.write('[x,fval] = fmincon(@objfun,x0,[],[],[],[],lb);\n')
	cmd_file.write('csvwrite(\'result.csv\', x);\n')
	cmd_file.close()

def run_fmincon(root):
	make_files(root)
	#server = 'scs.itd.umich.edu'
	output_file = 'result.csv'
	matlab_cmd = 'matlab -nojvm -nodisplay -r \'do_opt;quit\' &> /dev/null'
	#os.system('scp objfun.m %s:%s/' % (server, d))
	#os.system('scp do_opt.m %s:%s/' % (server, d))
	#os.system('ssh %s "cd %s; %s"' % (server, d, matlab_cmd))
	#os.system('scp %s:%s/%s .' % (server, d, output_file))
	assert os.system(matlab_cmd) == 0, "Matlab run failed"
	result = [float(s) for s in open(output_file).read().split(',')]
	return result

if __name__ == '__main__':
	from treegen import TreeGen
	tgen = TreeGen()
	root = tgen.generate_random()
	result = run_fmincon(root)
	print result
