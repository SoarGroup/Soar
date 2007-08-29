import psyco
from ply import lex
from ply import yacc
import gdllex
import gdlyacc
psyco.cannotcompile(lex)
psyco.cannotcompile(yacc)
psyco.cannotcompile(gdllex)
psyco.cannotcompile(gdlyacc)
psyco.full()
