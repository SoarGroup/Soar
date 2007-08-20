import os, sys
import gdlyacc
from GDL import *
from PositionIndex import PositionIndex
import run_mapper

import pdb

# constants to ignore, along with numbers
exclude = ['north','south','east','west']

BASE_BOOST = 1
SAMEPOS_BOOST = 3

def build_constant_relations(int_rep, map = {}):
	consts2rels = {} # const -> [(pos, rel)]
	for s in int_rep.get_statics():
		rel = s.get_relation()
		for p in PositionIndex.get_all_positions(s):
			term = p.fetch(s)
			if isinstance(term, Constant) and isinstance(term.get_name(), str) and term.get_name() not in exclude:
				if rel in map:
					rel = map[rel]
				consts2rels.setdefault(term.get_name(), []).append((p, rel))
	return consts2rels

# get the mapping
pred_map = run_mapper.run_mapper(sys.argv[1], sys.argv[2])

consts2preds_src = {} # const -> [(pos, pred)]
consts2preds_tgt = {} # const -> [(pos, pred)]

gdlyacc.parse_file(sys.argv[1])
consts2preds_src = build_constant_relations(gdlyacc.int_rep, pred_map)

gdlyacc.parse_file(sys.argv[2])
consts2preds_tgt = build_constant_relations(gdlyacc.int_rep)

const_match_scores = []
for src_const in consts2preds_src:
	for tgt_const in consts2preds_tgt:
		print "%s => %s" % (src_const, tgt_const)
		if src_const not in consts2preds_src:
			const_match_scores.append((src_const, tgt_const, 0))
			continue
		if tgt_const not in consts2preds_tgt:
			const_match_scores.append((src_const, tgt_const, 0))
			continue

		src_pred2pos = {} # pred -> [pos]
		tgt_pred2pos = {} # pred -> [pos]
		for pos, pred in consts2preds_src[src_const]:
			src_pred2pos.setdefault(pred,[]).append(pos)
		for pos, pred in consts2preds_tgt[tgt_const]:
			tgt_pred2pos.setdefault(pred,[]).append(pos)
		common_preds = [r for r in src_pred2pos.keys() if r in tgt_pred2pos.keys()]

		score = 0
		for p in common_preds:
			common_pos = set(src_pred2pos[p] + tgt_pred2pos[p])
			if len(common_pos) > 0:
				print "  Match position for %s" % p
			else:
				print "  Match %s" % p
			score += len(common_pos) * SAMEPOS_BOOST + BASE_BOOST

		const_match_scores.append((src_const, tgt_const, score))

const_match_scores.sort(lambda x, y: x[2] - y[2])

const_match = {}
while len(const_match_scores) > 0:
	src_const, tgt_const, score = const_match_scores.pop() # "best" match
	const_match_scores = filter(lambda x: x[0] != src_const and x[1] != tgt_const, const_match_scores)
	const_match[src_const] = tgt_const

for src_pred, tgt_pred in pred_map.items():
	print 'PREDICATE %s %s' % (src_pred, tgt_pred)

for src, tgt in const_match.items():
	print 'CONSTANT %s %s' % (src, tgt)
