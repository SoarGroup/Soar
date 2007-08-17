import sys, os
base_dir = os.path.join('..','..')
sys.path.append(os.path.join(base_dir, 'scripts', 'pyparser'))
import gdlyacc
from partialmap import PartialMap
from predicate import get_predicates
import pdb

def find_max(seq, func):
	if len(seq) == 0:
		return -1
	max = func(seq[0])
	mpos = 0
	for i in range(1,len(seq)):
		v = func(seq[i])
		if max < v:
			max = v
			mpos = i
	return (mpos, max)

class CommitPoint:
	def __init__(self, map):
		self.map = map
	
	def suppress_last(self):
		self.map.suppress_match(self.rule_match[0], self.rule_match[1])

def do_mapping(src_int_rep, tgt_int_rep):
	src_preds = get_predicates(src_int_rep.get_all_rules())
	tgt_preds = get_predicates(tgt_int_rep.get_all_rules())

	# a list whose elements record which commitments were made at each
	# step, which commitments are suppressed, number of other rule matches 
	# invalidated, and the score
	history = [] # [(partial map, rule match, suppressed, #invalidated, score)]

	mapping = PartialMap.create(src_int_rep, src_preds, tgt_int_rep, tgt_preds)
	history.append(CommitPoint(mapping))
	
	best_map = None

	for x in range(1):
		bottomed_out = False
		while not bottomed_out:
			sr, tr, bmap, score = mapping.get_best_rule_match()
			if sr is None:
				# there's no matches at this point, probably because they're all
				# suppressed
				history[-1].can_unroll = False
				bottomed_out = True
				continue

			next_mapping = mapping.copy()
			invalidated = next_mapping.add_rule_match(sr, tr, bmap)
			history[-1].rule_match = (sr, tr)
			history[-1].invalidated = invalidated
			history[-1].score = score
			if next_mapping.complete():
				bottomed_out = True
				num_matched = len(mapping.get_matched_preds())
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

if __name__ == '__main__':
	gdlyacc.parse_file(sys.argv[1])
	src_int_rep = gdlyacc.int_rep.copy()
	gdlyacc.parse_file(sys.argv[2])
	tgt_int_rep = gdlyacc.int_rep.copy()
	best_map = do_mapping(src_int_rep, tgt_int_rep)
	best_map.print_pred_matches(sys.stdout)
