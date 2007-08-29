import os, sys, pdb
import math, random, tempfile
import heuristic
from rule import *
from state import State
from treegen import TreeGen
from astar import AStar
import fmincon

sys.path.append('../rule_mapper')
import rule_mapper
import options

def make_bins(l, n):
	if n == 0:
		return dict((x, 0) for x in l)
	r = l[:]
	random.shuffle(r)
	bins = {}
	b = 1
	for x in r:
		bins[x] = b
		b += 1
		if b > n:
			b = 1
	return bins

def gen_min(root, predicates):
	all_states = []
	root.get_all_states(all_states)
	#root.make_graphviz(open('%s_game.gdl' % prefix,'w'))

	rules = []
	root.make_max_rules(rules, frozenset(predicates))

	rs = RuleSet(rules, frozenset(all_states), set(predicates))
	rs.minimize_rules()
	assert rs.check_consistency()

	return rs

def gen_split(root, predicates):
	all_states = []
	root.get_all_states(all_states)
	#root.make_graphviz(open('%s_game.gdl' % prefix,'w'))

	rules = []
	root.make_max_rules(rules, frozenset(predicates))

	rs = RuleSet(rules, frozenset(all_states), set(predicates))
	
	split_rules = []
	for r in rules:
		if r.is_goal_rule():
			split_rules.append(r)
		else:
			split = rs.split_rule(r)
			if not split is None:
				split_rules.append(split[0])
				split_rules.append(split[1])
			else:
				# since the rule can't be split, just take it as is
				split_rules.append(r)
	
	split_rs = RuleSet(split_rules, frozenset(all_states), set(predicates))
	split_rs.minimize_rules()
	assert split_rs.check_consistency()

	return split_rs
	#split_rs.make_rule_graph(open('%s_srules.gdl' % prefix, 'w'))
	#split_rs.make_kif(root.preds, kif)

def state_preds_lower(s):
	s.preds = frozenset(p.lower() for p in s.preds)

def gen_src_tgt_split():
	tree_gen = TreeGen()
	src_root = tree_gen.generate_random()
	#src_root = tree_gen.generate_indicators({'I1': 3})
	src_rs = gen_min(src_root, tree_gen.predicates)
	src_fd, src_name = tempfile.mkstemp('.kif', 'src_rules')
	src_rs.make_kif(src_root, os.fdopen(src_fd, 'w'))
	
	tgt_root = src_root.deep_copy()
	# change all predicates in the target to be lowercase version of source
	tgt_root.map(state_preds_lower)
	preds_lower = [p.lower() for p in tree_gen.predicates]
	tgt_rs = gen_split(tgt_root, preds_lower)
	tgt_fd, tgt_name = tempfile.mkstemp('.kif', 'tgt_rules')
	tgt_rs.make_kif(tgt_root, os.fdopen(tgt_fd, 'w'))
	return src_root, src_rs, tgt_root, tgt_rs

def gen_src_tgt_split_files():
	src_root, src_rs, tgt_root, tgt_rs = gen_src_tgt_split()
	src_fd, src_name = tempfile.mkstemp('.kif', 'src_rules')
	src_rs.make_kif(src_root, os.fdopen(src_fd, 'w'))
	tgt_fd, tgt_name = tempfile.mkstemp('.kif', 'tgt_rules')
	tgt_rs.make_kif(tgt_root, os.fdopen(tgt_fd, 'w'))
	return src_root, src_name, tgt_root, tgt_name

def variance(l):
	m = sum(l) / float(len(l))
	return sum((m - x) ** 2 for x in l) / len(l)

def pred_variance(root):
	preds = list(root.get_all_predicates())
	preds.sort()
	dists = {}
	root.dist_to_goal(dists)
	preds2dists = {}
	for s, d in dists.items():
		for p in s:
			preds2dists.setdefault(p, []).append(d)

	return dict((p, variance(d)) for p, d in preds2dists.items())

def test_heuristic_gen():
	tree_gen = TreeGen()
	root = tree_gen.generate_indicators({2 : ['I1']})
	pvar = pred_variance(root)
	max_var = 1.5 * max(pvar.values())
	pvar1 = dict((p, max_var - v) for p, v in pvar.items())
	pvar1s = sum(pvar1.values())
	weights = dict((p, root.num_occurrences(p) * v / pvar1s) for p, v in pvar1.items())
	for p, w in weights.items():
		print '%s: %f' % (p, w)
	
	preds = list(root.get_all_predicates())
	preds.sort()
	weights = fmincon.run_fmincon(root)

	h = heuristic.Heuristic(dict(zip(preds,weights)))

	root.make_graphviz(open('temp.dot', 'w'))
	search = AStar(root, lambda x: h(x.preds))
	iter = search.gen
	iterations = 0
	try:
		while not iter.next():
			iterations += 1
	except StopIteration:
		print iterations
		print [''.join(s.preds) for s in search.solution]

def test_all(nbins):
	src_root, src_file, tgt_root, tgt_file = gen_src_tgt_split_files()
	
	src_preds = list(src_root.get_all_predicates())
	src_preds.sort()
	src_weights = fmincon.run_fmincon(src_root)

	# the heuristic for the source
	hs = heuristic.Heuristic(dict(zip(src_preds, src_weights)))

	src_search = AStar(src_root, lambda x: hs(x.preds))
	iter = src_search.gen
	iterations = 0
	try:
		while not iter.next():
			iterations += 1
	except StopIteration:
		src_iters = iterations

	print 'The source problem takes %d iterations' % src_iters
	print 'Solution is', [''.join(s.preds) for s in src_search.solution]
	
	src_bins = make_bins(src_preds, nbins)
	for p, b in src_bins.items():
		print p, b
	tgt_bins = dict((p.lower(), b) for p, b in src_bins.items())

	mapping = rule_mapper.map_kifs(src_file, tgt_file, src_bins, tgt_bins)
	str_mapping = dict((s.get_name(), t.get_name()) for s,t in mapping.items())
	print "The mapping is ..."
	for s,t in str_mapping.items():
		print s, t
	
	num_correct = sum(int(s.lower() == t) for s,t in str_mapping.items())
	print "Number of correct predicates:", num_correct

	tgt_preds = []
	tgt_weights = []
	for p,w in zip(src_preds, src_weights):
		if p in str_mapping:
			# we assign the weight of the source predicate to the target
			tgt_preds.append(str_mapping[p])
			tgt_weights.append(w)

	ht = heuristic.Heuristic(dict(zip(tgt_preds, tgt_weights)))
	tgt_search = AStar(tgt_root, lambda x: ht(x.preds))
	iter = tgt_search.gen
	try:
		while not iter.next():
			iterations += 1
	except StopIteration:
		tgt_iters = iterations

	print 'The target problem takes %d iterations' % tgt_iters
	print 'Solution is', [''.join(s.preds) for s in tgt_search.solution]

	return src_iters, tgt_iters, num_correct

if __name__ == '__main__':
	random.seed(0)
	#test_heuristic_gen()
	for allow_partial in [True, False]:
		options.ALLOW_PARTIAL_BODY_MAPS = allow_partial
		for b in range(1,11):
			for it in range(20):
				src_iters, tgt_iters, num_correct = test_all(b)
				if allow_partial:
					log = open('partial_maps.log','a')
				else:
					log = open('no_partial_maps.log','a')
				log.write('%d %d %d %d\n' % (b, src_iters, tgt_iters, num_correct))
				log.close()
	
