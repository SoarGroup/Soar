#!/usr/bin/python

from UniqueNameGen import UniqueNameGenerator
from SoarProd import *
from ElementGGP import ElementGGP as ElemGGP
from GGPRule import GGPRule
from GGPSentence import GGPSentence
import re
import ElementGGP
import pdb
from Comparison import Comparison

name_gen = UniqueNameGenerator()

rule_index = 0

def SentenceIsComp(sentence):
	Comparison_Ops = ['distinct', '<', '>', '>=']
	return sentence.name() in Comparison_Ops

def same_signature(sent1, sent2):
	if sent1.name() != sent2.name():
		return False

	if sent1.num_terms() != sent2.num_terms():
		return False

	for i in range(sent1.num_terms()):
		if (sent1.term(i).type() == 'function') ^ (sent2.term(i).type() == 'function'):
			return False
		if sent1.term(i).type() == 'function':
			if not same_signature(sent1.term(i), sent2.term(i)):
				return False
	
	return True

def SoarifyStr(s):
	p = re.compile('(\s|\?|\(|\)|\.)')
	result = p.sub('_', str(s))
	if len(result.strip()) == 0:
		return 'a'
		
	return result.strip()

class GDLSoarVarMapper:
	def __init__(self, var_gen):
		self.__var_gen = var_gen
		self.__var_map = dict()
	
	def copy(self):
		c = GDLSoarVarMapper(self.__var_gen.copy())
		c.__var_map = dict(self.__var_map.items())
		return c

	def get_var(self, gdl_var):
		if gdl_var[0] == '?':
			v = gdl_var[1:]
		else:
			v = gdl_var

		if self.__var_map.has_key(v):
			return self.__var_map[v]
		else:
			# try to name gdl variables and soar variables similarly
			soar_var = self.__var_gen.get_name(v)
			self.__var_map[v] = soar_var
			return soar_var
	
	def __contains__(self, v):
		if v[0] == '?':
			return v[1:] in self.__var_map
		else:
			return v in self.__var_map

# makes a typical skeleton production for use in GGP
def MakeTemplateProduction(name, type = "", game_name = 'game'):
	if type != "":
		prod_name = name_gen.get_name("%s*%s" % (type, name))
	else:
		prod_name = name_gen.get_name(name)
	p = SoarProd(prod_name, game_name)
	return p

def MakeApplyRule(sp):
	op_names = sp.get_proposed_op_names()

	assert len(op_names) > 0, "Operator doesn't propose any operators"
	assert len(op_names) == 1, "Proposing more than one operator with one rule"

	prod_name = sp.get_name().replace('propose', 'apply')
	ap = SoarProd(prod_name, sp.get_state_name())
	ap.add_operator_test(op_names[0])
	return ap
	
def HandleComparison(sentence, sp, var_map):
	if sentence.term(0).type() == 'variable':
		lhs_index = 0
		rhs_index = 1
		reverse = False
	else:
		assert sentence.term(1).type() == 'variable', "Have to have at least one variable in a comparison"
		lhs_index = 1
		rhs_index = 0
		reverse = True

	if sentence.name() == "distinct":
		v1 = var_map.get_var(str(sentence.term(lhs_index)))
		if sentence.term(rhs_index).type() == "variable":
			v2 = var_map.get_var(str(sentence.term(rhs_index)))
			sp.add_id_predicate(v1, "<>", v2)
		else:
			sp.add_predicate(v1, "<>", str(sentence.term(rhs_index)))
	elif sentence.name() in ['<', '>', '>=']:
		rev_map = {'<':'>', '>':'<', '>=':'<=', '<=':'>='}
		if reverse:
			comp = rev_map[sentence.name()]
		else:
			comp = sentence.name()
		v1 = var_map.get_var(str(sentence.term(lhs_index)))
		if sentence.term(rhs_index).type() == "variable":
			v2 = var_map.get_var(str(sentence.term(rhs_index)))
			sp.add_id_predicate(v1, comp, v2)
		else:
			sp.add_predicate(v1, comp, str(sentence.term(rhs_index)))

# creates conditions on the production to match results of math ops
# returns the id that will be bound to the result
def GetMathRes(maths, prod, var_map, deps, i, rule_index, used=[]):
	op_trans = {'+':'plus', '-':'minus', '*':'times', '/':'divide'}
	op = op_trans[maths[i].name()]
	math_code = 'math-%d-%d' % (rule_index, i)
	math_id = prod.get_or_make_id_chain(['math'])[0]

	# check if the test for this math result already exists
	# if it does, then just return the id of the result
	existing = prod.get_ids(math_id, math_code)
	if len(existing) > 0:
		assert len(existing) == 1
		return prod.get_ids(existing[0], 'result')[0]

	res_id = prod.add_id_attrib_chain(math_id, [math_code, 'result'])

	# add conditions for the operands
	for j in [0,1]:
		if deps[i][j] >= 0 and deps[i][j] not in used:
			# this variable is the result of another calculation
			# add a check for that calculation result into the production
			id = GetMathRes(maths, prod, var_map, deps, deps[i][j], rule_index, used + [deps[i][j]])
			prod.add_bound_id_attrib(res_id, 'op%d' % j, id)
		else:
			if maths[i].term(j).type() == 'variable':
				# this variable should already be bound by some regular condition
				assert maths[i].term(j).name() in var_map
				prod.add_bound_id_attrib(res_id, 'op%d' % j, var_map.get_var(maths[i].term(j).name()))
			else:
				# constant
				prod.add_attrib(res_id, 'op%d' % j, str(maths[i].term(j)))
	
	# add the condition for the result
	return prod.add_id_attrib(res_id, 'result', var_map.get_var(maths[i].term(2).name()))

def HandleMath(maths, base_prod, var_map, rule_index, state_name):

	# build a map of result variables to the calculation index
	# that generated it
	result_vars = {} # var -> index
	for i, m in enumerate(maths):
		assert m.num_terms() == 3, 'Math condition only has %d terms' % m.num_terms()
		rvar = m.term(2)
		assert rvar.type() == 'variable', 'Math result not variable'
		assert rvar not in result_vars, 'Two math results share the same variable'
		result_vars[rvar] = i
	
	# resolve interdependencies among calculations
	deps = {} # index -> [index]
	for i, m in enumerate(maths):
		for t in [0,1]:
			if m.term(t).type() == 'variable' and m.term(t) in result_vars:
				deps.setdefault(i, []).append(result_vars[m.term(t)])
			else:
				deps.setdefault(i, []).append(-1)

	# create intermediate productions
	int_prods = []
	for i, m in enumerate(maths):
		math_code = 'math-%d-%d' % (rule_index, i)
		# make a production to create the <s> ^math.math-x-y structure
		mk_struct_prod = SoarProd(name_gen.get_name('apply*make-%s-struct' % math_code), state_name)
		mk_struct_prod.add_operator_test('init')
		math_id = mk_struct_prod.get_or_make_id_chain(['math'])[0]
		mk_struct_prod.add_create_id(math_id, math_code)
		int_prods.append(mk_struct_prod)
		
		# make the production for the actual operator
		prod_name = name_gen.get_name(base_prod.get_name() + SoarifyStr(str(m)))
		prod = base_prod.copy(prod_name)
		var_map1 = var_map.copy()
		# add conditions to bind the ids for the operands
		ops = []
		for j, d in enumerate(deps[i]):
			if d >= 0:
				# have to get this operand from an intermediate result
				ops.append((GetMathRes(maths, prod, var_map1, deps, d, rule_index), 1))
			else:
				if m.term(j).type() == 'variable':
					# get this from a regular condition
					assert m.term(j).name() in var_map1
					ops.append((var_map1.get_var(m.term(j).name()), 1))
				else:
					# constant
					ops.append((str(m.term(j)), 0))

		# make an action to put this op,op,result triplet on the state
		math_id = prod.get_or_make_id_chain_existing(['math', math_code])[0]
		query_id = prod.add_create_id(math_id, 'query')
		op_strs = []
		for j in [0,1]:
			if ops[j][1] == 1:
				# id
				prod.add_create_bound_id(query_id, 'op%d' % j, ops[j][0])
				op_strs.append('<%s>' % ops[j][0])
			else:
				# const
				prod.add_create_constant(query_id, 'op%d' % j, ops[j][0])
				op_strs.append(ops[j][0])
				
		prod.add_create_constant(query_id, 'result', '(%s %s %s)' % (m.name(), op_strs[0], op_strs[1]))
		int_prods.append(prod)
	
	# modify body of base production to test for necessary calculations
	used = []
	for v, i in result_vars.items():
		GetMathRes(maths, base_prod, var_map, deps, i, rule_index, used)
	
	return int_prods

def GetMathResultVar(sentence):
	if sentence.name() in ['+', '-', '*', '/']:
		return sentence.term(2).name()
	else:
		return sentence.term(1).name()

def ParseGDLBodyToCondition(body, base_prod, var_map, game_name):
	global rule_index

	math_ops = ['+','-','*','/']
	math_conds = []
	math_result_vars = []
	normal_conds = []
	depend_on_math = []

	for b in body:
		if b.name() in math_ops:
			math_conds.append(b)
			math_result_vars.append(GetMathResultVar(b))
		else:
			normal_conds.append(b)
	
	for b in normal_conds:
		dependent = False
		for v in b.get_variables():
			if v in math_result_vars:
				depend_on_math.append(b)
				dependent = True
				break

		if not dependent:
			if SentenceIsComp(b):
				HandleComparison(b, base_prod, var_map)
			else:
				b.make_soar_conditions(base_prod, var_map)
	
	math_prods = HandleMath(math_conds, base_prod, var_map, rule_index, game_name)
	rule_index += 1

	for b in depend_on_math:
		if SentenceIsComp(b):
			HandleComparison(b, base_prod, var_map)
		else:
			b.make_soar_conditions(base_prod, var_map)

	return math_prods

def MakeInitRule(game_name, role, init_conds, fact_rules, min_success_score):
	sp = MakeTemplateProduction("init-%s" % game_name, "propose", "");
	sp.add_attrib(sp.get_state_id(), "superstate", "nil")
	sp.add_neg_attrib(sp.get_state_id(), "name", "")
	op_id = sp.add_operator_prop("init-%s" % game_name, "+ >")

	asp = MakeApplyRule(sp)

	asp.add_create_constant(asp.get_state_id(), 'name', game_name)
	asp.add_create_constant(asp.get_state_id(), 'next-action', '0')

	asp.add_create_id(asp.get_state_id(), 'desired')

	gs_id = asp.add_create_id(asp.get_state_id(), 'gs')
	fact_id = asp.add_create_id(asp.get_state_id(), 'facts')
#	game_elabs_var = state_action.add_id_wme_action("elaborations")
	asp.add_create_constant(gs_id, 'action-counter', '0')

	if role == "":
		print "Warning: No role defined"
	else:
		asp.add_create_constant(gs_id, 'role', role)

	var_map = GDLSoarVarMapper(UniqueNameGenerator())
	for ic in init_conds:
		ic.head().make_soar_actions(asp, var_map)

	for f in fact_rules:
		f.head().make_soar_actions(asp, var_map)
	return [sp, asp]

# Process axioms of the form
#
# (<= (head)
#     (body1)
#     (body2)
#     ...
# )
def TranslateImplication(game_name, rule, min_success_score, make_remove_rule):
	head = rule.head()
	body = rule.body()
	
	if head.name() == "terminal":
		return TranslateTerminal(game_name, head, body)
	elif head.name() == "legal":
		return TranslateLegal(game_name, head, body)
	elif head.name() == "next":
		return TranslateNext(game_name, head, body, make_remove_rule)
	elif head.name() == "goal":
		return TranslateGoal(game_name, head, body, min_success_score)
	else:
		# this can only be some implied relation
		return TranslateImpliedRelation(game_name, head, body)
		

def TranslateImpliedRelation(game_name, head, body):
	sp = MakeTemplateProduction(SoarifyStr(str(head)), 'elaborate')
	var_map = GDLSoarVarMapper(sp.get_name_gen())
	extras = ParseGDLBodyToCondition(body, sp, var_map, game_name)
	head.make_soar_actions(sp, var_map)
	return [sp] + extras
	
def TranslateTerminal(game_name, head, body):
	sp = MakeTemplateProduction(SoarifyStr(str(head)), 'elaborate')
	var_map = GDLSoarVarMapper(sp.get_name_gen())
	extras = ParseGDLBodyToCondition(body, sp, var_map, game_name)

	# add test for analyze state operator to make sure we've fired all
	# state update rules
	sp.add_operator_test('analyze-state')
	
	# different actions depending on if we're in the selection space
	# or operating for real
	sp_sel = sp.copy(name_gen.get_name(sp.get_name()))
	sp_sel.add_id_attrib(sp_sel.get_state_id(), 'duplicate-of')
	sp_sel.add_create_id(sp_sel.get_state_id(), 'terminal')
	
	sp.add_neg_id_attrib(sp.get_state_id(), 'duplicate-of')
	sp.add_rhs_func_call('halt')
	
	return [sp, sp_sel] + extras

def TranslateLegal(game_name, head, body):
	move = head.term(1).name()
	
	sp = MakeTemplateProduction(SoarifyStr(str(head)), "propose", game_name)
	var_map = GDLSoarVarMapper(sp.get_name_gen())
	#for cond in body:
	#	if not SentenceIsComp(cond):
	#		cond.make_soar_conditions(sp, var_map)
	extras = ParseGDLBodyToCondition(body, sp, var_map, game_name)
	
	# have to also check that no moves have been made
	olink_id = sp.get_or_make_id_chain(['io','output-link'])[0]
	sp.add_neg_id_attrib(olink_id, '<cmd-name>')
	
	head.make_soar_actions(sp, var_map)
	#ParseComparisons(body, sp, var_map)
	
	# apply rule

	ap = MakeApplyRule(sp)
	op_id = ap.get_ids(ap.get_state_id(), 'operator')[0]
	ol_id = ap.get_or_make_id_chain(['io','output-link'])[0]

	ap_var_map = GDLSoarVarMapper(ap.get_name_gen())
	head.term(1).make_soar_cond_no_id(ap, op_id, ap_var_map, 1)

	#move_act = ap.add_action(ap.add_action(ol_var).add_id_wme_action(move))
	#head.term(1).make_soar_action_no_id(ap, move_act, ap_var_map)
	head.term(1).make_soar_action(ap, ol_id, 0, ap_var_map)
	
	return [sp, ap] + extras

def TranslateNext(game_name, head, body, make_remove_rule = True):
	ap = MakeTemplateProduction(SoarifyStr(str(head)), 'apply', game_name)
	var_map = GDLSoarVarMapper(ap.get_name_gen())
	extras = ParseGDLBodyToCondition(body, ap, var_map, game_name)
	ap.add_operator_test('update-state')
	head.make_soar_actions(ap, var_map)
	
	# there are no frame axioms, so make productions that get rid of this after one
	# step
	if make_remove_rule:
		rap = MakeTemplateProduction("remove*%s" % SoarifyStr(str(head)), 'apply', game_name)
		var_map = GDLSoarVarMapper(rap.get_name_gen())
		rap.add_operator_test('update-state')
		head.make_soar_actions(rap, var_map, remove = True)
		return [ap, rap] + extras
	else:
		return [ap] + extras


def TranslateGoal(game_name, head, body, score):
	sp = MakeTemplateProduction(SoarifyStr(str(head)), 'elaborate', game_name)
	var_map = GDLSoarVarMapper(sp.get_name_gen())
	sp.add_id_attrib(sp.get_state_id(), 'terminal')
	desired_id = sp.add_id_attrib(sp.get_state_id(), 'desired')
	
	extras = ParseGDLBodyToCondition(body, sp, var_map, game_name)
	
	if int(str(head.term(1))) >= score:
		sp.add_create_bound_id(sp.get_state_id(), "success-detected", desired_id)
	else:
		sp.add_create_bound_id(sp.get_state_id(), "failure-detected", desired_id)

	return [sp] + extras

def GenCombinations(bounds, index = 0):
	if index == len(bounds) - 1:
		result = []
		for x in range(bounds[index]):
			result.append([x])
		return result

	result = []
	ends = GenCombinations(bounds, index + 1)
	for x in range(bounds[index]):
		result.extend([ [x] + e for e in ends ])
	
	return result



# if a variable is in the replacement map, replace it with the appropriate variable
# otherwise append a prefix to the variable name
def PrepareBodyVars(sentence, replacement_map, prefix):
	for i in range(sentence.num_terms()):
		c = sentence.term(i)
		if c.type() == "variable":
			if c.name() in replacement_map:
				sentence.term(i).rename(replacement_map[c.name()])
			else:
				sentence.term(i).rename('%s%s' % (prefix, c.name()))
		elif c.type() == "function":
			PrepareBodyVars(c, replacement_map, prefix)

def ProcessFrameAxioms(axioms, game_name):
	prefix = 'std_soar_var'
	# standardize all variable names. This is so that if two head literals are the same, they will
	# match string-wise. Also, if any rule has a constant rather than a variable in a variable
	# place, replace it with the variable, and add an extra equality structure to it for later use

	preds_to_bodies = {} # preds -> [(body, comparisons)]
	preds_to_heads = {} # preds -> head

	for prefix_i, rule in enumerate(axioms):
		head = rule.head()
		pred = head.term(0).name()
		preds_to_heads[pred] = head
		head_analogue = head.true_analogue()
		body = rule.body()

		arity = head.term(0).num_terms()
		substitutions = [ '%s%d' % (prefix, i) for i in range(arity)]
		(const_subs, var_subs) = head.term(0).standardize_vars(substitutions)

		body_var_subs = dict([(v, '%s%d' % (prefix, i)) for i, v in var_subs.items()])
		# if a rule has a constant in the place of a variable, that is equivalent
		# to having a constraint that the variable equal the constant
		reg_conds = []
		comp_conds = []
		for i, const in const_subs.items():
			comp_conds.append(Comparison(substitutions[i], '=', const, False))

		for b in body:
			if not same_signature(head_analogue, b): # ignore frame rule
				PrepareBodyVars(b, body_var_subs, '__r%d__' % prefix_i)
#				if SentenceIsComp(b):
#					comp_conds.append(Comparison.make_from_GGP_sentence(b))
#				else:
#					reg_conds.append(b)
				# forget about handling comparisons separately
				reg_conds.append(b)

		preds_to_bodies.setdefault(pred, []).append((reg_conds, comp_conds))

	result = []
	# merge the frame axioms into production rules
	for pred, bodies in preds_to_bodies.items():
		result.extend(TranslateFrameAxioms(game_name, preds_to_heads[pred], bodies))
	
	return result

def TranslateFrameAxioms(game_name, head, bodies):
#	pdb.set_trace()
	productions = []

	# all regular conditions can be lumped together into -{ } conjunctions, but
	# each comparison condition must be separated out and treated separately since
	# they can't be negated correctly with neg. conjs. Have to take all combinations
	# of blocks of regular conditions and individual comparisons from each frame
	# axiom body
	
	new_bodies = []
	for b in bodies:
		# b[0] is a list of normal conditions. they should be treated as one block
		# b[1] is a list of comparisons, they should be separated
		if len(b[0]) > 0:
			new_bodies.append([b[0]] + b[1])
		else:
			new_bodies.append(b[1])
	
	combinations = GenCombinations([len(b) for b in new_bodies])
	if len(combinations) == 0:
		return []

	prod_name = SoarifyStr(str(head))
	frame_sentence = head.true_analogue()
	for combi, comb in enumerate(combinations):
		sp = MakeTemplateProduction("remove-frame-%s" % prod_name, "apply", game_name)
		var_mapper = GDLSoarVarMapper(sp.get_name_gen())

		# first, add the condition to check for presence of head relation, and the action
		# to remove the head relation
		frame_sentence.make_soar_conditions(sp, var_mapper)
		head.make_soar_actions(sp, var_mapper, remove = True)

		# combine the bodies of all frame axioms together
		for body_index, cond_index in enumerate(comb):
			b = new_bodies[body_index][cond_index]
			if isinstance(b, list):
				# this is a block of regular conditions
				sp.begin_negative_conjunction() # wrap all the added conditions into a negation
				extras = ParseGDLBodyToCondition(b, sp, var_mapper, game_name)
				sp.end_negative_conjunction()
			else:
				# this is a comparison
				b.complement()
				b.make_soar_condition(sp)
		
		sp.add_operator_test('update-state')
		productions.append(sp)
		productions.extend(extras)

	return productions

def TranslateDescription(game_name, description, filename):
	productions = []
	init_rules = []
	cond_elabs = set([]) 
	uncond_elabs = dict() # rel_name -> [rules]
	implications = []
	funcs_with_frame_rules = []
	role = ""

	best_score = -99999

	# first run through the set of rules, expand all "or" sentences
	new_description = []
	for axiom in description:
		if axiom.or_child() != None:
			new_description.extend(SplitOr(axiom))
		else:
			new_description.append(axiom)
	
	# second run through the set of rules, note the frame rules, function
	# constants with frame rules, parse out the "role" rules, "init" rules, and facts
	frame_rules = [] # [(head, body)]
	for r in new_description:
		rule = GGPRule(r)

		if rule.head().name() == "goal":
			# we want to take note of the best possible single score
			score_term = rule.head().term(1)
			if score_term.type() == "constant" and int(str(score_term)) > best_score:
				best_score = int(str(score_term))
		
		# now we want to filter out rules with frame axioms, "role" rules, "init" rules,
		# and facts, to be processed differently from normal rules
		if rule.head().name() == "next":
			frame_term = rule.head().true_analogue()
			# check to see if this is a frame rule
			# frame rules are rules of the type
			#
			# (<= (next <some term>)
			#     ...
			#     (true <some term>)
			#     ...)
			if frame_term in rule.body():
				# this is a frame rule, process it differently later
				frame_rules.append(rule)
				# we also remember that this function constant is supported by frame rules
				funcs_with_frame_rules.append(frame_term.term(0))
			else:
				# normal implications, to be processed in the next step
				implications.append(rule)
		
		elif rule.head().name() == "role":
			role = str(rule.head().term(0))
		elif rule.head().name() == "init":
			init_rules.append(rule)
		elif rule.head().name() not in GGPSentence.DEFINED_RELS:
			# these can be one of two possible things, conditional or unconditional
			# elaborations. I call unconditional elaborations facts, and they
			# should be put into the ^facts structure in the state to be shallow
			# copied during a look-ahead. On the other hand conditional elaborations
			# should be put on the ^elaborations structure. But if the same relation
			# appears in both conditional and unconditional elaborations, all
			# elaborations for that relation should be treated as conditional
			if rule.has_body():
				# conditional
				if rule.head().name() in uncond_elabs:
					# there were other unconditional elabs for this relation,
					# move them over to the conditional side
					implications.extend(uncond_elabs[rule.head().name()])
					uncond_elabs.pop(rule.head().name())
				cond_elabs.add(rule.head().name())
				implications.append(rule)
			else:
				# unconditional
				if rule.head().name() not in cond_elabs:
					if rule.head().name() not in uncond_elabs:
						uncond_elabs[rule.head().name()] = [rule]
					else:
						uncond_elabs[rule.head().name()].append(rule)
				else:
					implications.append(rule)
		else:
			# normal implications, to be processed in the next step
			implications.append(rule)

	# Since we can't really tell what the best possible score will be, rely on a
	# heuristic that says that we've succeeded if we obtain a score higher than or equal
	# to the single highest reward
	if best_score == -99999:
		#raise Exception("There are no goal rules")
		print "Warning: There are no goal rules"
	
	fact_rules = []
	for r in uncond_elabs.values():
		fact_rules.extend(r)
		
	GGPSentence.fact_rels = set([ f.head().name() for f in fact_rules ])
	
	# here we process all normal rules
	
	for rule in implications:
		if rule.head().name() == "next":
			have_removal_rule = False
			for f in funcs_with_frame_rules:
				if f.covers(rule.head().term(0)):
					have_removal_rule = True
					break
				
			if have_removal_rule:
				productions.extend(TranslateImplication(game_name, rule, best_score, make_remove_rule = False))
			else:
				# if a "next" relation isn't supported by frame rules, we have to make
				# a rule that explicitly removes it every time it's been on the game state
				# for one step
				productions.extend(TranslateImplication(game_name, rule, best_score, make_remove_rule = True))
				funcs_with_frame_rules.append(rule.head().term(0))
		else:
			productions.extend(TranslateImplication(game_name, rule, best_score, make_remove_rule = False))

	# get the init rules
	productions.extend(MakeInitRule(game_name, role, init_rules, fact_rules, best_score))

	# process frame axioms
	productions.extend(ProcessFrameAxioms(frame_rules, game_name))

	# finally, write to file

	f = open(filename, 'w')
	f.write("source header.soar\n")

	for p in productions:
		f.write(str(p) + '\n')
	
def DeleteOrOperands(axiom, keep):
	if isinstance(axiom[0], str) and axiom[0].lower() == "or":
		return axiom[keep]

	for i in range(len(axiom)):
		c = axiom[i]
		if isinstance(c, ElemGGP):
			ret = DeleteOrOperands(c, keep)
			if ret != None:
				axiom[i] = ret
				return axiom

	return None

def SplitOr(axiom):
	result = []
	num_splits = len(axiom.or_child()) - 1
	for i in range(num_splits):
		a = axiom.deep_copy()
		result.append(DeleteOrOperands(a, i + 1))

	return result
