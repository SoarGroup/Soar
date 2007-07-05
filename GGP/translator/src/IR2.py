import sys
import pdb
from Model3 import *
from ConstrainedModel import ConstrainedModel
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

		# a map from predicates to models (set of grounds)
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
	def __update_head_model(r, cmodel, new_models):
		# add new grounds into the constrained model
		for bi, b in enumerate(r.get_body()):
			pred = b.get_predicate()
			if pred in new_models:
				#print "%s: %d" % (pred, models[pred].count())
				for g in new_models[pred]:
					cmodel.add_ground(bi, g)
			#print "--> %s ||| %d" % (str(b), cmodel.get_size())

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

	def __build_head_model(self, r):
		return IntermediateRep.__update_head_model(r, ConstrainedModel(), self.__models)

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
				open_rules.append((r, ConstrainedModel(r)))
			#else:
				#print "OMITTED:\n%s" % str(r)

		# now we're going to go through each open rule until they all get processed
		temp_open_rules = range(len(open_rules))
		new_models = {}
		for pred in self.__models:
			new_models[pred] = self.__models[pred].copy()

		while len(temp_open_rules) > 0:
			next_temp_open = []
			sys.stderr.write("%d\n" % len(temp_open_rules))
			next_new_models = {}
			for ri in temp_open_rules:
				r, cmodel = open_rules[ri]
				pred = r.get_head().get_predicate()
				head_model = IntermediateRep.__update_head_model(r, cmodel, new_models)
				# subtract out grounds already seen
				head_model.subtract(self.__models.setdefault(pred, Model()))
				head_model.subtract(new_models.setdefault(pred, Model()))

				if head_model.count() > 0:
					next_new_models.setdefault(pred, Model()).union(head_model)

			print "-----------"
			for pred in next_new_models:
				for g in next_new_models[pred]:
					print "NEW GROUND: %s" % str(g)
			print "-----------"

			# open the rules that have new grounds for their body predicates
			temp_open_rules = []
			for ri in range(len(open_rules)):
				r = open_rules[ri][0]
				preds = r.get_body_predicates()
				for p in preds:
					if p in next_new_models:
						#print ">>> opening %s" % str(r.get_head())
						temp_open_rules.append(ri)
						break
			
			# after these grounds have been seen, add them to the processed set
			for pred in new_models:
				self.__models.setdefault(pred, Model()).union(new_models[pred])
			new_models = next_new_models

		for pred in self.__models:
			print pred
			self.__models[pred].print_m()

	@staticmethod
	def make_frame_axioms(rules):
		# cond_table[i][j] is the jth condition of the ith rule
		# cond_table[i][j] is either a (not b, None) for some b, or (b, not comp)
		# for some b and some comparison comp involving the variables in b
		cond_table = []
		for r in rules:
			cond_list = []
			constraints = r.get_constraints_by_body_index()
			for bi, b in enumerate(r.get_body()):
				# insert the negated condition
				negated = b.copy()
				negated.negate()
				cond_list.append((negated, None))
				# insert original condition, but with a constraint broken
				if (bi, bi) in constraints:
					for c in constraints[(bi, bi)]:
						complement_constraint = (c[0], c[1], c[2].complement())
						cond_list.append((b, complement_constraint))

		# now make intermediate soar production representations by taking all
		# possible combinations of one condition from each list in cond_table



#	def calc_frame_axioms(self):
#		# Predicate -> [(head model, list of rules)]
#		frame_axioms = {}
#		for r in self.__rules:
#			if r.is_frame_axiom():
#				pred = r.get_head().get_predicate()
#				head_model = self.__build_head_model(r)
#				found = False
#				fa_list = frame_axioms.setdefault([], pred)
#				for model_list_pair in fa_list:
#					if head_model = model_list_pair[0]:
#						model_list_pair[1].append(r)
#						found = True
#						break
#				if not found:
#					fa_list.append((head_model, [r]))
#
#		for pred in frame_axioms:
#			for model_list_pair in frame_axioms[pred]:
#				
