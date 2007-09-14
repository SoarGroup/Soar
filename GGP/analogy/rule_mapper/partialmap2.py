# This is the version that doesn't explicitly map moves, but when moves are
# matched, the predicates that are their effects are put into the same types

import sys
from bestlist import BestList
from ggp_utils import *
import predicate
from GDL import Sentence
from move_effects import calc_move_effects
import placetype
from equivalence_class import EquivalenceClass
import options
#from IPython.Debugger import Pdb
#pdb = Pdb('Linux')
import pdb

class PartialMap:
	
	# the order in which rules should be matched. The rationale behind
	# matching move rules first is that there are usually fewer move rules
	# and this allows for less of a chance of messing up, while giving
	# future rule matches more to work off
	RULE_MATCH_ORDER = [ Sentence.MOVE, Sentence.ELAB, Sentence.STATE]

	__place_type_weights = { placetype.AGENT : 2.0,
	                         placetype.OBJECT : 1.0,
	                         placetype.NUM_MIN : 1.5,
	                         placetype.NUM_MAX : 1.5,
	                         placetype.COORD : 0.8,
	                         placetype.NUM : 0.6,
	                         placetype.UNKNOWN : 0.3 }

	@staticmethod
	def create(src_int_rep, src_preds, tgt_int_rep, tgt_preds):
		"""src_preds - [(pred, type)]
		   tgt_preds - [(pred, type)]"""

		self = PartialMap()

		# map from predicate name to predicate structure
		self.__src_preds = dict((p.get_name(), p) for p,t in src_preds)
		self.__src_pred_types = dict(src_preds)
		self.__tgt_preds = dict((p.get_name(), p) for p,t in tgt_preds)
		self.__tgt_pred_types = dict(tgt_preds)

		# calculate move effects
		self.__src_move_effects = dict((move, [self.__src_preds[n] for n in names]) for move, names in calc_move_effects(src_int_rep).items())
		self.__tgt_move_effects = dict((move, [self.__tgt_preds[n] for n in names]) for move, names in calc_move_effects(tgt_int_rep).items())

		self.__pred_match_scores = {}
		# map all predicates that are identical in source and target onto each other
		common_pred_names = set(self.__src_preds.keys()).intersection(set(self.__tgt_preds.keys()))
		common_pred_map = dict((self.__src_preds[i], self.__tgt_preds[i]) for i in common_pred_names)
		self.__matched_preds = {}
		for s, t in common_pred_map.items():
			self.__add_predicate_match(s, t)

		self.__matched_rules = {}
		# candidate rule matchings
		# (src_rule, tgt_rule) -> (body map, score)
		self.__cand_rule_matches = {}

		# create all possible matches
		src_rules = [src_int_rep.get_legal_rules(), \
		             src_int_rep.get_update_rules(), \
		             src_int_rep.get_elab_rules(), \
		             src_int_rep.get_goal_rules(), \
		             src_int_rep.get_terminal_rules()]

		tgt_rules = [tgt_int_rep.get_legal_rules(), \
		             tgt_int_rep.get_update_rules(), \
		             tgt_int_rep.get_elab_rules(), \
		             tgt_int_rep.get_goal_rules(), \
		             tgt_int_rep.get_terminal_rules()]
		
		for i in range(5):
			# take all possible pairs of source, target rules
			for sr, tr in cross_product(src_rules[i], tgt_rules[i]):
				body_maps = []
				# don't include the head predicate in the body, because the head
				# will always matched to the head of the other rule
				shp = sr.get_head().get_predicate()
				thp = tr.get_head().get_predicate()
				srpbody = [self.__src_preds[b.get_predicate()] for b in sr.get_body() if b.get_predicate() != shp and not b.is_negated()]
				srnbody = [self.__src_preds[b.get_predicate()] for b in sr.get_body() if b.get_predicate() != shp and b.is_negated()]
				trpbody = [self.__tgt_preds[b.get_predicate()] for b in tr.get_body() if b.get_predicate() != thp and not b.is_negated()]
				trnbody = [self.__tgt_preds[b.get_predicate()] for b in tr.get_body() if b.get_predicate() != thp and b.is_negated()]

				# generators galore!
				pmaps = possible_matchings(srpbody, trpbody)
				# since this is nested in the second for, we can't use a generator
				nmaps = [m for m in possible_matchings(srnbody, trnbody)]
				for pm in pmaps:
					for nm in nmaps:
						# merge the positive maps and negative maps
						total_map = pm.copy()
						total_map.update(nm)
						score = self.__body_map_score(sr, tr, total_map)
						# if a body map score == 0, then that map is illegal
						if score > 0:
							body_maps.append((total_map, score))

				if len(body_maps) > 0:
					self.__cand_rule_matches[(sr,tr)] = BestList(lambda x: x[1], body_maps)
		
		# the suppressed set is for temporarily preventing some rule matches
		# from being selected. useful when unrolling
		self.__suppressed = set()

		return self
	
	def __init__(self):
		pass

	def copy(self):
		c = PartialMap()
		# don't need to copy these, they never get modified
		c.__src_preds = self.__src_preds
		c.__tgt_preds = self.__tgt_preds
		c.__src_move_effects = self.__src_move_effects

		c.__tgt_move_effects = self.__tgt_move_effects
		c.__src_pred_types = self.__src_pred_types.copy()
		c.__tgt_pred_types = self.__tgt_pred_types.copy()
		c.__matched_preds = self.__matched_preds.copy()
		c.__matched_rules = self.__matched_rules.copy()
		c.__cand_rule_matches = dict((k, v.copy()) for k, v in self.__cand_rule_matches.items())
		c.__pred_match_scores = self.__pred_match_scores.copy()
		c.__suppressed = self.__suppressed.copy()
		return c

	def __pred_arg_match_score(self, sp, tp):
		weights = PartialMap.__place_type_weights

		if sp.get_arity() == 0 and tp.get_arity() == 0:
			# without any arguments, we can't really compare how "similar"
			# these two predicates are
			return 1
		
		# calculate the score as the number of arguments of the same type
		# divided by the total number of arguments
		intersect = placetype.type_intersect(sp.get_place_types(), tp.get_place_types())
		intersect_score = sum((weights[x[0]] + weights[x[1]]) * x[2]  for x in intersect)
		return intersect_score / max(abs(sp.get_arity() - tp.get_arity()),1)

	def pred_match_score(self, sp, tp):
		p = (sp, tp) # for optimization
		score = self.__pred_match_scores.get(p, None)
		if score is None:
			score = self.__calc_pred_match_score(sp, tp)
			self.__pred_match_scores[p] = score
			return score
		else:
			return score
	
	def __calc_pred_match_score(self, sp, tp):
		# check to make sure the predicates don't already have conflicting mappings
		if sp in self.__matched_preds:
			if self.__matched_preds[sp] != tp:
				return options.MISMATCHED_PRED_SCORE
			else:
				return options.MATCHED_PRED_SCORE
		else:
			if tp in self.__matched_preds.values():
				return options.MISMATCHED_PRED_SCORE
		
		# ensure that predicate types are mappable
		# legal/does -/-> next/true, elabs
		if Sentence.MOVE in [sp.get_type(), tp.get_type()] and \
				sp.get_type() != tp.get_type():
			return options.MISMATCHED_PRED_SCORE
		
		# if two predicates are in the same category, give the mapping a score boost.
		# if they are in different categories, give a penalty. Otherwise, 
		spt = self.__src_pred_types[sp]
		tpt = self.__tgt_pred_types[tp]

		# if either predicate doesn't have a type, then don't punish or reward
		if spt == 0 or tpt == 0:
			cat_mult = 1
		elif spt == tpt:
			cat_mult = options.BIN_MATCH_MULT
		else:
			cat_mult = options.BIN_MISMATCH_MULT

		# if all above conditions are met, then make a rating based on argument types
		return cat_mult * (self.__pred_arg_match_score(sp, tp) + 0.1)

	def __body_map_score(self, sr, tr, m):
		"Warning: can potentially change mapping"

		total = 0
		for sp, tp in m.items():
			pm_score = self.pred_match_score(sp, tp)
			if pm_score == 0:
				if options.ALLOW_PARTIAL_MAPS:
					# if two predicates are illegal to match, then don't include it
					# in the match
					del m[sp]
				else:
					# if two predicates don't match, then the entire mapping is illegal
					return 0
			else:
				total += pm_score

		#num_unmatched = max(len(sr.get_body()), len(tr.get_body())) - len(m)
		#return max(0.1, total - num_unmatched) 
		return total

	def __effect_map_score(self, effect_map):
		"Effect map score is pretty much just like body map score"

		score = 0
		for se, te in effect_map.items():
			s = self.pred_match_score(se, te)
			if s == 0:
				return 0
			else:
				score += s
		
		return score

	def __head_match_score(self, sr, tr):
		srhead = sr.get_head()
		trhead = tr.get_head()
		if srhead.get_relation() == 'goal':
			assert trhead.get_relation() == 'goal'
			if srhead.get_term(1) == trhead.get_term(1):
				return 1
			else:
				# this is only assuming that there is at least one
				# 0 score goal and one 100 score goal for each scenario
				return 0
		elif srhead.get_relation() == 'terminal':
			assert trhead.get_relation() == 'terminal'
			return 1
		else:
			shp = self.__src_preds[srhead.get_predicate()]
			thp = self.__tgt_preds[trhead.get_predicate()]
			return self.pred_match_score(shp, thp)

	def __rule_match_score(self, sr, tr):
		if (sr, tr) not in self.__cand_rule_matches:
			return (None, 0)
		
		# try to matche the head sentences first
		head_match_score = self.__head_match_score(sr, tr)
		if head_match_score == 0:
			if not options.ALLOW_PARTIAL_MAPS:
				# if heads don't match, rules can't match
				return (None, 0)

		# find the best body mapping
		best_map, best_score = self.__cand_rule_matches[(sr,tr)].get_best()

		best_score += options.HEAD_SCORE_FACTOR * head_match_score
		
		num_preds_matched = self.__num_preds_already_matched(sr, tr, best_map)
		best_score *= (options.MATCHED_RULE_MULT ** num_preds_matched)
		return (best_map, best_score)

	def __num_preds_already_matched(self, sr, tr, bm):
		n = 0
		shp = sr.get_head()
		
		if shp in self.__matched_preds:
			n += 1

		for sp in bm:
			if sp in self.__matched_preds:
				n += 1

		return n

	def __update_rule_match(self, sr, tr):
		# recalculate body mapping score for all possible mappings
		body_maps = self.__cand_rule_matches[(sr,tr)]
		i = 0
		while i < len(body_maps):
			new_score = self.__body_map_score(sr, tr, body_maps[i][0])
			if new_score == 0:
				# no longer valid map
				del body_maps[i]
			else:
				body_maps[i] = (body_maps[i][0], new_score)
				i += 1

		#heapify(body_maps)

	def __update_all_rule_matches(self):
		to_erase = []
		for sr, tr in self.__cand_rule_matches:
			# check if heads still match
			head_match_score = self.__head_match_score(sr, tr)
			if head_match_score == 0:
				to_erase.append((sr, tr))
				continue
			
			self.__update_rule_match(sr, tr)

			if len(self.__cand_rule_matches[(sr,tr)]) == 0:
				to_erase.append((sr,tr))

		debug_print('disqualified %d rules' % len(to_erase))
		for sr, tr in to_erase:
			#debug_print('RULE MATCH\n%s %s\nDISQUALIFIED' % (sr, tr))
			del self.__cand_rule_matches[(sr,tr)]
		
		return len(to_erase)
	
	def __update_bins_with_move_effects(self, src_move, tgt_move):
		# only update the unmatched predicates
		src_effects = set(self.__src_move_effects[src_move]) - set(self.__matched_preds.keys())
		tgt_effects = set(self.__tgt_move_effects[tgt_move]) - set(self.__matched_preds.values())

		# I'm too lazy to maintain a counter
		base_type = 100000 + abs(hash(src_move)) + abs(hash(tgt_move))

		# if two effects had the same non-zero bin before, then distinguish them from the
		# other effects
		ec = EquivalenceClass()
		for se in src_effects:
			for te in tgt_effects:
				st = self.__src_pred_types[se]
				tt = self.__tgt_pred_types[te]
				if st != 0 and tt != 0 and st == tt:
					ec.make_equivalent(se, te)

		for e in src_effects:
			if ec.has_member(e):
				self.__src_pred_types[e] = base_type+ec.get_class_index(e)+1
			else:
				self.__src_pred_types[e] = base_type

		for e in tgt_effects:
			if ec.has_member(e):
				self.__tgt_pred_types[e] = base_type+ec.get_class_index(e)+1
			else:
				self.__tgt_pred_types[e] = base_type

		self.__update_pred_match_scores(src_effects, tgt_effects)

	def __add_predicate_match(self, sp, tp):
		if sp in self.__matched_preds:
			assert tp == self.__matched_preds[sp]
		else:
			assert tp not in self.__matched_preds.values()

			debug_print("matching predicates %s ==> %s" % (sp, tp))
			
			# update cache

			if options.USE_MOVE_EFFECTS and sp.get_type() == Sentence.MOVE:
				# if the predicates are does predicates, then update the predicate categories
				# by assigning move effects of source and target rules to the same category
				assert tp.get_type() == Sentence.MOVE
				self.__update_bins_with_move_effects(sp.get_name(), tp.get_name())

			for sp1, tp1 in self.__pred_match_scores:
				if sp == sp1 or tp == tp1:
					self.__pred_match_scores[(sp1,tp1)] = options.MISMATCHED_PRED_SCORE

			self.__pred_match_scores[(sp,tp)] = options.MATCHED_PRED_SCORE

			self.__matched_preds[sp] = tp

	def __update_pred_match_scores(self, src_update, tgt_update):
		for sp, tp in self.__pred_match_scores:
			if sp in src_update or tp in tgt_update:
				self.__pred_match_scores[(sp,tp)] = self.__calc_pred_match_score(sp, tp)

	def add_rule_match(self, sr, tr, body_map):
		assert sr not in self.__matched_rules
		assert tr not in self.__matched_rules.values()
		
		debug_print("=== matching rules ===")
		debug_print(sr)
		debug_print(tr)
		debug_print("--> predicate matches")

		self.__matched_rules[sr] = tr

		# delete all other match candidates involving these rules
		for rule_pair in self.__cand_rule_matches.keys():
			if rule_pair[0] == sr or rule_pair[1] == tr:
				del self.__cand_rule_matches[rule_pair]

		# add predicate maps
		# first add head predicates
		# note that if partial maps are allowed, head predicates might not
		# match, so if they don't, don't try to add the head predicate mapping
		if self.__head_match_score(sr, tr) > 0:
			spn = sr.get_head().get_predicate()
			tpn = tr.get_head().get_predicate()
			if spn != None and tpn != None:
				sp = self.__src_preds[spn]
				tp = self.__tgt_preds[tpn]
				self.__add_predicate_match(sp, tp)

		# body predicates
		for sp, tp in body_map.items():
			# if self.pred_match_score(sp, tp) >= 1:
			# why the hell was the previous line there?
			self.__add_predicate_match(sp, tp)

		debug_print("=== done matching rules ===")

		return self.__update_all_rule_matches()

	def get_best_rule_match(self):
		# mapping from rule type to (sr, tr, map, score)
		bests = {}
		for sr, tr in self.__cand_rule_matches:
			if (sr, tr) in self.__suppressed:
				continue

			type = sr.get_type()
			m, score = self.__rule_match_score(sr, tr)
			best = bests.setdefault(type, (None, None, None, 0))
			if score > best[-1]:
				bests[type] = (sr, tr, m, score)
		
		# once a rule match that's no in the suppressed set has been chosen,
		# the suppressed set is deleted
		self.__suppressed = set()

		for t in PartialMap.RULE_MATCH_ORDER:
			if t in bests:
				return bests[t]
		
		assert False, "Terrible!!!"
	
	def get_pred_matches(self):
		return self.__matched_preds

	def print_pred_matches(self, file):
		for sp, tp in self.__matched_preds.items():
			file.write('%s %s\n' % (sp.get_name(), tp.get_name()))

	def complete(self):
		return len(self.__cand_rule_matches) == 0

	def suppress_match(self, sr, tr):
		"Prevent a particular rule from being chosen for one cycle"
		self.__suppressed.add((sr,tr))
	
	def score(self):
		score = 0
		for sp, tp in self.__matched_preds.items():
			score += self.pred_match_score(sp, tp)

		return score

	def src_dof(self, r):
		"""Returns the number of rules r can possibly map to"""
		count = 0
		for x in self.__cand_rule_matches:
			if x[0] == r:
				count += 1
		return count

	def src_rdof(self, r):
		sr2dof = {}
		max = 0
		for sr, tr in self.__cand_rule_matches:
			if sr in sr2dof:
				sr2dof[sr] += 1
			else:
				sr2dof[sr] = 1
			v = sr2dof[sr]
			if v > max:
				max = v
		if r not in sr2dof:
			return 0
		else:
			return sr2dof[r] / float(max)

	def tgt_dof(self, r):
		"""Returns the number of rules r can possibly map to"""
		count = 0
		for x in self.__cand_rule_matches:
			if x[1] == r:
				count += 1
		return count

	def tgt_rdof(self, r):
		tr2dof = {}
		max = 0
		for sr, tr in self.__cand_rule_matches:
			if tr in tr2dof:
				tr2dof[tr] += 1
			else:
				tr2dof[tr] = 1
			v = tr2dof[tr]
			if v > max:
				max = v
		if r not in tr2dof:
			return 0
		else:
			return tr2dof[r] / float(max)
