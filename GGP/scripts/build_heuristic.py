import sys
import gdlyacc

def make_heuristic(ir):
	
	STAIR, TOWER = range(2)
	
	type = None
	win_rules = [r for r in ir.get_goal_rules() if r.get_head().get_term(1).get_name() == 100]
	assert len(win_rules) == 1
	for g in win_rules:
		for b in g.get_body():
			if b.get_predicate() == 'tower':
				type = TOWER
				needed_height = int(b.get_term(0).get_term(1).get_name())
	
	if type == TOWER:
		step_height = 0
		for i in ir.get_inits():
			if i.get_predicate() == 'object-dimensions':
				h = int(i.get_term(2).get_name())
				if h > step_height:
					step_height = h
	else:
		for g in win_rules:
			for b in g.get_body():
				if b.get_predicate() == 'stairway':
					needed_height = int(b.get_term(2).get_name())
					step_height = int(b.get_term(3).get_name())
					break
	
	prod = """sp {elaborate*build*heuristic
   (state <s> ^superstate nil)
-->
   (<s> ^minimum-tower-height %d
        ^maximum-object-height %d)}""" % (needed_height, step_height)
	
	return prod

if __name__ == '__main__':
	gdlyacc.parse_file(sys.argv[1])
	print make_heuristic(gdlyacc.int_rep)
