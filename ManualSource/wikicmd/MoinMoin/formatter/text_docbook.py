# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - DocBook Formatter

    @copyright: 2005,2008 by Mikko Virkkilä <mvirkkil@cc.hut.fi>
    @copyright: 2005 by MoinMoin:AlexanderSchremmer (small modifications)
    @copyright: 2005 by MoinMoin:Petr Pytelka <pyta@lightcomp.com> (small modifications)

    @license: GNU GPL, see COPYING for details.
"""

import os, re

from xml.dom import getDOMImplementation
from xml.dom.ext.reader import Sax
from xml.dom.ext import Node

from MoinMoin.formatter import FormatterBase
from MoinMoin import wikiutil
from MoinMoin.error import CompositeError
from MoinMoin.action import AttachFile

#For revision history
from MoinMoin.logfile import editlog
from MoinMoin import user


class InternalError(CompositeError):
    pass

try:
    dom = getDOMImplementation("4DOM")
except ImportError:
    raise InternalError("You need to install 4suite to use the DocBook formatter.")


class Formatter(FormatterBase):
    #TODO: How to handle revision history and other meta-info from included files?
    #      The problem is that we don't know what the original page is, since
    #      the Inlcude-macro doesn't pass us the information.

    # this list is extended as the page is parsed. Could be optimized by adding them here?
    section_should_break = ['abstract', 'para', 'emphasis']

    blacklisted_macros = ('TableOfContents', 'ShowSmileys', 'Navigation')

    # If the current node is one of the following and we are about the emit
    # text, the text should be wrapped in a paragraph
    wrap_text_in_para = ('listitem', 'glossdef', 'article', 'chapter', 'tip', 'warning', 'note', 'caution', 'important')

    # from dtd
    _can_contain_section = ("section", "appendix", "article", "chapter", "patintro", "preface")

    def __init__(self, request, doctype="article", **kw):
        FormatterBase.__init__(self, request, **kw)
        self.request = request

        '''
        If the formatter is used by the Include macro, it will set
        is_included=True in which case we know we need to call startDocument
        and endDocument from startContent and endContent respectively, since
        the Include macro will not be calling them, and the formatter doesn't
        work properly unless they are called.
        '''
        if kw.has_key("is_included") and kw["is_included"]:
            self.include_kludge = True
        else:
            self.include_kludge = False

        self.doctype = doctype
        self.curdepth = 0
        self.cur = None

    def startDocument(self, pagename):
        self.doc = dom.createDocument(None, self.doctype, dom.createDocumentType(
            self.doctype, "-//OASIS//DTD DocBook XML V4.4//EN",
            "http://www.docbook.org/xml/4.4/docbookx.dtd"))

        self.title = pagename
        self.root = self.doc.documentElement

        if not self.include_kludge and self.doctype == "article":
            info = self.doc.createElement("articleinfo")
            self.root.appendChild(info)
            self._addTitleElement(self.title, targetNode=info)
            self._addRevisionHistory(targetNode=info)
        else:
            self._addTitleElement(self.title, targetNode=self.root)

        self.cur = self.root
        return ""

    def startContent(self, content_id="content", **kw):
        if self.include_kludge and not self.cur:
            return self.startDocument("OnlyAnIdiotWouldCreateSuchaPage")
        return ""

    def endContent(self):
        if self.include_kludge:
            return self.endDocument()
        return ""

    def endDocument(self):
        from xml.dom.ext import PrettyPrint, Print
        import StringIO

        f = StringIO.StringIO()
        Print(self.doc, f)
        txt = f.getvalue()
        f.close()

        self.cur = None
        return txt

    def text(self, text, **kw):
        if text == "\\n":
            srcText = "\n"
        else:
            srcText = text

        if srcText and self._isInsidePreformatted():

            if self.cur.lastChild is not None and self.cur.lastChild.nodeType == Node.CDATA_SECTION_NODE:
                # We can add it to a previous CDATA section
                self.cur.lastChild.nodeValue = self.cur.lastChild.nodeValue + srcText
            else:
                # We create a new cdata section
                self.cur.appendChild(self.doc.createCDATASection(srcText))

        elif self.cur.nodeName in self.wrap_text_in_para:
            """
            If we already wrapped one text item in a para, we should add to that para
            and not create a new one. Another question is if we should add a space?
            """
            if self.cur.lastChild is not None and self.cur.lastChild.nodeName == 'para':
                self.cur.lastChild.appendChild(self.doc.createTextNode(srcText))
            else:
                self.paragraph(1)
                self.text(text)
                self.paragraph(0)
        else:
            self.cur.appendChild(self.doc.createTextNode(srcText))
        return ""

    def heading(self, on, depth, **kw):
        while self.cur.nodeName in self.section_should_break:
            self.cur = self.cur.parentNode

        if on:
            # try to go to higher level if needed
            if depth <= self.curdepth:
                # number of levels we want to go higher
                numberOfLevels = self.curdepth - depth + 1
                for dummy in range(numberOfLevels):
                    # find first non section node
                    while not self.cur.nodeName in self._can_contain_section:
                        self.cur = self.cur.parentNode

                    if self.cur.nodeName == "section":
                        self.cur = self.cur.parentNode

            section = self.doc.createElement("section")
            self.cur.appendChild(section)
            self.cur = section

            title = self.doc.createElement("title")
            self.cur.appendChild(title)
            self.cur = title
            self.curdepth = depth
        else:
            self.cur = self.cur.parentNode

        return ""

    def paragraph(self, on, **kw):
        FormatterBase.paragraph(self, on)

        # Let's prevent empty paras
        if not on:
            if not self._hasContent(self.cur):
                oldnode = self.cur
                self.cur = oldnode.parentNode
                self.cur.removeChild(oldnode)
                return ""

        # Let's prevent para inside para
        if on and self.cur.nodeName == "para":
            return ""
        return self._handleNode("para", on)

    def linebreak(self, preformatted=1):
        """
        If preformatted, it will simply output a linebreak.
        If we are in a paragraph, we will close it, and open another one.
        """
        if preformatted:
            self.text('\\n')
        elif self.cur.nodeName == "para":
            self.paragraph(0)
            self.paragraph(1)
        else:
            self._emitComment("Warning: Probably not emitting right sort of linebreak")
            self.text('\n')
        return ""

### Inline ##########################################################

    def strong(self, on, **kw):
        return self._handleFormatting("emphasis", on, (('role', 'strong'), ))

    def emphasis(self, on, **kw):
        return self._handleFormatting("emphasis", on)

    def underline(self, on, **kw):
        return self._handleFormatting("emphasis", on, (('role', 'underline'), ))

    def highlight(self, on, **kw):
        return self._handleFormatting("emphasis", on, (('role', 'highlight'), ))

    def sup(self, on, **kw):
        return self._handleFormatting("superscript", on)

    def sub(self, on, **kw):
        return self._handleFormatting("subscript", on)

    def strike(self, on, **kw):
        # does not yield <strike> using the HTML XSLT files here ...
        # but seems to be correct
        return self._handleFormatting("emphasis", on,
                                      (('role', 'strikethrough'), ))

    def code(self, on, **kw):
        # Let's prevent empty code
        if not on:
            if not self._hasContent(self.cur):
                oldnode = self.cur
                self.cur = oldnode.parentNode
                self.cur.removeChild(oldnode)
                return ""
        return self._handleFormatting("code", on)

    def preformatted(self, on, **kw):
        return self._handleFormatting("screen", on)


### Lists ###########################################################

    def number_list(self, on, type=None, start=None, **kw):
        docbook_ol_types = {'1': "arabic",
                            'a': "loweralpha",
                            'A': "upperalpha",
                            'i': "lowerroman",
                            'I': "upperroman"}

        if type and docbook_ol_types.has_key(type):
            attrs = [("numeration", docbook_ol_types[type])]
        else:
            attrs = []

        return self._handleNode('orderedlist', on, attrs)

    def bullet_list(self, on, **kw):
        return self._handleNode("itemizedlist", on)

    def listitem(self, on, style=None, **kw):
        if self.cur.nodeName == "glosslist" or self.cur.nodeName == "glossentry":
            return self.definition_desc(on)
        if on and self.cur.nodeName == "listitem":
            """If we are inside a listitem, and someone wants to create a new one, it
            means they forgot to close the old one, and we need to do it for them."""
            self.listitem(0)

        args = []
        if on and style:
            styles = self._convertStylesToDict(style)
            if styles.has_key('list-style-type'):
                args.append(('override', styles['list-style-type']))

        return self._handleNode("listitem", on, attributes=args)

    def definition_list(self, on, **kw):
        return self._handleNode("glosslist", on)

    def definition_term(self, on, compact=0, **kw):
        if on:
            self._handleNode("glossentry", on)
            self._handleNode("glossterm", on)
        else:
            if self._hasContent(self.cur):
                self._handleNode("glossterm", on)
                self._handleNode("glossentry", on)
            else:
                # No term info :(
                term = self.cur
                entry = term.parentNode
                self.cur = entry.parentNode
                self.cur.removeChild(entry)
        return ""

    def definition_desc(self, on, **kw):
        if on:
            if self.cur.nodeName == "glossentry":
                # Good, we can add it here.
                self._handleNode("glossdef", on)
                return ""

            # We are somewhere else, let's see...
            if self.cur.nodeName != "glosslist":
                self._emitComment("Trying to add a definition, but we arent in a glosslist")
                return ""
            if not self.cur.lastChild or self.cur.lastChild.nodeName != "glossentry":
                self._emitComment("Trying to add a definition, but there is no entry")
                return ""

            # Found it, calling again
            self.cur = self.cur.lastChild
            return self.definition_desc(on)
        else:
            if not self._hasContent(self.cur):
                # Seems no valuable info was added
                assert(self.cur.nodeName == "glossdef")
                toRemove = self.cur
                self.cur = toRemove.parentNode
                self.cur.removeChild(toRemove)

            while self.cur.nodeName != "glosslist":
                self.cur = self.cur.parentNode
        return ""

### Links ###########################################################
    # TODO: Fix anchors to documents which are included. Needs probably to be
    #       a postprocessing rule. Could be done by having the anchors have
    #       the "linkend" value of PageName#anchor. Then at post process the
    #       following would be done for all urls:
    #        - get all ulinks with an anchor part in their url
    #        - get the ulink's PageName#anchor -part by removing baseurl part
    #        - if any of our <anchor> elements have the same PageName#anchor
    #          value as our <ulink>, then replace the ulink with a link
    #          element.
    #       Note: This would the case when someone wants to link to a
    #             section on the original webpage impossible. The link would
    #             instead point within the docbook page and not to the webpage.


    def pagelink(self, on, pagename='', page=None, **kw):
        FormatterBase.pagelink(self, on, pagename, page, **kw)
        return self.interwikilink(on, 'Self', pagename, **kw)

    def interwikilink(self, on, interwiki='', pagename='', **kw):
        if not on:
            return self.url(on, **kw)

        wikitag, wikiurl, wikitail, wikitag_bad = wikiutil.resolve_interwiki(self.request, interwiki, pagename)
        wikiurl = wikiutil.mapURL(self.request, wikiurl)
        href = wikiutil.join_wiki(wikiurl, wikitail)
        if kw.has_key("anchor"):
            href="%s#%s"%(href, kw['anchor'])

        if pagename == self.page.page_name:
            kw['is_self']=True

        return self.url(on, href, **kw)

    def url(self, on, url=None, css=None, **kw):
        if url and url.startswith("/"):
            # convert to absolute path:
            url = "%s%s"%(self.request.base_url, url)

        if not on:
            self._cleanupUlinkNode()

        if kw.has_key("anchor") and kw.has_key("is_self") and kw["is_self"]:
            #handle the case where we are pointing to somewhere insidee our own document
            return self._handleNode("link", on, attributes=(('linkend', kw["anchor"]), ))
        else:
            return self._handleNode("ulink", on, attributes=(('url', url), ))

    def anchordef(self, name):
        self._handleNode("anchor", True, attributes=(('id', name), ))
        self._handleNode("anchor", False)
        return ""

    def anchorlink(self, on, name='', **kw):
        linkid = kw.get('id', None)
        attrs = []
        if name != '':
            attrs.append(('endterm', name))
        if id is not None:
            attrs.append(('linkend', linkid))
        elif name != '':
            attrs.append(('linkend', name))

        return self._handleNode("link", on, attrs)

### Attachments ######################################################

    def attachment_link(self, on, url=None, **kw):
        assert on in (0, 1, False, True) # make sure we get called the new way, not like the 1.5 api was
        # we do not output a "upload link" when outputting docbook
        if on:
            pagename, filename = AttachFile.absoluteName(url, self.page.page_name)
            fname = wikiutil.taintfilename(filename)
            target = AttachFile.getAttachUrl(pagename, filename, self.request)
            return self.url(1, target, title="attachment:%s" % url)
        else:
            return self.url(0)

    def attachment_image(self, url, **kw):
        """
        Figures out the absolute path to the image and then hands over to
        the image function. Any title is also handed over, and an additional
        title suggestion is made based on filename. The image function will
        use the suggestion if no other text alternative is found.

        If the file is not found, then a simple text will replace it.
        """
        _ = self.request.getText
        pagename, filename = AttachFile.absoluteName(url, self.page.page_name)
        fname = wikiutil.taintfilename(filename)
        fpath = AttachFile.getFilename(self.request, pagename, fname)
        if not os.path.exists(fpath):
            return self.text("[attachment:%s]" % url)
        else:
            return self.image(
                src=AttachFile.getAttachUrl(pagename, filename, self.request, addts=1),
                attachment_title=url,
                **kw)


    def attachment_drawing(self, url, text, **kw):
        _ = self.request.getText
        pagename, filename = AttachFile.absoluteName(url, self.page.page_name)
        fname = wikiutil.taintfilename(filename)
        drawing = fname
        fname = fname + ".png"
        filename = filename + ".png"
        fpath = AttachFile.getFilename(self.request, pagename, fname)
        if not os.path.exists(fpath):
            return self.text("[drawing:%s]" % url)
        else:
            src = AttachFile.getAttachUrl(pagename, filename, self.request, addts=1)
            return self.image(alt=drawing, src=src, html_class="drawing")

### Images and Smileys ##############################################

    def image(self, src=None, **kw):
        if src:
            kw['src'] = src
        media = self.doc.createElement('inlinemediaobject')

        imagewrap = self.doc.createElement('imageobject')
        media.appendChild(imagewrap)

        image = self.doc.createElement('imagedata')
        if kw.has_key('src'):
            src = kw['src']
            if src.startswith("/"):
                # convert to absolute path:
                src = self.request.url_root + src
            image.setAttribute('fileref', src)
        if kw.has_key('width'):
            image.setAttribute('width', str(kw['width']))
        if kw.has_key('height'):
            image.setAttribute('depth', str(kw['height']))
        imagewrap.appendChild(image)

        # Look for any suitable title, order is important.
        title = ''
        for a in ('title', 'html_title', 'alt', 'html_alt', 'attachment_title'):
            if kw.has_key(a):
                title = kw[a]
                break
        if title:
            txtcontainer = self.doc.createElement('textobject')
            self._addTextElem(txtcontainer, "phrase", title)
            media.appendChild(txtcontainer)

        self.cur.appendChild(media)
        return ""

    def transclusion(self, on, **kw):
        # TODO, see text_html formatter
        self._emitComment('transclusion is not implemented in DocBook formatter')
        return ""

    def transclusion_param(self, **kw):
        # TODO, see text_html formatter
        self._emitComment('transclusion parameters are not implemented in DocBook formatter')
        return ""

    def smiley(self, text):
        return self.request.theme.make_icon(text)

    def icon(self, type):
        return '' # self.request.theme.make_icon(type)


### Code area #######################################################

    def code_area(self, on, code_id, code_type=None, show=0, start=-1, step=-1, msg=None):
        """Creates a formatted code region using screen or programlisting,
        depending on if a programming language was defined (code_type).

        The code_id is not used for anything in this formatter, but is just
        there to remain compatible with the HTML formatter's function.

        Line numbering is supported natively by DocBook so if linenumbering
        is requested the relevant attribute will be set.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """

        if not on:
            return self._handleNode(None, on)

        show = show and 'numbered' or 'unnumbered'
        if start < 1:
            start = 1

        programming_languages = {"ColorizedJava": "java",
                                 "ColorizedPython": "python",
                                 "ColorizedCPlusPlus": "c++",
                                 "ColorizedPascal": "pascal",
                                }

        if code_type is None:
            attrs = (('linenumbering', show),
                     ('startinglinenumber', str(start)),
                     ('format', 'linespecific'),
                     )
            return self._handleNode("screen", on, attributes=attrs)
        else:
            if programming_languages.has_key(code_type):
                code_type = programming_languages[code_type]

            attrs = (('linenumbering', show),
                     ('startinglinenumber', str(start)),
                     ('language', code_type),
                     ('format', 'linespecific'),
                     )
            return self._handleNode("programlisting", on, attributes=attrs)

    def code_line(self, on):
        if on:
            self.cur.appendChild(self.doc.createTextNode('\n'))
        return ''

    def code_token(self, on, tok_type):
        """
        DocBook has some support for semantic annotation of code so the
        known tokens will be mapped to DocBook entities.
        """
        toks_map = {'ID': 'methodname',
                    'Operator': '',
                    'Char': '',
                    'Comment': 'lineannotation',
                    'Number': '',
                    'String': 'phrase',
                    'SPChar': '',
                    'ResWord': 'token',
                    'ConsWord': 'symbol',
                    'Error': 'errortext',
                    'ResWord2': 'type',
                    'Special': '',
                    'Preprc': '',
                    'Text': '',
                   }
        if toks_map.has_key(tok_type) and toks_map[tok_type]:
            return self._handleFormatting(toks_map[tok_type], on)
        else:
            return ""
### Macro ###########################################################

    def macro(self, macro_obj, name, args, markup=None):
        """As far as the DocBook formatter is conserned there are three
        kinds of macros: Bad, Handled and Unknown.

        The Bad ones are the ones that are known not to work, and are on its
        blacklist. They will be ignored and an XML comment will be written
        noting that the macro is not supported.

        Handled macros are such macros that code is written to handle them.
        For example for the FootNote macro it means that instead of executing
        the macro, a DocBook footnote entity is created, with the relevant
        pieces of information filles in.

        The Unknown are handled by executing the macro and capturing any
        textual output. There shouldn't be any textual output since macros
        should call formatter methods. This is unfortunately not always true,
        so the output it is then fed in to an xml parser and the
        resulting nodes copied to the DocBook-dom tree. If the output is not
        valid xml then a comment is written in the DocBook that the macro
        should be fixed.

        """
        # Another alternative would be to feed the output to rawHTML or even
        # combining these two approaches. The _best_ alternative would be to
        # fix the macros.
        excludes=("articleinfo", "title")

        if name in self.blacklisted_macros:
            self._emitComment("The macro %s doesn't work with the DocBook formatter." % name)

        elif name == "FootNote":
            footnote = self.doc.createElement('footnote')
            self._addTextElem(footnote, "para", str(args))
            self.cur.appendChild(footnote)

        elif name == "Include":
            was_in_para = self.cur.nodeName == "para"
            if was_in_para:
                self.paragraph(0)

            # Regular Expression to match editlink arg, remove it because it causes trouble.
            _arg_editlink = r'(,\s*(?P<editlink>editlink))?'
            macro_args = re.sub(_arg_editlink, '', args)

            text = FormatterBase.macro(self, macro_obj, name, macro_args)
            if text.strip():
                self._copyExternalNodes(Sax.FromXml(text).documentElement.childNodes, exclude=excludes)
            if was_in_para:
                self.paragraph(1)

        else:
            text = FormatterBase.macro(self, macro_obj, name, args)
            if text:
                from xml.parsers.expat import ExpatError
                try:
                    xml_dom = Sax.FromXml(text).documentElement.childNodes
                    self._copyExternalNodes(xml_dom, exclude=excludes)
                except ExpatError:
                    self._emitComment("The macro %s caused an error and should be blacklisted. It returned the data '%s' which caused the docbook-formatter to choke. Please file a bug." % (name, text))

        return u""

### Util functions ##################################################

    def _copyExternalNodes(self, nodes, deep=1, target=None, exclude=()):
        if not target:
            target = self.cur

        for node in nodes:
            if node.nodeName in exclude:
                pass
            elif target.nodeName == "para" and node.nodeName == "para":
                self._copyExternalNodes(node.childNodes, target=target)
                self.cur = target.parentNode
            else:
                target.appendChild(self.doc.importNode(node, deep))

    def _emitComment(self, text):
        text = text.replace("--", "- -") # There cannot be "--" in XML comment
        self.cur.appendChild(self.doc.createComment(text))

    def _handleNode(self, name, on, attributes=()):
        if on:
            node = self.doc.createElement(name)
            self.cur.appendChild(node)
            if len(attributes) > 0:
                for name, value in attributes:
                    node.setAttribute(name, value)
            self.cur = node
        else:
            """
                Because we prevent para inside para, we might get extra "please
                exit para" when we are no longer inside one.

                TODO: Maybe rethink the para in para case
            """
            if name == "para" and self.cur.nodeName != "para":
                return ""

            self.cur = self.cur.parentNode
        return ""

    def _handleFormatting(self, name, on, attributes=()):
        # We add all the elements we create to the list of elements that should not contain a section
        if name not in self.section_should_break:
            self.section_should_break.append(name)
        return self._handleNode(name, on, attributes)

    def _isInsidePreformatted(self):
        """Walks all parents and checks if one is of a preformatted type, which
           means the child would need to be preformatted == embedded in a cdata
           section"""
        n = self.cur
        while n:
            if n.nodeName in ("screen", "programlisting"):
                return True
            n = n.parentNode
        return False

    def _hasContent(self, node):
        if node.attributes and len(node.attributes):
            return True
        for child in node.childNodes:
            if child.nodeType == Node.TEXT_NODE and child.nodeValue.strip():
                return True
            elif child.nodeType == Node.CDATA_SECTION_NODE and child.nodeValue.strip():
                return True

            if self._hasContent(child):
                return True
        return False

    def _addTitleElement(self, titleTxt, targetNode=None):
        if not targetNode:
            targetNode = self.cur
        self._addTextElem(targetNode, "title", titleTxt)

    def _convertStylesToDict(self, styles):
        '''Takes the CSS styling information and converts it to a dict'''
        attrs = {}
        for s in styles.split(";"):
            if s.strip(' "') == "":
                continue
            if ":" not in s:
                continue
            (key, value) = s.split(":", 1)
            key = key.strip(' "')
            value = value.strip(' "')

            if key == 'vertical-align':
                key = 'valign'
            elif key == 'text-align':
                key = 'align'
            elif key == 'background-color':
                key = 'bgcolor'

            attrs[key] = value
        return attrs

    def _cleanupUlinkNode(self):
        """
        Moin adds the url as the text to a link, if no text is specified.
        Docbook does it when a docbook is rendered, so we don't want moin to
        do it and so if the url is exactly the same as the text node inside
        the ulink, we remove the text node.
        """
        if self.cur.nodeName == "ulink" and len(self.cur.childNodes) == 1 \
                and self.cur.firstChild.nodeType == Node.TEXT_NODE \
                and self.cur.firstChild.nodeValue.strip() == self.cur.getAttribute('url').strip():
            self.cur.removeChild(self.cur.firstChild)

    def _addTextElem(self, target, elemName, text):
        """
        Creates an element of the name elemName and adds a text node to it
        with the nodeValue of text. The new element is then added as a child
        to the element target.
        """
        newElement = self.doc.createElement(elemName)
        newElement.appendChild(self.doc.createTextNode(text))
        target.appendChild(newElement)


    def _addRevisionHistory(self, targetNode):
        """
        This will generate a revhistory element which it will populate with
        revision nodes. Each revision has the revnumber, date and author-
        initial elements, and if a comment was supplied, the comment element.

        The date elements format depends on the users settings, so it will
        be in the same format as the revision history as viewed in the
        page info on the wiki.

        The authorinitials will be the UserName or if it was an anonymous
        edit, then it will be the hostname/ip-address.

        The revision history of included documents is NOT included at the
        moment due to technical difficulties.
        """
        _ = self.request.getText
        log = editlog.EditLog(self.request, rootpagename=self.title)
        user_cache = {}

        history = self.doc.createElement("revhistory")

        # read in the complete log of this page
        for line in log.reverse():
            if not line.action in ('SAVE', 'SAVENEW', 'SAVE/REVERT', 'SAVE/RENAME', ):
                #Let's ignore adding of attachments
                continue
            revision = self.doc.createElement("revision")

            # Revision number (without preceeding zeros)
            self._addTextElem(revision, "revnumber", line.rev.lstrip('0'))

            # Date of revision
            date_text = self.request.user.getFormattedDateTime(
                wikiutil.version2timestamp(line.ed_time_usecs))
            self._addTextElem(revision, "date", date_text)

            # Author or revision
            if not (line.userid in user_cache):
                user_cache[line.userid] = user.User(self.request, line.userid, auth_method="text_docbook:740")
            author = user_cache[line.userid]
            if author and author.name:
                self._addTextElem(revision, "authorinitials", author.name)
            else:
                self._addTextElem(revision, "authorinitials", line.hostname)

            # Comment from author of revision
            comment = line.comment
            if not comment:
                if '/REVERT' in line.action:
                    comment = _("Revert to revision %(rev)d.") % {'rev': int(line.extra)}
                elif '/RENAME' in line.action:
                    comment = _("Renamed from '%(oldpagename)s'.") % {'oldpagename': line.extra}
            if comment:
                self._addTextElem(revision, "revremark", comment)

            history.appendChild(revision)

        if history.firstChild:
            #only add revision history is there is history to add
            targetNode.appendChild(history)

### Not supported ###################################################

    def rule(self, size=0, **kw):
        self._emitComment('rule (<hr>) is not applicable to DocBook')
        return ""

    def small(self, on, **kw):
        if on:
            self._emitComment('"~-smaller-~" is not applicable to DocBook')
        return ""

    def big(self, on, **kw):
        if on:
            self._emitComment('"~+bigger+~" is not applicable to DocBook')
        return ""

    def rawHTML(self, markup):
        if markup.strip() == "":
            return ""

        if "<" not in markup and ">" not in markup:
            # Seems there are no tags.
            # Let's get all the "entity references".
            cleaned = markup
            import re
            entities = re.compile("&(?P<e>[a-zA-Z]+);").findall(cleaned)
            from htmlentitydefs import name2codepoint
            for ent in entities:
                if name2codepoint.has_key(ent):
                    cleaned = cleaned.replace("&%s;" % ent, unichr(name2codepoint[ent]))

            # Then we replace all escaped unicodes.
            escapedunicodes = re.compile("&#(?P<h>[0-9]+);").findall(markup)
            for uni in escapedunicodes:
                cleaned = cleaned.replace("&#%s;" % uni, unichr(int(uni)))

            self.text(cleaned)

        self._emitComment("RAW HTML: "+markup)
        return ""

    def div(self, on, **kw):
        """A div cannot really be supported in DocBook as it carries no
        semantic meaning, but the special cases can be handled when the class
        of the div carries the information.

        A dictionary is used for mapping between class names and the
        corresponding DocBook element.

        A MoinMoin comment is represented in DocBook by the remark element.

        The rest of the known classes are the admonitions in DocBook:
        warning, caution, important, note and hint

        Note: The remark entity can only contain inline elements, so it is
              very likely that the use of a comment div will produce invalid
              DocBook.
        """
        # Map your styles to docbook elements.
        # Even though comment is right now the only one that needs to be
        # mapped, having two different ways is more complicated than having
        # a single common way. Code clarity and generality first, especially
        # since we might want to do more div to docbook mappings in the future.
        class_to_docbook = {"warning":   "warning",
                            "caution":   "caution",
                            "important": "important",
                            "note":      "note",
                            "tip":       "tip",
                            "comment":   "remark"}

        if on and kw.get('css_class'):
            css_classes = kw.get('css_class').split()
            for style in class_to_docbook.keys():
                if style in css_classes:
                    return self._handleNode(class_to_docbook[style], on)

        elif not on:
            if self.cur.nodeName in class_to_docbook.values():
                return self._handleNode(self.cur.nodeName, on)

        return ""

    def span(self, on, **kw):
        """A span cannot really be supported in DocBook as it carries no
        semantic meaning, but the special case of a comment can be handled.

        A comment is represented in DocBook by the remark element.

        A comment span is recognized by the fact that it has the class
        "comment". Other cases of div use are ignored.
        """
        css_class = kw.get('css_class')
        if on and css_class and 'comment' in css_class.split():
            self._handleFormatting("remark", on)
        if not on and self.cur.nodeName == "remark":
            self._handleFormatting("remark", on)
        return ""



### Tables ##########################################################

    def table(self, on, attrs=(), **kw):
        if(on):
            if attrs:
                self.curtable = Table(self, self.doc, self.cur, dict(attrs))
            else:
                self.curtable = Table(self, self.doc, self.cur)
            self.cur = self.curtable.tableNode
        else:
            self.cur = self.curtable.finalizeTable()
            self.curtable = None
        return ""

    def table_row(self, on, attrs=(), **kw):
        if(on):
            if attrs:
                self.curtable.addRow(dict(attrs))
            else:
                self.cur = self.curtable.addRow()
        return ""

    def table_cell(self, on, attrs=(), **kw):
        if(on):
            if attrs:
                self.cur = self.curtable.addCell(dict(attrs))
            else:
                self.cur = self.curtable.addCell()
        return ""

class Table:
    '''The Table class is used as a helper for collecting information about
    what kind of table we are building. When all relelvant data is gathered
    it calculates the different spans of the cells and columns.

    Note that it expects all arguments to be passed in a dict.
    '''

    def __init__(self, formatter, doc, parent, argsdict={}):
        self.formatter = formatter
        self.doc = doc

        self.tableNode = self.doc.createElement('informaltable')
        parent.appendChild(self.tableNode)
        self.colWidths = {}
        self.tgroup = self.doc.createElement('tgroup')
        # Bug in yelp, the two lines below don't affect rendering
        #self.tgroup.setAttribute('rowsep', '1')
        #self.tgroup.setAttribute('colsep', '1')
        self.curColumn = 0
        self.maxColumn = 0
        self.row = None
        self.tableNode.appendChild(self.tgroup)

        self.tbody = self.doc.createElement('tbody') # Note: This gets appended in finalizeTable

    def finalizeTable(self):
        """Calculates the final width of the whole table and the width of each
        column. Adds the colspec-elements and applies the colwidth attributes.
        Inserts the tbody element to the tgroup and returns the tables container
        element.

        A lot of the information is gathered from the style attributes passed
        to the functions
        """
        self.tgroup.setAttribute('cols', str(self.maxColumn))
        for colnr in range(0, self.maxColumn):
            colspecElem = self.doc.createElement('colspec')
            colspecElem.setAttribute('colname', 'col_%s' % str(colnr))
            if self.colWidths.has_key(str(colnr)) and self.colWidths[str(colnr)] != "1*":
                colspecElem.setAttribute('colwidth', self.colWidths[str(colnr)])
            self.tgroup.appendChild(colspecElem)
        self.tgroup.appendChild(self.tbody)
        return self.tableNode.parentNode

    def addRow(self, argsdict={}):
        self.curColumn = 0
        self.row = self.doc.createElement('row')
        # Bug in yelp, doesn't affect the outcome.
        self.row.setAttribute("rowsep", "1") #Rows should have lines between them
        self.tbody.appendChild(self.row)
        return self.row

    def addCell(self, argsdict={}):
        if 'style' in argsdict:
            argsdict.update(self.formatter._convertStylesToDict(argsdict['style'].strip('"')))

        cell = self.doc.createElement('entry')
        cell.setAttribute('rowsep', '1')
        cell.setAttribute('colsep', '1')

        self.row.appendChild(cell)
        self._handleSimpleCellAttributes(cell, argsdict)
        self._handleColWidth(argsdict)
        self.curColumn += self._handleColSpan(cell, argsdict)

        self.maxColumn = max(self.curColumn, self.maxColumn)

        return cell

    def _handleColWidth(self, argsdict={}):
        if not argsdict.has_key("width"):
            return
        argsdict["width"] = argsdict["width"].strip('"')
        if not argsdict["width"].endswith("%"):
            self.formatter._emitComment("Width %s not supported" % argsdict["width"])
            return

        self.colWidths[str(self.curColumn)] = argsdict["width"][:-1] + "*"

    def _handleColSpan(self, element, argsdict={}):
        """Returns the number of colums this entry spans"""
        if not argsdict or not argsdict.has_key('colspan'):
            return 1
        assert(element.nodeName == "entry")
        extracols = int(argsdict['colspan'].strip('"')) - 1
        element.setAttribute('namest', "col_" + str(self.curColumn))
        element.setAttribute('nameend', "col_" + str(self.curColumn + extracols))
        return 1 + extracols

    def _handleSimpleCellAttributes(self, element, argsdict={}):
        if not argsdict:
            return
        assert(element.nodeName == "entry")

        safe_values_for = {'valign': ('top', 'middle', 'bottom'),
                           'align': ('left', 'center', 'right'),
                          }

        if argsdict.has_key('rowspan'):
            extrarows = int(argsdict['rowspan'].strip('"')) - 1
            element.setAttribute('morerows', str(extrarows))

        if argsdict.has_key('align'):
            value = argsdict['align'].strip('"')
            if value in safe_values_for['align']:
                element.setAttribute('align', value)
            else:
                self.formatter._emitComment("Alignment %s not supported" % value)
                pass

        if argsdict.has_key('valign'):
            value = argsdict['valign'].strip('"')
            if value in safe_values_for['valign']:
                element.setAttribute('valign', value)
            else:
                self.formatter._emitComment("Vertical alignment %s not supported" % value)
                pass


