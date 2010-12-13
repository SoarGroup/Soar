# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Formatter Package and FormatterBase

    See "base.py" for the formatter interface.

    @copyright: 2000-2004 by Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""
import re

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin.util import pysupport
from MoinMoin import wikiutil

modules = pysupport.getPackageModules(__file__)


class FormatterBase:
    """ This defines the output interface used all over the rest of the code.

        Note that no other means should be used to generate _content_ output,
        while navigational elements (HTML page header/footer) and the like
        can be printed directly without violating output abstraction.
    """

    hardspace = ' '

    def __init__(self, request, **kw):
        self.request = request
        self._ = request.getText

        self._store_pagelinks = kw.get('store_pagelinks', 0)
        self._terse = kw.get('terse', 0)
        self.pagelinks = []
        self.in_p = 0
        self.in_pre = 0
        self._highlight_re = None
        self._base_depth = 0

    def set_highlight_re(self, hi_re=None):
        """ set the highlighting regular expression (e.g. for search terms)

        @param hi_re: either a valid re as str/unicode (you may want to use
                      re.escape before passing generic strings!) or a compiled
                      re object. raises re.error for invalid re.
        """
        if isinstance(hi_re, (str, unicode)):
            hi_re = re.compile(hi_re, re.U + re.IGNORECASE)
        self._highlight_re = hi_re

    def lang(self, on, lang_name):
        return ""

    def setPage(self, page):
        self.page = page

    def sysmsg(self, on, **kw):
        """ Emit a system message (embed it into the page).

            Normally used to indicate disabled options, or invalid markup.
        """
        return ""

    # Document Level #####################################################

    def startDocument(self, pagename):
        return ""

    def endDocument(self):
        return ""

    def startContent(self, content_id="content", **kw):
        if self.page:
            self.request.uid_generator.begin(self.page.page_name)
        return ""

    def endContent(self):
        if self.page:
            self.request.uid_generator.end()
        return ""

    # Links ##############################################################

    def pagelink(self, on, pagename='', page=None, **kw):
        """ make a link to page <pagename>. Instead of supplying a pagename,
            it is also possible to give a live Page object, then page.page_name
            will be used.
        """
        if not self._store_pagelinks or not on or kw.get('generated'):
            return ''
        if not pagename and page:
            pagename = page.page_name
        pagename = wikiutil.normalize_pagename(pagename, self.request.cfg)
        if pagename and pagename not in self.pagelinks:
            self.pagelinks.append(pagename)

    def interwikilink(self, on, interwiki='', pagename='', **kw):
        """ calls pagelink() for internal interwikilinks
            to make sure they get counted for self.pagelinks.
            IMPORTANT: on and off must be called with same parameters, see
                       also the text_html formatter.
        """
        wikitag, wikiurl, wikitail, wikitag_bad = wikiutil.resolve_interwiki(self.request, interwiki, pagename)
        if wikitag == 'Self' or wikitag == self.request.cfg.interwikiname:
            return self.pagelink(on, wikitail, **kw)
        return ''

    def url(self, on, url=None, css=None, **kw):
        raise NotImplementedError

    # Attachments ######################################################

    def attachment_link(self, on, url=None, **kw):
        raise NotImplementedError
    def attachment_image(self, url, **kw):
        raise NotImplementedError
    def attachment_drawing(self, url, text, **kw):
        raise NotImplementedError

    def attachment_inlined(self, url, text, **kw):
        from MoinMoin.action import AttachFile
        import os
        _ = self.request.getText
        pagename, filename = AttachFile.absoluteName(url, self.page.page_name)
        fname = wikiutil.taintfilename(filename)
        fpath = AttachFile.getFilename(self.request, pagename, fname)
        ext = os.path.splitext(filename)[1]
        Parser = wikiutil.getParserForExtension(self.request.cfg, ext)
        if Parser is not None:
            try:
                content = file(fpath, 'r').read()
                # Try to decode text. It might return junk, but we don't
                # have enough information with attachments.
                content = wikiutil.decodeUnknownInput(content)
                colorizer = Parser(content, self.request, filename=filename)
                colorizer.format(self)
            except IOError:
                pass

        return (self.attachment_link(1, url) +
                self.text(text) +
                self.attachment_link(0))

    def anchordef(self, name):
        return ""

    def line_anchordef(self, lineno):
        return ""

    def anchorlink(self, on, name='', **kw):
        return ""

    def line_anchorlink(self, on, lineno=0):
        return ""

    def image(self, src=None, **kw):
        """An inline image.

        Extra keyword arguments are according to the HTML <img> tag attributes.
        In particular an 'alt' or 'title' argument should give a description
        of the image.
        """
        title = src
        for titleattr in ('title', 'html__title', 'alt', 'html__alt'):
            if titleattr in kw:
                title = kw[titleattr]
                break
        if title:
            return '[Image:%s]' % title
        return '[Image]'

    # generic transclude/include:
    def transclusion(self, on, **kw):
        raise NotImplementedError
    def transclusion_param(self, **kw):
        raise NotImplementedError

    def smiley(self, text):
        return text

    def nowikiword(self, text):
        return self.text(text)

    # Text and Text Attributes ###########################################

    def text(self, text, **kw):
        if not self._highlight_re:
            return self._text(text)

        result = []
        lastpos = 0
        match = self._highlight_re.search(text)
        while match and lastpos < len(text):
            # add the match we found
            result.append(self._text(text[lastpos:match.start()]))
            result.append(self.highlight(1))
            result.append(self._text(match.group(0)))
            result.append(self.highlight(0))

            # search for the next one
            lastpos = match.end() + (match.end() == lastpos)
            match = self._highlight_re.search(text, lastpos)

        result.append(self._text(text[lastpos:]))
        return ''.join(result)

    def _text(self, text):
        raise NotImplementedError

    def strong(self, on, **kw):
        raise NotImplementedError

    def emphasis(self, on, **kw):
        raise NotImplementedError

    def underline(self, on, **kw):
        raise NotImplementedError

    def highlight(self, on, **kw):
        raise NotImplementedError

    def sup(self, on, **kw):
        raise NotImplementedError

    def sub(self, on, **kw):
        raise NotImplementedError

    def strike(self, on, **kw):
        raise NotImplementedError

    def code(self, on, **kw):
        raise NotImplementedError

    def preformatted(self, on, **kw):
        self.in_pre = on != 0

    def small(self, on, **kw):
        raise NotImplementedError

    def big(self, on, **kw):
        raise NotImplementedError

    # special markup for syntax highlighting #############################

    def code_area(self, on, code_id, **kw):
        raise NotImplementedError

    def code_line(self, on):
        raise NotImplementedError

    def code_token(self, tok_text, tok_type):
        raise NotImplementedError

    # Paragraphs, Lines, Rules ###########################################

    def linebreak(self, preformatted=1):
        raise NotImplementedError

    def paragraph(self, on, **kw):
        self.in_p = on != 0

    def rule(self, size=0, **kw):
        raise NotImplementedError

    def icon(self, type):
        return type

    # Lists ##############################################################

    def number_list(self, on, type=None, start=None, **kw):
        raise NotImplementedError

    def bullet_list(self, on, **kw):
        raise NotImplementedError

    def listitem(self, on, **kw):
        raise NotImplementedError

    def definition_list(self, on, **kw):
        raise NotImplementedError

    def definition_term(self, on, compact=0, **kw):
        raise NotImplementedError

    def definition_desc(self, on, **kw):
        raise NotImplementedError

    def heading(self, on, depth, **kw):
        raise NotImplementedError

    # Tables #############################################################

    def table(self, on, attrs={}, **kw):
        raise NotImplementedError

    def table_row(self, on, attrs={}, **kw):
        raise NotImplementedError

    def table_cell(self, on, attrs={}, **kw):
        raise NotImplementedError

    # Dynamic stuff / Plugins ############################################

    def macro(self, macro_obj, name, args, markup=None):
        # call the macro
        try:
            return macro_obj.execute(name, args)
        except ImportError, err:
            errmsg = unicode(err)
            if not name in errmsg:
                raise
            if markup:
                return (self.span(1, title=errmsg) +
                        self.text(markup) +
                        self.span(0))
            else:
                return self.text(errmsg)
    def _get_bang_args(self, line):
        if line.startswith('#!'):
            try:
                name, args = line[2:].split(None, 1)
            except ValueError:
                return ''
            else:
                return args
        return None

    def parser(self, parser_name, lines):
        """ parser_name MUST be valid!
            writes out the result instead of returning it!
        """
        # attention: this is copied into text_python!
        parser = wikiutil.searchAndImportPlugin(self.request.cfg, "parser", parser_name)
        args = None
        if lines:
            args = self._get_bang_args(lines[0])
            logging.debug("formatter.parser: parser args %r" % args)
            if args is not None:
                lines = lines[1:]
        if lines and not lines[0]:
            lines = lines[1:]
        if lines and not lines[-1].strip():
            lines = lines[:-1]
        p = parser('\n'.join(lines), self.request, format_args=args)
        p.format(self)
        del p
        return ''

    # Other ##############################################################

    def div(self, on, **kw):
        """ open/close a blocklevel division """
        return ""

    def span(self, on, **kw):
        """ open/close a inline span """
        return ""

    def rawHTML(self, markup):
        """ This allows emitting pre-formatted HTML markup, and should be
            used wisely (i.e. very seldom).

            Using this event while generating content results in unwanted
            effects, like loss of markup or insertion of CDATA sections
            when output goes to XML formats.
        """

        import formatter, htmllib
        from MoinMoin.util import simpleIO

        # Regenerate plain text
        f = simpleIO()
        h = htmllib.HTMLParser(formatter.AbstractFormatter(formatter.DumbWriter(f)))
        h.feed(markup)
        h.close()

        return self.text(f.getvalue())

    def escapedText(self, on, **kw):
        """ This allows emitting text as-is, anything special will
            be escaped (at least in HTML, some text output format
            would possibly do nothing here)
        """
        return ""

    def comment(self, text, **kw):
        return ""

    # ID handling #################################################

    def sanitize_to_id(self, text):
        '''
        Take 'text' and return something that is a valid ID
        for this formatter.
        The default returns the first non-space character of the string.

        Because of the way this is used, it must be idempotent,
        i.e. calling it on an already sanitized id must yield the
        original id.
        '''
        return text.strip()[:1]

    def make_id_unique(self, id):
        '''
        Take an ID and make it unique in the current namespace.
        '''
        ns = self.request.uid_generator.include_id
        if not ns is None:
            ns = self.sanitize_to_id(ns)
        id = self.sanitize_to_id(id)
        id = self.request.uid_generator(id, ns)
        return id

    def qualify_id(self, id):
        '''
        Take an ID and return a string that is qualified by
        the current namespace; this default implementation
        is suitable if the dot ('.') is valid in IDs for your
        formatter.
        '''
        ns = self.request.uid_generator.include_id
        if not ns is None:
            ns = self.sanitize_to_id(ns)
            return '%s.%s' % (ns, id)
        return id
