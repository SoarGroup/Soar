#!/usr/bin/python

from ply import yacc
from gdllex import tokens
import pdb

keywords = ['role', 'init', 'true', 'next', 'legal', 'does', 'goal', 'terminal', 'not', 'or']
preserve = ['2d', 'location', 'greaterThan', 'lessThan', 'plus', 'minus', 'step', 'succ', 'move', 'block', 'int']

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

sym_map = SymMap()

def mod_string(s):
	global sym_map

	if s in preserve:
		return s

	if s in sym_map:
		return sym_map[s]
	
	s_old = s
	for vowel in 'aeiouAEIOU':
		s_new = s_old.replace(vowel, '')
		if len(s_new) == 0:
			sym_map[s] = s_old
			return
		else:
			s_old = s_new

	if len(s_new) == 0:
		sym_map[s] = s_old
	else:
		sym_map[s]= s_new
	
	return sym_map[s]

def p_rule_list(p):
	'''rule_list : rule rule_list
	             | empty'''
	pass

def p_rule_atomic_sentence(p):
	'''rule : atomic_sentence
	        | LPAREN ARROW atomic_sentence condition_list RPAREN'''

	if len(p) == 2:
		print p[1]
	else:
		print '(<= %s %s)' % (p[3], p[4])

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
	p[0] = '?%s' % mod_string(p[1][1:])

def p_term_str_constant(p):
	'term : NAME'
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
		p[0] = '%s %s' % (p[1], p[2])
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

if len(sys.argv) < 2:
	print "need kif file"
	sys.exit(1)

file = open(sys.argv[1], 'r').read()

yacc.parse(file)
