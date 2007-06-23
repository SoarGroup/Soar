
class GDLConstant:
	def __init__(self, s):
		self.__name = s
	
	def __str__(self):
		return self.__name
	
	def __eq__(self, other):
		if not isinstance(other, GGPConstant):
			return False
		
		return self.__name == other.__name
	
	def __ne__(self, other):
		return not self == other
	
	def make_soar_cond(self, sp, cond, place, var_map, negate = False):
		assert place > 0
		cond.add_ground_predicate("p%d" % place, self.__name)
		return cond
		
	def make_soar_action(self, sp, action, place, var_map):
		assert place > 0
		action.add_ground_wme_action("p%d" % place, self.__name)
		return action
	
	def name(self):
		return self.__name
	
	def type(self):
		return "constant"
	
	def copy(self):
		return GGPConstant(self.__name)
	
	def mangle(self, mangled, name_gen):
		# can't mangle constants
		return
	
	def covers(self, other):
		return self == other

class GDLVariable:
	def __init__(self, s):
		self.__name = s
	
	def __str__(self):
		return "?%s" % self.__name
	
	def __eq__(self, other):
		if not isinstance(other, GGPVariable):
			return False
		
		return self.__name == other.__name
	
	def __ne__(self, other):
		return not self == other
	
	def name(self):
		return self.__name
	
	def rename(self, name):
		self.__name = name
	
	def make_soar_cond(self, sp, cond, place, var_map, negate = False):
		assert place > 0
		cond.add_id_predicate("p%d" % place, name = var_map.get_var(self.__name))
		return cond
		
	def make_soar_action(self, sp, action, place, var_map):
		assert place > 0
		action.add_id_wme_action("p%d" % place, name = var_map.get_var(self.__name))
		return action
	
	def type(self):
		return "variable"
	
	def copy(self):
		return GGPVariable("?%s" % self.__name)
	
	def mangle(self, mangled, name_gen):
		if self.__name in mangled:
			self.__name = mangled[self.__name]
		else:
			new_var = name_gen.get_name(self.__name)
			mangled[self.__name] = new_var
			self.__name = new_var
	
	def covers(self, other):
		return isinstance(other, GGPVariable) or isinstance(other, GGPConstant)

# functions can appear in two forms in soar rules
#
# (rel (x y (f a b)))
#
# becomes
#
# (<rel> ^p1 x
#        ^p2 y
#        ^p3 <f>)
# (<f> ^name f
#      ^p1 a
#      ^p2 b)
#
# or 
#
# (<rel> ^p1 x
#        ^p2 y
#        ^f <f>)
# (<f> ^p1 a
#      ^p2 b)

class GDLFunction:

	def __init__(self, name, terms):
		self.__name = name
		self.__terms = terms
	
	def __str__(self):
		s = self.__name
		if len(self.__terms) == 0:
			return self.__name
		
		for t in self.__terms:
			s += " %s" % str(t)
		
		return "(%s)" % s
	
	def name(self):
		return self.__name
	
	def type(self):
		return "function"
	
	def term(self, i):
		return self.__terms[i]
	
	def num_terms(self):
		return len(self.__terms)
	
	def copy(self):
		c = GGPFunction()
		c.__name = self.__name
		c.__terms = [ t.copy() for t in self.__terms ]
		return c
	
	def make_soar_cond(self, sp, cond, place, var_map, negate = False):
		if place == 0:
			if len(self.__terms) == 0:
				var = cond.add_id_predicate(self.__name, negate = negate)
				return var
			else:
				fcond = sp.add_condition(cond.add_id_predicate(self.__name, negate = negate))
					
				for t, i in zip(self.__terms, range(len(self.__terms))):
					t.make_soar_cond(sp, fcond, i+1, var_map)
				
				return fcond
		else:
			fcond = sp.add_condition(cond.add_id_predicate("p%d" % place))
			
			fcond.add_ground_predicate("name", self.__name)
			for t, i in zip(self.__terms, range(len(self.__terms))):
				t.make_soar_cond(sp, fcond, i+1, var_map)
			
			return fcond

	def make_soar_cond_no_id(self, sp, cond, var_map, place_offset = 0):
		for t, i in zip(self.__terms, range(len(self.__terms))):
			t.make_soar_cond(sp, cond, i + place_offset, var_map)
	
	def make_soar_action(self, sp, action, place, var_map):
		if place == 0:
			fact = sp.add_action(action.add_id_wme_action(self.__name))
			for t, i in zip(self.__terms, range(len(self.__terms))):
				t.make_soar_action(sp, fact, i+1, var_map)
		else:
			fact = sp.add_action(action.add_id_wme_action("p%d" % place))
			fact.add_ground_wme_action("name", self.__name)
			for t, i in zip(self.__terms, range(len(self.__terms))):
				t.make_soar_action(sp, fact, i+1, var_map)
	
	def make_soar_action_no_id(self, sp, action, var_map, place_offset = 0):
		for t, i in zip(self.__terms, range(len(self.__terms))):
			t.make_soar_action(sp, action, i + place_offset, var_map)
	
	def mangle(self, mangled, name_gen):
		for t in self.__terms:
			t.mangle(mangled, name_gen)
	
	def __eq__(self, other):
		if not isinstance(other, GGPFunction):
			return False
		
		if self.__name != other.__name or len(self.__terms) != len(other.__terms):
			return False
		
		for i, j in zip(self.__terms, other.__terms):
			if i != j:
				return False
		
		return True
	
	def __ne__(self, other):
		return not self == other
	
	# The concept of "covers" is only approximate. Specifically, whether one variable
	# covers another variable or constant is dependent on the constraints put on that
	# variable by the conditions in the body of a GGP rule. There's no way to figure
	# that out from just looking at the function.
	def covers(self, other):
		if not isinstance(other, GGPFunction):
			return False
		
		if self.__name != other.__name:
			return False
		
		for st, ot in zip(self.__terms, other.__terms):
			if not st.covers(ot):
				return False
		
		return True

class GDLLiteral:
	
	def __init__(self, relation, terms, negated):
		self.__rel = relation
		self.__terms = terms
					
	def __eq__(self, other):
		if not isinstance(other, GGPSentence):
			return False
		
		if self.__rel != other.__rel or \
		   self.__negated != other.__negated or \
		   len(self.__terms) != len(other.__terms):
			return False
		
		for i in range(len(self.__terms)):
			if self.__terms[i] != other.__terms[i]:
				return False
		
		return True
	
	def __ne__(self, other):
		return not self == other
	
	def relation(self):
		return self.__rel
	
	def term(self, i):
		return self.__terms[i]
	
	def num_terms(self):
		return len(self.__terms)
	
	def mangle_vars(self, mangled, name_gen):
		for t in self.__terms:
			t.mangle(mangled, name_gen)

class GDLNegatableLiteral(GDLLiteral):

	def negate(self):
		nc = self.copy()
		nc.__negated = not nc.__negated
		return nc

class GDLTrueLiteral(GDLNegatableLiteral):
	def __init__(self, func, negated):
		self.__rel = "true"
		self.__terms = [func]
		self.__negated = negated

	def copy(self):
		 return GDLTrueLiteral(self.__terms[0].copy(), self.__negated)

	def make_soar_conditions(self, sp, var_map):
		gs_conds = sp.get_conditions("gs")
		#assert len(gs_conds) <= 1, "More than one game state condition??"
		if len(gs_conds) == 0:
			gs_cond = sp.add_condition(sp.state_cond().add_id_predicate("gs"))
		else:
			gs_cond = gs_conds[0]
		
		if self.__negated:
			if self.__terms[0].type() == "function" and self.__terms[0].num_terms() > 0:
				gs_cond = sp.add_condition(gs_cond.head_var)
				fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
				sp.make_negative_conjunction([gs_cond, fcond])
			else:
				fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map, True)
		else:
			fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)

class GDLNextLiteral(GDLLiteral):
	def __init__(self, func):
		self.__rel = "next"
		self.__terms = [func]
		
	def make_soar_actions(self, sp, var_map, remove = False):		
		gs_conds = sp.get_conditions("gs")
		if len(gs_conds) == 0:
			gs_cond = sp.add_condition(sp.state_cond().add_id_predicate("gs"))
		else:
			gs_cond = gs_conds[0]
		act = sp.add_action(gs_cond.head_var)
			
		if remove:
			assert self.__rel == "next" # we would never move an init fact
			func_name = self.__terms[0].name()
			#fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
			# of course we only want to remove variables that are already bound, right?
			# so we shouldn't create a new condition if it exists
			existing_ids = gs_cond.get_ids(func_name)
			if len(existing_ids) == 0:
				# okay, so create a binding here, since there weren't any before
				# but this is suspicious
				fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
				if isinstance(fcond, str):
					id_to_remove = fcond
				else:
					id_to_remove = fcond.head_var
				
				print "&&& WARNING &&&: Adding action to remove an unbound variable"
			else:
				assert len(existing_ids) == 1, "More than one bound variable, don't know which to pick"
				id_to_remove = existing_ids[0]
				
			act.remove_id_wme_action(func_name, id_to_remove)
		else:
			self.__terms[0].make_soar_action(sp, act, 0, var_map)

class GDLInitLiteral(GDLLiteral):
	def __init__(self, func):
		self.__rel = 'init'
		self.__terms = [func]
	
	def make_soar_actions(self, sp, var_map, remove = False):		
		# for init actions, the gs id hasn't been added yet. Get it from
		# the list of actions in the production instead of the conditions
		head_acts = sp.get_actions(sp.state_id)
		gs_act = head_acts[0].get_wme_actions("gs")
		act = sp.add_action(gs_act[0][2][1:-1]) # strip off the < and >
		
		if remove:
			assert self.__rel == "next" # we would never move an init fact
			func_name = self.terms[0].name()
			#fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
			# of course we only want to remove variables that are already bound, right?
			# so we shouldn't create a new condition if it exists
			existing_ids = gs_cond.get_ids(func_name)
			if len(existing_ids) == 0:
				# okay, so create a binding here, since there weren't any before
				# but this is suspicious
				fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
				if isinstance(fcond, str):
					id_to_remove = fcond
				else:
					id_to_remove = fcond.head_var
				
				print "&&& WARNING &&&: Adding action to remove an unbound variable"
			else:
				assert len(existing_ids) == 1, "More than one bound variable, don't know which to pick"
				id_to_remove = existing_ids[0]
				
			act.remove_id_wme_action(func_name, id_to_remove)
		else:
			self.__terms[0].make_soar_action(sp, act, 0, var_map)
		
class GDLDoesLiteral(GDLNegatableLiteral):
	def __init__(self, role, move, negated):
		self.__terms = [role, move]
		self.__negated = negated

	def copy(self):
		return GDLDoesLiteral(self.__terms[0], self.__terms[1], self.__negated)

	def make_soar_conditions(self, sp, var_map):
		move_conds = sp.get_conditions("io.input-link.last-moves")
		if len(move_conds) == 0:
			in_conds = sp.get_il_cond()
			move_cond = sp.add_condition(in_conds.add_id_predicate("last-moves"))
		else:
			move_cond = move_conds[0]
		
		# the does relation has two places, a constant representing the role and 
		# a function constant representing the move
		role_cond = sp.add_condition(move_cond.add_id_predicate(self.__terms[0]))
		if self.__terms[1].num_terms() == 0:
			fcond = self.__terms[1].make_soar_cond(sp, role_cond, 0, var_map, self.__negated)
		else:
			fcond = self.__terms[1].make_soar_cond(sp, role_cond, 0, var_map)
			if self.__negated and fcond != None:
				assert not isinstance(fcond, str), [str(t) for t in self.__terms] + [self.__terms[1].num_terms(), self.__terms[1].type()]
				sp.make_negative_conjunction([role_cond, fcond])

class GDLLegalLiteral(GDLLiteral):
	def __init__(self, role, move):
		self.__rel = "legal"
		self.__terms = [role, move]

	def make_soar_actions(self, sp, var_map, remove = False):		
		# legal is a two place relation, constant, function
		op_name = SoarifyStr(str(self.__terms[1]))
		op_act = sp.add_op_proposal("+")
		op_act.add_ground_wme_action("name", op_name)
		self.__terms[1].make_soar_action_no_id(sp, op_act, var_map, 1)

class GDLGoalLiteral(GDLLiteral):
	def __init__(self, role, score):
		self.__rel = "goal"
		self.__terms = [role, score]

class GDLUserLiteral(GDLNegatableLiteral):
	def __init__(self, relation, terms, negated):
		self.__rel = relation
		self.__terms = terms
		self.__negated = negated

	def make_soar_conditions(self, sp, var_map):
		if self.__rel not in GGPSentence.fact_rels:
			rel_conds = sp.get_conditions("elaborations", self.__negated)
			if len(rel_conds) == 0:
				rel_cond = sp.add_condition(sp.state_cond().add_id_predicate("elaborations"))
			else:
				rel_cond = rel_conds[0]
		else:
			rel_conds = sp.get_conditions("facts")
			if len(rel_conds) == 0:
				rel_cond = sp.add_condition(sp.state_cond().add_id_predicate("facts"))
			else:
				rel_cond = rel_conds[0]
				
		if self.__negated and len(self.__terms) > 0:
			rel_cond = sp.add_condition(rel_cond.head_var)
			my_rel_cond = sp.add_condition(rel_cond.add_id_predicate(self.__rel))
			fconds = []
			for t, i in zip(self.__terms, range(len(self.__terms))):
				if t.type() == "function":
					fconds.append(t.make_soar_cond(sp, my_rel_cond, i + 1, var_map, True))
				else:
					t.make_soar_cond(sp, my_rel_cond, i + 1, var_map, True)
			sp.make_negative_conjunction([rel_cond, my_rel_cond] + fconds)
		else:
			my_rel_cond = sp.add_condition(rel_cond.add_id_predicate(self.__rel, self.__negated))
			for t, i in zip(self.__terms, range(len(self.__terms))):
				t.make_soar_cond(sp, my_rel_cond, i + 1, var_map)

	
	def make_soar_actions(self, sp, var_map, remove = False):		
		assert self.__rel not in GGPSentence.DEFINED_RELS, str(self)
		# this is either a fact or an elaboration
		if self.__rel not in GGPSentence.fact_rels:
			conds = sp.get_conditions("elaborations")
			if len(conds) == 0:
				cond = sp.add_condition(sp.state_cond().add_id_predicate("elaborations"))
			else:
				cond = conds[0]
			
			act = sp.add_action(cond.head_var)
		else:
			# for fact actions, the fact id hasn't been added yet. Get it from
			# the list of actions in the production instead of the conditions
			head_acts = sp.get_actions(sp.state_id)
			fact_act = head_acts[0].get_wme_actions("facts")
			act = sp.add_action(fact_act[0][2][1:-1]) # strip off the < and >
			
		if remove:
			var = cond.add_id_predicate(self.__rel)
			act.remove_id_wme_action(self.__rel, var)
		else:
			param_act = sp.add_action(act.add_id_wme_action(self.__rel))
			for t, i in zip(self.__terms, range(len(self.__terms))):
				t.make_soar_action(sp, param_act, i + 1, var_map)
