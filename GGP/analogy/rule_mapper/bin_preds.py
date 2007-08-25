import sys, os
import GDL
import gdlyacc

def make_bins(rules, inits, num_bins):
	bin_num = 0
	pred2bin = {}
	for r in rules:
		preds = [r.get_head().get_predicate()] + [b.get_predicate() for b in r.get_body()]
		for p in preds:
			if p is not None and p not in pred2bin:
				pred2bin[p] = bin_num
				bin_num += 1
				if bin_num == num_bins: 
					bin_num = 0

	for p in [i.get_predicate() for i in inits]:
		if p not in pred2bin:
			pred2bin[p] = bin_num
			bin_num += 1
			if bin_num == num_bins: 
				bin_num = 0

	return pred2bin

if __name__ == '__main__':
	gdlyacc.parse_file(sys.argv[1])
	for p, b in make_bins(gdlyacc.int_rep.get_all_rules(), 4).items():
		print '%s -> %d' % (p, b)
