# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - highlighting parser using the Pygments highlighting library

    @copyright: 2008 Radomir Dopieralski <moindev@sheep.art.pl>
    @license: GNU GPL, see COPYING for details.
"""

import re

import pygments
import pygments.util
import pygments.lexers
import pygments.formatter
from pygments.token import Token

from MoinMoin import config, wikiutil
from MoinMoin.parser import parse_start_step
from MoinMoin.support.python_compatibility import hash_new
from MoinMoin.Page import Page

Dependencies = ['user'] # the "Toggle line numbers link" depends on user's language


def extensions_from_lexer_filenames(filenames):
    # pygment's lexer.filenames is like ['*.py', 'Python'], but we only want
    # the filename extensions list (like ['.py', ]):
    return [filename[1:] for filename in filenames if filename.startswith('*.')]

def extensions_for_all_lexers():
    """
    get supported filename extensions for all pygments lexers
    """
    extensions = []
    for name, short, patterns, mime in pygments.lexers.get_all_lexers():
        extensions.extend(extensions_from_lexer_filenames(patterns))
    return extensions


class PygmentsFormatter(pygments.formatter.Formatter):
    """ a formatter for Pygments that uses the moin formatter for creating output """
    line_re = re.compile(r'(\n)')

    def __init__(self, formatter, **kw):
        pygments.formatter.Formatter.__init__(self)
        self.result = []
        self.formatter = formatter
        self.start_line = kw.get('start_line', 0)

    def get_class(self, ttype):
        if ttype in Token.Text:
            return None
        # Reuse existing MoinMoin css class names
        elif ttype in Token.Operator.Word:
            return 'ResWord'
        elif ttype in Token.Comment.Preproc:
            return 'Preprc'
        elif ttype in Token.Keyword.Constant:
            return 'ConsWord'
        elif ttype in Token.Keyword:
            return 'ResWord'
        elif ttype in Token.Name.Builtin:
            return 'ResWord'
        elif ttype in Token.Name.Constant:
            return 'ConsWord'
        elif ttype in Token.String.Char:
            return 'Char'
        elif ttype in Token.String.Escape:
            return 'SPChar'
        elif ttype in Token.String:
            return 'String'
        elif ttype in Token.Number:
            return 'Number'
        elif ttype in Token.Name:
            return 'ID'
        elif ttype in Token.Comment:
            return 'Comment'
        elif ttype in Token.Generic.Heading:
            return 'Comment'
        elif ttype in Token.Generic.Subheading:
            return 'DiffSeparator'
        elif ttype in Token.Generic.Inserted:
            return 'DiffAdded'
        elif ttype in Token.Generic.Deleted:
            return 'DiffRemoved'
        elif ttype in Token.Generic.Strong:
            return 'DiffChanged'
        elif ttype in Token.Generic.Output:
            return 'LineNumber'
        elif ttype in Token.Generic.Prompt:
            return 'ID'
        else:
            # skip tags that have no class defined
            return None
            # ... or use the token's name when nothing apropriate
            # return str(ttype).replace(".", " ")

    def add_next_line(self, line_parts):
        fmt = self.formatter
        self.lineno += 1
        self.result.append(fmt.code_line(1))
        self.result.append(fmt.line_anchordef(self.lineno))
        self.result += line_parts
        self.result.append(fmt.code_line(0))

    def format(self, tokensource, outfile):
        fmt = self.formatter
        self.lineno = self.start_line
        line_parts = []
        for ttype, value in tokensource:
            class_ = self.get_class(ttype)
            if value:
                for line in self.line_re.split(value):
                    if line == '\n':
                        self.add_next_line(line_parts)
                        line_parts = []
                        continue
                    if class_:
                        line_parts.append(fmt.code_token(1, class_))
                    line_parts.append(fmt.text(line))
                    if class_:
                        line_parts.append(fmt.code_token(0, class_))
        if line_parts and line_parts != [u'']: # Don't output an empty line at the end.
            self.add_next_line(line_parts)


class Parser:
    parsername = "highlight"  # compatibility wrappers override this with the pygments lexer name
    Dependencies = Dependencies
    extensions = extensions_for_all_lexers()

    def __init__(self, raw, request, filename=None, format_args='', **kw):
        self.request = request
        self.raw = raw
        self.filename = filename
        self.start_line = kw.get('start_line', 0)

        if self.parsername == 'highlight':
            # user is directly using the highlight parser
            parts = format_args.split(None)
            if parts:
                self.syntax = parts[0]
            else:
                self.syntax = 'text'
            if len(parts) > 1:
                params = ' '.join(parts[1:])
            else:
                params = ''
        else:
            # a compatibility wrapper inherited from this class
            self.syntax = self.parsername
            params = format_args
        self.show_nums, self.num_start, self.num_step, attrs = parse_start_step(request, params)

    def format(self, formatter):
        _ = self.request.getText
        fmt = PygmentsFormatter(formatter, start_line=self.start_line)

        # adding line number anchors for process instruction lines
        for lineno in range(1, self.num_start + 1):
            fmt.result.append(formatter.line_anchordef(lineno))

        fmt.result.append(formatter.div(1, css_class="highlight %s" % self.syntax))
        self._code_id = hash_new('sha1', self.raw.encode(config.charset)).hexdigest()
        msg = None
        if self.filename is not None:
            try:
                lexer = pygments.lexers.get_lexer_for_filename(self.filename)
            except pygments.util.ClassNotFound:
                fmt.result.append(formatter.text(self.filename))
                lexer = pygments.lexers.TextLexer()
        else:
            try:
                lexer = pygments.lexers.get_lexer_by_name(self.syntax)
            except pygments.util.ClassNotFound:
                f = self.request.formatter
                url = ''.join([
                               f.url(1, href=Page(self.request, _("HelpOnParsers")).url(self.request, escape=0)),
                               _("HelpOnParsers"),
                               f.url(0)])
                msg = _("Syntax highlighting not supported for '%(syntax)s', see %(highlight_help_page)s.") % {"syntax": wikiutil.escape(self.syntax),
                                                                                                               "highlight_help_page": url
                                                                                                              }
                lexer = pygments.lexers.TextLexer()

        fmt.result.append(formatter.code_area(1, self._code_id, self.parsername, self.show_nums, self.num_start, self.num_step, msg))
        pygments.highlight(self.raw, lexer, fmt)
        fmt.result.append(formatter.code_area(0, self._code_id))
        fmt.result.append(formatter.div(0))
        self.request.write("".join(fmt.result))

