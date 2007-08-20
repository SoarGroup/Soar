import os, sys
import gdlyacc
from GDL import *
from PositionIndex import PositionIndex
import run_mapper

# constants to ignore, along with numbers
exclude = ['north','south','east','west', 'yes', 'no']

kpath = './klinux'

def makeGraph(file, label, grounds, pred2typeNum = {}):
	const2predpos = {}
	groundOrder = []

	if len(pred2typeNum) > 0:
		existingTypeNums = pred2typeNum.values()[:]
		existingTypeNums.sort()
		nextTypeNum = existingTypeNums[-1] + 1
	else:
		nextTypeNum = 1

	for g in grounds:
		p = g.get_predicate()
		if p == 'location':
			continue

		has_obj_const = False
		groundIndex = len(groundOrder)
		for i, t in enumerate(g.get_terms()):
			if isinstance(t.get_name(), str) and t.get_name() not in exclude:
				const2predpos.setdefault(t.get_name(), []).append((groundIndex, i))
				has_obj_const = True
		if not has_obj_const:
			continue
			
		groundOrder.append(g)

		if p in pred2typeNum:
			type = pred2typeNum[p]
		else:
			# new predicate
			type = nextTypeNum
			nextTypeNum += 1
			pred2typeNum[p] = type
			if p in predMap:
				# source and target predicates mapped on to each other have same type
				pred2typeNum[predMap[p]] = type

	file.write("#label\n%s\n#types\n" % label)
	for g in groundOrder:
		file.write('%d ' % pred2typeNum[g.get_predicate()])

	file.write('\n#edges\n')
	for predposlist in const2predpos.values():
		for i in range(len(predposlist)):
			for j in range(i+1, len(predposlist)):
				n1 = predposlist[i][0]
				n2 = predposlist[j][0]
				file.write('%d %d 1\n' % (n1, n2))
	
	return dict(enumerate(groundOrder))

# get the mapping
predMap = run_mapper.run_mapper(sys.argv[1], sys.argv[2])

# run k
typeNumMap = {}
graphfile = open('graphs', 'w')

gdlyacc.parse_file(sys.argv[1])
grounds = gdlyacc.int_rep.get_statics() + gdlyacc.int_rep.get_inits()
src_i2g = makeGraph(graphfile, 'source', grounds, typeNumMap)

gdlyacc.parse_file(sys.argv[2])
grounds = gdlyacc.int_rep.get_statics() + gdlyacc.int_rep.get_inits()
tgt_i2g = makeGraph(graphfile, 'target', grounds, typeNumMap)

graphfile.close()

lines = os.popen('%s graph +f %s' % (kpath, 'graphs')).readlines()

src_tgt_align = lines[0][lines[0].find('Alignment') + 10:-2].split()
for pair in src_tgt_align:
	src_i, tgt_i = tuple(pair[1:-1].split(','))
	print '%s ==> %s' % (src_i2g[int(src_i)], tgt_i2g[int(tgt_i)])
