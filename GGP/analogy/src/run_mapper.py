import sys, os, tempfile
base_dir = os.path.join('..','..')
mapper = os.path.join(base_dir, 'analogy', 'src', 'mapper')
sys.path.append(os.path.join(base_dir, 'scripts', 'pyparser'))
import xkif_gen

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

	mapping = {}
	for line in os.popen('%s %s %s' % (mapper, src_xkif, tgt_xkif)).readlines():
		print line
		if line.startswith('MATCH'):
			tokens = line.split()
			assert len(tokens) == 3, line
			mapping[tokens[1]] = tokens[2]
	
	if del_tmp:
		os.remove(src_xkif)
		os.remove(tgt_xkif)

	return mapping

if __name__ == '__main__':
	if sys.argv[1] == '-d':
		mapping = run_mapper(sys.argv[2], sys.argv[3], False)
	else:
		mapping = run_mapper(sys.argv[1], sys.argv[2])
	for src in mapping:
		print '%s %s' % (src, mapping[src])
