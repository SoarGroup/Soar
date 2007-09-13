import sys, os, random
import gdlyacc
from partialmap2 import PartialMap
from predicate import get_predicates
import placetype
import options
import pdb

class CommitPoint:
	def __init__(self, map):
		self.map = map
	
	def suppress_last(self):
		self.map.suppress_match(self.rule_match[0], self.rule_match[1])

def do_mapping(src_int_rep, tgt_int_rep, src_pred_bins = {}, tgt_pred_bins = {}):
	src_preds = get_predicates(src_int_rep.get_all_rules(), src_int_rep.get_roles())
	tgt_preds = get_predicates(tgt_int_rep.get_all_rules(), tgt_int_rep.get_roles())

	src_pred_bins1 = src_pred_bins.copy()
	tgt_pred_bins1 = tgt_pred_bins.copy()
	
	# try the heuristic that any predicate that has a number is a different type from
	# predicates that don't involve numbers
	num_place_types = set([placetype.COORD, placetype.NUM_MAX, placetype.NUM_MIN, placetype.NUM])
	for pred_set, type_bins in [(src_preds, src_pred_bins1), (tgt_preds, tgt_pred_bins1)]:
		for p in pred_set:
			if p not in type_bins and len(set(p.get_place_types()) & num_place_types) > 0:
				type_bins[p] = 100

	# 0 is the default bin that can map to anything else. All predicates that aren't
	# assigned bins get 0
	src_preds1 = [(p, src_pred_bins1.get(p, 0)) for p in src_preds]
	tgt_preds1 = [(p, tgt_pred_bins1.get(p, 0)) for p in tgt_preds]

	# a list whose elements record which commitments were made at each
	# step, which commitments are suppressed, number of other rule matches 
	# invalidated, and the score
	history = [] # [(partial map, rule match, suppressed, #invalidated, score)]

	mapping = PartialMap.create(src_int_rep, src_preds1, tgt_int_rep, tgt_preds1)
	history.append(CommitPoint(mapping))
	
	best_map = None
	for x in range(options.NUM_RETRIES):
		bottomed_out = False
		while not bottomed_out:
			next_mapping = mapping.copy()

			sr, tr, bmap, score = next_mapping.get_best_rule_match()

			if sr is None:
				# there's no matches at this point, probably because they're all
				# suppressed
				history[-1].can_unroll = False
				bottomed_out = True
				break

			invalidated = next_mapping.add_rule_match(sr, tr, bmap)
			history[-1].rule_match = (sr, tr)
			history[-1].invalidated = invalidated
			history[-1].score = score
			if next_mapping.complete():
				bottomed_out = True
				num_matched = len(next_mapping.get_pred_matches())
				#print "Matched %d out of %d source and %d target predicates" % \
				#		(num_matched, len(src_preds), len(tgt_preds))
				if best_map == None:
					best_map = next_mapping
					#print 'best map score is now ',  best_map.score()
				else:
					if best_map.score() < next_mapping.score():
						best_map = next_mapping
						#print 'best map score is now ',  best_map.score()
			else:
				mapping = next_mapping
				history.append(CommitPoint(mapping))
	
		# okay, now we have to decide where to unroll to
		# unroll to most invalidated
		#unroll_i, invalidated = find_max(history, lambda x: x.invalidated)
		#print "unrolling to %d, with %d invalidations" % (unroll_i, invalidated)
		# unroll to most degrees of freedom
		#unroll_i, dof = find_max(history, lambda x: x.map.src_rdof(x.rule_match[0]))
		#print "unrolling to %d, with %d dof" % (unroll_i, dof)

		unroll_i = 0
		history = history[:unroll_i+1]
		mapping = history[unroll_i].map
		history[unroll_i].suppress_last()
		print "UNROLL UNROLL UNROLL UNROLL UNROLL UNROLL UNROLL UNROLL"

	return best_map

def map_kifs(src_rules, tgt_rules, src_pred_bins = {}, tgt_pred_bins={}):
	gdlyacc.parse_file(src_rules)
	src_int_rep = gdlyacc.int_rep.copy()
	gdlyacc.parse_file(tgt_rules)
	tgt_int_rep = gdlyacc.int_rep.copy()
	
	best_map = do_mapping(src_int_rep, tgt_int_rep, src_pred_bins, tgt_pred_bins)
	return best_map.get_pred_matches()

if __name__ == '__main__':
	random.seed(0)
	sys.path.append('../test_gen')
#	import psycocompile

	matches = map_kifs(sys.argv[1], sys.argv[2])
	for s, t in matches.items():
		print s.get_name(), t.get_name()

