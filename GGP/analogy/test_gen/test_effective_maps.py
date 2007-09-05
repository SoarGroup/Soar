import os, sys, pdb, shutil
import math, random, tempfile
import heuristic
from ggp_utils import *
from rule import *
from state import State
from treegen import TreeGen
from astar import AStar
import fmincon

def state_preds_lower(s):
	s.preds = frozenset(p.lower() for p in s.preds)

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

def ind_mapping_gen(indicators, others, num_preds, iters_per):
	tgt_others = [p.lower() for p in others]
	for num_ind_matches in range(len(indicators)+1):
		for n in range(iters_per):
			matched_inds = [(i, i.lower()) for i in random.sample(indicators, num_ind_matches)]
			random.shuffle(tgt_others)
			num_rest = num_preds - num_ind_matches
			rest = zip(others, tgt_others)[:num_rest]
			num_rest_matches = len(filter(lambda x: x[0].lower() == x[1], rest))
			yield (num_ind_matches, num_rest_matches, dict(matched_inds + rest))

def reg_mapping_gen(preds, iters_per):
	for num_matches in range(len(preds)+1):
		for n in range(iters_per):
			matched_preds = [(i, i.lower()) for i in random.sample(preds, num_matches)]
			src_others = set(preds) - set([mp[0] for mp in matched_preds])
			other_matches = []
			for sp in src_others:
				tgt_others = [p.lower() for p in src_others if p != sp]
				if len(tgt_others) == 0:
					other_matches.append((sp, sp.lower()))
				else:
					other_matches.append((sp, random.choice(tgt_others)))
			yield (num_matches, dict(matched_preds + other_matches))

def test_goal_dist_heuristic():
	min_len = 4
	max_len = 10
	indicators = dict((d, ['I%d' % d]) for d in range(1, max_len))
	indicator_set = set(reduce(lambda x,y: x+y, indicators.values()))
	tree_gen = TreeGen()
	tree_gen.min_branch_len = min_len
	tree_gen.max_branch_len = max_len

	src_root = tree_gen.generate_indicators(indicators)
	src_root.make_graphviz(open('src_tree.dot','w'))
	tgt_root = src_root.deep_copy()
	# change all predicates in the target to be lowercase version of source
	tgt_root.map(state_preds_lower)
	tgt_root.make_graphviz(open('tgt_tree.dot','w'))

	src_preds = list(src_root.get_all_predicates())
	
	num_preds = len(src_preds)

	src_preds.sort()
	src_weights = fmincon.run_fmincon(src_root)

	# the heuristic for the source
	hs = heuristic.Heuristic(dict(zip(src_preds, src_weights)))
	#hs = heuristic.Heuristic(indicators)

	src_search = AStar(src_root, lambda x: hs(x.preds))
	iter = src_search.gen
	iterations = 0
	try:
		while not iter.next():
			iterations += 1
	except StopIteration:
		src_iters = iterations

	non_ind_preds = [p for p in src_preds if p not in indicator_set]
		
	data = []

	for n_ind, n_rest, mapping in ind_mapping_gen(indicator_set, non_ind_preds, num_preds, 100):
	#for n_matches, mapping in reg_mapping_gen(src_preds, 100):
		#print "The mapping is ..."
		#for s,t in mapping.items():
		#	print s, t
		
		tgt_preds2weights = {}
		for p,w in zip(src_preds, src_weights):
			if p in mapping:
				# we assign the weight of the source predicate to the target
				tgt_preds2weights[mapping[p]] = w

		ht = heuristic.Heuristic(tgt_preds2weights)
		tgt_search = AStar(tgt_root, lambda x: ht(x.preds))
		iter = tgt_search.gen
		iterations = 0
		try:
			while not iter.next():
				iterations += 1
		except StopIteration:
			tgt_iters = iterations

		#print 'The target problem takes %d iterations' % tgt_iters
		#print 'Solution is', [''.join(s.preds) for s in tgt_search.solution]

		data.append((n_ind, n_rest, tgt_iters))
		#data.append((n_matches, tgt_iters))
		
	n_preds = len(src_preds)
	return (n_preds, src_iters, data)

if __name__ == '__main__':
	#import psycocompile
	random.seed(0)

	n_preds, src_iters, data = test_goal_dist_heuristic()
	avgs = average_tuple_list(data, 0)

