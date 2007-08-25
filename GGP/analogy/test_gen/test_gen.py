import pdb
from rule import *
from state import State
from treegen import TreeGen
import random

def do_it(root, predicates, prefix):
	all_states = []
	root.get_all_states(all_states)
	#root.make_graphviz(open('%s_game.gdl' % prefix,'w'))

	rules = []
	root.make_max_rules(rules, frozenset(predicates))

	rs = RuleSet(rules, frozenset(all_states), set(predicates))
	rs.minimize_rules()
	print rs.check_consistency()
	#rs.make_rule_graph(open('%s_rules.gdl' % prefix, 'w'))
	rs.make_kif(root.preds, open('%s_rules.kif' % prefix, 'w'))
	rs.write_rules(open('rules', 'w'))
	

def do_it_split(root, predicates, prefix):
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
				print "split"
				split_rules.append(split[0])
				split_rules.append(split[1])
			else:
				print "couldn't split"
				# since the rule can't be split, just take it as is
				split_rules.append(r)
	
	split_rs = RuleSet(split_rules, frozenset(all_states), set(predicates))
	split_rs.minimize_rules()
	print split_rs.check_consistency()
	#split_rs.make_rule_graph(open('%s_srules.gdl' % prefix, 'w'))
	split_rs.make_kif(root.preds, open('%s_rules.kif' % prefix, 'w'))
	split_rs.write_rules(open('split_rules','w'))

def state_preds_lower(s):
	s.preds = frozenset(p.lower() for p in s.preds)

if __name__ == '__main__':
	random.seed()

	tree_gen = TreeGen()
	src_root = tree_gen.generate()
	do_it(src_root, tree_gen.predicates, 'src')
	
	tgt_root = src_root.deep_copy()
	# change all predicates in the target to be lowercase version of source
	tgt_root.map(state_preds_lower)
	preds_lower = [p.lower() for p in tree_gen.predicates]
	do_it_split(tgt_root, preds_lower, 'tgt')
