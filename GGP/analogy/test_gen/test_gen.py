import os, sys, pdb
import math, random, tempfile
import heuristic
from rule import *
from state import State
from treegen import TreeGen
from lpsolve55 import *
from astar import AStar

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
	#src_root = tree_gen.generate_random()
	src_root = tree_gen.generate_indicators({'I1': 3})
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
	return src_name, tgt_name

def calc_pred_estimates(root):
	# turning this into a list fixes the predicate order in the lp
	preds = list(root.get_all_predicates())
	preds.sort()
	dists = {}
	root.dist_to_goal(dists)

	# set up lpsolve
	# start with no constraints, and each predicate as a variable
	lp = lpsolve('make_lp', 0, len(preds))
	lpsolve('set_verbose', lp, IMPORTANT)
	# we want to minimize the value sum((B - Ax)^2), so actually we just want
	# to maximize x with the constraint that Ax <= B. So set c = [1...1]
	lpsolve('set_obj_fn', lp, [-1] * len(preds))
	# set up the Ax <= B constraints, where A is a matrix whose rows are the
	# memberships of each state, and B is the distance of that state to the
	# nearest leaf
	for p, d in dists.items():
		# membership is an array of 0/1 such that if predicate i is in the
		# state, then membership[i] = 1, otherwise membership[i] = 0
		membership = map(lambda x: int(x), [(i in p) for i in preds])
		# to get the distance estimate for a state, we average the estimates
		# for each predicate in the state. So we have to divide each 1 in the
		# membership array by the total number of 1's.
		num_preds = float(sum(membership))
		constraint = map(lambda x: x / num_preds, membership)
		#print constraint
		lpsolve('add_constraint', lp, constraint, LE, d)
	
	# solve the lp problem
	lpsolve('solve', lp)
	estimates, ret = lpsolve('get_variables', lp)
	
	return dict((preds[i], e) for i, e in enumerate(estimates))

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
	
	return

	import fmincon
	print fmincon.make_weighted_matlab_min_func(root, weights)

	#make_matlab_constraints(root)
	#make_min_func(root)
	estimates = calc_pred_estimates(root)
	#for p, e in estimates.items():
	#	print p, e
	for p, v in pvar.items():
		print '%s: %f' % (p, v)
	
	h = heuristic.Heuristic(estimates)
	print math.sqrt(heuristic.heuristic_mserr(h, root))
	preds = list(root.get_all_predicates())
	preds.sort()
	weights = [0,1.0447,0.0882,0,0,0.5500,9.8617,4.8973,2.7093,0.7603,3.6829,1.6404,0,0,0,0,0.5382,0,5.9140,1.5262,2.8374,0,2.3854,1.3851,0]

	h1 = heuristic.Heuristic(dict(zip(preds,weights)))
	print math.sqrt(heuristic.heuristic_mserr(h1, root))

	pred_accuracy = heuristic.pred_accuracy1(h1, root)
	#for p, a in pred_accuracy.items():
	#	print '%s --> %f %f' % (p, h1(p), a)

	root.make_graphviz(open('temp.dot', 'w'))
	search = AStar(root, lambda x: h1(x.preds))
	iter = search.gen
	iterations = 0
	try:
		while not iter.next():
			iterations += 1
	except StopIteration:
		print iterations
		print [''.join(s.preds) for s in search.solution]

if __name__ == '__main__':
	random.seed(0)
	test_heuristic_gen()
	#src, tgt = gen_src_tgt_split()
	#print src, tgt
