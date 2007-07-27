import os, sys
basedir = os.path.join('..','..')
sys.path.append(os.path.join(basedir, 'scripts','pyparser'))
sys.path.append(os.path.join(basedir, 'analogy','src'))

import gdlyacc
from GDL import *
from PositionIndex import PositionIndex
import run_mapper

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
		if src_const in consts2rels_src:
			src_set = set([i[1] for i in consts2rels_src[src_const]])
		else:
			const_match_scores.append((src_const, tgt_const, 0))
			continue
		if tgt_const in consts2rels_tgt:
			tgt_set = set([i[1] for i in consts2rels_tgt[tgt_const]])
		else:
			const_match_scores.append((src_const, tgt_const, 0))
			continue
		int_size = len(src_set.intersection(tgt_set))
		const_match_scores.append((src_const, tgt_const, int_size))

const_match_scores.sort(lambda x, y: x[2] - y[2])

const_match = {}
while len(const_match_scores) > 0:
	src_const, tgt_const, score = const_match_scores.pop() # "best" match
	const_match_scores = filter(lambda x: x[0] != src_const and x[1] != tgt_const, const_match_scores)
	const_match[src_const] = tgt_const

for src, tgt in const_match.items():
	print '%s ==> %s' % (src, tgt)
