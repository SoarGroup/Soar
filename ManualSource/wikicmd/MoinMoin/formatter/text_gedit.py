# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - "text/html+css" Formatter for feeding the GUI editor

    @copyright: 2005-2006 Bastian Blank, Florian Festi, Thomas Waldmann, Reimar Bauer
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import log
logging = log.getLogger(__name__)

from MoinMoin.formatter import FormatterBase, text_html
from MoinMoin import wikiutil
from MoinMoin.Page import Page
from MoinMoin.action import AttachFile

class Formatter(text_html.Formatter):
    """ Send HTML data for the GUI editor """

    # Block elements ####################################################

    def heading(self, on, depth, **kw):
        # remember depth of first heading, and adapt counting depth accordingly
        if not self._base_depth:
            self._base_depth = depth

        count_depth = max(depth - (self._base_depth - 1), 1)
        heading_depth = depth

        # closing tag, with empty line after, to make source more readable
        if not on:
            return self._close('h%d' % heading_depth)
        else:
            return self._open('h%d' % heading_depth, **kw)

    # Links ##############################################################

    def pagelink(self, on, pagename='', page=None, **kw):
        """ Link to a page.

            formatter.text_python will use an optimized call with a page!=None
            parameter. DO NOT USE THIS YOURSELF OR IT WILL BREAK.

            See wikiutil.link_tag() for possible keyword parameters.
        """
        FormatterBase.pagelink(self, on, pagename, page, **kw)
        if page is None:
            page = Page(self.request, pagename, formatter=self)
        return page.link_to(self.request, on=on, **kw)

    def interwikilink(self, on, interwiki='', pagename='', **kw):
        """
        @keyword title: override using the interwiki wikiname as title
        """
        if not on:
            return self.url(0) # return '</a>'
        html_class = 'badinterwiki' # we use badinterwiki in any case to simplify reverse conversion
        href = wikiutil.quoteWikinameURL(pagename) or "/" # FCKeditor behaves strange on empty href
        title = kw.get('title', interwiki)
        return self.url(1, href, title=title, css=html_class) # interwiki links with pages with umlauts

    def attachment_inlined(self, url, text, **kw):
        url = wikiutil.escape(url)
        text = wikiutil.escape(text)
        if url == text:
            return '<span style="background-color:#ffff11">{{attachment:%s}}</span>' % url
        else:
            return '<span style="background-color:#ffff11">{{attachment:%s|%s}}</span>' % (url, text)

    def attachment_link(self, on, url=None, **kw):
        assert on in (0, 1, False, True) # make sure we get called the new way, not like the 1.5 api was
        _ = self.request.getText
        querystr = kw.get('querystr', {})
        assert isinstance(querystr, dict) # new in 1.6, only support dicts
        if 'do' not in querystr:
            querystr['do'] = 'view'
        if on:
            pagename = self.page.page_name
            target = AttachFile.getAttachUrl(pagename, url, self.request, do=querystr['do'])
            return self.url(on, target, title="attachment:%s" % wikiutil.quoteWikinameURL(url))
        else:
            return self.url(on)

    def attachment_image(self, url, **kw):
        _ = self.request.getText
        # we force the title here, needed later for html>wiki converter
        kw['title'] = "attachment:%s" % wikiutil.quoteWikinameURL(url)
        pagename = self.page.page_name
        if '/' in url:
            pagename, target = AttachFile.absoluteName(url, pagename)
            url = url.split('/')[-1]
        kw['src'] = AttachFile.getAttachUrl(pagename, url, self.request, addts=1)
        return self.image(**kw)

    def attachment_drawing(self, url, text, **kw):
        # Todo get it to start the drawing editor on a click
        try:
            drawing_action = AttachFile.get_action(self.request, url, do='modify')
            assert drawing_action is not None
            attachment_drawing = wikiutil.importPlugin(self.request.cfg, 'action',
                                              drawing_action, 'gedit_drawing')
            return attachment_drawing(self, url, text, **kw)
        except (wikiutil.PluginMissingError, wikiutil.PluginAttributeError, AssertionError):
            return url

    def icon(self, type):
        return self.request.theme.make_icon(type, title='smiley:%s' % type)

    smiley = icon

    def nowikiword(self, text):
        return '<span style="background-color:#ffff11">!</span>' + self.text(text)

    # Dynamic stuff / Plugins ############################################

    def macro(self, macro_obj, name, args, markup=None):
        if markup is not None:
            result = markup
        elif args is not None:
            result = "<<%s(%s)>>" % (name, args)
        else:
            result = "<<%s>>" % name
        return '<span style="background-color:#ffff11">%s</span>' % wikiutil.escape(result)

    def parser(self, parser_name, lines):
        """ parser_name MUST be valid!
        """
        result = [self.preformatted(1)]
        for line in lines:
            result.append(self.text(line))
            result.append(self.linebreak(preformatted=1))
        result.append(self.preformatted(0))

        return "".join(result)

    # Other ##############################################################

    style2attribute = {
        'width': 'width',
        'height': 'height',
        'background': 'bgcolor',
        'background-color': 'bgcolor',
        #if this is used as table style="text-align: right", it doesn't work
        #if it is transformed to align="right":
        #'text-align': 'align',
        #'vertical-align': 'valign'
        }

    def _style_to_attributes(self, attrs):
        if 'style' not in attrs:
            return attrs
        unknown = []
        for entry in attrs['style'].split(';'):
            try:
                key, value = entry.split(':')
            except ValueError:
                unknown.append(entry)
                continue
            key, value = key.strip(), value.strip()
            if key in self.style2attribute:
                attrs[self.style2attribute[key]] = value
            else:
                unknown.append("%s:%s" % (key, value))
        if unknown:
            attrs['style'] = ';'.join(unknown)
        else:
            del attrs['style']
        return attrs

    def _checkTableAttr(self, attrs, prefix):
        #logging.debug(repr(attrs))
        attrs = text_html.Formatter._checkTableAttr(self, attrs, prefix)
        #logging.debug(repr(attrs))
        attrs = self._style_to_attributes(attrs)
        #logging.debug(repr(attrs))
        return attrs

    _allowed_table_attrs = {
        'table': ['class', 'id', 'style', 'bgcolor', 'width', 'height', ],
        'row': ['class', 'id', 'style', 'bgcolor', 'width', 'height', ],
        '': ['colspan', 'rowspan', 'class', 'id', 'style', 'bgcolor', 'width', 'height', ],
    }

    def table(self, on, attrs=None, **kw):
        """ Create table

        @param on: start table
        @param attrs: table attributes
        @rtype: string
        @return start or end tag of a table
        """
        result = []
        if on:
            # Open table
            if not attrs:
                attrs = {}
            else:
                #result.append(self.rawHTML("<!-- ATTRS1: %s -->" % repr(attrs)))
                attrs = self._checkTableAttr(attrs, 'table')
                #result.append(self.rawHTML("<!-- ATTRS2: %s -->" % repr(attrs)))
            result.append(self._open('table', newline=1, attr=attrs,
                                     allowed_attrs=self._allowed_table_attrs['table'],
                                     **kw))
        else:
            # Close table then div
            result.append(self._close('table'))

        return ''.join(result)

    def comment(self, text, **kw):
        text = text.rstrip() # workaround for growing amount of blanks at EOL
        return self.preformatted(1, css_class='comment') + self.text(text) + self.preformatted(0)

    def strong(self, on, **kw):
        tag = 'b'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def emphasis(self, on, **kw):
        tag = 'i'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def underline(self, on, **kw):
        tag = 'u'
        if on:
            return self._open(tag, allowed_attrs=[], **kw)
        return self._close(tag)

    def line_anchordef(self, lineno):
        return '' # not needed for gui editor feeding

    def line_anchorlink(self, on, lineno=0):
        return '' # not needed for gui editor feeding

    def span(self, on, **kw):
        previous_state = self.request.user.show_comments
        self.request.user.show_comments = True
        ret = text_html.Formatter.span(self, on, **kw)
        self.request.user.show_comments = previous_state
        return ret
