import types
import psyco
from ply import lex
from ply import yacc
import gdllex
import gdlyacc

def prevent_compile(obj):
	for a in dir(obj):
		attr = getattr(obj,a)
		if type(attr) == types.FunctionType:
			psyco.cannotcompile(attr)

prevent_compile(lex)
prevent_compile(yacc)
prevent_compile(gdllex)
prevent_compile(gdlyacc)

psyco.full()
