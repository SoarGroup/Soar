import sys, os
import gdlyacc
import GDL
import pdb

def chase_to_moves(r, preds2rules, seen):
	moves = []
	if r in seen:
		# probably stuck in a recursive rule
		return []
	seen.add(r)
	for b in r.get_body():
		if b.get_type() == GDL.Sentence.MOVE and not b.is_negated():
			moves.append(b.get_predicate())

	for b in r.get_body():
		if b.get_type() == GDL.Sentence.ELAB and not b.is_negated():
			if b.get_predicate() not in preds2rules:
				# fact
				continue
			for r1 in preds2rules[b.get_predicate()]:
				chase_res = chase_to_moves(r1, preds2rules, seen)
				moves.extend(chase_res)
	return moves

def calc_move_effects(int_rep):
	preds2rules = {}
	for r in int_rep.get_elab_rules():
		head_pred = r.get_head().get_predicate()
		preds2rules.setdefault(head_pred,[]).append(r)

	move2effects={}
	for r in int_rep.get_update_rules():
		moves = chase_to_moves(r, preds2rules, set())
		for m in moves:
			move2effects.setdefault(m,set()).add(r.get_head().get_predicate())

	return move2effects

if __name__ == '__main__':
	gdlyacc.parse_file(sys.argv[1])
	move2effects = calc_move_effects(gdlyacc.int_rep)
	for m, caused in move2effects.items():
		print '=================================================='
		print '->%s' % m
		for c in caused:
			print c
