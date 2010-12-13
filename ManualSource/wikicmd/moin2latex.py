#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import sys
sys.path.append('.')
from MoinMoin.formatter import FormatterBase
from MoinMoin.parser.text_moin_wiki import Parser

# LaTeX Formatter
class Formatter(FormatterBase):
	"""
    MoinMoin - "text/latex" Formatter

    Copyright 2005 Johannes Berg <johannes@sipsolutions.net>
    Copyright (c) 2003 by João Neves <moin@silvaneves.org>
    Copyright (c) 2000, 2001, 2002 by Jürgen Hermann <jh@web.de>

    All rights reserved, see COPYING for details.
	"""
	
	hardspace = ' '

	def __init__(self, request, **kw):
		apply(FormatterBase.__init__, (self, request), kw)
		self.verbatim = False
		self.itemized = False

	def text2latex(self, text):
		"Escape special characters if not in verbatim mode"
		if self.verbatim: return text
		text = text.replace('\\', '$\\backslash$ ');
		text = text.replace('$', r'\$');
		text = text.replace(r'\$\backslash\$', r'$\backslash$')
		text = text.replace('#', r'\#');
		text = text.replace('%', r'\%');
		text = text.replace('^', r'\^{}');
		text = text.replace('&', r'\&');
		text = text.replace('_', r'\_');
		text = text.replace('{', r'\{');
		text = text.replace('}', r'\}');
		text = text.replace('~', r'\~{}');
		text = text.replace('"', r'\"{}');
		text = text.replace(u'ä', r'"a');
		text = text.replace(u'ü', r'"u');
		text = text.replace(u'ö', r'"o');
		text = text.replace(u'Ä', r'"A');
		text = text.replace(u'Ü', r'"U');
		text = text.replace(u'Ö', r'"O');
		text = text.replace(u'ß', r'\ss{}');
		return text

	def write_text(self, text):
		if self.item is None:
			return text
		else:
			self.item = (self.item[0], self.item[1]+text)
			return ''

	def sysmsg(self, text, **kw):
		return self.write_text('')

	def pagelink(self, on, pagename, text=None, **kw):
		#import pdb; pdb.set_trace()
		assert pagename == 'CommandLineInterface', pagename
		if on:
			return self.write_text('\\hyperref[%s]{' % kw['anchor'])
		else:
			return self.write_text('}')

	def url(self, on, url=None, css=None, **kw):
		return ''

	def text(self, text):
		return self.write_text(self.text2latex(text))

	def rule(self, size=0):
		size = min(size, 10)
		ch = "---~=*+#####"[size]
		return self.write_text('\\vrule \n')

	def strong(self, on):
		return self.write_text(['{\\bf ', '}'][not on])

	def emphasis(self, on):
		return self.write_text(['{\\em ', '}'][not on])

	def highlight(self, on):
		return self.write_text(['{\\tt ', '}'][not on])

	def number_list(self, on, type=None, start=None):
		self.itemized = on
		if on:
			text = "\\begin{enumerate}"
		else:
			text = '\\end{enumerate}\n'
		return self.write_text(text)

	def bullet_list(self, on):
		self.itemized = on
		return self.write_text(['\\begin{itemize}\n', '\n\\end{itemize}\n'][not on])

	def listitem(self, on, **kw):
		if not self.itemized: return ''
		self._in_li = on != 0
		if on:
			return self.write_text('\\item ')
		else:
			return ''

	def sup(self, on):
		return self.write_text(['\\textsuperscript{', '}'][not on])

	def sub(self, on):
		return self.write_text(['\\textsubscript{', '}'][not on])

	def code(self, on, **kw):
		return self.write_text(['{\\tt ', '}'][not on])

	def code_area(self, on, code_id, code_type='code', show=0, start=-1, step=-1):
		res = self.preformatted(on)
		self.verbatim = False
		return self.write_text(res)

	def code_token(self, on, tok_type):
		return self.write_text('')

	def code_line(self, on):
		return self.write_text('\n')

	def preformatted(self, on):
		FormatterBase.preformatted(self, on)
		self.verbatim = on
		return self.write_text(['\\begin{verbatim}\n', '\\end{verbatim}\n'][not on])

	def smiley(self, text):
		return self.write_text(self.text2latex(text))

	def paragraph(self, on, **kw):
		FormatterBase.paragraph(self, on)
		return self.write_text(['', '\n\n'][not on])

	def linebreak(self, preformatted=1):
		if preformatted==1:
			return self.write_text('\n')
		else:
			return self.write_text('\\newline')

	def heading(self, on, depth, **kw):
		if on:
			if depth <= 2:
				t = '\\%ssection*{' % ('sub' * depth)
			elif depth <= 4:
				t = '\\%sparagraph{' % ('sub' * (depth - 3))
			else:
				t = ''
		else:
			if depth > 4:
				t = ''
			elif depth > 1:
				t = '}\n'
			else:
				n = kw['id']
				t = '}\n\\label{%s}\n\\index{%s}\n' % (n, n)
		return self.write_text(t)

	rows = []
	row = []
	item = None

	def table(self, on, attrs={}):
		def count_cols(row):
			cols = 0
			for cell in row:
				if cell[0].has_key('colspan'):
					cols += int(cell[0]['colspan'][1:-1])
				else:
					cols += 1
			return cols

		if on:
			self.rows = []
			self.item = None
			self.row = []
			return ''
		
		# not on:
		if self.rows == []: return ''
		cols = count_cols(self.rows[0])
		rows = len(self.rows)
		_table = [[0 for i in xrange(0,cols)] for j in xrange(0,rows)]
		_rownum = -1
		for _row in self.rows:
			_rownum += 1
			_cellnum = -1
			for _cell in _row:
				_cellnum += 1

				while _table[_rownum][_cellnum] is None or type(_table[_rownum][_cellnum]) == type(()):
					_cellnum += 1

				if _cell[0].get('rowspan') == '"1"':
					del _cell[0]['rowspan']
				if _cell[0].get('colspan') == '"1"':
					del _cell[0]['colspan']

				_rowspan = int(_cell[0].get('rowspan', '"1"')[1:-1])
				_colspan = int(_cell[0].get('colspan', '"1"')[1:-1])

				for j in xrange(0,_rowspan):
					for i in xrange(0,_colspan):
						_table[_rownum+j][_cellnum+i] = None
					_table[_rownum+j][_cellnum] = ({'colspan':'"%d"'%_colspan},None)
				_table[_rownum][_cellnum] = _cell


		table = '\n\\begin{flushleft}\n\\begin{tabularx}{\\textwidth}{|l|%s}\n' % ((cols-1) * 'X|')
		for _row in _table:
			row = ''
			cellnum = 0
			_lines = []
			_do_line = True
			for _cell in _row:
				cellnum+=1
				if _cell == 0:
					return 'INVALID TABLE'
				if _cell is None:
					if _do_line:
						_lines += [cellnum]
					continue
				_rowspan = int(_cell[0].get('rowspan', '"1"')[1:-1])
				_colspan = int(_cell[0].get('colspan', '"1"')[1:-1])
				format = '%s'
				if not (_cell[1] is None):
					_do_line = True
					_lines += [cellnum]
				else:
					_do_line = False
					_cell = (_cell[0], u'')
				if _rowspan > 1:
					format = r'\multirow{%d}*{%%s}' % _rowspan
				if _colspan > 1:
					format = r'\multicolumn{%d}{|l|}{ %s }' % (_colspan, format)
				row += (format+' & ') % _cell[1].replace('\n',' ')
			for l in _lines:
				table += r'\cline{%d-%d}' % (l,l)
			table += row[0:-3] + '\\\\ \n'
		table += '\\hline\n\\end{tabularx}\n\end{flushleft}\n'
		return table


	def table_row(self, on, attrs={}):
		if not on:
			self.rows += [self.row]
			self.row = []
		return ''

	def table_cell(self, on, attrs={}):
		if not on:
			self.row += [self.item]
			self.item = None
		else:
			self.item = (attrs,'')
		return ''

	def underline(self, on):
		return self.write_text(['\\underline{', '}'][not on])

	def definition_list(self, on):
		return self.write_text(['\\begin{description}\n', '\\end{description}\n'][not on])

	def definition_term(self, on, compact=0):
		return self.write_text(['\\item[', '] '][not on])

	def definition_desc(self, on):
		return self.write_text('')

	def attachment_image(self, fname):
		return self.image(src=fname)
	def image(self, **kw):
		# I am using alt for caption, but how to integrate the image?
		text = ''
		imgname = kw['src'].split('=')[-1]
		nid = self.next_img_data
		self.next_img_data = ''
		return '\\includegraphics%s{%s}' % (nid, imgname)
		#if kw.has_key('alt'):
		#    text += '\\begin{picture}\n'
		#    text += '\\caption{%s}\n' % kw[alt]
		#    text += '\\end{picture}\n'
		return self.write_text(text)

	def johill_sidecall_emit_latex(self, code):
		# nothing else for now
		return code
	
	next_img_data = ''
	def johill_sidecall_next_image_data(self, data):
		self.next_img_data = '['+data+']'
	
	def open(self, on, **kw):
		return ""
	def close(self, on, **kw):
		return ""

class Cfg:
	_site_plugin_lists = {}
	_plugin_modules = []
	bang_meta = False
	
class Request:
	getText = None
	form = None
	cfg = Cfg()
	pragma = {}
	
	def write(self, text):
		print text,

class Page:
	hilite_re = None
	page_name = 'arst'
		
req = Request()
p = Parser(open(sys.argv[1]).read(), req)
f = Formatter(req)
f.page = Page()

p.format(f)
