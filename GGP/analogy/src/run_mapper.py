import sys, os, tempfile
import xkif_gen
import gdlyacc

if 'GGP_PATH' in os.environ:
	mapper = os.path.join(os.environ['GGP_PATH'], 'analogy', 'src', 'mapper')
else:
	mapper = 'mapper'

def run_mapper(src_kif, tgt_kif, del_tmp = True):
	xkif_gen.parse_file(src_kif)
	tmp_fd, src_xkif = tempfile.mkstemp(suffix='.xkif')
	print >> sys.stderr, src_xkif
	xkif = os.fdopen(tmp_fd, 'w')
	xkif.write(xkif_gen.output)
	xkif.close()

	xkif_gen.parse_file(tgt_kif)
	tmp_fd, tgt_xkif = tempfile.mkstemp(suffix='.xkif')
	print >> sys.stderr, tgt_xkif
	xkif = os.fdopen(tmp_fd, 'w')
	xkif.write(xkif_gen.output)
	xkif.close()

	# find types
	gdlyacc.parse_file(src_kif)
	tmp_fd, src_types = tempfile.mkstemp()
	print >> sys.stderr, src_types
	os.fdopen(tmp_fd,'w').write(os.popen('python infer_types.py %s' % src_kif).read())

	gdlyacc.parse_file(tgt_kif)
	tmp_fd, tgt_types = tempfile.mkstemp()
	print >> sys.stderr, tgt_types
	os.fdopen(tmp_fd,'w').write(os.popen('python infer_types.py %s' % tgt_kif).read())

	mapping = {}
	output = os.popen('%s %s %s %s %s' % (mapper, src_xkif, tgt_xkif, src_types, tgt_types)).readlines()
	for line in output:
		print line
		if line.startswith('MATCH'):
			tokens = line.split()
			assert len(tokens) == 3, line
			mapping[tokens[1]] = tokens[2]
	
	if del_tmp:
		os.remove(src_xkif)
		os.remove(tgt_xkif)
		os.remove(src_types)
		os.remove(tgt_types)

	return mapping

if __name__ == '__main__':
	if sys.argv[1] == '-d':
		mapping = run_mapper(sys.argv[2], sys.argv[3], False)
	else:
		mapping = run_mapper(sys.argv[1], sys.argv[2])
	for src in mapping:
		print '%s %s' % (src, mapping[src])
