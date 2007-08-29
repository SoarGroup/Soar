"""An implementation of the A* searching algorithm.
dyoo@hkn.eecs.berkeley.edu
I got so disgusted at my previous attempt at A*, so here I go again.
Hopefully this version will be easier on the eyes.
A* is a search algorithm that's similar to Dijkstra's algorithm: given
a graph, a start node, and a goal, A* will search for the shortest
path toward the goal.
To help it get there faster, we can provide a heuristic that evaluates
how far we are from that goal.  With a good heuristic, finding an
optimal solution takes MUCH less time.
The main function that one would use is aStar().  Take a look at
_testAStar() to see how it's used.
jackdied@yahoo.com
* Re-jiggered to use generators
* unrolled Child func
* hooked it up to a C priority queue
"""
import heapq

__version__ = "$Revision: 1.15 $"
class AStar:

	def __init__(self, start, heuristic):
		self.solution = None
		goal_test = lambda x: x.is_goal()
		self.gen = self.main_loop(start, heuristic, goal_test)
		return

	def done(self):
		"""IF YOU DONT CALL DONE, OUR CIRCULAR REFERENCE
		(THROUGH THE GENERATOR) WILL EAT THE WORLD"""
		del self.gen
		return

	def main_loop(self, start, heur, goal_test):
		# make everything a local alias for a [5%] speed boost
		seen = {}
		g_costs = {start : 1}
		parents = {start : start}
		pqueue = []
		start_cost = heur(start)
		#pqueue.push((start_cost, start))
		heapq.heappush(pqueue, (start_cost, start))
		seen[start] = start_cost
		while (len(pqueue)):
			#next_cost, next_node = pqueue.pop()
			next_cost, next_node = heapq.heappop(pqueue)
			#g_costs[next_node] = g_costs[parents[next_node]] + d_calc(next_node, parents[next_node])
			g_costs[next_node] = g_costs[parents[next_node]] + 1
			if goal_test(next_node):
				self.solution = getPathToGoal(start, next_node, parents)
				return
			children = next_node.get_children()
			for child in children:
				if g_costs.has_key(child): continue
				#f = g_costs[next_node] + d_calc(next_node, child) + heur(child)
				f = g_costs[next_node] + heur(child) + 1
				if (not seen.has_key(child) or seen[child] > f):
					seen[child] = f
					#pqueue.push((f, child))
					heapq.heappush(pqueue, (f, child))
					parents[child] = next_node
			yield None
		assert False, "What's supposed to happen here?"
		#self.solution = getPathToGoal(start, goal, parents)
		#return

#	def __del__(self):
#		print "I'm  FREEEEEEE in AStar"

def getPathToGoal(start, goal, parents):
	"""Given the hash of parental links, follow back and return the
	chain of ancestors."""
	try:
		results = []
		while (goal != start):
			results.append(goal)
			goal = parents[goal]
		# end while (goal != start)
		results.append(start)
		results.reverse()
		return results
	except KeyError: return []
