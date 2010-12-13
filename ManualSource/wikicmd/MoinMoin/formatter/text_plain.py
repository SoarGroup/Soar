# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - "text/plain" Formatter

    @copyright: 2000-2002 Juergen Hermann <jh@web.de>
                2007 by Timo Sirainen <tss@iki.fi>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.formatter import FormatterBase


class Formatter(FormatterBase):
    """
        Send plain text data.
    """

    hardspace = u' '

    def __init__(self, request, **kw):
        FormatterBase.__init__(self, request, **kw)
        self._in_code_area = 0
        self._in_code_line = 0
        self._code_area_state = [0, -1, -1, 0]
        self._lists = []
        self._url = None
        self._text = None  # XXX does not work with links in headings!!!!!
        self._text_stack = []
        self._skip_text = False
        self._wrap_skip_text = False
        self._textbuf = ''
        self._indent = 0
        self._listitem_on = []
        self._empty_line_count = 2
        self._paragraph_ended = False
        self._paragraph_skip_begin = True

    def startDocument(self, pagename):
        line = u'\n'.rjust(len(pagename) + 2, u'*')
        return self.wrap(u"%s %s \n%s" % (line, pagename, line))

    def endContent(self):
        return self.flush(True)

    def sysmsg(self, on, **kw):
        return self.wrap((u'\n\n*** ', u' ***\n\n')[not on])

    def pagelink(self, on, pagename='', page=None, **kw):
        FormatterBase.pagelink(self, on, pagename, page, **kw)
        if on:
            if not self._textbuf or self._textbuf[-1] in ('\n', ' '):
                result = self.wrap(u'<')
            else:
                result = self.wrap(u' <')
            self.text_on(True)
            self.add_missing_space()
            return result
        else:
            linktext = self._text
            self.text_off()
            orig_pagename = pagename
            if pagename.find('/'):
                pagename = pagename.replace('/', '.')
            pagename += '.txt'
            if linktext == orig_pagename:
                return self.wrap(u'%s>' % pagename)
            else:
                return self.wrap(u'%s> [%s]' % (linktext, pagename))

    def interwikilink(self, on, interwiki='', pagename='', **kw):
        if on:
            self.add_missing_space()
            self._url = u"%s:%s" % (interwiki, pagename)
            self.text_on()
            return u''
        else:
            text = self._text
            self.text_off()
            if text == self._url:
                result = ''
            else:
                result = self.wrap(u' [%s]' % (self._url))
            self._url = None
            return result

    def url(self, on, url='', css=None, **kw):
        if on:
            self.add_missing_space()
            self._url = url
            self.text_on()
            return u''
        else:
            text = self._text
            self.text_off()
            if text == self._url or 'mailto:' + text == self._url:
                result = ''
            else:
                result = self.wrap(u' [%s]' % (self._url))
            self._url = None
            return result

    def attachment_link(self, on, url=None, **kw):
        if on:
            if 'title' in kw and kw['title']:
                if kw['title'] != url:
                    return self.wrap(u'[attachment:%s ' % url)
            return self.wrap(u'[attachment:')
        return self.wrap(']')

    def attachment_image(self, url, **kw):
        title = ''
        for a in (u'title', u'html__title', u'alt', u'html_alt'):
            if a in kw:
                title = ':' + kw[a]
        return self.wrap("[image:%s%s]" % (url, title))

    def attachment_drawing(self, url, text, **kw):
        return self.wrap("[drawing:%s]" % text)

    def text(self, text, **kw):
        if self._text is not None:
            self._text += text
        if self._wrap_skip_text:
            return ''
        return self.wrap(text)

    def rule(self, size=0, **kw):
        size = min(size, 10)
        ch = u"---~=*+#####"[size]
        self.paragraph_begin()
        result = self.wrap((ch * (79 - self._indent)))
        self.paragraph_end()
        return result

    def strong(self, on, **kw):
        if on:
            self.add_missing_space()
        return self.wrap(u'*')

    def emphasis(self, on, **kw):
        if on:
            self.add_missing_space()
        return self.wrap(u'/')

    def highlight(self, on, **kw):
        return u''

    def number_list(self, on, type=None, start=None, **kw):
        if on:
            if self._lists:
                # No empty lines between sublists
                self._paragraph_ended = False
            self.paragraph_begin()
            self._lists.append(0)
            self._listitem_on.append(False)
        elif self._lists:
            self.paragraph_end()
            num = self._lists.pop()
            listitem_on = self._listitem_on.pop()
            if listitem_on:
                prefix = ' %d. ' % (num)
                self._indent -= len(prefix)
        return ''

    def bullet_list(self, on, **kw):
        if on:
            if self._lists:
                # No empty lines between sublists
                self._paragraph_ended = False
            self.paragraph_begin()
            self._lists.append(-1)
            self._listitem_on.append(False)
        else:
            self.paragraph_end()
            self._lists.pop()
            listitem_on = self._listitem_on.pop()
            if listitem_on:
                self._indent -= 3
        return ''

    def listitem(self, on, **kw):
        self._paragraph_ended = False
        if not on:
            # we can't rely on this
            self.paragraph_end()
            return ''

        result = ''
        num = self._lists.pop()
        listitem_on = self._listitem_on.pop()
        if listitem_on and on:
            # we didn't receive on=False for previous listitem
            self.paragraph_end()
            if num >= 0:
                prefix = ' %d. ' % (num)
                self._indent -= len(prefix)
            else:
                self._indent -= 3

        if num >= 0:
            num += 1
            prefix = ' %d. ' % (num)
        else:
            # FIXME: also before tables, at leat in LDA.Sieve.txt
            prefix = ' * '
        self._lists.append(num)
        self._listitem_on.append(on)

        result += self.wrap(prefix)
        self._indent += len(prefix)
        self._paragraph_skip_begin = True
        return result

    def sup(self, on, **kw):
        if on:
            return self.wrap(u'^')
        else:
            return ''

    def sub(self, on, **kw):
        return self.wrap(u'_')

    def strike(self, on, **kw):
        if on:
            self.add_missing_space()
        return self.wrap(u'__')

    def code(self, on, **kw):
        if on:
            self.add_missing_space()
        return self.wrap(u"'")

    def preformatted(self, on, **kw):
        FormatterBase.preformatted(self, on)
        snip = u'%s\n' % u'---%<'.ljust(78 - self._indent, u'-')
        if on:
            self.paragraph_begin()
            return self.wrap(snip)
        else:
            if self._textbuf and not self._textbuf.endswith('\n'):
                self._textbuf += '\n'
            result = self.wrap(snip)
            self.paragraph_end()
            return result

    def small(self, on, **kw):
        if on:
            self.add_missing_space()
        return u''

    def big(self, on, **kw):
        if on:
            self.add_missing_space()
        return u''

    def code_area(self, on, code_id, code_type='code', show=0, start=-1,
                  step=-1, msg=None):
        snip = u'%s\n' % u'---CodeArea'.ljust(78 - self._indent, u'-')
        if on:
            self.paragraph_begin()
            self._in_code_area = 1
            self._in_code_line = 0
            self._code_area_state = [show, start, step, start]
            return self.wrap(snip)
        else:
            if self._in_code_line:
                return self.wrap(self.code_line(0) + snip)
            result = self.wrap(snip)
            self.paragraph_end()
            return result

    def code_line(self, on):
        res = u''
        if not on or (on and self._in_code_line):
            res += u'\n'
        if on:
            if self._code_area_state[0] > 0:
                res += u' %4d  ' % self._code_area_state[3]
                self._code_area_state[3] += self._code_area_state[2]
        self._in_code_line = on != 0
        return self.wrap(res)

    def code_token(self, on, tok_type):
        return ""

    def add_missing_space(self):
        if self._textbuf and self._textbuf[-1].isalnum():
            self._textbuf += ' '

    def paragraph(self, on, **kw):
        FormatterBase.paragraph(self, on)
        if on:
            self.paragraph_begin()
        else:
            self.paragraph_end()
        return ''

    def linebreak(self, preformatted=1):
        return self.wrap(u'\n')

    def smiley(self, text):
        return self.wrap(text)

    def heading(self, on, depth, **kw):
        if on:
            self.paragraph_begin()
            self.text_on()
            result = ''
        else:
            if depth == 1:
                ch = u'='
            else:
                ch = u'-'

            result = u'\n%s\n' % (ch * len(self._text))
            self.text_off()
            result = self.wrap(result)
            self.paragraph_end()
        return result

    def get_table_sep(self, col_widths):
        result = ''
        for width in col_widths:
            result += '+' + ('-' * width)
        return result + '+\n'

    def fix_col_widths(self):
        min_widths = self._table_column_min_len
        max_widths = self._table_column_max_len
        max_len = sum(max_widths)
        # take the needed space equally from all columns
        count = len(max_widths)
        idx, skip = 0, 0
        available_len = 79 - count - 1
        while max_len > available_len:
            if max_widths[idx] > min_widths[idx]:
                max_widths[idx] -= 1
                max_len -= 1
                skip = 0
            else:
                skip += 1
                if skip == count:
                    # there are only too wide columns
                    break
            if idx == count - 1:
                idx = 0
            else:
                idx += 1
        return max_widths

    def table(self, on, attrs={}, **kw):
        if on:
            self._table = []
            self._table_column_min_len = []
            self._table_column_max_len = []
            result = self.flush(True)
        else:
            result = u''
            col_widths = self.fix_col_widths()
            for row in self._table:
                result += self.get_table_sep(col_widths)
                more = True
                while more:
                    more = False
                    num = 0
                    result += '|'
                    for col in row:
                        # break at next LF
                        lf_idx = col.find('\n')
                        if lf_idx != -1:
                            more = True
                            col_len = lf_idx
                            next_idx = lf_idx + 1
                        else:
                            col_len = len(col)
                            next_idx = col_len
                        # possibly break earlier if we need to wrap
                        if col_len > col_widths[num]:
                            idx = col.rfind(' ', 0, col_widths[num])
                            if idx == -1:
                                idx = col.find(' ', col_widths[num])
                            if idx != -1:
                                col_len = idx
                                next_idx = idx + 1
                            more = True
                        result += ' ' + col[:col_len]
                        result += (' ' * (col_widths[num] - col_len - 1)) + '|'
                        row[num] = col[next_idx:]
                        num += 1
                    result += '\n'
            result += self.get_table_sep(col_widths)
            self._table = None
            self._table_column_min_len = None
            self._table_column_max_len = None
            self._empty_line_count = 0
            self.paragraph_end()
        return result

    def table_row(self, on, attrs={}, **kw):
        if on:
            self._table.append([])
        return u''

    def table_cell(self, on, attrs={}, **kw):
        if on:
            self.text_on()
            self._wrap_skip_text = True
        else:
            # keep track of the longest word and the longest line in the cell
            self._text = self._text.strip()
            max_line_len = 0
            max_word_len = 0
            for line in self._text.split('\n'):
                if len(line) > max_line_len:
                    max_line_len = len(line)
            for word in self._text.split(' '):
                if len(word) > max_word_len:
                    max_word_len = len(word)
            # one preceding and trailing cell whitespace
            max_word_len += 2
            max_line_len += 2

            rownum = len(self._table) - 1
            colnum = len(self._table[rownum])
            if len(self._table_column_max_len) <= colnum:
                self._table_column_min_len.append(max_word_len)
                self._table_column_max_len.append(max_line_len)
            else:
                if max_word_len > self._table_column_min_len[colnum]:
                    self._table_column_min_len[colnum] = max_word_len
                if self._table_column_max_len[colnum] < max_line_len:
                    self._table_column_max_len[colnum] = max_line_len
            self._table[rownum].append(self._text)
            self.text_off()
        return u''

    def underline(self, on, **kw):
        return self.wrap(u'_')

    def definition_list(self, on, **kw):
        if on:
            self.paragraph_begin()
        else:
            self.paragraph_end()
        return u''

    def definition_term(self, on, compact=0, **kw):
        result = u''
        #if not compact:
        #    result = result + u'\n'
        if not on:
            result = result + u':'
        return self.wrap(result)

    def definition_desc(self, on, **kw):
        if on:
            self._indent += 2
            self.paragraph_begin()
        else:
            self.paragraph_end()
            self._textbuf += '\n'
            self._indent -= 2
        return ''

    def image(self, src=None, **kw):
        for a in (u'title', u'html__title', u'alt', u'html_alt'):
            if a in kw:
                return self.wrap(kw[a] + ' [' + src + ']')
        return self.wrap('[' + src + ']')

    def lang(self, on, lang_name):
        return ''

    def paragraph_begin(self):
        if self._paragraph_ended:
            self._textbuf += '\n'
        elif not self._paragraph_skip_begin:
            if self._textbuf and not self._textbuf.endswith('\n'):
                self._textbuf += '\n'
        self._paragraph_ended = False
        self._paragraph_skip_begin = False

    def paragraph_end(self):
        if self._textbuf and not self._textbuf.endswith('\n'):
            self._textbuf += '\n'
        self._paragraph_ended = True

    def wrap(self, text):
        if not text:
            return ''
        if self._wrap_skip_text:
            # we're inside table
            #self._text += 'w{' + text + '}'
            self._text += text
            return ''

        self._paragraph_ended = False
        self._paragraph_skip_begin = False

        # add indents after all LFs. kind of dirty to split twice though..
        lines = text.split('\n')
        text = lines.pop(0)
        while lines:
            text += '\n%s%s' % (' ' * self._indent, lines.pop(0))

        if not self._textbuf or self._textbuf.endswith('\n'):
            self._textbuf += ' ' * self._indent
        self._textbuf += text

        lines = self._textbuf.split('\n')
        self._textbuf = ''
        text = ''
        while lines:
            self._textbuf += lines.pop(0)
            if lines:
                # LFs found
                text += self.flush(True)
            if len(self._textbuf) > 80 and \
                    self._textbuf.find(' ', self._indent) != -1:
                # wrap time
                text += self.flush(False)
        return text

    def flush(self, addlf):
        result = ''

        while len(self._textbuf) >= 80:
            # need to wrap
            last_space = self._textbuf.rfind(' ', self._indent, 80)
            if last_space == -1:
                # a long line. split at the next possible space
                last_space = self._textbuf.find(' ', self._indent)
                if last_space == -1:
                    break
            result += self._textbuf[:last_space] + '\n'
            self._empty_line_count = 0
            self._textbuf = ' ' * self._indent + self._textbuf[last_space + 1:]
        self._textbuf = self._textbuf.rstrip()

        if not self._textbuf:
            if not addlf:
                return result
            self._empty_line_count += 1
            if self._empty_line_count >= 2:
                return result
        else:
            self._empty_line_count = 0

        if addlf:
            result += self._textbuf + '\n'
            self._textbuf = ''
        return result

    def text_on(self, skip_text=False):
        if self._text is None:
            self._text_stack.append(None)
        else:
            self._text_stack.append(self._text)
            #self._text_stack.append('[' + self._text + ']')
        self._text_stack.append(self._skip_text)
        self._text_stack.append(self._wrap_skip_text)
        self._text = ""
        self._skip_text = skip_text
        if skip_text:
            self._wrap_skip_text = True

    def text_off(self):
        prev_skip_text = self._skip_text
        self._wrap_skip_text = self._text_stack.pop()
        self._skip_text = self._text_stack.pop()
        old_text = self._text_stack.pop()
        if old_text is None:
            self._text = None
        else:
            if not prev_skip_text:
                #self._text = 'o#' + old_text + '#|#' + self._text + '#'
                self._text = old_text + self._text
            else:
                self._text = old_text
