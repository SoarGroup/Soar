# newer, better version of a Soar production builder

from UniqueNameGen import UniqueNameGenerator
import pdb

def mvar(id):
	return "<%s>" % id

class Conjunction:
	@staticmethod
	def MakeTopConjunction(state_id, name_gen):
		c = Conjunction(None, name_gen, False)
		c.__ids.append(state_id)
		return c

	def __init__(self, parent, name_gen, negated):
		self.__negated = negated
		self.__name_gen = name_gen
		self.__ids = []
		self.__id_attribs = {} # id -> [(attrib, id)]
		self.__attribs = {} # id -> [(attrib, rhs)]
		self.__preds = {} # id -> [(predicate, rhs)]
		self.__par = parent
		self.__children = []
	
	def copy(self, parent, name_gen):
		c = Conjunction(parent, name_gen, self.__negated)
		c.__ids = self.__ids[:]
		for id, attribs in self.__id_attribs.items():
			c.__id_attribs[id] = attribs[:]
		for id, attribs in self.__attribs.items():
			c.__attribs[id] = attribs[:]
		for id, preds in self.__preds.items():
			c.__preds[id] = preds[:]
		c.__children = [child.copy(c, name_gen) for child in self.__children]
		return c

	def id_in_scope(self, id):
		if id in self.__ids:
			return True
		else:
			if self.__par != None:
				return self.__par.id_in_scope(id)
			else:
				return False

	def add_predicate(self, id, predicate, rhs):
		assert self.id_in_scope(id), "Unknown identifier %s" % id
		if (predicate, rhs) not in self.__preds.setdefault(id,[]):
			self.__preds[id].append((predicate, rhs))

	def add_id_attrib(self, id, attrib, name = ""):
		assert self.id_in_scope(id), "Unknown identifier %s" % id
		if name != "":
			new_id = name # maybe we should have a different function for this case?
		elif attrib[0] == '<':
			# variable attribute
			new_id = self.__name_gen.get_name(attrib[1])
		else:
			new_id = self.__name_gen.get_name(attrib[0])

		self.__ids.append(new_id)
		self.__id_attribs.setdefault(id, []).append((attrib, new_id))
		return new_id
	
	def add_id_var_attrib(self, id, var_attrib, name = ""):
		new_id = self.add_id_attrib(id, '<%s>' % var_attrib, name)
		self.__ids.append(var_attrib)
		return new_id

	def add_id_attrib_chain(self, id, attrib_chain):
		assert self.id_in_scope(id), "Unknown identifier %s" % id
		parent_id = id
		for attrib in attrib_chain:
			next_parent_id = self.add_id_attrib(parent_id, attrib)
			parent_id = next_parent_id
		return parent_id

	def get_all_ids(self):
		all_ids = self.__ids[:]
		for child in self.__children:
			all_ids.extend(child.get_all_ids())
		return all_ids

	def get_ids(self, lhs_id, attrib):
		assert self.id_in_scope(lhs_id), "Unknown identifier %s" % lhs_id
		attribs = self.__id_attribs.get(lhs_id, [])
		ids = []
		for a in attribs:
			if a[0] == attrib:
				ids.append(a[1])
		return ids

	def get_ids_by_chain(self, lhs_id, attrib_chain):
		curr_ids = [lhs_id]
		for attrib in attrib_chain:
			next_ids = []
			for id in curr_ids:
				id_attribs = self.__id_attribs.get(id, [])
				for ida in id_attribs:
					if ida[0] == attrib:
						next_ids.append(ida[1])
			if len(next_ids) == 0:
				return []
			curr_ids = next_ids
		return curr_ids

	def get_or_make_id_chain(self, lhs_id, attrib_chain):
		ids = self.get_ids_by_chain(lhs_id, attrib_chain)
		if len(ids) == 0:
			return [self.add_id_attrib_chain(lhs_id, attrib_chain)]
		else:
			return ids

	def get_or_make_id_chain_existing(self, lhs_id, attrib_chain):
		curr_ids = [lhs_id]
		for i, attrib in enumerate(attrib_chain):
			next_ids = []
			for id in curr_ids:
				id_attribs = self.__id_attribs.get(id, [])
				for ida in id_attribs:
					if ida[0] == attrib:
						next_ids.append(ida[1])
			if len(next_ids) == 0:
				# ran out, chain off existing id
				return [self.add_id_attrib_chain(curr_ids[0], attrib_chain[i:])]
			else:
				curr_ids = next_ids
		return curr_ids


	def add_attrib(self, id, attrib, rhs):
		assert self.id_in_scope(id), "Unknown identifier %s" % id
		self.__attribs.setdefault(id, []).append((attrib, rhs))
	
	def add_child(self, negated):
		child = Conjunction(self, self.__name_gen, negated)
		self.__children.append(child)
		return child

	def get_parent(self):
		return self.__par

	def to_str(self, indent, state_id):
		temp_preds = dict(self.__preds.items())
		indentstrhead = ' ' * indent
		indentstr = ' ' * (indent + 2)
		# if there's no state id, then assume it's
		# already declared
		state_declared = (state_id == "")
		if self.__negated:
			s = indentstrhead + "-{\n"
		else:
			s = indentstrhead + "{\n"
		for id, attribs in self.__id_attribs.items():
			if id == state_id and not state_declared:
				s += indentstr + "(state %s" % mvar(id)
				state_declared = True
			else:
				s += indentstr + "(%s" % mvar(id)

			for attrib, rhs_id in attribs:
				if rhs_id in temp_preds:
					s += " ^%s { %s" % (attrib, mvar(rhs_id))
					for comp, rhs in temp_preds[rhs_id]:
						s += " %s %s" % (comp, rhs)
					del temp_preds[rhs_id]
					s += "}"
				else:
					s += " ^%s %s" % (attrib, mvar(rhs_id))
			s += ")\n"
		for id, attribs in self.__attribs.items():
			if id == state_id and not state_declared:
				s += indentstr + "(state %s" % mvar(id)
				state_declared = True
			else:
				s += indentstr + "(%s" % mvar(id)

			for attrib, rhs in attribs:
				s += " ^%s %s" % (attrib, rhs)
			s += ")\n"

		assert len(temp_preds) == 0, "Some predicates not used??"

		for child in self.__children:
			s += child.to_str(indent + 2, "") + "\n"
		s += indentstrhead + "}"
		return s

class SoarProd:
	def __init__(self, name, state_name):
#		if name == 'apply*remove-_next__holding__std_soar_var0__':
#			pdb.set_trace()
		self.__name = name
		self.__state_name = state_name
		self.__name_gen = UniqueNameGenerator()
		self.__state_id = self.__name_gen.get_name("s")
		
		self.__top_conj = Conjunction.MakeTopConjunction(self.__state_id, self.__name_gen)
		if state_name != '':
			self.__top_conj.add_attrib(self.__state_id, "name", state_name)
		self.__curr_conj = self.__top_conj

		self.__create_id_actions = {} # id -> [(attrib, id, prefs)]
		self.__actions = {} # id -> [(attrib, rhs, action)]
		self.__new_ids = [] # ids created by actions
		self.__rhs_funcs = []
	
	def copy(self, name):
		c = SoarProd(name, self.__state_name)
		c.__name_gen = self.__name_gen.copy()
		c.__state_id = self.__state_id
		c.__top_conj = self.__top_conj.copy(None, c.__name_gen)
		c.__curr_conj = c.__top_conj # dangerous?

		for id, id_actions in self.__create_id_actions:
			c.__create_id_actions[id] = id_actions[:]
		for id, actions in self.__actions:
			c.__actions[id] = actions[:]

		c.__new_ids = self.__new_ids[:]
		c.__rhs_funcs = self.__rhs_funcs[:]

		return c

	def get_name(self):
		return self.__name

	def get_state_id(self):
		return self.__state_id

	def get_state_name(self):
		return self.__state_name

	def get_name_gen(self):
		return self.__name_gen

	def add_predicate(self, id, predicate, rhs):
		self.__curr_conj.add_predicate(id, predicate, rhs)
	
	def add_id_attrib(self, id, attrib, name = ""):
		assert attrib[0] != '<', 'Use new system to add variable attributes'
		return self.__curr_conj.add_id_attrib(id, attrib, name)

	def add_id_var_attrib(self, id, var_attrib, name = ""):
		return self.__curr_conj.add_id_var_attrib(id, var_attrib, name)
	
	def add_id_attrib_chain(self, id, attrib_chain):
		return self.__curr_conj.add_id_attrib_chain(id, attrib_chain)
	
	def get_ids(self, lhs_id, attrib):
		return self.__curr_conj.get_ids(lhs_id, attrib)

	def get_ids_by_chain(self, attrib_chain):
		return self.__top_conj.get_ids_by_chain(self.__state_id, attrib_chain)

	def get_or_make_id_chain(self, attrib_chain):
		return self.__top_conj.get_or_make_id_chain(self.__state_id, attrib_chain)

	def get_or_make_id_chain_existing(self, attrib_chain):
		return self.__top_conj.get_or_make_id_chain_existing(self.__state_id, attrib_chain)

	def add_attrib(self, id, attrib, rhs):
		self.__curr_conj.add_attrib(id, attrib, rhs)

# actions

	def add_action(self, id, attrib, rhs, action):
		assert id in self.__top_conj.get_all_ids() or id in self.__new_ids, "Unknown identifier %s" % id
		self.__actions.setdefault(id, []).append((attrib, rhs, action))

	def add_create_id(self, id, attrib, name = "", prefs = ""):
		assert id in self.__top_conj.get_all_ids() or id in self.__new_ids, "Unknown identifier %s" % id
		if name != "":
			assert name in self.__top_conj.get_all_ids() or id in self.__new_ids, "Unknown identifier %s" % id
			self.__create_id_actions.setdefault(id, []).append((attrib, name, prefs))
			return name
		else:
			new_id = self.__name_gen.get_name(attrib[0])
			self.__create_id_actions.setdefault(id, []).append((attrib, new_id, prefs))
			self.__new_ids.append(new_id)
			return new_id

	def get_new_ids_by_chain(self, lhs_id, attrib_chain):
		curr_ids = [lhs_id]
		for attrib in attrib_chain:
			next_ids = []
			for id in curr_ids:
				create_actions = self.__create_id_actions.get(id, [])
				for c_attrib, c_id, prefs in create_actions:
					if c_attrib == attrib:
						next_ids.append(c_id)
			if len(next_ids) == 0:
				return []
			curr_ids = next_ids
		return curr_ids
	
	def add_rhs_func_call(self, function):
		self.__rhs_funcs.append(function)

	def begin_negative_conjunction(self):
		self.__curr_conj = self.__curr_conj.add_child(True)

	def end_negative_conjunction(self):
		self.__curr_conj = self.__curr_conj.get_parent()

	# convenience functions
	def add_neg_id_attrib(self, id, attrib):
		self.begin_negative_conjunction()
		new_id = self.add_id_attrib(id, attrib)
		self.end_negative_conjunction()
		return new_id

	def add_neg_attrib(self, id, attrib, rhs):
		self.begin_negative_conjunction()
		new_id = self.add_attrib(id, attrib, rhs)
		self.end_negative_conjunction()
		return new_id

	def add_bound_id_attrib(self, lhs_id, attrib, rhs_id):
		assert self.__curr_conj.id_in_scope(rhs_id)
		self.add_attrib(lhs_id, attrib, '<%s>' % rhs_id)

	def add_id_predicate(self, lhs_id, predicate, rhs_id):
		self.__curr_conj.add_predicate(lhs_id, predicate, mvar(rhs_id))

	def add_destroy_id(self, lhs_id, attrib, rhs_id):
		self.add_action(lhs_id, attrib, mvar(rhs_id), '-')
	
	def add_create_bound_id(self, lhs_id, attrib, rhs_id):
		self.add_action(lhs_id, attrib, mvar(rhs_id), '')

	def add_create_constant(self, id, attrib, const):
		self.add_action(id, attrib, const, '')

	def add_destroy_constant(self, id, attrib, const):
		self.add_action(id, attrib, const, '-')
	
	def add_operator_pref(self, op_id, prefs):
		self.add_action(self.__state_id, "operator", mvar(op_id), prefs)
	
	def add_operator_prop(self, name, prefs = '+'):
		op_id = self.add_create_id(self.__state_id, 'operator', prefs = prefs)
		self.add_create_constant(op_id, 'name', name)
		return op_id

	def add_operator_test(self, op_name):
		op_id = self.add_id_attrib(self.__state_id, 'operator')
		self.add_attrib(op_id, 'name', op_name)

	def get_proposed_op_names(self):
		op_names = []
		for attrib, op_id, pref in self.__create_id_actions.get(self.__state_id, []):
			if attrib == "operator" and '+' in pref:
				# find the name
				op_elabs = self.__actions[op_id]
				name_found = False
				for attrib2, rhs2, action2 in op_elabs:
					if attrib2 == 'name':
						op_names.append(rhs2)
						name_found = True
						break
				assert name_found, "Operator proposed without name"
		return op_names

	def __str__(self):
		indentstr = ' ' * 3
		s = "sp {%s\n" % self.__name
		s += self.__top_conj.to_str(0, self.__state_id) + "\n"
		s +="-->\n"

		for id, actions in self.__actions.items():
			s += indentstr + "(%s" % mvar(id)
			for attrib, rhs, act in actions:
				s += " ^%s %s %s" % ( attrib, rhs, act)
			s += ")\n"

		for id, actions in self.__create_id_actions.items():
			s += indentstr + "(%s" % mvar(id)
			for attrib, rhs_id, pref in actions:
				s += " ^%s %s %s" % ( attrib, mvar(rhs_id), pref)
			s += ")\n"

		for func in self.__rhs_funcs:
			s += indentstr + "(%s)\n" % func
		s += '}'
		return s
