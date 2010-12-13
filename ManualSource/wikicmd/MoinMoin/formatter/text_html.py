# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - "text/html+css" Formatter

    @copyright: 2000-2004 by Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""
import os.path, re

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin.formatter import FormatterBase
from MoinMoin import wikiutil, i18n
from MoinMoin.Page import Page
from MoinMoin.action import AttachFile
from MoinMoin.support.python_compatibility import set

# insert IDs into output wherever they occur
# warning: breaks toggle line numbers javascript
_id_debug = False

line_anchors = True
prettyprint = False

# These are the HTML elements that we treat as block elements.
_blocks = set(['dd', 'div', 'dl', 'dt', 'form', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6',
               'hr', 'li', 'ol', 'p', 'pre', 'table', 'tbody', 'td', 'tfoot', 'th',
               'thead', 'tr', 'ul', 'blockquote', ])

# These are the HTML elements which are typically only used with
# an opening tag without a separate closing tag.  We do not
# include 'script' or 'style' because sometimes they do have
# content, and also IE has a parsing bug with those two elements (only)
# when they don't have a closing tag even if valid XHTML.

_self_closing_tags = set(['area', 'base', 'br', 'col', 'frame', 'hr', 'img',
                          'input', 'isindex', 'link', 'meta', 'param'])

# We only open those tags and let the browser auto-close them:
_auto_closing_tags = set(['p'])

# These are the elements which generally should cause an increase in the
# indention level in the html souce code.
_indenting_tags = set(['ol', 'ul', 'dl', 'li', 'dt', 'dd', 'tr', 'td'])

# These are the elements that discard any whitespace they contain as
# immediate child nodes.
_space_eating_tags = set(['colgroup', 'dl', 'frameset', 'head', 'map' 'menu',
                          'ol', 'optgroup', 'select', 'table', 'tbody', 'tfoot',
                          'thead', 'tr', 'ul'])

# These are standard HTML attributes which are typically used without any
# value; e.g., as boolean flags indicated by their presence.
_html_attribute_boolflags = set(['compact', 'disabled', 'ismap', 'nohref',
                                 'noresize', 'noshade', 'nowrap', 'readonly',
                                 'selected', 'wrap'])

# These are all the standard HTML attributes that are allowed on any element.
_common_attributes = set(['accesskey', 'class', 'dir', 'disabled', 'id', 'lang',
                          'style', 'tabindex', 'title'])


def rewrite_attribute_name(name, default_namespace='html'):
    """
    Takes an attribute name and tries to make it HTML correct.

    This function takes an attribute name as a string, as it may be
    passed in to a formatting method using a keyword-argument syntax,
    and tries to convert it into a real attribute name.  This is
    necessary because some attributes may conflict with Python
    reserved words or variable syntax (such as 'for', 'class', or
    'z-index'); and also to help with backwards compatibility with
    older versions of MoinMoin where different names may have been
    used (such as 'content_id' or 'css').

    Returns a tuple of strings: (namespace, attribute).

    Namespaces: The default namespace is always assumed to be 'html',
    unless the input string contains a colon or a double-underscore;
    in which case the first such occurance is assumed to separate the
    namespace prefix from name.  So, for example, to get the HTML
    attribute 'for' (as on a <label> element), you can pass in the
    string 'html__for' or even '__for'.

    Hyphens:  To better support hyphens (which are not allowed in Python
    variable names), all occurances of two underscores will be replaced
    with a hyphen.  If you use this, then you must also provide a
    namespace since the first occurance of '__' separates a namespace
    from the name.

    Special cases: Within the 'html' namespace, mainly to preserve
    backwards compatibility, these exceptions ars recognized:
    'content_type', 'content_id', 'css_class', and 'css'.
    Additionally all html attributes starting with 'on' are forced to
    lower-case.  Also the string 'xmlns' is recognized as having
    no namespace.

    Examples:
        'id' -> ('html', 'id')
        'css_class' -> ('html', 'class')
        'content_id' -> ('html', 'id')
        'content_type' -> ('html', 'type')
        'html__for' -> ('html', 'for)
        'xml__space' -> ('xml', 'space')
        '__z__index' -> ('html', 'z-index')
        '__http__equiv' -> ('html', 'http-equiv')
        'onChange' -> ('html', 'onchange')
        'xmlns' -> ('', 'xmlns')
        'xmlns__abc' -> ('xmlns', 'abc')

    (In actuality we only deal with namespace prefixes, not any real
    namespace URI...we only care about the syntax not the meanings.)
    """

    # Handle any namespaces (just in case someday we support XHTML)
    if ':' in name:
        ns, name = name.split(':', 1)
    elif '__' in name:
        ns, name = name.split('__', 1)
    elif name == 'xmlns':
        ns = ''
    else:
        ns = default_namespace

    name.replace('__', '-')
    if ns == 'html':
        # We have an HTML attribute, fix according to DTD
        if name == 'content_type': # MIME type such as in <a> and <link> elements
            name = 'type'
        elif name == 'content_id': # moin historical convention
            name = 'id'
        elif name in ('css_class', 'css'): # to avoid python word 'class'
            name = 'class'
        elif name.startswith('on'): # event handler hook
            name = name.lower()
    return ns, name


def extend_attribute_dictionary(attributedict, ns, name, value):
    """Add a new attribute to an attribute dictionary, merging values where possible.

    The attributedict must be a dictionary with tuple-keys of the form:
    (namespace, attrname).

    The given ns, name, and value will be added to the dictionary.  It
    will replace the old value if it existed, except for a few special
    cases where the values are logically merged instead (CSS class
    names and style rules).

    As a special case, if value is None (not just ''), then the
    attribute is actually deleted from the dictionary.
    """

    key = ns, name
    if value is None:
        if key in attributedict:
            del attributedict[key]
    else:
        if ns == 'html' and key in attributedict:
            if name == 'class':
                # CSS classes are appended by space-separated list
                value = attributedict[key] + ' ' + value
            elif name == 'style':
                # CSS styles are appended by semicolon-separated rules list
                value = attributedict[key] + '; ' + value
            elif name in _html_attribute_boolflags:
                # All attributes must have a value. According to XHTML those
                # traditionally used as flags should have their value set to
                # the same as the attribute name.
                value = name
        attributedict[key] = value


class Formatter(FormatterBase):
    """
        Send HTML data.
    """

    hardspace = '&nbsp;'
    indentspace = ' '

    def __init__(self, request, **kw):
        FormatterBase.__init__(self, request, **kw)
        self._indent_level = 0

        self._in_code = 0 # used by text_gedit
        self._in_code_area = 0
        self._in_code_line = 0
        self._code_area_js = 0
        self._code_area_state = ['', 0, -1, -1, 0]

        # code format string. id - code block id, num - line number.
        # Caution: upon changing, also check line numbers hide/show js.
        self._code_id_format = "%(id)s_%(num)d"

        self._show_section_numbers = None
        self.pagelink_preclosed = False
        self._is_included = kw.get('is_included', False)
        self.request = request
        self.cfg = request.cfg
        self.no_magic = kw.get('no_magic', False) # disabled tag auto closing

        if not hasattr(request, '_fmt_hd_counters'):
            request._fmt_hd_counters = []

    # Primitive formatter functions #####################################

    # all other methods should use these to format tags. This keeps the
    # code clean and handle pathological cases like unclosed p and
    # inline tags.

    def _langAttr(self, lang=None):
        """ Return lang and dir attribute
        (INTERNAL USE BY HTML FORMATTER ONLY!)

        Must be used on all block elements - div, p, table, etc.
        @param lang: if defined, will return attributes for lang. if not
            defined, will return attributes only if the current lang is
            different from the content lang.
        @rtype: dict
        @return: language attributes
        """
        if not lang:
            lang = self.request.current_lang
            # Actions that generate content in user language should change
            # the content lang from the default defined in cfg.
            if lang == self.request.content_lang:
                # lang is inherited from content div
                return {}

        #attr = {'xml:lang': lang, 'lang': lang, 'dir': i18n.getDirection(lang),}
        attr = {'lang': lang, 'dir': i18n.getDirection(lang), }
        return attr

    def _formatAttributes(self, attr=None, allowed_attrs=None, **kw):
        """ Return HTML attributes formatted as a single string. (INTERNAL USE BY HTML FORMATTER ONLY!)

        @param attr: dict containing keys and values
        @param allowed_attrs: A list of allowable attribute names
        @param kw: other arbitrary attributes expressed as keyword arguments.
        @rtype: string
        @return: formated attributes or empty string

        The attributes and their values can either be given in the
        'attr' dictionary, or as extra keyword arguments.  They are
        both merged together.  See the function
        rewrite_attribute_name() for special notes on how to name
        attributes.

        Setting a value to None rather than a string (or string
        coercible) will remove that attribute from the list.

        If the list of allowed_attrs is provided, then an error is
        raised if an HTML attribute is encountered that is not in that
        list (or is not a common attribute which is always allowed or
        is not in another XML namespace using the double-underscore
        syntax).
        """

        # Merge the attr dict and kw dict into a single attributes
        # dictionary (rewriting any attribute names, extracting
        # namespaces, and merging some values like css classes).
        attributes = {} # dict of key=(namespace,name): value=attribute_value
        if attr:
            for a, v in attr.items():
                a_ns, a_name = rewrite_attribute_name(a)
                extend_attribute_dictionary(attributes, a_ns, a_name, v)
        if kw:
            for a, v in kw.items():
                a_ns, a_name = rewrite_attribute_name(a)
                extend_attribute_dictionary(attributes, a_ns, a_name, v)

        # Add title attribute if missing, but it has an alt.
        if ('html', 'alt') in attributes and ('html', 'title') not in attributes:
            attributes[('html', 'title')] = attributes[('html', 'alt')]

        # Force both lang and xml:lang to be present and identical if
        # either exists.  The lang takes precedence over xml:lang if
        # both exist.
        #if ('html', 'lang') in attributes:
        #    attributes[('xml', 'lang')] = attributes[('html', 'lang')]
        #elif ('xml', 'lang') in attributes:
        #    attributes[('html', 'lang')] = attributes[('xml', 'lang')]

        # Check all the HTML attributes to see if they are known and
        # allowed.  Ignore attributes if in non-HTML namespaces.
        if allowed_attrs:
            for name in [key[1] for key in attributes if key[0] == 'html']:
                if name in _common_attributes or name in allowed_attrs:
                    pass
                elif name.startswith('on'):
                    pass  # Too many event handlers to enumerate, just let them all pass.
                else:
                    # Unknown or unallowed attribute.
                    err = 'Illegal HTML attribute "%s" passed to formatter' % name
                    raise ValueError(err)

        # Finally, format them all as a single string.
        if attributes:
            # Construct a formatted string containing all attributes
            # with their values escaped.  Any html:* namespace
            # attributes drop the namespace prefix.  We build this by
            # separating the attributes into three categories:
            #
            #  * Those without any namespace (should only be xmlns attributes)
            #  * Those in the HTML namespace (we drop the html: prefix for these)
            #  * Those in any other non-HTML namespace, including xml:

            xmlnslist = ['%s="%s"' % (k[1], wikiutil.escape(v, 1))
                         for k, v in attributes.items() if not k[0]]
            htmllist = ['%s="%s"' % (k[1], wikiutil.escape(v, 1))
                        for k, v in attributes.items() if k[0] == 'html']
            otherlist = ['%s:%s="%s"' % (k[0], k[1], wikiutil.escape(v, 1))
                         for k, v in attributes.items() if k[0] and k[0] != 'html']

            # Join all these lists together in a space-separated string.  Also
            # prefix the whole thing with a space too.
            htmllist.sort()
            otherlist.sort()
            all = [''] + xmlnslist + htmllist + otherlist
            return ' '.join(all)
        return ''

    def _open(self, tag, newline=False, attr=None, allowed_attrs=None,
              is_unique=False, **kw):
        """ Open a tag with optional attributes (INTERNAL USE BY HTML FORMATTER ONLY!)

        @param tag: html tag, string
        @param newline: render tag so following data is on a separate line
        @param attr: dict with tag attributes
        @param allowed_attrs: list of allowed attributes for this element
        @param is_unique: ID is already unique
        @param kw: arbitrary attributes and values
        @rtype: string ?
        @return: open tag with attributes as a string
        """
        # If it is self-closing, then don't expect a closing tag later on.
        is_self_closing = (tag in _self_closing_tags) and ' /' or ''

        # make ID unique
        id = None
        if not is_unique:
            if attr and 'id' in attr:
                id = self.make_id_unique(attr['id'])
                id = self.qualify_id(id)
                attr['id'] = id
            if 'id' in kw:
                id = self.make_id_unique(kw['id'])
                id = self.qualify_id(id)
                kw['id'] = id
        else:
            if attr and 'id' in attr:
                id = attr['id']
            if 'id' in kw:
                id = kw['id']

        if tag in _blocks:
            # Block elements
            result = []

            # Add language attributes, but let caller overide the default
            attributes = self._langAttr()
            if attr:
                attributes.update(attr)

            # Format
            attributes = self._formatAttributes(attributes, allowed_attrs=allowed_attrs, **kw)
            result.append('<%s%s%s>' % (tag, attributes, is_self_closing))
            if newline:
                result.append(self._newline())
            if _id_debug and id:
                result.append('(%s) ' % id)
            tagstr = ''.join(result)
        else:
            # Inline elements
            tagstr = '<%s%s%s>' % (tag,
                                      self._formatAttributes(attr, allowed_attrs, **kw),
                                      is_self_closing)
        return tagstr

    def _close(self, tag, newline=False):
        """ Close tag (INTERNAL USE BY HTML FORMATTER ONLY!)

        @param tag: html tag, string
        @param newline: render tag so following data is on a separate line
        @rtype: string
        @return: closing tag as a string
        """
        if tag in _self_closing_tags or (tag in _auto_closing_tags and not self.no_magic):
            # This tag was already closed
            tagstr = ''
        elif tag in _blocks:
            # Block elements
            result = []
            if newline:
                result.append(self._newline())
            result.append('</%s>' % (tag))
            tagstr = ''.join(result)
        else:
            # Inline elements
            tagstr = '</%s>' % tag

        if newline:
            tagstr += self._newline()
        return tagstr

    # Public methods ###################################################

    def startContent(self, content_id='content', newline=True, **kw):
        """ Start page content div.

        A link anchor is provided at the beginning of the div, with
        an id of 'top' (modified by the request ID cache).
        """

        if hasattr(self, 'page'):
            self.request.uid_generator.begin(self.page.page_name)

        result = []
        # Use the content language
        attr = self._langAttr(self.request.content_lang)
        attr['id'] = content_id
        result.append(self._open('div', newline=False, attr=attr,
                                 allowed_attrs=['align'], **kw))
        result.append(self.anchordef('top'))
        if newline:
            result.append('\n')
        return ''.join(result)

    def endContent(self, newline=True):
        """ Close page content div.

        A link anchor is provided at the end of the div, with
        an id of 'bottom' (modified by the request ID cache).
        """

        result = []
        result.append(self.anchordef('bottom'))
        result.append(self._close('div', newline=newline))
        if hasattr(self, 'page'):
            self.request.uid_generator.end()
        return ''.join(result)

    def lang(self, on, lang_name):
        """ Insert text with specific lang and direction.

            Enclose within span tag if lang_name is different from
            the current lang
        """
        tag = 'span'
        if lang_name != self.request.current_lang:
            # Enclose text in span using lang attributes
            if on:
                attr = self._langAttr(lang=lang_name)
                return self._open(tag, attr=attr)
            return self._close(tag)

        # Direction did not change, no need for span
        return ''

    # Links ##############################################################

    def pagelink(self, on, pagename='', page=None, **kw):
        """ Link to a page.

            formatter.text_python will use an optimized call with a page!=None
            parameter. DO NOT USE THIS YOURSELF OR IT WILL BREAK.

            See wikiutil.link_tag() for possible keyword parameters.
        """
        FormatterBase.pagelink(self, on, pagename, page, **kw)
        if 'generated' in kw:
            del kw['generated']
        if page is None:
            page = Page(self.request, pagename, formatter=self)
        if self.request.user.show_nonexist_qm and on and not page.exists():
            self.pagelink_preclosed = True
            return (page.link_to(self.request, on=1, **kw) +
                    self.text("?") +
                    page.link_to(self.request, on=0, **kw))
        elif not on and self.pagelink_preclosed:
            self.pagelink_preclosed = False
            return ""
        else:
            return page.link_to(self.request, on=on, **kw)

    def interwikilink(self, on, interwiki='', pagename='', **kw):
        """
        @keyword title: override using the interwiki wikiname as title
        """
        querystr = kw.get('querystr', {})
        wikitag, wikiurl, wikitail, wikitag_bad = wikiutil.resolve_interwiki(self.request, interwiki, pagename)
        wikiurl = wikiutil.mapURL(self.request, wikiurl)
        if wikitag == 'Self': # for own wiki, do simple links
            wikitail = wikiutil.url_unquote(wikitail)
            try: # XXX this is the only place where we access self.page - do we need it? Crashes silently on actions!
                pagename = wikiutil.AbsPageName(self.page.page_name, wikitail)
            except:
                pagename = wikitail
            return self.pagelink(on, pagename, **kw)
        else: # return InterWiki hyperlink
            if on:
                href = wikiutil.join_wiki(wikiurl, wikitail)
                if querystr:
                    separator = ('?', '&')['?' in href]
                    href = '%s%s%s' % (href, separator, wikiutil.makeQueryString(querystr))
                anchor = kw.get('anchor')
                if anchor:
                    href = '%s#%s' % (href, self.sanitize_to_id(anchor))
                if wikitag_bad:
                    html_class = 'badinterwiki'
                else:
                    html_class = 'interwiki'
                title = kw.get('title', wikitag)
                return self.url(1, href, title=title, css=html_class) # interwiki links with umlauts
            else:
                return self.url(0)

    def url(self, on, url=None, css=None, do_escape=None, **kw):
        """
        Inserts an <a> element (you can give any A tag attributes as kw args).

        @param on: 1 to start the link, 0 to end the link (no other arguments are needed when on==0).
        @param url: the URL to link to; will go through Wiki URL mapping.
        @param css: a space-separated list of CSS classes
        @param do_escape: DEPRECATED and not used any more, please remove it from your code!
                          We will remove this parameter in moin 1.8 (it used to filter url
                          param through wikiutil.escape, but text_html formatter's _open
                          will do it again, so this just leads to double escaping now).
        """
        if do_escape is not None:
            if do_escape:
                logging.warning("Deprecation warning: MoinMoin.formatter.text_html.url being called with do_escape=1/True parameter, please review caller.")
            else:
                logging.warning("Deprecation warning: MoinMoin.formatter.text_html.url being called with do_escape=0/False parameter, please remove it from the caller.")
        if on:
            attrs = self._langAttr()

            # Handle the URL mapping
            if url is None and 'href' in kw:
                url = kw['href']
                del kw['href']
            if url is not None:
                url = wikiutil.mapURL(self.request, url)
                attrs['href'] = url

            if css:
                attrs['class'] = css

            markup = self._open('a', attr=attrs, **kw)
        else:
            markup = self._close('a')
        return markup

    def anchordef(self, id):
        """Inserts an invisible element used as a link target.

        Inserts an empty <span> element with an id attribute, used as an anchor
        for link references.  We use <span></span> rather than <span/>
        for browser portability.
        """
        # Don't add newlines, \n, as it will break pre and
        # line-numbered code sections (from line_achordef() method).
        #return '<a id="%s"></a>' % (id, ) # do not use - this breaks PRE sections for IE
        id = self.make_id_unique(id)
        id = self.qualify_id(id)
        return '<span class="anchor" id="%s"></span>' % id

    def line_anchordef(self, lineno):
        if line_anchors:
            return self.anchordef("line-%d" % lineno)
        else:
            return ''

    def anchorlink(self, on, name='', **kw):
        """Insert an <a> link pointing to an anchor on the same page.

        Call once with on=1 to start the link, and a second time with
        on=0 to end it.  No other arguments are needed on the second
        call.

        The name argument should be the same as the id provided to the
        anchordef() method, or some other elment.  It should NOT start
        with '#' as that will be added automatically.

        The id argument, if provided, is instead the id of this link
        itself and not of the target element the link references.
        """
        attrs = self._langAttr()
        if name:
            name = self.sanitize_to_id(name)
            attrs['href'] = '#' + self.qualify_id(name)
        if 'href' in kw:
            del kw['href']
        if on:
            str = self._open('a', attr=attrs, **kw)
        else:
            str = self._close('a')
        return str

    def line_anchorlink(self, on, lineno=0):
        if line_anchors:
            return self.anchorlink(on, name="line-%d" % lineno)
        else:
            return ''

    # Attachments ######################################################

    def attachment_link(self, on, url=None, querystr=None, **kw):
        """ Link to an attachment.

            @param on: 1/True=start link, 0/False=end link
            @param url: filename.ext or PageName/filename.ext
        """
        assert on in (0, 1, False, True) # make sure we get called the new way, not like the 1.5 api was
        _ = self.request.getText
        if querystr is None:
            querystr = {}
        assert isinstance(querystr, dict) # new in 1.6, only support dicts
        if 'do' not in querystr:
            querystr['do'] = 'view'
        if on:
            pagename, filename = AttachFile.absoluteName(url, self.page.page_name)
            #logging.debug("attachment_link: url %s pagename %s filename %s" % (url, pagename, filename))
            fname = wikiutil.taintfilename(filename)
            if AttachFile.exists(self.request, pagename, fname):
                target = AttachFile.getAttachUrl(pagename, fname, self.request, do=querystr['do'])
                if not 'title' in kw:
                    kw['title'] = "attachment:%s" % url
                kw['css'] = 'attachment'
            else:
                target = AttachFile.getAttachUrl(pagename, fname, self.request, do='upload_form')
                kw['title'] = _('Upload new attachment "%(filename)s"') % {'filename': fname}
                kw['css'] = 'attachment nonexistent'
            return self.url(on, target, **kw)
        else:
            return self.url(on)

    def attachment_image(self, url, **kw):
        _ = self.request.getText
        pagename, filename = AttachFile.absoluteName(url, self.page.page_name)
        fname = wikiutil.taintfilename(filename)
        exists = AttachFile.exists(self.request, pagename, fname)
        if exists:
            kw['css'] = 'attachment'
            kw['src'] = AttachFile.getAttachUrl(pagename, fname, self.request, addts=1)
            title = _('Inlined image: %(url)s') % {'url': self.text(url)}
            if not 'title' in kw:
                kw['title'] = title
            # alt is required for images:
            if not 'alt' in kw:
                kw['alt'] = kw['title']
            return self.image(**kw)
        else:
            title = _('Upload new attachment "%(filename)s"') % {'filename': fname}
            img = self.icon('attachimg')
            css = 'nonexistent'
            target = AttachFile.getAttachUrl(pagename, fname, self.request, do='upload_form')
            return self.url(1, target, css=css, title=title) + img + self.url(0)

    def attachment_drawing(self, url, text, **kw):
        # ToDo try to move this to a better place e.g. __init__
        try:
            drawing_action = AttachFile.get_action(self.request, url, do='modify')
            assert drawing_action is not None
            attachment_drawing = wikiutil.importPlugin(self.request.cfg, 'action',
                                              drawing_action, 'attachment_drawing')
            return attachment_drawing(self, url, text, **kw)
        except (wikiutil.PluginMissingError, wikiutil.PluginAttributeError, AssertionError):
            return url

    # Text ##############################################################

    def _text(self, text):
        text = wikiutil.escape(text)
        if self._in_code:
            text = text.replace(' ', self.hardspace)
        return text

    # Inline ###########################################################

    def strong(self, on, **kw):
        """Creates an HTML <strong> element.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'strong'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def emphasis(self, on, **kw):
        """Creates an HTML <em> element.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'em'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def underline(self, on, **kw):
        """Creates a text span for underlining (css class "u").

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'span'
        if on:
            return self._open(tag, attr={'class': 'u'}, allowed_attrs=[], **kw)
        return self._close(tag)

    def highlight(self, on, **kw):
        """Creates a text span for highlighting (css class "highlight").

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'strong'
        if on:
            return self._open(tag, attr={'class': 'highlight'}, allowed_attrs=[], **kw)
        return self._close(tag)

    def sup(self, on, **kw):
        """Creates a <sup> element for superscript text.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'sup'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def sub(self, on, **kw):
        """Creates a <sub> element for subscript text.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'sub'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def strike(self, on, **kw):
        """Creates a text span for line-through (strikeout) text (css class 'strike').

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        # This does not use <strike> because has been deprecated in standard HTML.
        tag = 'span'
        if on:
            return self._open(tag, attr={'class': 'strike'},
                              allowed_attrs=[], **kw)
        return self._close(tag)

    def code(self, on, **kw):
        """Creates a <tt> element for inline code or monospaced text.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.

        Any text within this section will have spaces converted to
        non-break spaces.
        """
        tag = 'tt'
        self._in_code = on
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def small(self, on, **kw):
        """Creates a <small> element for smaller font.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'small'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def big(self, on, **kw):
        """Creates a <big> element for larger font.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'big'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)


    # Block elements ####################################################

    def preformatted(self, on, **kw):
        """Creates a preformatted text region, with a <pre> element.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        FormatterBase.preformatted(self, on)
        tag = 'pre'
        if on:
            return self._open(tag, newline=1, **kw)
        return self._close(tag)

    # Use by code area
    _toggleLineNumbersScript = """
<script type="text/javascript">
function isnumbered(obj) {
  return obj.childNodes.length && obj.firstChild.childNodes.length && obj.firstChild.firstChild.className == 'LineNumber';
}
function nformat(num,chrs,add) {
  var nlen = Math.max(0,chrs-(''+num).length), res = '';
  while (nlen>0) { res += ' '; nlen-- }
  return res+num+add;
}
function addnumber(did, nstart, nstep) {
  var c = document.getElementById(did), l = c.firstChild, n = 1;
  if (!isnumbered(c)) {
    if (typeof nstart == 'undefined') nstart = 1;
    if (typeof nstep  == 'undefined') nstep = 1;
    var n = nstart;
    while (l != null) {
      if (l.tagName == 'SPAN') {
        var s = document.createElement('SPAN');
        var a = document.createElement('A');
        s.className = 'LineNumber';
        a.appendChild(document.createTextNode(nformat(n,4,'')));
        a.href = '#' + did + '_' + n;
        s.appendChild(a);
        s.appendChild(document.createTextNode(' '));
        n += nstep;
        if (l.childNodes.length) {
          l.insertBefore(s, l.firstChild);
        }
        else {
          l.appendChild(s);
        }
      }
      l = l.nextSibling;
    }
  }
  return false;
}
function remnumber(did) {
  var c = document.getElementById(did), l = c.firstChild;
  if (isnumbered(c)) {
    while (l != null) {
      if (l.tagName == 'SPAN' && l.firstChild.className == 'LineNumber') l.removeChild(l.firstChild);
      l = l.nextSibling;
    }
  }
  return false;
}
function togglenumber(did, nstart, nstep) {
  var c = document.getElementById(did);
  if (isnumbered(c)) {
    remnumber(did);
  } else {
    addnumber(did,nstart,nstep);
  }
  return false;
}
</script>
"""

    def code_area(self, on, code_id, code_type='code', show=0, start=-1, step=-1, msg=None):
        """Creates a formatted code region, with line numbering.

        This region is formatted as a <div> with a <pre> inside it.  The
        code_id argument is assigned to the 'id' of the div element, and
        must be unique within the document.  The show, start, and step are
        used for line numbering.

        Note this is not like most formatter methods, it can not take any
        extra keyword arguments.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.

        the msg string is not escaped
        """
        _ = self.request.getText
        res = []
        if on:
            code_id = self.sanitize_to_id('CA-%s' % code_id)
            ci = self.qualify_id(self.make_id_unique(code_id))

            # Open a code area
            self._in_code_area = 1
            self._in_code_line = 0
            # id in here no longer used
            self._code_area_state = [None, show, start, step, start, ci]

            if msg:
                attr = {'class': 'codemsg'}
                res.append(self._open('div', attr={'class': 'codemsg'}))
                res.append(msg)
                res.append(self._close('div'))

            # Open the code div - using left to right always!
            attr = {'class': 'codearea', 'lang': 'en', 'dir': 'ltr'}
            res.append(self._open('div', attr=attr))

            # Add the script only in the first code area on the page
            if self._code_area_js == 0 and self._code_area_state[1] >= 0:
                res.append(self._toggleLineNumbersScript)
                self._code_area_js = 1

            # Add line number link, but only for JavaScript enabled browsers.
            if self._code_area_state[1] >= 0:
                toggleLineNumbersLink = r'''
<script type="text/javascript">
document.write('<a href="#" onclick="return togglenumber(\'%s\', %d, %d);" \
                class="codenumbers">%s<\/a>');
</script>
''' % (ci, self._code_area_state[2], self._code_area_state[3],
       _("Toggle line numbers"))
                res.append(toggleLineNumbersLink)

            # Open pre - using left to right always!
            attr = {'id': ci, 'lang': 'en', 'dir': 'ltr'}
            res.append(self._open('pre', newline=True, attr=attr, is_unique=True))
        else:
            # Close code area
            res = []
            if self._in_code_line:
                res.append(self.code_line(0))
            res.append(self._close('pre'))
            res.append(self._close('div'))

            # Update state
            self._in_code_area = 0

        return ''.join(res)

    def code_line(self, on):
        res = ''
        if not on or (on and self._in_code_line):
            res += '</span>\n'
        if on:
            res += '<span class="line">'
            if self._code_area_state[1] > 0:
                res += ('<span class="LineNumber"><a href="#%(fmt)s">%%(num)4d</a> </span><span class="LineAnchor" id="%(fmt)s"></span>' % {'fmt': self._code_id_format, }) % {
                    'id': self._code_area_state[5],
                    'num': self._code_area_state[4],
                    }
                self._code_area_state[4] += self._code_area_state[3]
        self._in_code_line = on != 0
        return res

    def code_token(self, on, tok_type):
        return ['<span class="%s">' % tok_type, '</span>'][not on]

    # Paragraphs, Lines, Rules ###########################################

    def _indent_spaces(self):
        """Returns space(s) for indenting the html source so list nesting is easy to read.

        Note that this mostly works, but because of caching may not always be accurate."""
        if prettyprint:
            return self.indentspace * self._indent_level
        else:
            return ''

    def _newline(self):
        """Returns the whitespace for starting a new html source line, properly indented."""
        if prettyprint:
            return '\n' + self._indent_spaces()
        else:
            return ''

    def linebreak(self, preformatted=1):
        """Creates a line break in the HTML output.

        If preformatted is true a <br> element is inserted, otherwise
        the linebreak will only be visible in the HTML source.
        """
        if self._in_code_area:
            preformatted = 1
        return ['\n', '<br>\n'][not preformatted] + self._indent_spaces()

    def paragraph(self, on, **kw):
        """Creates a paragraph with a <p> element.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        if self._terse:
            return ''
        FormatterBase.paragraph(self, on)
        tag = 'p'
        if on:
            tagstr = self._open(tag, **kw)
        else:
            tagstr = self._close(tag)
        return tagstr

    def rule(self, size=None, **kw):
        """Creates a horizontal rule with an <hr> element.

        If size is a number in the range [1..6], the CSS class of the rule
        is set to 'hr1' through 'hr6'.  The intent is that the larger the
        size number the thicker or bolder the rule will be.
        """
        if size and 1 <= size <= 6:
            # Add hr class: hr1 - hr6
            return self._open('hr', newline=1, attr={'class': 'hr%d' % size}, **kw)
        return self._open('hr', newline=1, **kw)

    # Images / Transclusion ##############################################

    def icon(self, type):
        return self.request.theme.make_icon(type)

    smiley = icon

    def image(self, src=None, **kw):
        """Creates an inline image with an <img> element.

        The src argument must be the URL to the image file.
        """
        if src:
            kw['src'] = src
        return self._open('img', **kw)

    def transclusion(self, on, **kw):
        """Transcludes (includes/embeds) another object."""
        if on:
            return self._open('object',
                              allowed_attrs=['archive', 'classid', 'codebase',
                                             'codetype', 'data', 'declare',
                                             'height', 'name', 'standby',
                                             'type', 'width', ],
                              **kw)
        else:
            return self._close('object')

    def transclusion_param(self, **kw):
        """Give a parameter to a transcluded object."""
        return self._open('param',
                          allowed_attrs=['name', 'type', 'value', 'valuetype', ],
                          **kw)

    # Lists ##############################################################

    def number_list(self, on, type=None, start=None, **kw):
        """Creates an HTML ordered list, <ol> element.

        The 'type' if specified can be any legal numbered
        list-style-type, such as 'decimal','lower-roman', etc.

        The 'start' argument if specified gives the numeric value of
        the first list item (default is 1).

        Call once with on=1 to start the list, and a second time
        with on=0 to end it.
        """
        tag = 'ol'
        if on:
            attr = {}
            if type is not None:
                attr['type'] = type
            if start is not None:
                attr['start'] = start
            tagstr = self._open(tag, newline=1, attr=attr, **kw)
        else:
            tagstr = self._close(tag, newline=1)
        return tagstr

    def bullet_list(self, on, **kw):
        """Creates an HTML ordered list, <ul> element.

        The 'type' if specified can be any legal unnumbered
        list-style-type, such as 'disc','square', etc.

        Call once with on=1 to start the list, and a second time
        with on=0 to end it.
        """
        tag = 'ul'
        if on:
            tagstr = self._open(tag, newline=1, **kw)
        else:
            tagstr = self._close(tag, newline=1)
        return tagstr

    def listitem(self, on, **kw):
        """Adds a list item, <li> element, to a previously opened
        bullet or number list.

        Call once with on=1 to start the region, and a second time
        with on=0 to end it.
        """
        tag = 'li'
        if on:
            tagstr = self._open(tag, newline=1, **kw)
        else:
            tagstr = self._close(tag, newline=1)
        return tagstr

    def definition_list(self, on, **kw):
        """Creates an HTML definition list, <dl> element.

        Call once with on=1 to start the list, and a second time
        with on=0 to end it.
        """
        tag = 'dl'
        if on:
            tagstr = self._open(tag, newline=1, **kw)
        else:
            tagstr = self._close(tag, newline=1)
        return tagstr

    def definition_term(self, on, **kw):
        """Adds a new term to a definition list, HTML element <dt>.

        Call once with on=1 to start the term, and a second time
        with on=0 to end it.
        """
        tag = 'dt'
        if on:
            tagstr = self._open(tag, newline=1, **kw)
        else:
            tagstr = self._close(tag, newline=0)
        return tagstr

    def definition_desc(self, on, **kw):
        """Gives the definition to a definition item, HTML element <dd>.

        Call once with on=1 to start the definition, and a second time
        with on=0 to end it.
        """
        tag = 'dd'
        if on:
            tagstr = self._open(tag, newline=1, **kw)
        else:
            tagstr = self._close(tag, newline=0)
        return tagstr

    def heading(self, on, depth, **kw):
        # remember depth of first heading, and adapt counting depth accordingly
        if not self._base_depth:
            self._base_depth = depth

        count_depth = max(depth - (self._base_depth - 1), 1)

        # check numbering, possibly changing the default
        if self._show_section_numbers is None:
            self._show_section_numbers = self.cfg.show_section_numbers
            numbering = self.request.getPragma('section-numbers', '').lower()
            if numbering in ['0', 'off']:
                self._show_section_numbers = 0
            elif numbering in ['1', 'on']:
                self._show_section_numbers = 1
            elif numbering in ['2', '3', '4', '5', '6']:
                # explicit base level for section number display
                self._show_section_numbers = int(numbering)

        heading_depth = depth

        # closing tag, with empty line after, to make source more readable
        if not on:
            return self._close('h%d' % heading_depth) + '\n'

        # create section number
        number = ''
        if self._show_section_numbers:
            # count headings on all levels
            self.request._fmt_hd_counters = self.request._fmt_hd_counters[:count_depth]
            while len(self.request._fmt_hd_counters) < count_depth:
                self.request._fmt_hd_counters.append(0)
            self.request._fmt_hd_counters[-1] = self.request._fmt_hd_counters[-1] + 1
            number = '.'.join([str(x) for x in self.request._fmt_hd_counters[self._show_section_numbers-1:]])
            if number: number += ". "

        # Add space before heading, easier to check source code
        result = '\n' + self._open('h%d' % heading_depth, **kw)

        if self.request.user.show_topbottom:
            result += "%s%s%s%s%s%s" % (
                       self.anchorlink(1, "bottom"), self.icon('bottom'), self.anchorlink(0),
                       self.anchorlink(1, "top"), self.icon('top'), self.anchorlink(0))

        return "%s%s" % (result, number)


    # Tables #############################################################

    _allowed_table_attrs = {
        'table': ['class', 'id', 'style'],
        'row': ['class', 'id', 'style'],
        '': ['colspan', 'rowspan', 'class', 'id', 'style', 'abbr'],
    }

    def _checkTableAttr(self, attrs, prefix):
        """ Check table attributes

        Convert from wikitable attributes to html 4 attributes.

        @param attrs: attribute dict
        @param prefix: used in wiki table attributes
        @rtype: dict
        @return: valid table attributes
        """
        if not attrs:
            return {}

        result = {}
        s = [] # we collect synthesized style in s
        for key, val in attrs.items():
            # Ignore keys that don't start with prefix
            if prefix and key[:len(prefix)] != prefix:
                continue
            key = key[len(prefix):]
            val = val.strip('"')
            # remove invalid attrs from dict and synthesize style
            if key == 'width':
                s.append("width: %s" % val)
            elif key == 'height':
                s.append("height: %s" % val)
            elif key == 'bgcolor':
                s.append("background-color: %s" % val)
            elif key == 'align':
                s.append("text-align: %s" % val)
            elif key == 'valign':
                s.append("vertical-align: %s" % val)
            # Ignore unknown keys
            if key not in self._allowed_table_attrs[prefix]:
                continue
            result[key] = val
        st = result.get('style', '').split(';')
        st = '; '.join(st + s)
        st = st.strip(';')
        st = st.strip()
        if not st:
            try:
                del result['style'] # avoid empty style attr
            except:
                pass
        else:
            result['style'] = st
        #logging.debug("_checkTableAttr returns %r" % result)
        return result


    def table(self, on, attrs=None, **kw):
        """ Create table

        @param on: start table
        @param attrs: table attributes
        @rtype: string
        @return start or end tag of a table
        """
        result = []
        if on:
            # Open div to get correct alignment with table width smaller
            # than 100%
            result.append(self._open('div', newline=1))

            # Open table
            if not attrs:
                attrs = {}
            else:
                attrs = self._checkTableAttr(attrs, 'table')
            result.append(self._open('table', newline=1, attr=attrs,
                                     allowed_attrs=self._allowed_table_attrs['table'],
                                     **kw))
            result.append(self._open('tbody', newline=1))
        else:
            # Close tbody, table, and then div
            result.append(self._close('tbody'))
            result.append(self._close('table'))
            result.append(self._close('div'))

        return ''.join(result)

    def table_row(self, on, attrs=None, **kw):
        tag = 'tr'
        if on:
            if not attrs:
                attrs = {}
            else:
                attrs = self._checkTableAttr(attrs, 'row')
            return self._open(tag, newline=1, attr=attrs,
                             allowed_attrs=self._allowed_table_attrs['row'],
                             **kw)
        return self._close(tag) + '\n'

    def table_cell(self, on, attrs=None, **kw):
        tag = 'td'
        if on:
            if not attrs:
                attrs = {}
            else:
                attrs = self._checkTableAttr(attrs, '')
            return '  ' + self._open(tag, attr=attrs,
                             allowed_attrs=self._allowed_table_attrs[''],
                             **kw)
        return self._close(tag) + '\n'

    def text(self, text, **kw):
        txt = FormatterBase.text(self, text, **kw)
        if kw:
            return self._open('span', **kw) + txt + self._close('span')
        return txt

    def escapedText(self, text, **kw):
        txt = wikiutil.escape(text)
        if kw:
            return self._open('span', **kw) + txt + self._close('span')
        return txt

    def rawHTML(self, markup):
        return markup

    def sysmsg(self, on, **kw):
        tag = 'div'
        if on:
            return self._open(tag, attr={'class': 'message'}, **kw)
        return self._close(tag)

    def div(self, on, **kw):
        css_class = kw.get('css_class')
        # the display of comment class divs depends on a user setting:
        if css_class and 'comment' in css_class.split():
            style = kw.get('style')
            display = self.request.user.show_comments and "display:''" or "display:none"
            if not style:
                style = display
            else:
                style += "; %s" % display
            kw['style'] = style
        tag = 'div'
        if on:
            return self._open(tag, **kw)
        return self._close(tag)

    def span(self, on, **kw):
        css_class = kw.get('css_class')
        # the display of comment class spans depends on a user setting:
        if css_class and 'comment' in css_class.split():
            style = kw.get('style')
            display = self.request.user.show_comments and "display:''" or "display:none"
            if not style:
                style = display
            else:
                style += "; %s" % display
            kw['style'] = style
        tag = 'span'
        if on:
            return self._open(tag, **kw)
        return self._close(tag)

    def sanitize_to_id(self, text):
        return wikiutil.anchor_name_from_text(text)

