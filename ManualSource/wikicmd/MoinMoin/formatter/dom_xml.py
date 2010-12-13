# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - DOM XML Formatter

    @copyright: 2000-2004 by Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

line_anchors = True

from xml.dom import minidom
from MoinMoin import wikiutil
from MoinMoin.formatter import FormatterBase

#def print_dom(element, indent=''):
#    print indent + element.tagName
#    for child in element.get

class Formatter(FormatterBase):
    """ This defines the output interface used all over the rest of the code.

        Note that no other means should be used to generate _content_ output,
        while navigational elements (HTML page header/footer) and the like
        can be printed directly without violating output abstraction.
    """
    hardspace = ' '

    # those tags will be automatically reopened if they were auto-closed due to
    # an enclosing tag being closed:
    format_tags = ['b', 'em', 'highlight', 'sup', 'sub', 'strike', 'code', 'u']

    # those tags want a <p> around them:
    need_p = [] # format_tags[:]
    need_p.extend(['ol', 'a', 'pagelink', 'interwiki', 'macro']) # XXX add more tags

    # those tags inhibit auto-generation of a <p> after them:
    no_p_after = ['h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'p', 'ol', 'ul', 'pre',
                  'small', 'big', 'table', 'td', 'tr', 'dt',
                  'codearea', 'codeline', 'codetoken',
                  'sysmesg']

    # if key tag is opened, auto-close all tags in value list if they are open
    close_on_open = {
        'h1': ['p'],
        'li': ['li'],
        'p': ['p'],
        #'pre': ['p'],
        }

    for i in xrange(2, 7):
        close_on_open['h%i' % i] = close_on_open['h1']

    # if key tag is closed, auto-close all tags in value list if they are open
    close_on_close = {
        'table': ['td', 'tr'],
        'td': ['tr'], # XXX WTF? this doesn't look correct.
        'tr': ['td'],
        'ol': ['li'],
        'ul': ['li'],
        }

    # FIXME - this overrides the values defined above
    close_on_open = {}
    close_on_close = {}

    def __init__(self, request, **kw):
        FormatterBase.__init__(self, request, **kw)

        self.document = minidom.Document()
        self.document.documentElement = self.document.createElement('xml')
        self.position = self.document.documentElement
        self.tag_stack = [('xml', {})]

    def setPage(self, page):
        self.page = page

    def _open_tag(self, tag, **attrs):
        """ low level function: opens tag right now

        @param tag: tag name, string
        @param attrs: attributes keywords, ascii or unicode
        """
        if tag == 'p':
            FormatterBase.paragraph(self, 1)
        self.tag_stack.append((tag, attrs))
        node = self.document.createElement(tag)
        for name, value in attrs.items():
            if value:
                node.setAttribute(name, unicode(value))
        self.position.appendChild(node)
        self.position = node
        return ''

    def _close_tag(self, tag):
        """ low level function: closes tag right now
            must be the last opened tag!!!
        """
        if tag == 'p':
            FormatterBase.paragraph(self, 0)
        if self.tag_stack[-1][0] != tag:
            raise ValueError, "closing of <%s> expected, but <%s> closed" % (self.tag_stack[-1][0], tag)
        self.position = self.position.parentNode
        return self.tag_stack.pop()

    def _add_tag(self, tag, **attrs):
        """ low level function: insert self closing tag right now
            does check_p
        """
        self._check_p(tag)
        node = self.document.createElement(tag)
        for name, value in attrs.items():
            if value:
                node.setAttribute(name, str(value))
        self.position.appendChild(node)
        return ''

    def _text(self, text):
        if text.strip():
            self._check_p()
            self.position.appendChild(self.document.createTextNode(text))
        return ''

    def _set_tag(self, tag, on, **attrs):
        if on:
            close_on_open = self.close_on_open.get(tag, [])
            tags_to_reopen = []
            while True:
                last_tag = self.tag_stack[-1][0]
                if last_tag in close_on_open:
                    self._close_tag(last_tag)
                elif last_tag in self.format_tags:
                    tags_to_reopen.append(self._close_tag(last_tag))
                else:
                    break
            # XXX check if enclosing tag is ok

            if tag in self.need_p:
                self._check_p(tag)

            self._open_tag(tag, **attrs)
            tags_to_reopen.reverse()
            for tag_name, args in tags_to_reopen:
                self._open_tag(tag_name, **args)
        else:
            tags_to_reopen = []
            close_on_close = self.close_on_close.get(tag, [])
            # walk up
            while self.tag_stack:
                # collect format tags
                last_tag = self.tag_stack[-1][0]
                if last_tag == tag:
                    break
                elif last_tag in close_on_close:
                    self._close_tag(last_tag)
                elif last_tag in self.format_tags:
                    tags_to_reopen.append(self._close_tag(last_tag))
                else:
                    self.request.write("tag_stack: %r\n" % self.tag_stack)
                    self.request.write(self.document.documentElement.toprettyxml(" "))
                    raise ValueError, "closing of <%s> expected, but <%s> closed" % (last_tag, tag)
            self._close_tag(tag)
            tags_to_reopen.reverse()
            for tag_name, args in tags_to_reopen:
                self._open_tag(tag_name, **args)
        return ''

    def _check_p(self, opening_tag=None):
        if (opening_tag is not None) and (opening_tag not in self.need_p):
            return
        for tag in self.tag_stack:
            if tag[0] in self.no_p_after:
                return
        if self.in_p:
            return
        self._open_tag('p', type=str(opening_tag))

    def sysmsg(self, on, **kw):
        """ Emit a system message (embed it into the page).

            Normally used to indicate disabled options, or invalid markup.
        """
        return self._set_tag('sysmesg', on, **kw)

    def startDocument(self, pagename):
        return ''

    def endDocument(self):
        #return self.document.documentElement.toxml()
        return self.document.documentElement.toprettyxml("  ")

    def lang(self, on, lang_name):
        return self._set_tag('lang', on, value=lang_name)

    def pagelink(self, on, pagename='', page=None, **kw):
        FormatterBase.pagelink(self, pagename, page, **kw)
        if not pagename and page is not None:
            pagename = page.page_name
        kw['pagename'] = pagename
        return self._set_tag('pagelink', on, **kw)

    def interwikilink(self, on, interwiki='', pagename='', **kw):
        kw['wiki'] = interwiki
        kw['pagename'] = pagename
        return self._set_tag('interwiki', on, **kw)

    def macro(self, macro_obj, name, args, markup=None):
        # call the macro
        return self._add_tag('macro', name=name, args=(args or ''))

    def parser(self, parser_name, lines):
        """ parser_name MUST be valid!
            writes out the result instead of returning it!
        """
        node = self.document.createElement('parser')
        node.setAttribute('name', parser_name)
        node.appendChild(self.document.createTextNode('\n'.join(lines)))
        return (self._set_tag('parser', True, name=parser_name) +
                self.text('\n'.join(lines)) +
                self._set_tag('parser', False))

    def url(self, on, url='', css=None, **kw):
        kw['href'] = str(url)
        if css:
            kw['class'] = str(css)
        return self._set_tag('a', on, **kw)

    def attachment_link(self, on, url=None, **kw):
        kw['href'] = url
        kw['type'] = 'link'
        if on:
            return self._set_tag('attachment', 1, **kw)
        else:
            return self._set_tag('attachment', 0, **kw)

    def attachment_image(self, url, **kw):
        kw['href'] = url
        kw['type'] = 'image'
        return self._add_tag('attachment', **kw)

    def attachment_drawing(self, url, **kw):
        kw['href'] = url
        kw['type'] = 'drawing'
        return self._add_tag('attachment', **kw)

    def attachment_inlined(self, url, **kw):
        kw['href'] = url
        kw['type'] = 'inline'
        return self._add_tag('attachment', **kw)

    def rule(self, size=0, **kw):
        return self._add_tag('hr', size=str(size))

    def icon(self, type):
        return self._add_tag('icon', type=type)

    def smiley(self, type):
        return self._add_tag('smiley', type=type)

    def strong(self, on, **kw):
        return self._set_tag('b', on)

    def emphasis(self, on, **kw):
        return self._set_tag('em', on)

    def highlight(self, on, **kw):
        return self._set_tag('highlight', on)

    def number_list(self, on, type=None, start=None, **kw):
        return self._set_tag('ol', on, type=type, start=start)

    def bullet_list(self, on, **kw):
        return self._set_tag('ul', on)

    def listitem(self, on, **kw):
        return self._set_tag('li', on)

    def sup(self, on, **kw):
        return self._set_tag('sup', on)

    def sub(self, on, **kw):
        return self._set_tag('sub', on)

    def strike(self, on, **kw):
        return self._set_tag('strike', on)

    def code(self, on, **kw):
        return self._set_tag('code', on)

    def preformatted(self, on, **kw):
        self.in_pre = on != 0
        return self._set_tag('pre', on)

    def paragraph(self, on, **kw):
        FormatterBase.paragraph(self, on)
        return self._set_tag('p', on)

    def linebreak(self, preformatted=1):
        if self.tag_stack[-1][0] == 'pre':
            return self.text('\n')
        else:
            return self._add_tag('br')

    def heading(self, on, depth, **kw):
        return self._set_tag('h%d' %depth, on, **kw)

    def _check_attrs(self, attrs):
        result = {}
        for name, value in attrs.iteritems():
            result[str(name)] = value
        return result

    def table(self, on, attrs={}, **kw):
        return self._set_tag('table', on, **self._check_attrs(attrs))

    def table_row(self, on, attrs={}, **kw):
        return self._set_tag('tr', on, **self._check_attrs(attrs))

    def table_cell(self, on, attrs={}, **kw):
        return self._set_tag('td', on, **self._check_attrs(attrs))

    def anchordef(self, name):
        return self._add_tag('anchor', name=name)

    def anchorlink(self, on, name, **kw):
        return self.url(on, "#" + name, **kw)

    def line_anchordef(self, lineno):
        if line_anchors:
            return self.anchordef("line-%d" % lineno)
        else:
            return ''

    def underline(self, on, **kw):
        return self._set_tag('u', on)

    def definition_list(self, on, **kw):
        return self._set_tag('dl', on)

    def definition_term(self, on, compact=0, **kw):
        # XXX may be not correct
        # self._langAttr() missing
        if compact and on:
            return self._set_tag('dt compact', on)
        else:
            return self._set_tag('dt', on)

    def definition_desc(self, on, **kw):
        # self._langAttr() missing
        return self._set_tag('dd', on)

    def image(self, src=None, **kw):
        """ Take HTML <IMG> tag attributes in `attr`.

            Attribute names have to be lowercase!
        """
        if src:
            kw['src'] = src
        return self._add_tag('img', **kw)

    def transclusion(self, on, **kw):
        return self._set_tag('object', on, **kw)

    def transclusion_param(self, **kw):
        return self._add_tag('param', **kw)

    def escapedText(self, text, **kw):
        return wikiutil.escape(text)

    def small(self, on, **kw):
        return self._set_tag('small', on)

    def big(self, on, **kw):
        return self._set_tag('big', on)

    def code_area(self, on, code_id, code_type='code', show=0, start=-1, step=-1, msg=None):
        kw = {'id': code_id,
              'type': code_type,
              'show': show,
             }
        if start != -1:
            kw['start'] = start
        if step != -1:
            kw['step'] = step
        return self._set_tag('codearea', on, **kw)

    def code_line(self, on):
        return self._set_tag('codeline', on)

    def code_token(self, on, tok_type):
        return self._set_tag('codetoken', on, type=tok_type)


