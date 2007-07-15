import sys
import pdb
from GDL import *

class IntermediateRep:
	def __init__(self):
		# rules with conditions
		self.__rules = []
		# facts and parameters
		self.__statics = []
		self.__roles = []
		self.__inits = []

	def print_rules(self):
		for s in self.__statics:
			print str(s)
		for r in self.__rules:
			print str(r)

		print len(self.__statics)
		print len(self.__rules)

	def add_rule(self, head, body):
		if len(body) == 0:
			if head.get_relation() == "init":
				self.__inits.append(head.get_term(0))
			elif head.get_relation() == "role":
				self.__roles.append(head.get_term(0))
			else:
				self.__statics.append(head)

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
			for vc in var_comps:
				rule.add_var_constraint(vc[0], vc[1], vc[2])
			# add the rule 
			self.__rules.append(rule)

	def get_rules(self):
		return self.__rules
