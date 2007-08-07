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

def cross_product(l1, l2):
	r = []
	for a1 in l1:
		r.extend((a1, a2) for a2 in l2)
	
	return r

def build_c2p(int_rep, map = {}):
	""" returns a map of constants to the predicates that they appear in """

	c2p = {} # const -> [(pos, pred)]
	for g in int_rep.get_statics() + int_rep.get_inits():
		pred = g.get_predicate()
		for p in PositionIndex.get_all_positions(g):
			term = p.fetch(g)
			if isinstance(term, Constant) and \
					isinstance(term.get_name(), str) and \
					term.get_name() not in exclude:
				c2p.setdefault(term.get_name(), []).append((p, pred))
	return c2p

def filter_matches(matches, cmap, pmap):
	""" filters out ground matches that violate the commitments already
    set by the current (partial) constant mapping

    cmap = constant mapping
    pmap = position mapping for this predicate """

	good_matches = []

	# is the same for all grounds, only have to calculate once
	all_src_p = pmap.keys()
	all_tgt_p = [pmap[p] for p in all_src_p]

	pos_pairs = zip(all_src_p, all_tgt_p)

	for src_g, tgt_g in matches:
		valid = True
		for sp, tp in pos_pairs:
			sc = sp.fetch(src_g)
			if sc in cmap:
				tc = tp.fetch(tgt_g)
				if cmap[sc] != tc:
					# violates commitment
					valid = False
					break
		if valid:
			good_matches.append((src_g, tgt_g))
	
	return good_matches

def commit_ground_match(src_g, tgt_g, cmap, pmap):
	""" make constant mapping commitments based on the matching of these two grounds

    cmap = constant map
    pmap = position map """

	for src_p in pmap:
		tgt_p = pmap[src_p]
		src_c = src_p.fetch(src_g)
		tgt_c = tgt_p.fetch(tgt_g)
		assert src_c not in cmap or cmap[src_c] == tgt_c, "Constant mapping inconsistency"
		if src_c not in cmap:
			cmap[src_c] = tgt_c


# get the mapping
pred_map = run_mapper.run_mapper(sys.argv[1], sys.argv[2])

consts2preds_src = {} # const -> [(pos, pred)]
consts2preds_tgt = {} # const -> [(pos, pred)]

gdlyacc.parse_file(sys.argv[1])
src_c2p = build_c2p(gdlyacc.int_rep, pred_map)
src_gnds = {} # pred -> [grounds]
for g in gdlyacc.int_rep.get_statics() + gdlyacc.int_rep.get_inits():
	src_gnds.setdefault(g.get_predicate(), []).append(g)

gdlyacc.parse_file(sys.argv[2])
tgt_c2p = build_c2p(gdlyacc.int_rep)
tgt_gnds = {} # pred -> [grounds]
for g in gdlyacc.int_rep.get_statics() + gdlyacc.int_rep.get_inits():
	tgt_gnds.setdefault(g.get_predicate(), []).append(g)

# this is temporary, in the future, order the predicates by how many other
# predicates it constrains
pred_order = filter(lambda x: x in pred_map, src_gnds.keys())

cmap = {} # the committed mapping
for src_p in pred_order:
	tgt_p = pred_map[src_p]

	if src_p not in src_gnds or tgt_p not in tgt_gnds:
		print >> sys.stderr, "PROBABLY A BAD MATCH BETWEEN %s AND %s" % (src_p, tgt_p)
		continue

	matches = cross_product(src_gnds[src_p], tgt_gnds[tgt_p])

	# get the position mapping this is fake right now, but we should get this
	# from a different script in the future right now just assume all the
	# constant positions are preserved
	tmp_src_g, tmp_tgt_g = matches[0]
	src_p = PositionIndex.get_all_positions(tmp_src_g)
	tgt_p = PositionIndex.get_all_positions(tmp_tgt_g)
	pmap = dict([(p, p) for p in src_p if p in tgt_p])

	# here we're going to match up all the grounds for this predicate
	# the order of the matching is random and can affect the quality of the
	# match, but I don't have any good idea about how to do it right now
	matches = filter_matches(matches, cmap, pmap)
	while len(matches) > 0:
		src_g, tgt_g = matches.pop()
		commit_ground_match(src_g, tgt_g, cmap, pmap)
		matches = filter_matches(matches, cmap, pmap)

for src_c, tgt_c in cmap.items():
	print '%s -> %s' % (src_c, tgt_c)
