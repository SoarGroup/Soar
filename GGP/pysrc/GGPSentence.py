from SoarProduction import *

def ClassifyTerm(elementGGP):
	if isinstance(elementGGP, str):
		if elementGGP[0] == '?':
			return GGPVariable(elementGGP)
		else:
			return GGPConstant(elementGGP)
	else:
		return GGPFunction(elementGGP)

class GGPTerm:
	def make_soar_cond(self, sp, cond, place, var_map, negate = False):
		raise NotImplementedError
	
	def make_soar_action(self, sp, action, place, var_map):
		raise NotImplementedError
	
	def type(self):
		raise NotImplementedError
	
	def mangle(self, mangled, name_gen):
		raise NotImplementedError
	
	def name(self):
		raise NotImplementedError

class GGPConstant(GGPTerm):
	def __init__(self, elementGGP):
		self.__name = elementGGP
	
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

class GGPVariable(GGPTerm):
	def __init__(self, elementGGP):
		self.__name = elementGGP[1:]
	
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

class GGPFunction(GGPTerm):

	def __init__(self, elementGGP = None):
		if elementGGP == None:
			self.__name = ""
			self.__terms = []
		else:
			self.__name = elementGGP[0]
			self.__terms = [ClassifyTerm(e) for e in list(elementGGP)[1:]]
	
	def __str__(self):
		s = self.__name
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
				cond.add_id_predicate(self.__name, negate = negate)
				return None
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

class GGPSentence:
	
	DEFINED_RELS = ["role", "init", "true", "does", "next", "legal", "goal", "terminal", "distinct"]
	
	fact_rels = set([])
	
	def __init__(self, elementGGP):
		if isinstance(elementGGP, str):
			self.__name = elementGGP.lower()
			assert self.__name == "terminal" or self.__name not in GGPSentence.DEFINED_RELS
			self.__terms = []
			self.__negated = False
		else:
			if elementGGP[0].lower() == "not":
				self.__negated = True
				relElem = elementGGP[1]
			else:
				self.__negated = False
				relElem = elementGGP
			
			if isinstance(relElem, str):
				self.__name = relElem.lower()
				assert self.__name == "terminal" or self.__name not in GGPSentence.DEFINED_RELS
				self.__terms = []
			else:
				self.__name = relElem[0].lower()

				if self.__name in ["true", "next", "init"]:
					# these are all one place relations where the only arg is a function
					assert len(relElem) == 2, "%s applied to wrong arity of %d" % (self.__name, len(relElem) - 1)
					# next and init cannot be negated
					assert self.__name == "true" or not self.__negated, "%s cannot be negated" % self.__name
					self.__terms = [GGPFunction(relElem[1])]
				elif self.__name in ["does", "legal"]:
					# two place relation, first being a role constant, second being a function
					assert len(relElem) == 3, "%s applied to wrong arity of %d" % (self.__name, len(relElem) - 1)
					self.__terms = [GGPConstant(relElem[1]), GGPFunction(relElem[2])]
				elif self.__name == "role":
					# one place relation with a single constant
					assert len(relElem) == 2, "role applied to wrong arity of %d" % len(relElem) - 1
					self.__terms = [GGPConstant(relElem[1])]
				elif self.__name == "goal":
					assert len(relElem) == 3, "goal applied to wrong arity of %d" % len(relElem) - 1
					self.__terms = [GGPConstant(relElem[1]), GGPConstant(relElem[2])]
				elif self.__name == "distinct":
					# two place relation with variable and variable or variable and constant
					assert len(relElem) == 3
					self.__terms = [ClassifyTerm(relElem[1]), ClassifyTerm(relElem[2])]
				elif self.__name not in GGPSentence.DEFINED_RELS:
					# we're kind of screwed here because we can't tell a 0 arity function constant from
					# a normal constant by just observing one case. The only way to distinguish a
					# 0 arity function constant is if we saw it after a true relation.
					self.__terms = [ClassifyTerm(e) for e in list(relElem)[1:]]
				else:
					assert False, "\"%s\" is malformed" % str(elementGGP)
					
	def __str__(self):
		s = self.__name
		for t in self.__terms:
			s += " %s" % str(t)
		
		return "(%s)" % s
	
	def __eq__(self, other):
		if not isinstance(other, GGPSentence):
			return False
		
		if self.__name != other.__name or \
		   self.__negated != other.__negated or \
		   len(self.__terms) != len(other.__terms):
			return False
		
		for i in range(len(self.__terms)):
			if self.__terms[i] != other.__terms[i]:
				return False
		
		return True
	
	def __ne__(self, other):
		return not self == other
	
	def name(self):
		return self.__name
	
	def term(self, i):
		return self.__terms[i]
	
	def num_terms(self):
		return len(self.__terms)
	
	def copy(self):
		c = GGPSentence("")
		c.__name = self.__name
		c.__negated = self.__negated
		c.__terms = [ t.copy() for t in self.__terms ]
		return c

	def negate(self):
		nc = self.copy()
		nc.__negated = not nc.__negated
		return nc
	
	def true_analogue(self):
		assert self.__name in ["next", "init", "true"]
		a = self.copy()
		a.__name = "true"
		return a
	
	def next_analogue(self):
		assert self.__name == "true"
		a = self.copy()
		a.__name = "next"
		return a
	
	def mangle_vars(self, mangled, name_gen):
		for t in self.__terms:
			t.mangle(mangled, name_gen)
			
	def make_soar_conditions(self, sp, var_map):
		if self.__name  == "true":
			gs_conds = sp.get_conditions("gs")
			assert len(gs_conds) <= 1, "More than one game state condition??"
			if len(gs_conds) == 0:
				gs_cond = sp.add_condition(sp.state_cond().add_id_predicate("gs"))
			else:
				gs_cond = gs_conds[0]
			
			if self.__negated:
				if self.__terms[0].type() == "function":
					gs_cond = sp.add_condition(gs_cond.head_var)
					fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
					sp.make_negative_conjunction([gs_cond, fcond])
				else:
					fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map, True)
			else:
				fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
				
		elif self.__name == "does":
			move_conds = sp.get_conditions("io.input-link.last-moves")
			if len(move_conds) == 0:
				in_conds = sp.get_il_cond()
				move_cond = sp.add_condition(in_conds.add_id_predicate("last-moves"))
			else:
				move_cond = move_conds[0]
			
			# the does relation has two places, a constant representing the role and 
			# a function constant representing the move
			role_cond = sp.add_condition(move_cond.add_id_predicate(self.__terms[0].name()))
			if self.__terms[1].num_terms() == 0:
				fcond = self.__terms[1].make_soar_cond(sp, role_cond, 0, var_map, self.__negated)
			else:
				fcond = self.__terms[1].make_soar_cond(sp, role_cond, 0, var_map)
			if self.__negated and fcond != None:
				sp.make_negative_conjunction([role_cond, fcond])
		
		else:
			assert self.__name not in GGPSentence.DEFINED_RELS
			if self.__name not in GGPSentence.fact_rels:
				rel_conds = sp.get_conditions("elaborations")
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
				my_rel_cond = sp.add_condition(rel_cond.add_id_predicate(self.__name))
				fconds = []
				for t, i in zip(self.__terms, range(len(self.__terms))):
					if t.type() == "function":
						fconds.append(t.make_soar_cond(sp, my_rel_cond, i + 1, var_map, True))
					else:
						t.make_soar_cond(sp, my_rel_cond, i + 1, var_map, True)
				sp.make_negative_conjunction([rel_cond, my_rel_cond] + fconds)
			else:
				my_rel_cond = sp.add_condition(rel_cond.add_id_predicate(self.__name, self.__negated))
				for t, i in zip(self.__terms, range(len(self.__terms))):
					t.make_soar_cond(sp, my_rel_cond, i + 1, var_map)
	
	def make_soar_actions(self, sp, var_map, remove = False):		
		if self.__name in ["next", "init"]:
			# next is a one place relation whose single argument is a function
			if self.__name == "next":
				gs_conds = sp.get_conditions("gs")
				if len(gs_conds) == 0:
					gs_cond = sp.add_condition(sp.state_cond().add_id_predicate("gs"))
				else:
					gs_cond = gs_conds[0]
				act = sp.add_action(gs_cond.head_var)
			else:
				# for init actions, the gs id hasn't been added yet. Get it from
				# the list of actions in the production instead of the conditions
				head_acts = sp.get_actions(sp.state_id)
				gs_act = head_acts[0].get_wme_actions("gs")
				act = sp.add_action(gs_act[0][2][1:-1]) # strip off the < and >
			
			if remove:
				assert self.__name == "next" # we would never move an init fact
				func_name = self.__terms[0].name()
				fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
				act.remove_id_wme_action(func_name, fcond.head_var)
			else:
				self.__terms[0].make_soar_action(sp, act, 0, var_map)
				
		elif self.__name == "legal":
			# legal is a two place relation, constant, function
			op_name = SoarifyStr(str(self.__terms[1]))
			op_act = sp.add_op_proposal("+")
			op_act.add_ground_wme_action("name", op_name)
			self.__terms[1].make_soar_action_no_id(sp, op_act, var_map, 1)
		else:
			assert self.__name not in GGPSentence.DEFINED_RELS, str(self)
			# this is either a fact or an elaboration
			if self.__name not in GGPSentence.fact_rels:
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
				var = cond.add_id_predicate(self.__name)
				act.remove_id_wme_action(self.__name, var)
			else:
				param_act = sp.add_action(act.add_id_wme_action(self.__name))
				for t, i in zip(self.__terms, range(len(self.__terms))):
					t.make_soar_action(sp, param_act, i + 1, var_map)