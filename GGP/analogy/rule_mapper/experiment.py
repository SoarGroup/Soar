"""
Number of bins versus ratio of correct predicate matches. Also vary allowing partial body maps.
"""
import os, sys
base_dir = os.path.join('..')
sys.path.append(os.path.join(base_dir, 'test_gen'))

import options
import partialmap
import rule_mapper2
import test_gen

import psyco
psyco.bind(partialmap)
psyco.bind(partialmap.PartialMap)

max_bins = 10

# set options
options.ALLOW_PARTIAL_BODY_MAPS = (sys.argv[1] == '1')

for bins in range(1,max_bins+1):
	options.BINS = bins
	score = 0
	for i in range(options.ITERATIONS_PER_PAIR):
		# make the rules
		src_rules, tgt_rules = test_gen.gen_split_tgt()
		
		# run the mapper
		total_preds = int(os.popen('python count_preds.py %s' % src_rules).read())
		pred_matches = rule_mapper2.main(src_rules, tgt_rules)
		num_matched = sum([sp.get_name().lower() == tp.get_name() for sp, tp in pred_matches.items()])

		os.remove(src_rules)
		os.remove(tgt_rules)
		
		ratio = num_matched / float(total_preds)
		sys.stderr.write('.')
		sys.stderr.flush()
		score += ratio

	sys.stderr.write('\n')
	print '%d %f' % (bins, score / options.ITERATIONS_PER_PAIR)
