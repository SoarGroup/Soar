class Heuristic:
	def __init__(self, weights):
		"weights is a table from predicate to its weight"
		self.__w = weights
	
	def __call__(self, state):
		if len(state) == 0:
			return 0
		return (reduce(lambda x,y:x+y, (self.__w[p] for p in state)) / len(state))

def heuristic_mserr(h, root):
	sq_sum = 0.0
	actual_dists = {}
	root.dist_to_goal(actual_dists)
	for s, d in actual_dists.items():
		sq_sum += (d - h(s)) ** 2
	return sq_sum / len(actual_dists)

def pred_accuracy(h, root):
	# predicate -> [mean squared error, num states]
	mserr = {}
	actual_dists = {}
	root.dist_to_goal(actual_dists)
	for s, d in actual_dists.items():
		for p in s:
			mserr.setdefault(p,[0,0])
			mserr[p][0] += (d - h(p)) ** 2
			mserr[p][1] += 1
	
	return dict((p, e[0] / float(e[1])) for p, e in mserr.items())

def pred_accuracy1(h, root):
	"divide by number of predicates in state"

	# predicate -> [mean squared error, num states]
	mserr = {}
	actual_dists = {}
	root.dist_to_goal(actual_dists)
	for s, d in actual_dists.items():
		ss = float(len(s))
		for p in s:
			mserr.setdefault(p,[0,0])
			mserr[p][0] += (d - h(p)/ss) ** 2
			mserr[p][1] += 1
	
	return dict((p, e[0] / float(e[1])) for p, e in mserr.items())
