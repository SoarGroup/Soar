import os, sys
basedir = os.path.join('..','..')
sys.path.append(os.path.join(basedir, 'scripts','pyparser'))
sys.path.append(os.path.join(basedir, 'analogy','src'))

import gdlyacc
from GDL import *
from PositionIndex import PositionIndex
import run_mapper

# constants to ignore, along with numbers
exclude = ['north','south','east','west', 'yes', 'no']

# get the mapping
#pred_map = run_mapper.run_mapper(sys.argv[1], sys.argv[2])

src_grounds = {}
const2predpos = {}

gdlyacc.parse_file(sys.argv[1])

print 'graph g {'
for x, g in enumerate(gdlyacc.int_rep.get_statics() + gdlyacc.int_rep.get_inits()):
	if g.get_predicate() == 'location':
		continue

	has_obj_const = False
	for i, t in enumerate(g.get_terms()):
		if isinstance(t.get_name(), str) and t.get_name() not in exclude:
			const2predpos.setdefault(t.get_name(), []).append((x, i))
			has_obj_const = True
	if has_obj_const:
		print '%d [label="%s"]' % (x, str(g))

for predposlist in const2predpos.values():
	for i in range(len(predposlist)):
		for j in range(i+1, len(predposlist)):
			print '%d -- %d' % (predposlist[i][0], predposlist[j][0])

print '}'
