import sys
from comb_perm import *
import predicate
from GDL import Sentence
import placetype
from move_effects import calc_move_effects
from find_max import find_max
import options
import pdb

def debug_print(s):
	print s
	pass

def possible_matchings(s1, s2):
	# we can't match two instances of the same predicate in one rule
	# to different target predicates
	unique_s1 = list(set(s1))
	unique_s2 = list(set(s2))
	if len(unique_s1) < len(unique_s2):
		return (dict(zip(unique_s1, subset)) for subset in xcombinations(unique_s2, len(unique_s1)))
	else:
		return (dict(zip(subset, unique_s2)) for subset in xcombinations(unique_s1, len(unique_s2)))

def cross_product(l1, l2):
	for a1 in l1:
		for a2 in l2:
			yield (a1, a2)

class PartialMap:

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

		# maintain a special structure for legal rules
		src_move_effects = calc_move_effects(src_int_rep)
		tgt_move_effects = calc_move_effects(tgt_int_rep)
		
		self.__src_move_effects = {}
		self.__tgt_move_effects = {}
		for move, effects in src_move_effects.items():
			pred_effects = [self.__src_preds[p] for p in effects]
			self.__src_move_effects[move] = pred_effects
		for move, effects in tgt_move_effects.items():
			pred_effects = [self.__tgt_preds[p] for p in effects]
			self.__tgt_move_effects[move] = pred_effects

		# map all predicates that are identical in source and target onto each other
		common_pred_names = set(self.__src_preds.keys()).intersection(set(self.__tgt_preds.keys()))
		common_pred_map = dict((self.__src_preds[i], self.__tgt_preds[i]) for i in common_pred_names)

		self.__matched_preds = common_pred_map
		self.__matched_rules = {}
		# candidate rule matchings
		# (src_rule, tgt_rule) -> (body map, score)
		self.__cand_rule_matches = {}
		# special extra map for legal rules
		# (src_rule, tgt_rule) -> (effect map, score)
		self.__cand_legal_matches = {}

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
				# don't include the head predicate in the body, because the head
				# will always matched to the head of the other rule
				srbody = [self.__src_preds[b.get_predicate()] for b in sr.get_body() \
						if b.get_predicate() != sr.get_head().get_predicate()]
				trbody = [self.__tgt_preds[b.get_predicate()] for b in tr.get_body() \
						if b.get_predicate() != tr.get_head().get_predicate()]
	
				# generators galore!
				maps = possible_matchings(srbody, trbody)
				maps_scores = ((bm, self.__body_map_score(sr, tr, bm)) for bm in maps)
				# if a body map score == 0, then that map is illegal
				self.__cand_rule_matches[(sr,tr)] = [map_score_pair for map_score_pair in maps_scores if map_score_pair[1] > 0]

				if i == 0:
					# for legal rules, make possible effect maps
					src_move = sr.get_head().get_predicate()
					tgt_move = tr.get_head().get_predicate()
					src_effects = self.__src_move_effects[src_move]
					tgt_effects = self.__tgt_move_effects[tgt_move]
					
					if len(src_effects) == 9 and len(tgt_effects) == 9:
						stop = True
					else:
						stop = False
					effect_maps = possible_matchings(src_effects, tgt_effects)
					maps_scores = ((m, self.__effect_map_score(m)) for m in effect_maps)
					self.__cand_legal_matches[(sr,tr)] = [map_score_pair for map_score_pair in maps_scores if map_score_pair[1] > 0]
		
		return self
	
	def __init__(self):
		pass

	def copy(self):
		c = PartialMap()
		c.__src_preds = self.__src_preds
		c.__tgt_preds = self.__tgt_preds
		c.__src_pred_types = self.__src_pred_types.copy()
		c.__tgt_pred_types = self.__tgt_pred_types.copy()
		c.__matched_preds = self.__matched_preds.copy()
		c.__matched_rules = self.__matched_rules.copy()
		c.__cand_rule_matches = self.__cand_rule_matches.copy()
		c.__src_move_effects = self.__src_move_effects.copy()
		c.__tgt_move_effects = self.__tgt_move_effects.copy()
		c.__cand_legal_matches = self.__cand_legal_matches.copy()
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
		# check to make sure the predicates don't already have conflicting mappings
		if sp in self.__matched_preds:
			if self.__matched_preds[sp] != tp:
				return 0
			else:
				return 1
		else:
			if tp in self.__matched_preds.values():
				return 0
		
		# ensure that predicate types are mappable
		# legal/does -/-> next/true, elabs
		st = sp.get_type()
		tt = tp.get_type()
		if st != tt and (st == Sentence.MOVE or tt = Sentence.MOVE):
			return 0
		
		# ensure that predicates are in the same category
		spt = self.__src_pred_types[sp]
		tpt = self.__tgt_pred_types[tp]
		# if any of the predicates are of type 0 (no category), then
		# the mapping is legal
		if spt != 0 and tpt != 0 and spt != tpt:
			return 0

		# if all above conditions are met, then make a rating based on argument types
		return self.__pred_arg_match_score(sp, tp) + 0.1

	def __body_map_score(self, sr, tr, m):
		"Warning: can potentially change mapping"

		total = 0
		for sp, tp in m.items():
			pm_score = self.pred_match_score(sp, tp)
			if pm_score == 0:
				if options.ALLOW_PARTIAL_BODY_MAPS:
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
			# if heads don't match, rules can't match
			return (None, 0)

		# find the best body mapping
		best_score = 0
		best_map = {}
		for body_map, score in self.__cand_rule_matches[(sr,tr)]:
			if score > best_score:
				best_map = body_map
				best_score = score

		best_score += options.HEAD_SCORE_FACTOR * head_match_score
		return (best_map, best_score)

	def __legal_rule_match_score(self, sr, tr):
		"Special calculations for legal moves"

		best, score = find_max(self.__cand_legal_matches[(sr,tr)], lambda x: x[1])
		return (self.__cand_legal_matches[(sr,tr)][best][0], score)

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

	def __update_effect_match(self, sr, tr):
		effect_maps = self.__cand_legal_matches[(sr,tr)]
		i = 0
		while i < len(effect_maps):
			new_score = self.__effect_map_score(effect_maps[i][0])
			if new_score == 0:
				del effect_maps[i]
			else:
				effect_maps[i] = (effect_maps[i][0], new_score)
				i += 1

	def __update_all_rule_matches(self):
		to_erase = []
		for sr, tr in self.__cand_rule_matches:
			# check if heads still match
			head_match_score = self.__head_match_score(sr, tr)
			if head_match_score == 0:
				to_erase.append((sr, tr))
				continue
			
			self.__update_rule_match(sr, tr)
			if sr.get_type() == Sentence.MOVE:
				# legal rules have to also have possible effect maps
				self.__update_effect_match(sr, tr)

			if len(self.__cand_rule_matches[(sr,tr)]) == 0:
				to_erase.append((sr,tr))
			elif sr.get_type() == Sentence.MOVE and len(self.__cand_legal_matches[(sr,tr)]) == 0:
				to_erase.append((sr,tr))

		for sr, tr in to_erase:
			del self.__cand_rule_matches[(sr,tr)]
			if sr.get_type() == Sentence.MOVE:
				del self.__cand_legal_matches[(sr,tr)]
		
		return len(to_erase)
	

	def __add_predicate_match(self, sp, tp):
		if sp in self.__matched_preds:
			assert tp == self.__matched_preds[sp]
		else:
			assert tp not in self.__matched_preds.values()
			debug_print("matching predicates %s ==> %s" % (sp, tp))
			self.__matched_preds[sp] = tp
	
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

		if sr.get_type() == Sentence.MOVE:
			for rule_pair in self.__cand_legal_matches.keys():
				if rule_pair[0] == sr or rule_pair[1] == tr:
					del self.__cand_legal_matches[rule_pair]

		# add predicate maps
		# first add head predicates
		spn = sr.get_head().get_predicate()
		tpn = tr.get_head().get_predicate()
		if spn != None and tpn != None:
			sp = self.__src_preds[spn]
			tp = self.__tgt_preds[tpn]
			self.__add_predicate_match(sp, tp)

		# body predicates
		for sp, tp in body_map.items():
			if self.pred_match_score(sp, tp) >= 1:
				self.__add_predicate_match(sp, tp)

		debug_print("=== done matching rules ===")

		return self.__update_all_rule_matches()

	def get_best_rule_match(self):
		best_score = -1
		best_map = None
		best_sr = None
		best_tr = None
		for sr, tr in self.__cand_rule_matches:
			m, score = self.__rule_match_score(sr, tr)
			if score > best_score:
				best_score = score
				best_map = m
				best_sr = sr
				best_tr = tr
		
		return (best_sr, best_tr, best_map, best_score)
	
	def add_best_legal_rule_match(self):
		best_score = -1
		best_effect_map = None
		best_sr = None
		best_tr = None
		for rule_pair, maps  in self.__cand_legal_matches.items():
			i, score = find_max(maps, lambda x: x[1])
			if score > best_score:
				best_score = score
				best_effect_map = maps[i][0]
				best_sr = rule_pair[0]
				best_tr = rule_pair[1]
		
		if best_sr is None:
			return (None, None, 0, 0)
		#pdb.set_trace()
		for se, te in best_effect_map.items():
			self.__add_predicate_match(se, te)
		
		self.__update_all_rule_matches()

		bmap, score = self.__rule_match_score(best_sr, best_tr)
		#assert score > 0
		invalidated = self.add_rule_match(best_sr, best_tr, bmap)
		return (best_sr, best_tr, invalidated, score)

	def get_pred_matches(self):
		return self.__matched_preds

	def print_pred_matches(self, file):
		for sp, tp in self.__matched_preds.items():
			file.write('%s %s\n' % (sp.get_name(), tp.get_name()))

	def complete(self):
		return len(self.__cand_rule_matches) == 0

	def suppress_match(self, sr, tr):
		del self.__cand_rule_matches[(sr, tr)]
	
	def score(self):
		score = 0
		for sp, tp in self.__matched_preds.items():
			score += self.pred_match_score(sp, tp)

		return score

	def src_dof(self, r):
		"""Returns the number of rules r can possibly map to"""
		return len([0 for x in self.__cand_rule_matches if x[0] == r])

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
		return len([0 for x in self.__cand_rule_matches if x[1] == r])

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
