import os, sys
import pdb
from rule import *
from state import State
from treegen import TreeGen
import random
import tempfile

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
	split_rs.make_kif(root.preds, kif)

def state_preds_lower(s):
	s.preds = frozenset(p.lower() for p in s.preds)

def gen_split_tgt():
	tree_gen = TreeGen()
	src_root = tree_gen.generate()
	src_rs = gen_min(src_root, tree_gen.predicates, os.fdopen(src_fd, 'w'))
	src_fd, src_name = tempfile.mkstemp('.kif', 'src_rules')
	src_rs.make_kif(os.fdopen(src_fd, 'w'))
	
	tgt_root = src_root.deep_copy()
	# change all predicates in the target to be lowercase version of source
	tgt_root.map(state_preds_lower)
	preds_lower = [p.lower() for p in tree_gen.predicates]
	tgt_rs = gen_split(tgt_root, preds_lower, os.fdopen(tgt_fd, 'w'))
	tgt_fd, tgt_name = tempfile.mkstemp('.kif', 'tgt_rules')
	tgt_rs.make_kif(os.fdopen(tgt_fd, 'w'))
	return src_name, tgt_name

def gen_state_dist():
	tree_gen = TreeGen()
	src_root = tree_gen.generate()
	preds = list(src_root.get_all_predicates())
	dists = {}
	src_root.dist_to_goal(dists)
	for p, d in dists.items():
		membership_str = ' '.join(map(lambda x: str(int(x)), [(i in p) for i in preds]))
		print '%s --> %d' % (membership_str, d)

if __name__ == '__main__':
	random.seed()
	#src, tgt = gen_split_tgt()
	#print src, tgt

	gen_state_dist()
