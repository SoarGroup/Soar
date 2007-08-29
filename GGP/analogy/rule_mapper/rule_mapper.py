import sys, os
import gdlyacc
from partialmap import PartialMap
from predicate import get_predicates
from find_max import find_max
import pdb

class CommitPoint:
	def __init__(self, map):
		self.map = map
	
	def suppress_last(self):
		self.map.suppress_match(self.rule_match[0], self.rule_match[1])

def do_mapping(src_int_rep, tgt_int_rep):
	src_preds = get_predicates(src_int_rep.get_all_rules(), src_int_rep.get_roles())
	tgt_preds = get_predicates(tgt_int_rep.get_all_rules(), tgt_int_rep.get_roles())

	# someday maybe we'll be able to bin predicates into different types, but until
	# that day ...
	src_preds = zip(src_preds, [0] * len(src_preds))
	tgt_preds = zip(tgt_preds, [0] * len(tgt_preds))
	# a list whose elements record which commitments were made at each
	# step, which commitments are suppressed, number of other rule matches 
	# invalidated, and the score
	history = [] # [(partial map, rule match, suppressed, #invalidated, score)]

	mapping = PartialMap.create(src_int_rep, src_preds, tgt_int_rep, tgt_preds)
	history.append(CommitPoint(mapping))
	
	best_map = None
	legals_mapped = False
	for x in range(1):
		bottomed_out = False
		while not bottomed_out:
			next_mapping = mapping.copy()
			if not legals_mapped:
				sr, tr, invalidated, score = next_mapping.add_best_legal_rule_match()
				if sr is None:
					legals_mapped = True
					continue
			else:
				sr, tr, bmap, score = next_mapping.get_best_rule_match()
				if sr is None:
					# there's no matches at this point, probably because they're all
					# suppressed
					history[-1].can_unroll = False
					bottomed_out = True
					continue

				invalidated = next_mapping.add_rule_match(sr, tr, bmap)

			history[-1].rule_match = (sr, tr)
			history[-1].invalidated = invalidated
			history[-1].score = score
			if next_mapping.complete():
				bottomed_out = True
				num_matched = len(mapping.get_pred_matches())
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
		unroll_i, dof = find_max(history, lambda x: x.map.src_rdof(x.rule_match[0]))
		#print "unrolling to %d, with %d dof" % (unroll_i, dof)

		history = history[:unroll_i+1]
		mapping = history[unroll_i].map
		history[unroll_i].suppress_last()

	return best_map

def main(src_rules, tgt_rules):
	gdlyacc.parse_file(src_rules)
	src_int_rep = gdlyacc.int_rep.copy()
	gdlyacc.parse_file(tgt_rules)
	tgt_int_rep = gdlyacc.int_rep.copy()
	
	best_map = do_mapping(src_int_rep, tgt_int_rep)
	return best_map.get_pred_matches()

if __name__ == '__main__':
	#import psyco
	#psyco.bind(PartialMap)
	matches = main(sys.argv[1], sys.argv[2])
	for s, t in matches.items():
		print s.get_name(), t.get_name()

