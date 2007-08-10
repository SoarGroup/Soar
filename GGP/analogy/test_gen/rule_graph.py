import pdb

def make_rule_graph(rules):
	predicates = set()
	nodes = set()
	nodes2rules = {} # node -> [rules that fire in that node]
	preds2nodes = {} # predicate -> [nodes that contain it]
	for r in rules:
		pcs = r.get_pconds()
		supernode = None
		to_erase = []
		for n in nodes:
			if pcs <= n:
				supernode = n
				break
			elif n <= pcs:
				supernode = pcs
				to_erase.append(n)
				nodes2rules.setdefault(pcs, []).extend(nodes2rules[n])
				del nodes2rules[n]
				for p in n:
					preds2nodes[p].remove(n)

		for n in to_erase:
			nodes.remove(n)

		if supernode == None:
			nodes.add(pcs)
			supernode = pcs
			for p in pcs:
				preds2nodes.setdefault(p, []).append(pcs)
		
		nodes2rules.setdefault(supernode,[]).append(r)

	edges = set()
	for n in nodes2rules:
		assert n in nodes
		firing = nodes2rules[n]
		for fr in firing:
			if fr.is_goal_rule():
				edges.add((n, 'goal'))
			else:
				#conn_nodes = reduce(lambda x, y: x + y, (preds2nodes[p] for p in fr.get_rhs()))
				#for n2 in conn_nodes:
				#	assert n2 in nodes
				#	edges.add((n, n2))
				
				# instead of making an edge to every node that contains any rhs predicate,
				# only make edges to the nodes that are supersets of all rhs predicates
				for n2 in nodes:
					if n2 != n and fr.get_rhs() <= n2:
						edges.add((n, n2))
	
	file = open('rules.gdl', 'w')
	file.write('digraph g {\n')
	for e in edges:
		n1 = list(e[0])
		n1.sort()
		n1 = ''.join(n1)
		if e[1] != 'goal':
			n2 = list(e[1])
			n2.sort()
			n2 = ''.join(n2)
		else:
			n2 = 'goal'
		file.write('%s -> %s\n' % (n1, n2))
	file.write('}')
	file.close()
