import sys

## xpermutations.py
## Generators for calculating a) the permutations of a sequence and
## b) the combinations and selections of a number of elements from a
## sequence. Uses Python 2.2 generators.
## 
## Similar solutions found also in comp.lang.python
## 
## Keywords: generator, combination, permutation, selection
## 
## See also: http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/105962
## See also: http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/66463
## See also: http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/66465

def xcombinations(items, n):
	if n==0: yield []
	else:
		for i in xrange(len(items)):
			for cc in xcombinations(items[:i]+items[i+1:],n-1):
				yield [items[i]]+cc

def xuniqueCombinations(items, n):
	if n==0: yield []
	else:
		for i in xrange(len(items)):
			for cc in xuniqueCombinations(items[i+1:],n-1):
				yield [items[i]]+cc
			
def xselections(items, n):
	if n==0: yield []
	else:
		for i in xrange(len(items)):
			for ss in xselections(items, n-1):
				yield [items[i]]+ss

def xpermutations(items):
	return xcombinations(items, len(items))

# the rest of the stuff is mine

def cross_product(l1, l2):
	for a1 in l1:
		for a2 in l2:
			yield (a1, a2)

def cross_join(list1, list2):
	"Like cross product, except the elements are joined with +"
	#if len(list1) == 0 or len(list2) == 0:
	#	return []
	for e1 in list1:
		for e2 in list2:
			yield e1 + e2
	#return reduce(lambda x,y: x+y, ([e1 + e2 for e2 in list2] for e1 in list1))

def possible_matchings(s1, s2):
	# we can't match two instances of the same predicate in one rule
	# to different target predicates
	unique_s1 = list(set(s1))
	unique_s2 = list(set(s2))
	if len(unique_s1) < len(unique_s2):
		return (dict(zip(unique_s1, subset)) for subset in xcombinations(unique_s2, len(unique_s1)))
	else:
		return (dict(zip(subset, unique_s2)) for subset in xcombinations(unique_s1, len(unique_s2)))

def vslice(l, i):
	for x in l:
		yield x[i]

def vslice1(l, i):
	return [x for x in vslice(l, i)]

def average_tuple_list(l, i):
	num_params = len(l[0]) - 1
	final = {}
	unique_vals = set(vslice(l,i))
	for v in unique_vals:
		chunk = [x[0:i] + x[i+1:] for x in l if x[i] == v]
		chunk_len = float(len(chunk))
		avgs = []
		for j in range(num_params):
			avgs.append(sum(vslice(chunk, j)) / chunk_len)
		final[v] = tuple(avgs)
	return final

def find_max(seq, proj):
	if len(seq) == 0:
		return (-1, -1)
	max = proj(seq[0])
	mpos = 0
	for i in xrange(1,len(seq)):
		v = proj(seq[i])
		if max < v:
			max = v
			mpos = i
	return (mpos, max)

def debug_print(s):
	#print >> sys.stderr, s
	pass
