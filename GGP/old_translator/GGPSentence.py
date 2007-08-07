from SoarProd import *

def ClassifyTerm(elementGGP):
	if isinstance(elementGGP, str):
		if elementGGP[0] == '?':
			return GGPVariable(elementGGP)
		else:
			return GGPConstant(elementGGP)
	else:
		return GGPFunction(elementGGP)

class GGPConstant:
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
	
	def get_variables(self):
		return []

	def make_soar_cond(self, sp, id, place, var_map, negate = False):
		assert place > 0
		sp.add_attrib(id, 'p%d' % place, self.__name)
		return id
		
	def make_soar_action(self, sp, id, place, var_map):
		assert place > 0
		sp.add_create_constant(id, 'p%d' % place, self.__name)
		return id
	
	def name(self):
		return self.__name
	
	def type(self):
		return "constant"
	
	def copy(self):
		return GGPConstant(self.__name)
	
#	def mangle(self, mangled, name_gen):
#		# can't mangle constants
#		return

	def mangle(self, prefix):
		pass
	
	def covers(self, other):
		return self == other

class GGPVariable:
	def __init__(self, elementGGP):
		if elementGGP[0] == '?':
			self.__name = elementGGP[1:]
		else:
			self.__name = elementGGP
	
	def __str__(self):
		return "?%s" % self.__name
	
	def __eq__(self, other):
		if not isinstance(other, GGPVariable):
			return False
		
		return self.__name == other.__name
	
	def __ne__(self, other):
		return not self == other

	def get_variables(self):
		return [self.__name]
	
	def name(self):
		return self.__name
	
	def rename(self, name):
#		if self.__name == "strength":
#			pdb.set_trace()
		self.__name = name
	
	def make_soar_cond(self, sp, id, place, var_map, negate = False):
		assert place > 0
		sp.add_id_attrib(id, 'p%d' % place, var_map.get_var(self.__name))
		return id
		
	def make_soar_action(self, sp, id, place, var_map):
		assert place > 0
		sp.add_create_id(id, 'p%d' % place, var_map.get_var(self.__name))
		return id
	
	def type(self):
		return "variable"
	
	def copy(self):
		return GGPVariable("?%s" % self.__name)
	
#	def mangle(self, mangled, name_gen):
#		if self.__name in mangled:
#			self.__name = mangled[self.__name]
#		else:
#			new_var = name_gen.get_name(self.__name)
#			mangled[self.__name] = new_var
#			self.__name = new_var

	def mangle(self, prefix):
		if not self.__name.startswith("std_soar_var"):
			self.__name = "%s%s" % (prefix, self.__name)

	def covers(self, other):
		return isinstance(other, GGPVariable) or isinstance(other, GGPConstant)

	def __hash__(self):
		return hash(self.__name)

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

class GGPFunction:

	def __init__(self, elementGGP = None):
		if elementGGP == None:
			self.__name = ""
			self.__terms = []
		elif isinstance(elementGGP, str):
			self.__name = elementGGP
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
	
	def get_variables(self):
		vars = []
		for t in self.__terms:
			vars.extend(t.get_variables())
		return vars

	def num_terms(self):
		return len(self.__terms)
	
	def copy(self):
		c = GGPFunction()
		c.__name = self.__name
		c.__terms = [ t.copy() for t in self.__terms ]
		return c
	
	def make_soar_cond(self, sp, id, place, var_map):
		if place == 0:
			if len(self.__terms) == 0:
				return sp.add_id_attrib(id, self.__name)
			else:
				fid = sp.add_id_attrib(id, self.__name)
					
				for i, t in enumerate(self.__terms):
					t.make_soar_cond(sp, fid, i+1, var_map)
				
				return fid
		else:
			fid = sp.add_id_attrib(id, "p%d" % place)
			sp.add_attrib(fid, "name", self.__name)
			for i, t in enumerate(self.__terms):
				t.make_soar_cond(sp, fid, i+1, var_map)
			
			return fid

	def make_soar_cond_no_id(self, sp, id, var_map, place_offset = 0):
		for i, t in enumerate(self.__terms):
			t.make_soar_cond(sp, id, i + place_offset, var_map)
	
	def make_soar_action(self, sp, id, place, var_map):
		if place == 0:
			fid = sp.add_create_id(id, self.__name)
			for i, t in enumerate(self.__terms):
				t.make_soar_action(sp, fid, i+1, var_map)
		else:
			fid = sp.add_create_id(id, "p%d" % place)
			sp.add_create_constant(fid, 'name', self.__name)
			for i, t in enumerate(self.__terms):
				t.make_soar_action(sp, fid, i+1, var_map)
	
	def make_soar_action_no_id(self, sp, id, var_map, place_offset = 0):
		for i, t in enumerate(self.__terms):
			t.make_soar_action(sp, id, i + place_offset, var_map)
	
#	def mangle(self, mangled, name_gen):
#		for t in self.__terms:
#			t.mangle(mangled, name_gen)

	def mangle(self, prefix):
		for t in self.__terms:
			t.mangle(prefix)

	# convert places into specified variable, returning the subsitutions
	# made in the process
	def standardize_vars(self, vars_to_use):
		val_map = {} # map from place to original value
		var_map = {} # map from place to original variable
		for i, t in enumerate(self.__terms[:]):
			if t.type() == 'constant':
				val_map[i] = t.name()
				self.__terms[i] = GGPVariable(vars_to_use[i])
			elif t.type() == 'variable':
				var_map[i] = t.name()
				self.__terms[i] = GGPVariable(vars_to_use[i])
			else:
				print "WARNING: encountered embedded function, not supported yet"

		return (val_map, var_map)

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
			# must be a terminal or a user defined relation
			assert self.__name.lower() == "terminal" or self.__name.lower() not in GGPSentence.DEFINED_RELS
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
				# must be a terminal or a user defined relation
				self.__name = relElem
				assert self.__name.lower() == "terminal" or self.__name.lower() not in GGPSentence.DEFINED_RELS
				self.__terms = []
			else:
				# compound sentence
				if relElem[0].lower() in DEFINED_RELS:
					# use lower case for reserved words
					self.__name = relElem[0].lower()
				else:
					# preserve case for user defined types
					self.__name = relElem[0]

				if self.__name in ["true", "next", "init"]:
					# these are all one place relations where the only arg is a function
					assert len(relElem) == 2, "%s applied to wrong arity of %d in %s" % (self.__name, len(relElem) - 1, relElem[2])
					# next and init cannot be negated
					assert self.__name == "true" or not self.__negated, "%s cannot be negated" % self.__name
					self.__terms = [GGPFunction(relElem[1])]
				elif self.__name in ["does", "legal"]:
					# two place relation, first being a role constant, second being a function
					assert len(relElem) == 3, "%s applied to wrong arity of %d" % (self.__name, len(relElem) - 1)
					self.__terms = [ClassifyTerm(relElem[1]), GGPFunction(relElem[2])]
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
		
		if self.__negated:
			return "(not (%s))" % s
		else:
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

	def terms(self):
		return self.__terms

	def term(self, i):
		return self.__terms[i]
	
	def num_terms(self):
		return len(self.__terms)
	
	def get_variables(self):
		vars = []
		for t in self.__terms:
			vars.extend(t.get_variables())
		return vars

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
	
#	def mangle_vars(self, mangled, name_gen):
#		for t in self.__terms:
#			t.mangle(mangled, name_gen)

	def mangle_vars(self, prefix):
		for t in self.__terms:
			t.mangle(prefix)
			
	def make_soar_conditions(self, sp, var_map):
		if self.__name  == "true":
			gs_id = sp.get_or_make_id_chain(['gs'])[0]
			
			if self.__negated:
				sp.begin_negative_conjunction()

			fid = self.__terms[0].make_soar_cond(sp, gs_id, 0, var_map)
			
			if self.__negated:
				sp.end_negative_conjunction()
				
		elif self.__name == "does":
			move_id = sp.get_or_make_id_chain_existing(['io','input-link','last-moves'])[0]
			
			# the does relation has two places, a constant or variable
			# representing the role and a function constant representing the
			# move
			if self.__terms[0].type() == 'variable':
				role_id = sp.add_id_var_attrib(move_id, self.__terms[0].name())
				role_attrib = '<%s>' % self.__terms[0].name()
			else:
				assert self.__terms[0].type() == 'constant'
				role_id = sp.add_id_attrib(move_id, self.__terms[0].name())

			if self.__negated:
				sp.begin_negative_conjunction()

			fid = self.__terms[1].make_soar_cond(sp, role_id, 0, var_map)

			if self.__negated:
				sp.end_negative_conjunction()
		
		else:
			assert self.__name not in GGPSentence.DEFINED_RELS
			if self.__name not in GGPSentence.fact_rels:
				elab_id = sp.get_or_make_id_chain(["elaborations"])[0]
			else:
				elab_id = sp.get_or_make_id_chain(["facts"])[0]
					
			if self.__negated:
				sp.begin_negative_conjunction()

			rel_id = sp.add_id_attrib(elab_id, self.__name)
			fconds = []
			for i, t in enumerate(self.__terms):
				t.make_soar_cond(sp, rel_id, i + 1, var_map)

			if self.__negated:
				sp.end_negative_conjunction()
	
	def make_soar_actions(self, sp, var_map, remove = False):
		if self.__name in ["next", "init"]:
			# next is a one place relation whose single argument is a function
			if self.__name == "next":
				gs_id = sp.get_or_make_id_chain(["gs"])[0]
			else:
				# for init actions, the gs id hasn't been added yet. Get it from
				# the list of actions in the production instead of the conditions
				gs_id = sp.get_new_ids_by_chain(sp.get_state_id(), ['gs'])[0]
			
			if remove:
				assert self.__name == "next" # we would never move an init fact
				func_name = self.__terms[0].name()
				#fcond = self.__terms[0].make_soar_cond(sp, gs_cond, 0, var_map)
				# of course we only want to remove variables that are already bound, right?
				# so we shouldn't create a new condition if it exists
				existing_ids = sp.get_ids(gs_id, func_name)
				if len(existing_ids) == 0:
					# okay, so create a binding here, since there weren't any before
					# but this is suspicious
					fid = self.__terms[0].make_soar_cond(sp, gs_id, 0, var_map)
					#if isinstance(fcond, str):
					#	id_to_remove = fid
					#else:
					id_to_remove = fid
					
					print "&&& WARNING &&&: Adding action to remove an unbound variable"
				else:
					assert len(existing_ids) == 1, "More than one bound variable, don't know which to pick"
					id_to_remove = existing_ids[0]
					
				sp.add_destroy_id(gs_id, func_name, id_to_remove)
			else:
				self.__terms[0].make_soar_action(sp, gs_id, 0, var_map)
				
		elif self.__name == "legal":
			# legal is a two place relation, constant, function
			#op_name = SoarifyStr(str(self.__terms[1]))
			op_name = self.__terms[1].name()
			op_id = sp.add_operator_prop(op_name, "+")
			self.__terms[1].make_soar_action_no_id(sp, op_id, var_map, 1)
		else:
			assert self.__name not in GGPSentence.DEFINED_RELS, str(self)
			# this is either a fact or an elaboration
			if self.__name not in GGPSentence.fact_rels:
				lhs_id = sp.get_or_make_id_chain(["elaborations"])[0]
			else:
				# for fact actions, the fact id hasn't been added yet. Get it from
				# the list of actions in the production instead of the conditions
				lhs_id = sp.get_new_ids_by_chain(sp.get_state_id(), ['facts'])[0]
			
			if remove:
				remove_id = sp.add_id_attrib(lhs_id, self.__name)
				sp.add_destroy_id(lhs_id, self.__name, remove_id)
			else:
				new_id = sp.add_create_id(lhs_id, self.__name)
				for i, t in enumerate(self.__terms):
					t.make_soar_action(sp, new_id, i + 1, var_map)
