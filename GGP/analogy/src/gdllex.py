from ply import lex

tokens = (
		'VAR',
		'NAME',
		'NUM',
#		'RELATION',
		'ARROW',
		'OR',
		'NOT',
#		'DISTINCT',
		'RPAREN',
		'LPAREN',
		'COMMENT',
		)

t_VAR      = r'\?[a-zA-Z_][-\w_]*'
t_ARROW    = r'<='
t_LPAREN   = r'\('
t_RPAREN   = r'\)'

t_ignore = ' \t'

def t_NAME(t):
	r'[-\w_]+'
	try:
		t.value = int(t.value)
		t.type = 'NUM'
		return t
	except ValueError:
		# not a number
		if t.value.lower() == 'or':
			t.type = 'OR'
		elif t.value.lower() == 'not':
			t.type = 'NOT'
#		elif t.value.lower() == 'distinct':
#			t.type = 'DISTINCT'
#		elif t.value in relations:
#			t.type = 'RELATION'
		else:
			t.type = 'NAME'
		
	return t

def t_COMMENT(t):
	r';.*'
	pass

def t_newline(t):
	r'\n+'
	t.lexer.lineno += t.value.count('\n')

def t_error(t):
	print "Illegal character '%s'" % t.value[0]
	t.skip(1)

lex.lex()

gdl = open('parse_test.kif').read()

lex.input(gdl)

#while 1:
#	tok = lex.token()
#	if not tok:
#		break
#	print tok
