import os, sys
basedir = os.path.join('..','..')
sys.path.append(os.path.join(basedir, 'scripts','pyparser'))
sys.path.append(os.path.join(basedir, 'analogy','src'))

import gdlyacc
from GDL import *
from PositionIndex import PositionIndex
import run_mapper

import pdb

# constants to ignore, along with numbers
exclude = ['north','south','east','west']

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

consts2rels_src = {} # const -> [(pos, rel)]
consts2rels_tgt = {} # const -> [(pos, rel)]

gdlyacc.parse_file(sys.argv[1])
consts2rels_src = build_constant_relations(gdlyacc.int_rep, pred_map)

gdlyacc.parse_file(sys.argv[2])
consts2rels_tgt = build_constant_relations(gdlyacc.int_rep)

const_match_scores = []
for src_const in consts2rels_src:
	for tgt_const in consts2rels_tgt:
		#print "%s => %s" % (src_const, tgt_const)
		if src_const not in consts2rels_src:
			const_match_scores.append((src_const, tgt_const, 0))
			continue
		if tgt_const not in consts2rels_tgt:
			const_match_scores.append((src_const, tgt_const, 0))
			continue
		rel2pos = {} # rel -> [pos]
		everything = consts2rels_src[src_const] + consts2rels_tgt[tgt_const]
		for pos, rel in everything:
			rel2pos.setdefault(rel,[]).append(pos)
		
		score = 0
		for rel, poses in rel2pos.items():
			if len(poses) == 2:
				if poses[0] == poses[1]:
					#print "  Match position for %s" % rel
					score += 3 # extra points for position match
				else:
					#print "  Match %s" % rel
					score += 1
				
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
