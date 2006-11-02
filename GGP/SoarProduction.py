#!/usr/bin/python
from ElementGGP import ElementGGP

# generates variable names that are guaranteed to be unique
class UniqueNameGenerator:
	def __init__(self):
		self.var_indices = dict()

	def get_name(self, preferred):
		if len(preferred) == 0:
			preferred = "a"
			
		if self.var_indices.has_key(preferred):
			self.var_indices[preferred] += 1
			name = preferred + str(self.var_indices[preferred])
			self.var_indices[name] = 0
			return name
		else:
			self.var_indices[preferred] = 0
			return preferred
	
	def copy(self):
		c = UniqueNameGenerator()
		c.var_indices = dict(self.var_indices.items())
		return c

class SoarCondition:
		
	def __init__(self, head_var, var_gen):
		self.head_var = head_var
		self.var_gen = var_gen
		self.ground_preds = [] # [(negate, attrib, predicate)]
		self.id_preds = [] # [(negate, attrib, id, predicate)]
		self.attrib_ids = dict() # attrib -> [id]
	
	def copy(self, var_gen):
		c = SoarCondition(self.head_var, var_gen)
		c.ground_preds = self.ground_preds[:]
		c.id_preds = self.id_preds[:]
		c.attrib_ids = dict(self.attrib_ids.items())
		return c
	
	def add_ground_predicate(self, attrib, predicate, negate=False):
		if isinstance(predicate, ElementGGP):
			raise Exception("Sorry, but you're a homo.")
		self.ground_preds.append((negate, attrib, predicate))
		
	def add_id_predicate(self, attrib, negate=False, predicate = "", name = ""):
		if name == "":
			if attrib[0] == "<":
				id = self.var_gen.get_name(attrib[1])
			else:
				id = self.var_gen.get_name(attrib[0])
		else:
			id = name

		if self.attrib_ids.has_key(attrib):
			self.attrib_ids[attrib].append(id)
		else:
			self.attrib_ids[attrib] = [ id ]
			
		self.id_preds.append((negate, attrib, id, predicate))
		return id

	def get_ids(self, attrib):
		return self.attrib_ids[attrib]

	def get_attrib(self, id):
		for i in self.id_preds:
			if i[2] == id:
				return i[1]
		return ""

	def get_rhs_vars(self):
		vars = []
		for p in self.id_preds:
			vars.append(p[2])
		return set(vars)

	def is_empty(self):
		return len(self.ground_preds) == 0 and len(self.id_preds) == 0

	def to_str(self, distinctions = dict()):
		s = "<%s>" % self.head_var
		
		for pred in self.ground_preds:
			if pred[2].find(" ") < 0:
				pred_str = pred[2]
			else:
				pred_str = "{ %s }" % pred[2]

			if pred[0]:
				s += " -^%s %s" % (pred[1], pred_str)
			else:
				s += " ^%s %s" % (pred[1], pred_str)
		
		for pred in self.id_preds:
			ds = ""
			if distinctions.has_key(pred[2]):
				for d in distinctions[pred[2]]:
					ds += "<> <%s> " % d
			
			if pred[0]:
				s += " -^%s" % pred[1]
			else:
				s += " ^%s" % pred[1]
				
			if len(pred[3]) > 0 or len(ds) > 0:
				if pred[3] == "+": # total hack
					s += "  <%s> %s %s" % (pred[2], pred[3], ds)
				else:
					s += " { <%s> %s %s}" % (pred[2], pred[3], ds)
			else:
				s += " <" + pred[2] + ">"
		
		return "(%s)" % s

	def __str__(self):
		return self.to_str()

class SoarAction:
	def __init__(self, head_var, var_gen):
		self.head_var = head_var
		self.var_gen = var_gen
		self.wme_actions = [] # (add, attrib, val)
		self.op_props = [] # [(op_name, prefs)]
	
	def copy(self, var_gen):
		c = SoarAction(self.head_var, var_gen)
		c.wme_actions = self.wme_actions[:]
		c.op_props = self.op_props[:]
		return c
	
	def add_ground_wme_action(self, attrib, val):
		self.wme_actions.append((True, attrib, val))
	
	def remove_ground_wme_action(self, attrib, val):
		self.wme_actions.append((False, attrib, val))
	
	def add_id_wme_action(self, attrib, name=""):
		if name == "":
			id = self.var_gen.get_name(attrib)
		else:
			id = name
		self.wme_actions.append((True, attrib, "<%s>" % id))
		return id

	def remove_id_wme_action(self, attrib, name):
		vname = "<%s>" % name
		self.wme_actions.append((False, attrib, vname))
	
	def add_op_proposal(self, prefs, name = "o"):
		op_name = self.var_gen.get_name(name)
		self.op_props.append((op_name, prefs))
		return op_name

	def get_wme_actions(self, attr):
		l = []
		for a in self.wme_actions:
			if a[1] == attr:
				l.append(a)
		return l

	def __str__(self):
		if len(self.wme_actions) == 0 and len(self.op_props) == 0:
			return ""

		s = "<%s>" % self.head_var
		
		for a in self.wme_actions:
			if a[0]: # add
				s += " ^%s %s" % (a[1], a[2])
			else:
				s += " ^%s %s -" % (a[1], a[2])
	
		for p in self.op_props:
			s += " ^operator <%s> %s" % (p[0], p[1])
		
		return "(%s)" % s

class SoarProduction:
	def __init__(self, name):
		self.name = name
		self.var_gen = UniqueNameGenerator()
		self.state_id = self.var_gen.get_name("s")
		self.first_state_cond = None
		self.conds = []
		self.actions = []
		self.neg_conjs = dict()
		self.io_cond = None
		self.input_link = None
		self.output_link = None
		self.cond_paths = dict() # condition ref -> attrib
		self.var_distinctions = dict()

	def copy(self, name):
		c = SoarProduction(name)
		c.var_gen = self.var_gen.copy()
		c.state_id = self.state_id
		c.first_state_cond = self.first_state_cond.copy(c.var_gen)
		c.conds = [ i.copy(c.var_gen) for i in self.conds ]
		c.actions = [ i.copy(c.var_gen) for i in self.actions ]

		cond_map = dict()
		for i in range(len(c.conds)):
			cond_map[self.conds[i]] = c.conds[i]
		cond_map[self.first_state_cond] = c.first_state_cond

		for nc in self.neg_conjs.values():
			s = set()
			for cond in nc:
				s.add(cond_map[cond])
				c.neg_conjs[cond] = s

		if self.io_cond != None:
			c.io_cond = cond_map[self.io_cond]
		if self.input_link != None:
			c.input_link = cond_map[self.input_link]
		if self.output_link != None:
			c.output_link = cond_map[self.output_link]

		for cond in self.cond_paths:
			c.cond_paths[cond_map[cond]] = self.cond_paths[cond]

		c.var_distinctions = dict(self.var_distinctions.items())

		return c
	
	def add_condition(self, head_var = ""):
		if len(head_var) == 0:
			head_var = self.state_id
		
		c = SoarCondition(head_var, self.var_gen)
		if head_var == self.state_id:
			if self.first_state_cond == None:
				self.first_state_cond = c
			else:
				self.conds.append(c)
			self.cond_paths[c] = ""
			return c
		else:
			# figure out the attribute path of this condition
			for old_c in self.conds[:] + [self.first_state_cond]:
				if head_var in old_c.get_rhs_vars():
					attrib = old_c.get_attrib(head_var)
					if old_c.head_var == self.state_id:
						path = old_c.get_attrib(head_var)
					else:
						path = "%s.%s" % (self.cond_paths[old_c], attrib)

					self.cond_paths[c] = path
					self.conds.append(c)
					return c

			# didn't find a path, don't allow this to be attached
			raise Exception("Dangling lhs variable")
	
	def add_action(self, head_var = ""):
		if len(head_var) == 0:
			head_var = self.state_id
		a = SoarAction(head_var, self.var_gen)
		self.actions.append(a)
		return a

	def add_halt(self):
		self.actions.append("(halt)")

	def add_op_proposal(self, prefs, name =""):
		return self.add_action(self.add_action().add_op_proposal(prefs))

	def get_conditions(self, attrib_path):
		l = []
		for c in self.cond_paths:
			if self.cond_paths[c] == attrib_path:
				l.append(c)
		return l

	def get_actions(self, head_var):
		l = []
		for a in self.actions:
			if a.head_var == head_var:
				l.append(a)
		return l

	def get_proposed_operators(self):
		l = []
		for a in self.get_actions(self.state_id):
			for p in a.op_props:
				op_var = p[0]
				op_elabs = self.get_actions(op_var)
				for e in op_elabs:
					name_elab = e.get_wme_actions("name")
					if len(name_elab) > 0:
						l.append((op_var, name_elab[0][2]))
						break
		return l

	# this function only creates conditions to test for presence of
	# an operator with a certain name. It doesn't check for any kind
	# of operator elaborations
	def get_apply_rules(self, name_gen):
		rules = []
		props = self.get_proposed_operators()
		for p in props:
			op_name = p[1]
			prod_name = name_gen.get_name("apply*%s" % op_name)
			r = SoarProduction(prod_name)
			op_var = r.add_condition().add_id_predicate("operator")
			r.add_condition(op_var).add_ground_predicate("name", op_name)
			rules.append(r)

		return rules

	def get_il_cond(self):
		if self.input_link == None:
			if self.first_state_cond == None:
				self.first_state_cond = SoarCondition(self.state_id, self.var_gen)
			if self.io_cond == None:
				io_var = self.first_state_cond.add_id_predicate("io")
				self.io_cond = self.add_condition(io_var)

			il_var = self.io_cond.add_id_predicate("input-link")
			self.input_link = self.add_condition(il_var)
		return self.input_link

	def get_ol_cond(self):
		if self.output_link == None:
			if self.first_state_cond == None:
				self.first_state_cond = SoarCondition(self.state_id, self.var_gen)
			if self.io_cond == None:
				io_var = self.first_state_cond.add_id_predicate("io")
				self.io_cond = self.add_condition(io_var)

			ol_var = self.io_cond.add_id_predicate("output-link")
			self.output_link = self.add_condition(ol_var)
		return self.output_link

	def make_negative_conjunction(self, conjs):
		for c in conjs:
			self.neg_conjs[c] = conjs
	
	def add_var_distinction(self, v1, v2):
		if not self.var_distinctions.has_key(v1):
			self.var_distinctions[v1] = []
		self.var_distinctions[v1].append(v2)
		if not self.var_distinctions.has_key(v2):
			self.var_distinctions[v2] = []
		self.var_distinctions[v2].append(v1)

	def __calculate_distinctions(self, condition, bound_vars):
		distinct = dict()
		for v in condition.get_rhs_vars():
			if v not in bound_vars and self.var_distinctions.has_key(v):
				# only specify distinctions for yet unbound variables
				distinct[v] = []
				for dv in self.var_distinctions[v]:
					if dv in bound_vars:
						# only specify the distinction if the variable to be
						# distinguished from is already bound
						distinct[v].append(dv)
		return distinct


	def __str__(self):
		bound_vars = set([])
		used = set([])
		cond_str = "   (state %s" % self.first_state_cond.to_str()[1:]
		for c in self.conds:
			if not c.is_empty() and c not in used:

				if self.neg_conjs.has_key(c):
					cond_str += "\n   -{"
					for nc in self.neg_conjs[c]:
						distinct = self.__calculate_distinctions(nc, bound_vars)
						cond_str += "\n     " + nc.to_str(distinct)
						used.add(nc)
						bound_vars = bound_vars.union(nc.get_rhs_vars())
					cond_str += "\n   }"
				else:
					distinct = self.__calculate_distinctions(c, bound_vars)
					cond_str += "\n   " + c.to_str(distinct)
					used.add(c)
					bound_vars = bound_vars.union(c.get_rhs_vars())

		act_str = ""
		for a in self.actions:
			act_str  += "\n   " + str(a)

		return "sp {%s\n%s\n-->%s\n}" % (self.name, cond_str, act_str)

