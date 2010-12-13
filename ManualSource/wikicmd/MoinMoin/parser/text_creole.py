# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Creole wiki markup parser

    See http://wikicreole.org/ for latest specs.

    Notes:
    * No markup allowed in headings.
      Creole 1.0 does not require us to support this.
    * No markup allowed in table headings.
      Creole 1.0 does not require us to support this.
    * No (non-bracketed) generic url recognition: this is "mission impossible"
      except if you want to risk lots of false positives. Only known protocols
      are recognized.
    * We do not allow ":" before "//" italic markup to avoid urls with
      unrecognized schemes (like wtf://server/path) triggering italic rendering
      for the rest of the paragraph.

    @copyright: 2007 MoinMoin:RadomirDopieralski (creole 0.5 implementation),
                2007 MoinMoin:ThomasWaldmann (updates)
    @license: GNU GPL, see COPYING for details.
"""

import re
import StringIO
from MoinMoin import config, wikiutil
from MoinMoin.macro import Macro
from MoinMoin import config
from _creole import Parser as CreoleParser
from _creole import Rules as CreoleRules

Dependencies = []

_ = lambda x: x

class Parser:
    """
    Glue the DocParser and DocEmitter with the
    MoinMoin current API.
    """

    extensions = ['.creole']
    # Enable caching
    caching = 1
    Dependencies = Dependencies
    quickhelp = _(u"""\
 Emphasis:: <<Verbatim(//)>>''italics''<<Verbatim(//)>>; <<Verbatim(**)>>'''bold'''<<Verbatim(**)>>; <<Verbatim(**//)>>'''''bold italics'''''<<Verbatim(//**)>>; <<Verbatim(//)>>''mixed ''<<Verbatim(**)>>'''''bold'''<<Verbatim(**)>> and italics''<<Verbatim(//)>>;
 Horizontal Rule:: <<Verbatim(----)>>
 Force Linebreak:: <<Verbatim(\\\\)>>
 Headings:: = Title 1 =; == Title 2 ==; === Title 3 ===; ==== Title 4 ====; ===== Title 5 =====.
 Lists:: * bullets; ** sub-bullets; # numbered items; ## numbered sub items.
 Links:: <<Verbatim([[target]])>>; <<Verbatim([[target|linktext]])>>.
 Tables:: |= header text | cell text | more cell text |;

(!) For more help, see HelpOnEditing or HelpOnCreoleSyntax.
""")

    def __init__(self, raw, request, **kw):
        """Create a minimal Parser object with required attributes."""

        self.request = request
        self.form = request.form
        self.raw = raw
        self.rules = MoinRules(wiki_words=True,
                               url_protocols=config.url_schemas)

    def format(self, formatter):
        """Create and call the true parser and emitter."""

        document = CreoleParser(self.raw, self.rules).parse()
        result = Emitter(document, formatter, self.request, Macro(self),
                         self.rules).emit()
        self.request.write(result)

class MoinRules(CreoleRules):
    # For the link targets:
    proto = r'http|https|ftp|nntp|news|mailto|telnet|file|irc'
    extern = r'(?P<extern_addr>(?P<extern_proto>%s):.*)' % proto
    attach = r'''
            (?P<attach_scheme> attachment | drawing | image ):
            (?P<attach_addr> .* )
        '''
    interwiki = r'''
            (?P<inter_wiki> [A-Z][a-zA-Z]+ ) :
            (?P<inter_page> .* )
        '''
    page = r'(?P<page_name> .* )'

    def __init__(self, *args, **kwargs):
        CreoleRules.__init__(self, *args, **kwargs)
        # for addresses
        self.addr_re = re.compile('|'.join([self.extern, self.attach,
                                            self.interwiki, self.page]),
                                  re.X | re.U)

class Emitter:
    """
    Generate the output for the document
    tree consisting of DocNodes.
    """

    def __init__(self, root, formatter, request, macro, rules):
        self.root = root
        self.formatter = formatter
        self.request = request
        self.form = request.form
        self.macro = macro
        self.rules = rules

    def get_text(self, node):
        """Try to emit whatever text is in the node."""

        try:
            return node.children[0].content or ''
        except:
            return node.content or ''

    # *_emit methods for emitting nodes of the document:

    def document_emit(self, node):
        return self.emit_children(node)

    def text_emit(self, node):
        return self.formatter.text(node.content or '')

    def separator_emit(self, node):
        return self.formatter.rule()

    def paragraph_emit(self, node):
        return ''.join([
            self.formatter.paragraph(1),
            self.emit_children(node),
            self.formatter.paragraph(0),
        ])

    def bullet_list_emit(self, node):
        return ''.join([
            self.formatter.bullet_list(1),
            self.emit_children(node),
            self.formatter.bullet_list(0),
        ])

    def number_list_emit(self, node):
        return ''.join([
            self.formatter.number_list(1),
            self.emit_children(node),
            self.formatter.number_list(0),
        ])

    def list_item_emit(self, node):
        return ''.join([
            self.formatter.listitem(1),
            self.emit_children(node),
            self.formatter.listitem(0),
        ])

# Not used
#    def definition_list_emit(self, node):
#        return ''.join([
#            self.formatter.definition_list(1),
#            self.emit_children(node),
#            self.formatter.definition_list(0),
#        ])

# Not used
#    def term_emit(self, node):
#        return ''.join([
#            self.formatter.definition_term(1),
#            self.emit_children(node),
#            self.formatter.definition_term(0),
#        ])

# Not used
#    def definition_emit(self, node):
#        return ''.join([
#            self.formatter.definition_desc(1),
#            self.emit_children(node),
#            self.formatter.definition_desc(0),
#        ])

    def table_emit(self, node):
        return ''.join([
            self.formatter.table(1, attrs=getattr(node, 'attrs', '')),
            self.emit_children(node),
            self.formatter.table(0),
        ])

    def table_row_emit(self, node):
        return ''.join([
            self.formatter.table_row(1, attrs=getattr(node, 'attrs', '')),
            self.emit_children(node),
            self.formatter.table_row(0),
        ])

    def table_cell_emit(self, node):
        return ''.join([
            self.formatter.table_cell(1, attrs=getattr(node, 'attrs', '')),
            self.emit_children(node),
            self.formatter.table_cell(0),
        ])

    def table_head_emit(self, node):
        return ''.join([
            self.formatter.rawHTML('<th>'),
            self.emit_children(node),
            self.formatter.rawHTML('</th>'),
        ])

    def emphasis_emit(self, node):
        return ''.join([
            self.formatter.emphasis(1),
            self.emit_children(node),
            self.formatter.emphasis(0),
        ])

# Not used
#    def quote_emit(self, node):
#        return ''.join([
#            self.formatter.rawHTML('<q>'),
#            self.emit_children(node),
#            self.formatter.rawHTML('</q>'),
#        ])

    def strong_emit(self, node):
        return ''.join([
            self.formatter.strong(1),
            self.emit_children(node),
            self.formatter.strong(0),
        ])

# Not used
#    def smiley_emit(self, node):
#        return self.formatter.smiley(node.content)

    def header_emit(self, node):
        text = self.get_text(node)
        return ''.join([
            self.formatter.heading(1, node.level, id=text),
            self.formatter.text(text),
            self.formatter.heading(0, node.level),
        ])

    def code_emit(self, node):
# XXX The current formatter will replace all spaces with &nbsp;, so we need
# to use rawHTML instead, until that is fixed.
#        return ''.join([
#            self.formatter.code(1),
#            self.formatter.text(node.content or ''),
#            self.formatter.code(0),
#        ])
        return ''.join([
            self.formatter.rawHTML('<tt>'),
            self.formatter.text(node.content or ''),
            self.formatter.rawHTML('</tt>'),
        ])

# Not used
#    def abbr_emit(self, node):
#        return ''.join([
#            self.formatter.rawHTML('<abbr title="%s">' % node.title),
#            self.formatter.text(node.content or ''),
#            self.formatter.rawHTML('</abbr>'),
#        ])

    def link_emit(self, node):
        target = node.content
        m = self.rules.addr_re.match(target)
        if m:
            if m.group('page_name'):
                # link to a page
                word = m.group('page_name')
                if word.startswith(wikiutil.PARENT_PREFIX):
                    word = word[wikiutil.PARENT_PREFIX_LEN:]
                elif word.startswith(wikiutil.CHILD_PREFIX):
                    word = "%s/%s" % (self.formatter.page.page_name,
                        word[wikiutil.CHILD_PREFIX_LEN:])
                word, anchor = wikiutil.split_anchor(word)
                return ''.join([
                    self.formatter.pagelink(1, word, anchor=anchor),
                    self.emit_children(node) or self.formatter.text(target),
                    self.formatter.pagelink(0, word),
                ])
            elif m.group('extern_addr'):
                # external link
                address = m.group('extern_addr')
                proto = m.group('extern_proto')
                return ''.join([
                    self.formatter.url(1, address, css=proto),
                    self.emit_children(node) or self.formatter.text(target),
                    self.formatter.url(0),
                ])
            elif m.group('inter_wiki'):
                # interwiki link
                wiki = m.group('inter_wiki')
                page = m.group('inter_page')
                page, anchor = wikiutil.split_anchor(page)
                return ''.join([
                    self.formatter.interwikilink(1, wiki, page, anchor=anchor),
                    self.emit_children(node) or self.formatter.text(page),
                    self.formatter.interwikilink(0),
                ])
            elif m.group('attach_scheme'):
                # link to an attachment
                scheme = m.group('attach_scheme')
                attachment = m.group('attach_addr')
                url = wikiutil.url_unquote(attachment)
                text = self.get_text(node)
                return ''.join([
                        self.formatter.attachment_link(1, url),
                        self.formatter.text(text),
                        self.formatter.attachment_link(0)
                    ])
        return "".join(["[[", self.formatter.text(target), "]]"])

# Not used
#    def anchor_link_emit(self, node):
#        return ''.join([
#            self.formatter.url(1, node.content, css='anchor'),
#            self.emit_children(node),
#            self.formatter.url(0),
#        ])

    def image_emit(self, node):
        target = node.content
        text = self.get_text(node)
        m = self.rules.addr_re.match(target)
        if m:
            if m.group('page_name'):
                # inserted anchors
                url = wikiutil.url_unquote(target)
                if target.startswith('#'):
                    return self.formatter.anchordef(url[1:])
                # default to images
                return self.formatter.attachment_image(
                    url, alt=text, html_class='image')
            elif m.group('extern_addr'):
                # external link
                address = m.group('extern_addr')
                proto = m.group('extern_proto')
                url = wikiutil.url_unquote(address)
                return self.formatter.image(
                    src=url, alt=text, html_class='external_image')
            elif m.group('attach_scheme'):
                # link to an attachment
                scheme = m.group('attach_scheme')
                attachment = m.group('attach_addr')
                url = wikiutil.url_unquote(attachment)
                if scheme == 'image':
                    return self.formatter.attachment_image(
                        url, alt=text, html_class='image')
                elif scheme == 'drawing':
                    url = wikiutil.drawing2fname(url)
                    return self.formatter.attachment_drawing(url, text, alt=text)
                else:
                    pass
            elif m.group('inter_wiki'):
                # interwiki link
                pass
#        return "".join(["{{", self.formatter.text(target), "}}"])
        url = wikiutil.url_unquote(node.content)
        return self.formatter.attachment_inlined(url, text)

# Not used
#    def drawing_emit(self, node):
#        url = wikiutil.url_unquote(node.content)
#        text = self.get_text(node)
#        return self.formatter.attachment_drawing(url, text)

# Not used
#    def figure_emit(self, node):
#        text = self.get_text(node)
#        url = wikiutil.url_unquote(node.content)
#        return ''.join([
#            self.formatter.rawHTML('<div class="figure">'),
#            self.get_image(url, text), self.emit_children(node),
#            self.formatter.rawHTML('</div>'),
#        ])

# Not used
#    def bad_link_emit(self, node):
#        return self.formatter.text(''.join([
#            '[[',
#            node.content or '',
#            ']]',
#        ]))

    def macro_emit(self, node):
        macro_name = node.content
        args = node.args
        return self.formatter.macro(self.macro, macro_name, args)

# Not used
#    def section_emit(self, node):
#        return ''.join([
#            self.formatter.rawHTML(
#                '<div class="%s" style="%s">' % (node.sect, node.style)),
#            self.emit_children(node),
#            self.formatter.rawHTML('</div>'),
#        ])

    def break_emit(self, node):
        return self.formatter.linebreak(preformatted=0)

# Not used
#    def blockquote_emit(self, node):
#        return ''.join([
#            self.formatter.rawHTML('<blockquote>'),
#            self.emit_children(node),
#            self.formatter.rawHTML('</blockquote>'),
#        ])

    def preformatted_emit(self, node):
        parser_name = getattr(node, 'sect', '')
        if parser_name:
            # The formatter.parser will *sometimes* just return the result
            # and *sometimes* try to write it directly. We need to take both
            # cases into account!
            lines = node.content.split(u'\n')
            buf = StringIO.StringIO()
            try:
                try:
                    self.request.redirect(buf)
                    ret = self.formatter.parser(parser_name, lines)
                finally:
                    self.request.redirect()
                buf.flush()
                writ = buf.getvalue()
                buf.close()
                return ret + writ
            except wikiutil.PluginMissingError:
                pass
        return ''.join([
            self.formatter.preformatted(1),
            self.formatter.text(node.content),
            self.formatter.preformatted(0),
        ])

    def default_emit(self, node):
        """Fallback function for emitting unknown nodes."""

        return ''.join([
            self.formatter.preformatted(1),
            self.formatter.text('<%s>\n' % node.kind),
            self.emit_children(node),
            self.formatter.preformatted(0),
        ])

    def emit_children(self, node):
        """Emit all the children of a node."""

        return ''.join([self.emit_node(child) for child in node.children])

    def emit_node(self, node):
        """Emit a single node."""

        emit = getattr(self, '%s_emit' % node.kind, self.default_emit)
        return emit(node)

    def emit(self):
        """Emit the document represented by self.root DOM tree."""

        # Try to disable 'smart' formatting if possible
        magic_save = getattr(self.formatter, 'no_magic', False)
        self.formatter.no_magic = True
        output = '\n'.join([
            self.emit_node(self.root),
        ])
        # restore 'smart' formatting if it was set
        self.formatter.no_magic = magic_save
        return output

del _
