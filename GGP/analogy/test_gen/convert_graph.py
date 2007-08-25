import sys, re

edge_regex = re.compile(r'([A-Za-z_]+)\s*->\s*([A-Za-z_]+)')

def extract_nodes_edges(gdl_file):
	nodes = set()
	edges = set()

	for line in gdl_file:
		m = edge_regex.match(line)
		if m:
			nodes.add(m.group(1))
			nodes.add(m.group(2))
			edges.add((m.group(1), m.group(2)))

	return (nodes, edges)

def gdl2graphdiff(nodes, edges, label, graphdiff_file, node_types={}):
	# have to keep a consistent ordering
	nodes = list(nodes)
	nodes.sort()

	graphdiff_file.write('#label\n%s\n' % label)
	# make all nodes the same type
	graphdiff_file.write('#types\n')
	typeline = ''
	for n in nodes:
		if n in node_types:
			typeline += '%s ' % node_types[n]
		else:
			typeline += '1 ' # type 1 is the default for unmatched nodes
	
	graphdiff_file.write('%s\n' % typeline)
	graphdiff_file.write('#edges\n')
	name2num = dict((i[1],i[0]) for i in enumerate(nodes))
	for e in edges:
		graphdiff_file.write('%s %s 1\n' % (name2num[e[0]], name2num[e[1]]))
	
	for i, n in enumerate(nodes):
		print '%d %s' % (i, n)

def convert1(gdl_file, label, graphdiff_file):
	nodes, edges = extract_nodes_edges(gdl_file)
	gdl2graphdiff(nodes, edges, label, graphdiff_file)

def convert2(gdl_file1, label1, gdl_file2, label2, graphdiff_file):
	"""Convert two gdl graphs into graphdiff, making sure that common nodes share types"""

	nodes1, edges1 = extract_nodes_edges(gdl_file1)
	nodes2, edges2 = extract_nodes_edges(gdl_file2)

	common_nodes = nodes1 & nodes2
	node_types = dict((i[1], i[0]+10) for i in enumerate(common_nodes))

	gdl2graphdiff(nodes1, edges1, label1, graphdiff_file, node_types)
	gdl2graphdiff(nodes2, edges2, label2, graphdiff_file, node_types)

if __name__ == '__main__':
	if len(sys.argv) == 2:
		# gdl from stdin, label as argument
		label = sys.argv[1]
		convert1(sys.stdin, label, sys.stdout)
	elif len(sys.argv) == 3:
		# gdlfile label
		gdl = open(sys.argv[1])
		convert1(gdl, sys.stdout, sys.argv[2])
		gdl.close()
	else:
		assert len(sys.argv) == 5
		# gdlfile1 label1 gdlfile2 label2
		gdl1 = open(sys.argv[1])
		l1 = sys.argv[2]
		gdl2 = open(sys.argv[3])
		l2 = sys.argv[4]
		convert2(gdl1, l1, gdl2, l2, sys.stdout)
		gdl1.close()
		gdl2.close()
