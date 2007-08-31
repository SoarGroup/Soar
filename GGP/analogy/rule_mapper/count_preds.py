def count_preds(int_rep):
	preds = set()
	for r in int_rep.get_all_rules():
		sentences = [r.get_head()] + r.get_body()
		for p in (s.get_predicate() for s in sentences):
			if p is not None:
				preds.add(p)
	for p in (s.get_predicate() for s in int_rep.get_inits() + int_rep.get_statics()):
		if p is not None:
			preds.add(p)
	
	return len(preds)

if __name__ == '__main__':
	import gdlyacc
	import sys
	gdlyacc.parse_file(sys.argv[1])
	print count_preds(gdlyacc.int_rep)

