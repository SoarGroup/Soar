#!/usr/bin/python

from ply import yacc
from gdllex import tokens
import random
import pdb

keywords = ['role', 'init', 'true', 'next', 'legal', 'does', 'goal', 'terminal', 'not', 'or', 'distinct', '<', '>', '>=', '+', '-', '*', '/']

preserve = ['2d', 'location', 'greaterThan', 'lessThan', 'plus', 'minus', 'step', 'succ', 'move', 'blocked', 'int', 'north', 'south', 'east', 'west', 'exit']

preserve_constants = False
preserve_variables = False

alphabet = [chr(i) for i in range(ord('a'), ord('z')+1)]

class SymMap:
	def __init__(self):
		self.__map = {}
	
	def __getitem__(self, old):
		return self.__map[old]

	def __contains__(self, old):
		return old in self.__map

	def __setitem__(self, old, new):
		if old == new:
			temp = new + 'x'
		else:
			temp = new

		while temp in self.__map.values():
			temp += 'x'
		self.__map[old] = temp
	
	def has_collision(self, s):
		return s in self.__map.keys() or s in self.__map.values()

sym_map = SymMap()

def delete_vowels(s):
	global sym_map

	sl = s.lower()

	if sl in preserve:
		return sl

	if sl in sym_map:
		return sym_map[sl]
	
	s_old = sl
	for vowel in 'aeiouAEIOU':
		s_new = s_old.replace(vowel, '')
		if len(s_new) == 0:
			sym_map[sl] = s_old
			return sym_map[sl]
		else:
			s_old = s_new

	if len(s_new) == 0:
		sym_map[sl] = s_old
	else:
		sym_map[sl] = s_new
	
	return sym_map[sl]

def prefix(p, s):
	global sym_map

	sl = s.lower()

	if sl in preserve:
		return sl

	if sl in sym_map:
		return sym_map[sl]
	
	sym_map[sl] = p + sl
	
	return sym_map[sl]

def rand_str(s):
	global sym_map

	sl = s.lower()

	if sl in preserve:
		return sl

	if sl in sym_map:
		return sym_map[sl]

	assert not sym_map.has_collision(sl), "Oops, generated a string previously that matches this one, please rerun"

	s_new = ''
	while s_new == '' or sym_map.has_collision(s_new):
		length = random.randint(4, 8)
		s_new = ''
		for i in range(length):
			s_new += random.choice(alphabet)
	
	sym_map[sl] = s_new
	return s_new

def uppercase(s):
	return s.upper()

def p_rule_list(p):
	'''rule_list : rule rule_list
	             | empty'''
	pass

def p_rule_atomic_sentence(p):
	'''rule : atomic_sentence
	        | LPAREN ARROW atomic_sentence condition_list RPAREN'''

	if len(p) == 2:
		rules.append(p[1])
	else:
		rules.append('(<= %s\n%s)' % (p[3], p[4]))

def p_atomic_sentence(p):
	'''atomic_sentence : NAME
		                 | LPAREN NAME term_list RPAREN'''
	if len(p) == 2:
		if p[1] not in keywords:
			p[0] = mod_string(p[1])
		else:
			p[0] = p[1]

	else:
		if p[2] not in keywords:
			p[0] = '(%s %s)' % (mod_string(p[2]), p[3])
		else:
			p[0] = '(%s %s)' % (p[2], p[3])

def p_term_list(p):
	'''term_list : empty 
	             | term term_list'''
	if len(p) == 3:
		p[0] = '%s %s' % (p[1], p[2])
	else:
		p[0] = ''

def p_term_variable(p):
	'term : VAR'
	if preserve_variables:
		p[0] = p[1]
	else:
		p[0] = '?%s' % mod_string(p[1][1:])

def p_term_str_constant(p):
	'term : NAME'
	if preserve_constants:
		p[0] = p[1]
	else:
		p[0] = mod_string(p[1])

def p_term_num_constant(p):
	'term : NUM'
	p[0] = p[1]

def p_term_function(p):
	'term : function'
	p[0] = p[1]

def p_function(p):
	'function : LPAREN NAME term_list RPAREN'
	# a function is a predicate
	p[0] = '(%s %s)' % (mod_string(p[2]), p[3])

def p_condition_list(p):
	'''condition_list : empty 
	                  | condition condition_list'''
	if len(p) == 3:
		p[0] = '%s\n%s' % (p[1], p[2])
	else:
		p[0] = ""

def p_condition_literal(p):
	'condition : literal'
	p[0] = p[1]

#def p_condition_distinct(p):
#	'condition : distinct'

def p_condition_or(p):
	'condition : LPAREN OR condition_list RPAREN'
	p[0] = '(or %s)' % p[3]

def p_literal_atomic_sentence(p):
	'literal : atomic_sentence'
	p[0] = p[1]

def p_literal_not(p):
	'literal : LPAREN NOT atomic_sentence RPAREN'
	p[0] = '(not %s)' % p[3]

#def p_distinct_var(p):
#	'distinct : LPAREN DISTINCT VAR VAR RPAREN'
#
#def p_distinct_str(p):
#	'distinct : LPAREN DISTINCT VAR NAME RPAREN'
#
#def p_distinct_num(p):
#	'distinct : LPAREN DISTINCT VAR NUM RPAREN'

def p_error(t):
	print "error: %s at line %d" % (t.value, t.lexer.lineno)
	#print "Syntax error on line %d" % p.lineno(1)

def p_empty(p):
	'empty : '
	pass

yacc.yacc()

import sys

mod_string = delete_vowels

random.seed()

if len(sys.argv) < 2:
	print """
Usage: %s [switches] -f <kif file> [<kif file> <kif file> ...]

where the switches are: 
	-r          : replace symbols with random strings rather than deleting vowels
	-u          : replace symbols with upper case
	-p <prefix> : replace symbols with a prefixed version
	-c          : preserve constants
	-v          : preserve variable names
	-s <seed>   : set the random seed. If random seed is not set, the current time is used.
""" % (sys.argv[0])
	sys.exit(1)

for i in range(1,len(sys.argv)):
	if sys.argv[i] == '-r':
		mod_string = rand_str
	if sys.argv[i] == '-u':
		mod_string = uppercase
	if sys.argv[i] == '-c':
		preserve_constants = True
	if sys.argv[i] == '-v':
		preserve_variables = True
	if sys.argv[i] == '-s':
		random.seed(sys.argv[i+1])
	if sys.argv[i] == '-p':
		assert len(sys.argv) > i + 1, "Need to specify a prefix"
		prefix_str = sys.argv[i+1]
		mod_string = lambda x: prefix(prefix_str, x)
	if sys.argv[i] == '-f':
		fstart = i + 1
		break

for f in sys.argv[fstart:]:
	rules = []
	file = open(f, 'rU').read()
	yacc.parse(file)

	#random.shuffle(rules)
	outfile = open('%s.renamed' % f, 'w')
	for r in rules:
		print >> outfile, r

