# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - ReStructured Text Parser

    @copyright: 2004 Matthew Gilbert <gilbert AT voxmea DOT net>,
                2004 Alexander Schremmer <alex AT alexanderweb DOT de>
    @license: GNU GPL, see COPYING for details.

    REQUIRES docutils 0.3.10 or later (must be later than December 30th, 2005)
"""

import re
import new
import StringIO
import __builtin__
import sys

# docutils imports are below
from MoinMoin.parser.text_moin_wiki import Parser as WikiParser
from MoinMoin.Page import Page
from MoinMoin.action import AttachFile
from MoinMoin import wikiutil

Dependencies = [] # this parser just depends on the raw text

# --- make docutils safe by overriding all module-scoped names related to IO ---

# TODO: Add an error message to dummyOpen so that the user knows that
# they requested an unsupported feature of Docutils in MoinMoin.
def dummyOpen(x, y=None, z=None): return

class dummyIO(StringIO.StringIO):
    def __init__(self, destination=None, destination_path=None,
                 encoding=None, error_handler='', autoclose=1,
                 handle_io_errors=1, source_path=None):
        StringIO.StringIO.__init__(self)

class dummyUrllib2:
    def urlopen(a):
        return StringIO.StringIO()
    urlopen = staticmethod(urlopen)

# # # All docutils imports must be contained below here
try:
    import docutils
    from docutils.core import publish_parts
    from docutils.writers import html4css1
    from docutils.nodes import reference
    from docutils.parsers import rst
    from docutils.parsers.rst import directives, roles
# # # All docutils imports must be contained above here

    ErrorParser = None # used in the case of missing docutils
    docutils.io.FileOutput = docutils.io.FileInput = dummyIO
except ImportError:
    # we need to workaround this totally broken plugin interface that does
    # not allow us to raise exceptions
    class ErrorParser:
        caching = 0
        Dependencies = Dependencies # copy dependencies from module-scope

        def __init__(self, raw, request, **kw):
            self.raw = raw
            self.request = request

        def format(self, formatter):
            _ = self.request.getText
            from MoinMoin.parser.text import Parser as TextParser
            self.request.write(formatter.sysmsg(1) +
                               formatter.rawHTML(_('Rendering of reStructured text is not possible, please install Docutils.')) +
                               formatter.sysmsg(0))
            TextParser(self.raw, self.request).format(formatter)

    # Create a pseudo docutils environment
    docutils = html4css1 = dummyUrllib2()
    html4css1.HTMLTranslator = html4css1.Writer = object

def safe_import(name, globals = None, locals = None, fromlist = None):
    mod = __builtin__.__import__(name, globals, locals, fromlist)
    if mod:
        mod.open = dummyOpen
        mod.urllib2 = dummyUrllib2
    return mod

# Go through and change all docutils modules to use a dummyOpen and dummyUrllib2
# module. Also make sure that any docutils imported modules also get the dummy
# implementations.
for i in sys.modules.keys():
    if i.startswith('docutils') and sys.modules[i]:
        sys.modules[i].open = dummyOpen
        sys.modules[i].urllib2 = dummyUrllib2
        sys.modules[i].__import__ = safe_import

# --- End of dummy-code --------------------------------------------------------

def html_escape_unicode(node):
    # Find Python function that does this for me. string.encode('ascii',
    # 'xmlcharrefreplace') only 2.3 and above.
    for i in node:
        if ord(i) > 127:
            node = node.replace(i, '&#%d;' % (ord(i)))
    return node

class MoinWriter(html4css1.Writer):

    config_section = 'MoinMoin writer'
    config_section_dependencies = ('writers', )

    #"""Final translated form of `document`."""
    output = None

    def wiki_resolver(self, node):
        """
            Normally an unknown reference would be an error in an reST document.
            However, this is how new documents are created in the wiki. This
            passes on unknown references to eventually be handled by
            MoinMoin.
        """
        if hasattr(node, 'indirect_reference_name'):
            node['refuri'] = node.indirect_reference_name
        elif (len(node['ids']) != 0):
            # If the node has an id then it's probably an internal link. Let
            # docutils generate an error.
            return False
        elif node.hasattr('name'):
            node['refuri'] = node['name']
        else:
            node['refuri'] = node['refname']
        del node['refname']
        node.resolved = 1
        self.nodes.append(node)
        return True

    wiki_resolver.priority = 1

    def __init__(self, formatter, request):
        html4css1.Writer.__init__(self)
        self.formatter = formatter
        self.request = request
        # Add our wiki unknown_reference_resolver to our list of functions to
        # run when a target isn't found
        self.unknown_reference_resolvers = [self.wiki_resolver]
        # We create a new parser to process MoinMoin wiki style links in the
        # reST.
        self.wikiparser = WikiParser('', self.request)
        self.wikiparser.formatter = self.formatter
        self.wikiparser.hilite_re = None
        self.nodes = []
        # Make sure it's a supported docutils version.
        required_version = (0, 3, 10)
        current_version = tuple([int(i) for i in (docutils.__version__.split('.') + ['0', '0'])[:3]])
        if current_version < required_version:
            err = 'ERROR: The installed docutils version is %s;' % ('.'.join([str(i) for i in current_version]))
            err += ' version %s or later is required.' % ('.'.join([str(i) for i in required_version]))
            raise RuntimeError, err

    def translate(self):
        visitor = MoinTranslator(self.document,
                                 self.formatter,
                                 self.request,
                                 self.wikiparser,
                                 self)
        self.document.walkabout(visitor)
        self.visitor = visitor
        # Docutils 0.5 and later require the writer to have the visitor
        # attributes.
        if (hasattr(html4css1.Writer, 'visitor_attributes')):
            for attr in html4css1.Writer.visitor_attributes:
                setattr(self, attr, getattr(visitor, attr))
        self.output = html_escape_unicode(visitor.astext())

# mark quickhelp as translatable
_ = lambda x: x

class Parser:
    caching = 1
    Dependencies = Dependencies # copy dependencies from module-scope
    extensions = ['.rst', '.rest', ]
    quickhelp = _("""\
{{{
Emphasis: *italic* **bold** ``monospace``

Headings: Heading 1  Heading 2  Heading 3
          =========  ---------  ~~~~~~~~~

Horizontal rule: ----

Links: TrailingUnderscore_ `multi word with backticks`_ external_

.. _external: http://external-site.example.org/foo/

Lists: * bullets; 1., a. numbered items.
}}}
(!) For more help, see the
[[http://docutils.sourceforge.net/docs/user/rst/quickref.html|reStructuredText Quick Reference]].
""")

    def __init__(self, raw, request, **kw):
        self.raw = raw
        self.request = request
        self.form = request.form

    def format(self, formatter):
        # Create our simple parser
        parser = MoinDirectives(self.request)

        parts = publish_parts(
            source=self.raw,
            writer=MoinWriter(formatter, self.request),
            settings_overrides={
                'halt_level': 5,
                'traceback': True,
                'file_insertion_enabled': 0,
                'raw_enabled': 0,
                'stylesheet_path': '',
                'template': '',
            }
        )

        html = []
        if parts['title']:
            # Document title.
            html.append(formatter.rawHTML('<h1>%s</h1>' % parts['title']))
        # If there is only one subtitle it is propagated by Docutils
        # to a document subtitle and is held in parts['subtitle'].
        # However, if there is more than one subtitle then this is
        # empty and fragment contains all of the subtitles.
        if parts['subtitle']:
            html.append(formatter.rawHTML('<h2>%s</h2>' % parts['subtitle']))
        if parts['docinfo']:
            html.append(parts['docinfo'])
        html.append(parts['fragment'])
        self.request.write(html_escape_unicode('\n'.join(html)))

class RawHTMLList(list):
    """
        RawHTMLList catches all html appended to internal HTMLTranslator lists.
        It passes the HTML through the MoinMoin rawHTML formatter to strip
        markup when necessary. This is to support other formatting outputs
        (such as ?action=show&mimetype=text/plain).
    """

    def __init__(self, formatter):
        self.formatter = formatter

    def append(self, text):
        f = sys._getframe()
        if f.f_back.f_code.co_filename.endswith('html4css1.py'):
            if isinstance(text, (str, unicode)):
                text = self.formatter.rawHTML(text)
        list.append(self, text)

class MoinTranslator(html4css1.HTMLTranslator):

    def __init__(self, document, formatter, request, parser, writer):
        html4css1.HTMLTranslator.__init__(self, document)
        self.formatter = formatter
        self.request = request
        # Using our own writer when needed. Save the old one to restore
        # after the page has been processed by the html4css1 parser.
        self.original_write, self.request.write = self.request.write, self.capture_wiki_formatting
        self.wikiparser = parser
        self.wikiparser.request = request
        # MoinMoin likes to start the initial headers at level 3 and the title
        # gets level 2, so to comply with their styles, we do here also.
        # TODO: Could this be fixed by passing this value in settings_overrides?
        self.initial_header_level = 3
        # Temporary place for wiki returned markup. This will be filled when
        # replacing the default writer with the capture_wiki_formatting
        # function (see visit_image for an example).
        self.wiki_text = ''
        self.setup_wiki_handlers()
        self.setup_admonitions_handlers()

        # Make all internal lists RawHTMLLists, see RawHTMLList class
        # comment for more information.
        for i in self.__dict__:
            if isinstance(getattr(self, i), list):
                setattr(self, i, RawHTMLList(formatter))

    def depart_docinfo(self, node):
        """
            depart_docinfo assigns a new list to self.body, we need to re-make that
            into a RawHTMLList.
        """
        html4css1.HTMLTranslator.depart_docinfo(self, node)
        self.body = RawHTMLList(self.formatter)

    def capture_wiki_formatting(self, text):
        """
            Captures MoinMoin generated markup to the instance variable
            wiki_text.
        """
        # For some reason getting empty strings here which of course overwrites
        # what we really want (this is called multiple times per MoinMoin
        # format call, which I don't understand).
        self.wiki_text += text

    def process_wiki_text(self, text):
        """
            This sequence is repeated numerous times, so its captured as a
            single call here. Its important that wiki_text is blanked before we
            make the format call. format will call request.write which we've
            hooked to capture_wiki_formatting. If wiki_text is not blanked
            before a call to request.write we will get the old markup as well as
            the newly generated markup.

            TODO: Could implement this as a list so that it acts as a stack. I
            don't like having to remember to blank wiki_text.
        """
        self.wiki_text = ''
        self.wikiparser.raw = text
        self.wikiparser.format(self.formatter)

    def add_wiki_markup(self):
        """
            Place holder in case this becomes more elaborate someday. For now it
            only appends the MoinMoin generated markup to the html body and
            raises SkipNode.
        """
        self.body.append(self.wiki_text)
        self.wiki_text = ''
        raise docutils.nodes.SkipNode

    def astext(self):
        self.request.write = self.original_write
        return html4css1.HTMLTranslator.astext(self)

    def fixup_wiki_formatting(self, text):
        replacement = {'\n': '', '> ': '>'}
        for src, dst in replacement.items():
            text = text.replace(src, dst)
        # Fixup extraneous markup
        # Removes any empty span tags
        text = re.sub(r'\s*<\s*span.*?>\s*<\s*/\s*span\s*>', '', text)
        # Removes the first paragraph tag
        text = re.sub(r'^\s*<\s*p[^>]*?>', '', text)
        # Removes the ending paragraph close tag and any remaining whitespace
        text = re.sub(r'<\s*/\s*p\s*>\s*$', '', text)
        return text

    def visit_reference(self, node):
        """
            Pass links to MoinMoin to get the correct wiki space url. Extract
            the url and pass it on to the html4css1 writer to handle. Inline
            images are also handled by visit_image. Not sure what the "drawing:"
            link scheme is used for, so for now it is handled here.

            Also included here is a hack to allow MoinMoin macros. This routine
            checks for a link which starts with "<<". This link is passed to the
            MoinMoin formatter and the resulting markup is inserted into the
            document in the place of the original link reference.
        """
        if 'refuri' in node.attributes:
            refuri = node['refuri']
            prefix = ''
            link = refuri
            if ':' in refuri:
                prefix, link = refuri.lstrip().split(':', 1)

            # First see if MoinMoin should handle completely. Exits through add_wiki_markup.
            if refuri.startswith('<<') and refuri.endswith('>>'): # moin macro
                self.process_wiki_text(refuri)
                self.wiki_text = self.fixup_wiki_formatting(self.wiki_text)
                self.add_wiki_markup()

            if prefix == 'drawing':
                self.process_wiki_text("[[%s]]" % refuri)
                self.wiki_text = self.fixup_wiki_formatting(self.wiki_text)
                self.add_wiki_markup()

            # From here down, all links are handled by docutils (except
            # missing attachments), just fixup node['refuri'].
            if prefix == 'attachment':
                if not AttachFile.exists(self.request, self.request.page.page_name, link):
                    # Attachment doesn't exist, give to MoinMoin to insert upload text.
                    self.process_wiki_text("[[%s]]" % refuri)
                    self.wiki_text = self.fixup_wiki_formatting(self.wiki_text)
                    self.add_wiki_markup()
                # Attachment exists, just get a link to it.
                node['refuri'] = AttachFile.getAttachUrl(self.request.page.page_name, link, self.request)
                if not [i for i in node.children if i.__class__ == docutils.nodes.image]:
                    node['classes'].append(prefix)
            elif prefix == 'wiki':
                wiki_name, page_name = wikiutil.split_interwiki(link)
                wikitag, wikiurl, wikitail, err = wikiutil.resolve_interwiki(self.request, wiki_name, page_name)
                wikiurl = wikiutil.mapURL(self.request, wikiurl)
                node['refuri'] = wikiutil.join_wiki(wikiurl, wikitail)
                # Only add additional class information if the reference does
                # not have a child image (don't want to add additional markup
                # for images with targets).
                if not [i for i in node.children if i.__class__ == docutils.nodes.image]:
                    node['classes'].append('interwiki')
            elif prefix != '':
                # Some link scheme (http, file, https, mailto, etc.), add class
                # information if the reference doesn't have a child image (don't
                # want additional markup for images with targets).
                # Don't touch the refuri.
                if not [i for i in node.children if i.__class__ == docutils.nodes.image]:
                    node['classes'].append(prefix)
            else:
                # Default case - make a link to a wiki page.
                pagename, anchor = wikiutil.split_anchor(refuri)
                page = Page(self.request, wikiutil.AbsPageName(self.formatter.page.page_name, pagename))
                node['refuri'] = page.url(self.request, anchor=anchor)
                if not page.exists():
                    node['classes'].append('nonexistent')
        html4css1.HTMLTranslator.visit_reference(self, node)

    def visit_image(self, node):
        """
            Need to intervene in the case of inline images. We need MoinMoin to
            give us the actual src line to the image and then we can feed this
            to the default html4css1 writer. NOTE: Since the writer can't "open"
            this image the scale attribute doesn't work without directly
            specifying the height or width (or both).

            TODO: Need to handle figures similarly.
        """
        uri = node['uri'].lstrip()
        prefix = ''          # assume no prefix
        attach_name = uri
        if ':' in uri:
            prefix = uri.split(':', 1)[0]
            attach_name = uri.split(':', 1)[1]
        # if prefix isn't URL, try to display in page
        if not prefix.lower() in ('file', 'http', 'https', 'ftp'):
            if not AttachFile.exists(self.request, self.request.page.page_name, attach_name):
                # Attachment doesn't exist, MoinMoin should process it
                if prefix == '':
                    prefix = 'attachment:'
                self.process_wiki_text("{{%s%s}}" % (prefix, attach_name))
                self.wiki_text = self.fixup_wiki_formatting(self.wiki_text)
                self.add_wiki_markup()
            # Attachment exists, get a link to it.
            # create the url
            node['uri'] = AttachFile.getAttachUrl(self.request.page.page_name, attach_name, self.request, addts=1)
            if not node.hasattr('alt'):
                node['alt'] = node.get('name', uri)
        html4css1.HTMLTranslator.visit_image(self, node)

    def create_wiki_functor(self, moin_func):
        moin_callable = getattr(self.formatter, moin_func)
        def visit_func(self, node):
            self.wiki_text = ''
            self.request.write(moin_callable(1))
            self.body.append(self.wiki_text)
        def depart_func(self, node):
            self.wiki_text = ''
            self.request.write(moin_callable(0))
            self.body.append(self.wiki_text)
        return visit_func, depart_func

    def setup_wiki_handlers(self):
        """
            Have the MoinMoin formatter handle markup when it makes sense. These
            are portions of the document that do not contain reST specific
            markup. This allows these portions of the document to look
            consistent with other wiki pages.

            Setup dispatch routines to handle basic document markup. The
            hanlders dict is the html4css1 handler name followed by the wiki
            handler name.
        """
        handlers = {
            # Text Markup
            'emphasis': 'emphasis',
            'strong': 'strong',
            'literal': 'code',
            # Blocks
            'literal_block': 'preformatted',
            # Simple Lists
            # bullet-lists are handled completely by docutils because it uses
            # the node context to decide when to make a compact list
            # (no <p> tags).
            'list_item': 'listitem',
            # Definition List
            'definition_list': 'definition_list',
        }
        for rest_func, moin_func in handlers.items():
            visit_func, depart_func = self.create_wiki_functor(moin_func)
            visit_func = new.instancemethod(visit_func, self, MoinTranslator)
            depart_func = new.instancemethod(depart_func, self, MoinTranslator)
            setattr(self, 'visit_%s' % (rest_func), visit_func)
            setattr(self, 'depart_%s' % (rest_func), depart_func)

    # Enumerated list takes an extra paramter so we handle this differently
    def visit_enumerated_list(self, node):
        self.wiki_text = ''
        self.request.write(self.formatter.number_list(1, start=node.get('start', None)))
        self.body.append(self.wiki_text)

    def depart_enumerated_list(self, node):
        self.wiki_text = ''
        self.request.write(self.formatter.number_list(0))
        self.body.append(self.wiki_text)

    # Admonitions are handled here -=- tmacam
    def create_admonition_functor(self, admotion_class):
        def visit_func(self, node):
            self.wiki_text = ''
            self.request.write(self.formatter.div(1,
                                                  attr={'class': admotion_class},
                                                  allowed_attrs=[]))
            self.body.append(self.wiki_text)
        def depart_func(self, node):
            self.wiki_text = ''
            self.request.write(self.formatter.div(0))
            self.body.append(self.wiki_text)

        return visit_func, depart_func

    def setup_admonitions_handlers(self):
        """
            Admonitions are handled here... We basically surround admonitions
            in a div with class admonition_{name of the admonition}.
        """
        handled_admonitions = [
            'attention',
            'caution',
            'danger',
            'error',
            'hint',
            'important',
            'note',
            'tip',
            'warning',
        ]
        for adm in handled_admonitions:
            visit_func, depart_func = self.create_admonition_functor(adm)
            visit_func = new.instancemethod(visit_func, self, MoinTranslator)
            depart_func = new.instancemethod(depart_func, self, MoinTranslator)
            setattr(self, 'visit_%s' % (adm), visit_func)
            setattr(self, 'depart_%s' % (adm), depart_func)


class MoinDirectives:
    """
        Class to handle all custom directive handling. This code is called as
        part of the parsing stage.
    """

    def __init__(self, request):
        self.request = request

        # include MoinMoin pages
        directives.register_directive('include', self.include)

        # used for MoinMoin macros
        directives.register_directive('macro', self.macro)

        # disallow a few directives in order to prevent XSS
        # for directive in ('meta', 'include', 'raw'):
        for directive in ('meta', 'raw'):
            directives.register_directive(directive, None)

        # disable the raw role
        roles._roles['raw'] = None

        # As a quick fix for infinite includes we only allow a fixed number of
        # includes per page
        self.num_includes = 0
        self.max_includes = 10

    # Handle the include directive rather than letting the default docutils
    # parser handle it. This allows the inclusion of MoinMoin pages instead of
    # something from the filesystem.
    def include(self, name, arguments, options, content, lineno,
                content_offset, block_text, state, state_machine):
        # content contains the included file name

        _ = self.request.getText

        # Limit the number of documents that can be included
        if self.num_includes < self.max_includes:
            self.num_includes += 1
        else:
            lines = [_("**Maximum number of allowed includes exceeded**")]
            state_machine.insert_input(lines, 'MoinDirectives')
            return

        if len(content):
            pagename = content[0]
            page = Page(page_name=pagename, request=self.request)
            if not self.request.user.may.read(pagename):
                lines = [_("**You are not allowed to read the page: %s**") % (pagename, )]
            else:
                if page.exists():
                    text = page.get_raw_body()
                    lines = text.split('\n')
                    # Remove the "#format rst" line
                    if lines[0].startswith("#format"):
                        del lines[0]
                else:
                    lines = [_("**Could not find the referenced page: %s**") % (pagename, )]
            # Insert the text from the included document and then continue parsing
            state_machine.insert_input(lines, 'MoinDirectives')
        return

    include.has_content = include.content = True
    include.option_spec = {}
    include.required_arguments = 1
    include.optional_arguments = 0

    # Add additional macro directive.
    # This allows MoinMoin macros to be used either by using the directive
    # directly or by using the substitution syntax. Much cleaner than using the
    # reference hack (`<<SomeMacro>>`_). This however simply adds a node to the
    # document tree which is a reference, but through a much better user
    # interface.
    def macro(self, name, arguments, options, content, lineno,
                content_offset, block_text, state, state_machine):
        # content contains macro to be called
        if len(content):
            # Allow either with or without brackets
            if content[0].startswith('<<'):
                macro = content[0]
            else:
                macro = '<<%s>>' % content[0]
            ref = reference(macro, refuri=macro)
            ref['name'] = macro
            return [ref]
        return

    macro.has_content = macro.content = True
    macro.option_spec = {}
    macro.required_arguments = 1
    macro.optional_arguments = 0

if ErrorParser: # fixup in case of missing docutils
    Parser = ErrorParser

del _
