import pdb
from rule import *
from state import State
from treegen import TreeGen

def do_it(root, predicates, prefix):
	all_states = []
	root.get_all_states(all_states)
	root.make_graphviz(open('%s_game.gdl' % prefix,'w'))

	rules = []
	root.make_max_rules(rules, frozenset(predicates))

	rs = RuleSet(rules, all_states, set(predicates))
	#rs.minimize_rules()
	rs.extract_all_commons()
	rs.minimize_rules()

	rs.make_rule_graph(open('%s_rules.gdl' % prefix, 'w'))
	rs.make_kif(root.preds, open('%s_rules.kif' % prefix, 'w'))

	print rs.check_consistency()

def state_preds_lower(s):
	s.preds = frozenset(p.lower() for p in s.preds)

if __name__ == '__main__':
	random.seed(0)

	tree_gen = TreeGen()
	src_root = tree_gen.generate()
	do_it(src_root, tree_gen.predicates, 'src')
	
	tgt_root = src_root.deep_copy()
	# change all predicates in the target to be lowercase version of source
	tgt_root.map(state_preds_lower)
	preds_lower = [p.lower() for p in tree_gen.predicates]
	do_it(tgt_root, preds_lower, 'tgt')
