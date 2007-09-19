from PositionIndex import PositionIndex

comp_rels = ['<', '>', '>=', 'lessThan', 'greaterThan', 'succ']
math_op_rels = ['+', '-', '*', '/', 'min', 'minus', 'plus']
obj_loc_place = set([('location', PositionIndex([0,0]))])
coord_places = set([('location', PositionIndex([0,p])) for p in [1,2]])

# an equivalence class that has one of these places as a member must
# be a number
num_places = set([(c, PositionIndex([0])) for c in comp_rels] + \
                 [(c, PositionIndex([1])) for c in comp_rels] + \
                 [(o, PositionIndex([0])) for o in math_op_rels] + \
                 [(o, PositionIndex([1])) for o in math_op_rels] + \
                 [(o, PositionIndex([2])) for o in math_op_rels] + \
                 [('int', PositionIndex([0]))])

# these are the place types
AGENT, OBJECT, COORD, NUM_MAX, NUM_MIN, NUM, UNKNOWN = range(7)

# for string output
type_names = { 
		AGENT : 'agent',
		OBJECT : 'object',
		COORD : 'coord',
		NUM_MAX : 'num_max',
		NUM_MIN : 'num_min',
		NUM : 'num',
		UNKNOWN : 'unknown' }

# specificity ordering relations
type_order = { 
		OBJECT : [ AGENT ],
		NUM : [ COORD, NUM_MAX, NUM_MIN ],
		COORD : [ NUM_MAX, NUM_MIN ],
		UNKNOWN : [ AGENT, OBJECT, NUM, COORD, NUM_MAX, NUM_MIN ] }

# how similar one type is to another. A type has a similarity of 1 to itself,
# and a similarity of 0 to another type not listed in this table.
type_similarity = {
		AGENT : { OBJECT : 0.7, UNKNOWN : 0.1 },
		OBJECT : { AGENT : 0.7, UNKNOWN : 0.1 },
		COORD : { NUM : 0.7, NUM_MIN : 0.5, NUM_MAX : 0.5, UNKNOWN : 0.1 },
		NUM : { NUM_MIN : 0.7, NUM_MAX : 0.7, COORD : 0.7, UNKNOWN : 0.1 },
		NUM_MIN : { NUM : 0.7, NUM_MAX : 0.3, COORD : 0.5, UNKNOWN : 0.1 },
		NUM_MAX : { NUM : 0.7, NUM_MIN : 0.3, COORD : 0.5, UNKNOWN : 0.1 },
		UNKNOWN : dict((t, 0.1) for t in range(UNKNOWN))
		}

def type_intersect(s1, s2):
	""" Gives a score of how similar two sets of types are """
	if len(s1) == 0 or len(s2) == 0:
		return []

	if len(s1) < len(s2):
		shorter, longer = list(s1), list(s2)
		reverse = False
	else:
		shorter, longer = list(s2), list(s1)
		reverse = True

	# Sort the shorter list so that we meet UNKNOWNs at the very end.
	# This is so that unknown will have the lowest priority in mapping
	shorter.sort()

	type_map = []
	for t1 in shorter:
		best = 0
		best_type = None
		for t2 in longer:
			if t2 == t1:
				best = 1
				best_type = t2
				break

			if t2 in type_similarity[t1]:
				if type_similarity[t1][t2] > best:
					best = type_similarity[t1][t2]
					best_type = t2
		if best_type != None:
			longer.remove(best_type)
			if reverse:
				type_map.append((best_type, t1, best))
			else:
				type_map.append((t1, best_type, best))

	return type_map

# these preset places will always have these types
preset_types = {}

for p in obj_loc_place:
	preset_types[p] = OBJECT

for p in coord_places:
	preset_types[p] = COORD

for p in num_places:
	preset_types[p] = NUM
