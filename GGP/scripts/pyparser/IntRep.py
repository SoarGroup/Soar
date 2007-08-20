from GDL import *

class IntermediateRep:
	def __init__(self):
		# state update rules
		self.__update_rules = []
		# elab rules
		self.__elab_rules = []
		# rules that determine goals
		self.__goal_rules = []
		# rules that determine termination
		self.__term_rules = []
		# rules that determine legality
		self.__legal_rules = []
		# facts and parameters
		self.__statics = []
		self.__roles = []
		self.__inits = []
	
	def copy(self):
		c = IntermediateRep()
		for varname in c.__dict__:
			c.__dict__[varname] = self.__dict__[varname][:]
		return c

	def print_rules(self):
		for s in self.__statics:
			print str(s)
		for r in self.__rules:
			print str(r)

	def add_rule(self, head, body):
		if len(body) == 0:
			if head.get_relation() == "init":
				self.__inits.append(head.get_term(0))
			elif head.get_relation() == "role":
				self.__roles.append(head.get_term(0))
			else:
				self.__statics.append(head)
			return

		rule_bodies = []
		var_comps = []

		for b in body:
			if isinstance(b, list):
				# this is an or
				new_rule_bodies = []
				for rb in rule_bodies:
					for or_rel in b:
						new_rule_bodies.append(rb[:] + [or_rel])
				rule_bodies = new_rule_bodies
			elif b.get_relation() in CompRelations:
				var_comps.append((Comparison(b.get_relation()), b.get_term(0), b.get_term(1)))
			else:
				if len(rule_bodies) == 0:
					rule_bodies.append([b])
				else:
					for bb in rule_bodies:
						bb.append(b)
	
		for rb in rule_bodies:
			rule = Rule(head, rb)
			# process the variable comparisons
			for comp, t1, t2 in var_comps:
				rule.add_var_constraint(t1, t2, comp)
			# add the rule 
			if head.get_relation() == "goal":
				self.__goal_rules.append(rule)
			elif head.get_relation() == "terminal":
				self.__term_rules.append(rule)
			elif head.get_relation() == "next":
				self.__update_rules.append(rule)
			elif head.get_relation() == "legal":
				self.__legal_rules.append(rule)
			else:
				self.__elab_rules.append(rule)

	def get_all_rules(self):
		return self.__update_rules + \
			   self.__elab_rules + \
			   self.__legal_rules + \
			   self.__goal_rules + \
			   self.__term_rules
	
	def get_roles(self): return self.__roles
	def get_update_rules(self): return self.__update_rules
	def get_elab_rules(self): return self.__elab_rules
	def get_legal_rules(self): return self.__legal_rules
	def get_goal_rules(self): return self.__goal_rules
	def get_terminal_rules(self): return self.__term_rules

	def get_statics(self): return self.__statics
	def get_inits(self): return self.__inits

