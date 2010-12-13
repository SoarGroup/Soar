# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Base Source Parser

    @copyright: 2002 by Taesu Pyo <bigflood@hitel.net>,
                2005 by Oliver Graf <ograf@bitart.de>,
                2005-2008 MoinMoin:ThomasWaldmann

    @license: GNU GPL, see COPYING for details.


basic css:

pre.codearea     { font-style: sans-serif; color: #000000; }

pre.codearea span.ID       { color: #000000; }
pre.codearea span.Char     { color: #004080; }
pre.codearea span.Comment  { color: #808080; }
pre.codearea span.Number   { color: #008080; font-weight: bold; }
pre.codearea span.String   { color: #004080; }
pre.codearea span.SPChar   { color: #0000C0; }
pre.codearea span.ResWord  { color: #4040ff; font-weight: bold; }
pre.codearea span.ConsWord { color: #008080; font-weight: bold; }

"""

import re

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin import config, wikiutil
from MoinMoin.support.python_compatibility import hash_new
from MoinMoin.parser import parse_start_step


class FormatTextBase:
    pass

class FormatBeginLine(FormatTextBase):
    def formatString(self, formatter, word):
        return formatter.code_line(1)

class FormatEndLine(FormatTextBase):
    def formatString(self, formatter, word):
        return formatter.code_line(0)

class FormatText(FormatTextBase):

    def __init__(self, fmt):
        self.fmt = fmt

    def formatString(self, formatter, word):
        return (formatter.code_token(1, self.fmt) +
                formatter.text(word) +
                formatter.code_token(0, self.fmt))

class FormatTextID(FormatTextBase):

    def __init__(self, fmt, icase=False):
        if not isinstance(fmt, FormatText):
            fmt = FormatText(fmt)
        self.setDefaultFormat(fmt)
        self._ignore_case = icase
        self.fmt = {}

    def setDefaultFormat(self, fmt):
        self._def_fmt = fmt

    def addFormat(self, word, fmt):
        if self._ignore_case:
            word = word.lower()
        self.fmt[word] = fmt

    def formatString(self, formatter, word):
        if self._ignore_case:
            sword = word.lower()
        else:
            sword = word
        return self.fmt.get(sword, self._def_fmt).formatString(formatter, word)


class FormattingRuleSingle:

    def __init__(self, name, str_re, icase=False):
        self.name = name
        self.str_re = str_re

    def getStartRe(self):
        return self.str_re

    def getText(self, parser, hit):
        return hit


class FormattingRulePair:

    def __init__(self, name, str_begin, str_end, icase=False):
        self.name = name
        self.str_begin = str_begin
        self.str_end = str_end
        re_flags = re.M
        if icase:
            re_flags |= re.I
        self.end_re = re.compile(str_end, re_flags)

    def getStartRe(self):
        return self.str_begin

    def getText(self, parser, hit):
        match = self.end_re.search(parser.text, parser.lastpos)
        if not match:
            next_lastpos = parser.text_len
        else:
            next_lastpos = match.end() + (match.end() == parser.lastpos)
        r = parser.text[parser.lastpos:next_lastpos]
        parser.lastpos = next_lastpos
        return hit + r


class ParserBase:
    """ DEPRECATED highlighting parser - please use/extend pygments library """
    logging.warning('Using ParserBase is deprecated - please use/extend pygments syntax highlighting library.')

    parsername = 'ParserBase'
    tabwidth = 4

    # for dirty tricks, see comment in format():
    STARTL, STARTL_RE = u"^\n", ur"\^\n"
    ENDL, ENDL_RE = u"\n$", ur"\n\$"
    LINESEP = ENDL + STARTL

    def __init__(self, raw, request, **kw):
        self.raw = raw
        self.request = request
        self.show_nums, self.num_start, self.num_step, attrs = parse_start_step(request, kw.get('format_args', ''))

        self._ignore_case = False
        self._formatting_rules = []
        self._formatting_rules_n2r = {}
        self._formatting_rule_index = 0
        self.rule_fmt = {}
        #self.line_count = len(raw.split('\n')) + 1

    def setupRules(self):
        self.addRuleFormat("BEGINLINE", FormatBeginLine())
        self.addRuleFormat("ENDLINE", FormatEndLine())
        # we need a little dirty trick here, see comment in format():
        self.addRule("BEGINLINE", self.STARTL_RE)
        self.addRule("ENDLINE", self.ENDL_RE)

        self.def_format = FormatText('Default')
        self.reserved_word_format = FormatText('ResWord')
        self.constant_word_format = FormatText('ConsWord')
        self.ID_format = FormatTextID('ID', self._ignore_case)
        self.addRuleFormat("ID", self.ID_format)
        self.addRuleFormat("Operator")
        self.addRuleFormat("Char")
        self.addRuleFormat("Comment")
        self.addRuleFormat("Number")
        self.addRuleFormat("String")
        self.addRuleFormat("SPChar")
        self.addRuleFormat("ResWord")
        self.addRuleFormat("ResWord2")
        self.addRuleFormat("ConsWord")
        self.addRuleFormat("Special")
        self.addRuleFormat("Preprc")
        self.addRuleFormat("Error")

    def _addRule(self, name, fmt):
        self._formatting_rule_index += 1
        name = "%s_%s" % (name, self._formatting_rule_index) # create unique name
        self._formatting_rules.append((name, fmt))
        self._formatting_rules_n2r[name] = fmt

    def addRule(self, name, str_re):
        self._addRule(name, FormattingRuleSingle(name, str_re, self._ignore_case))

    def addRulePair(self, name, start_re, end_re):
        self._addRule(name, FormattingRulePair(name, start_re, end_re, self._ignore_case))

    def addWords(self, words, fmt):
        if not isinstance(fmt, FormatTextBase):
            fmt = FormatText(fmt)
        for w in words:
            self.ID_format.addFormat(w, fmt)

    def addReserved(self, words):
        self.addWords(words, self.reserved_word_format)

    def addConstant(self, words):
        self.addWords(words, self.constant_word_format)

    def addRuleFormat(self, name, fmt=None):
        if fmt is None:
            fmt = FormatText(name)
        self.rule_fmt[name] = fmt

    def format(self, formatter, form=None):
        """ Send the text.
        """

        self.setupRules()

        formatting_regexes = ["(?P<%s>%s)" % (n, f.getStartRe())
                              for n, f in self._formatting_rules]
        re_flags = re.M
        if self._ignore_case:
            re_flags |= re.I
        scan_re = re.compile("|".join(formatting_regexes), re_flags)

        self.text = self.raw

        # dirty little trick to work around re lib's limitations (it can't have
        # zero length matches at line beginning for ^ and at the same time match
        # something else at the beginning of the line):
        self.text = self.LINESEP.join([line.replace('\r', '') for line in self.text.splitlines()])
        self.text = self.STARTL + self.text + self.ENDL
        self.text_len = len(self.text)

        result = [] # collects output

        self._code_id = hash_new('sha1', self.raw.encode(config.charset)).hexdigest()
        result.append(formatter.code_area(1, self._code_id, self.parsername, self.show_nums, self.num_start, self.num_step))

        self.lastpos = 0
        match = scan_re.search(self.text)
        while match and self.lastpos < self.text_len:
            # add the rendering of the text left of the match we found
            text = self.text[self.lastpos:match.start()]
            if text:
                result.extend(self.format_normal_text(formatter, text))
            self.lastpos = match.end() + (match.end() == self.lastpos)

            # add the rendering of the match we found
            result.extend(self.format_match(formatter, match))

            # search for the next one
            match = scan_re.search(self.text, self.lastpos)

        # add the rendering of the text right of the last match we found
        text = self.text[self.lastpos:]
        if text:
            result.extend(self.format_normal_text(formatter, text))

        result.append(formatter.code_area(0, self._code_id))
        self.request.write(''.join(result))

    def format_normal_text(self, formatter, text):
        return [formatter.text(text.expandtabs(self.tabwidth))]

    def format_match(self, formatter, match):
        result = []
        for n, hit in match.groupdict().items():
            if hit is None:
                continue
            r = self._formatting_rules_n2r[n]
            s = r.getText(self, hit)
            c = self.rule_fmt.get(r.name, None)
            if not c:
                c = self.def_format
            if s:
                lines = s.expandtabs(self.tabwidth).split(self.LINESEP)
                for line in lines[:-1]:
                    result.append(c.formatString(formatter, line))
                    result.append(FormatEndLine().formatString(formatter, ''))
                    result.append(FormatBeginLine().formatString(formatter, ''))
                result.append(c.formatString(formatter, lines[-1]))
        return result

