import sys
import pdb
from Model import *
from GDL import *

class IntermediateRep:
	def __init__(self):
		# rules with conditions
		self.__rules = []
		# facts and parameters
		self.__statics = []
		self.__roles = []
		self.__inits = []

		self.__userrel_models = {}
		self.__persistent_models = {}
		self.__legal_models = {}

		# map from predicate to model (set of grounds)
		self.__models = {}

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

	@staticmethod
	def __build_head_model(r):
		# get the cross-product of the models of each condition,
		# and filter out all unqualified tuples based on the variable
		# comparison constraints.
		
		cmodel = ConstrainedModel(r)
		for b in r.get_body():
			if b.get_predicate() in self.__models:
				cmodel.add_model(self.__models[b.get_predicate()])
			else:
				# one body condition doesn't have any grounds, this rule doesn't generate
				# anything
				return Model()
			print "--> %s ||| %d" % (str(b), cmodel.get_size())

		# the constrained model should handle everything okay, now we just
		# have to get the valid grounds for the head out
		
		# first get a list of positions for every variable in the sentence
		head = r.get_head()
		vars = head.get_var_terms()
		hpos = [PositionIndex.get_positions(head, v) for v in vars]
		
		# for each variable, get an occurrence of it from the body
		bpos = []
		for v in vars:
			found = False
			for bi, b in enumerate(r.get_body()):
				p = PositionIndex.get_positions(b, v)
				if len(p) > 0:
					bpos.append((bi, p[0]))
					found = True
					break
			assert found, "Variable %s does not appear in any body condition of\n%s" % (str(v), str(r))

		assert len(bpos) == len(vars)

		val_list = cmodel.get_valid_combinations(bpos)
		# this gives us a list of list of values, we still have to
		# convert back into grounds
		
		head_model = Model()
		for values in val_list:
			if head.get_relation() == "next":
				head_ground = Sentence("true", head.get_terms())
			else:
				head_ground = head.copy()
			for i, hps in enumerate(hpos):
				for hp in hps:
					hp.set(head_ground, values[i])
			head_model.add_ground(head_ground)

		return head_model

	def find_models(self):
		open_rules = []

		# add all user defined relations that don't have any conditions to the set
		# of partially defined models for their respective predicates
		# i.e. facts and parameters
		for s in self.__statics:
			self.__models.setdefault(s.get_relation(), Model()).add_ground(s)

		# also add inits for grounds of persistent relations
		for func in self.__inits:
			ground = Sentence("true", [func])
			self.__models.setdefault(func.get_name(), Model()).add_ground(ground)

		# find all things with undetermined models
		for r in self.__rules:
			pred = r.get_head().get_predicate()
			# frame axioms will never introduce different grounds, so don't have to
			# process them
			if pred != None and not r.is_frame_axiom():
				open_rules.append(r)
			#else:
				#print "OMITTED:\n%s" % str(r)

		# now we're going to go through each open rule until they all get processed
		temp_open_rules = open_rules[:]
		while len(temp_open_rules) > 0:
			next_temp_open = []
			sys.stderr.write("%d\n" % len(temp_open_rules))
			print "NEW ITERATION"
			for r in temp_open_rules:
				print str(r)
				head_model = IntermediateRep.__build_head_model(r)
				pred = r.get_head().get_predicate()
				if pred in self.__models:
					old_ground_count = self.__models[pred].count()
				else:
					old_ground_count = 0
				self.__models.setdefault(pred, Model()).union(head_model)

				if self.__models[pred].count() != old_ground_count:
					print "Opening new rules"
					# more new grounds generated, open up the rules that depend on them
					for r1 in open_rules:
						if pred in r1.get_body_predicates() and r1 not in next_temp_open:
							print ">>> opening %s" % str(r1.get_head())
							next_temp_open.append(r1)
			temp_open_rules = next_temp_open

		for pred in self.__models:
			print pred
			self.__models[pred].print_m()
