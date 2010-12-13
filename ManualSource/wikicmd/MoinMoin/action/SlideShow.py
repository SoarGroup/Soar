# -*- coding: utf-8 -*-
"""
MoinMoin - SlideShow action

Treat a wiki page as a set of slides.  Displays a single slide at a
time, along with a navigation aid.

@copyright: 2005 Jim Clark,
            2005 Nir Soffer,
            2008 MoinMoin:ThomasWaldmann,
            2009 MoinMoin:ReimarBauer
@license: GNU GPL, see COPYING for details.
"""

import re, time

from MoinMoin import config, wikiutil, i18n, error
from MoinMoin.Page import Page

Dependencies = ['language']


class Error(error.Error):
    """ Raised for errors in this module """

# This could be delivered in a separate plugin, but
# it is more convenient to have everything in one module.

class WikiSlideParser(object):
    """ Parse slides using wiki format

    Typical usage::
        for title, start, end in WikiSlideParser().parse(text):
            slides.append((title, start, end))

    If you want to override this parser, you can add 'slideshow_wiki'
    parser plugin, that provides a SlideParser class.
    """
    _heading_pattern = re.compile(r"""
        # TODO: check, mhz found bug here
        (?P<skip>{{{(?:.*\n)+?}}}) |
        # Match headings level 1
        (?P<heading>^=\s(?P<text>.*)\s=$\n?)
        """, re.MULTILINE | re.UNICODE | re.VERBOSE)

    def parse(self, text):
        """ Parse slide data in text

        Wiki slides are defined by the headings, ignoring the text
        before the first heading. This parser finds all headings,
        skipping headings in preformatted code areas.

        Returns an iterator over slide data. For each slide, a tuple
        (title, bodyStart, bodyEnd) is returned. bodyStart and bodyEnd
        are indexes into text.
        """
        matches = [match for match in self._heading_pattern.finditer(text)
                   if match.start('skip') == -1]

        for i in range(len(matches)):
            title = matches[i].group('text').strip()
            bodyStart = matches[i].end('heading')
            try:
                bodyEnd = matches[i + 1].start('heading')
            except IndexError:
                bodyEnd = len(text)
            yield title, bodyStart, bodyEnd


class SlidePage(Page):
    """ A wiki page containing a slideshow

    The slides are parsed according to the page #format xxx processing
    instruction. This module implements only a wiki format slide parser.

    To support other formats like rst, add a 'slideshow_rst' parser
    plugin, providing SlideParser class, implementing the SlideParser
    protocol. See WikiSlideParser for details.
    """
    defaultFormat = 'wiki'
    defaultParser = WikiSlideParser

    def __init__(self, request, name, **keywords):
        Page.__init__(self, request, name, **keywords)
        self._slideIndex = None
        self.counter = ''

    def __len__(self):
        """ Return the slide count """
        return len(self.slideIndex())

    def isEmpty(self):
        return len(self) == 0

    # Slide accessing methods map 1 based slides to 0 based index.

    def titleAt(self, number):
        """ Return the title of slide number """
        try:
            return self.slideIndex()[number - 1][0]
        except IndexError:
            return 1

    def bodyAt(self, number):
        """ Return the body of slide number """
        try:
            start, end = self.slideIndex()[number - 1][1:]
            return self.get_raw_body()[start:end]
        except IndexError:
            return self.get_raw_body()

    # Private ----------------------------------------------------------------

    def slideIndex(self):
        if self._slideIndex is None:
            self.parseSlides()
        return self._slideIndex

    def parseSlides(self):
        body = self.get_raw_body()
        self._slideIndex = []
        parser = self.createSlideParser()
        for title, bodyStart, bodyEnd in parser.parse(body):
            self._slideIndex.append((title, bodyStart, bodyEnd))

    def createSlideParser(self):
        """ Import plugin and return parser class

        If plugin is not found, and format is not defaultFormat, raise an error.
        For defaultFormat, use builtin defaultParser in this module.
        """
        format = self.pi['format']
        plugin = 'slideshow_' + format
        try:
            Parser = wikiutil.importPlugin(self.request.cfg, 'parser', plugin, 'SlideParser')
        except wikiutil.PluginMissingError:
            if format != self.defaultFormat:
                raise Error('SlideShow does not support %s format.' % format)
            Parser = self.defaultParser
        return Parser()


class SlideshowAction:

    name = 'SlideShow'
    maxSlideLinks = 15

    def __init__(self, request, pagename, template):
        self.request = request
        self.page = SlidePage(self.request, pagename)
        self.template = template

        # Cache values used many times
        self.pageURL = self.page.url(request)

    def execute(self):
        _ = self.request.getText
        try:
            self.setSlideNumber()
            language = self.page.pi['language']
            self.request.content_type = "text/html; charset=%s" % (config.charset, )
            self.request.setContentLanguage(language)
            self.request.write(self.template % self)
        except Error, err:
            self.request.theme.add_msg(wikiutil.escape(unicode(err)), "error")
            self.page.send_page()

    # Private ----------------------------------------------------------------

    def setSlideNumber(self):
        try:
            slideNumber = int(self.request.values.get('n', 1))
            if not 1 <= slideNumber <= len(self.page):
                slideNumber = 1
        except ValueError:
            slideNumber = 1
        self.slideNumber = slideNumber

    def createParser(self, format, text):
        if format == "wiki":
            format = 'text_moin_wiki'
        try:
            Parser = wikiutil.importPlugin(self.request.cfg, 'parser', format,
                                           'Parser')
        except wikiutil.PluginMissingError:
            from MoinMoin.parser.text import Parser
        parser = Parser(text, self.request)
        return parser

    def createFormatter(self, format):
        try:
            Formatter = wikiutil.importPlugin(self.request.cfg, 'formatter',
                                              format, 'Formatter')
        except wikiutil.PluginMissingError:
            from MoinMoin.formatter.text_plain import Formatter

        formatter = Formatter(self.request)
        self.request.formatter = formatter
        formatter.page = self.page
        return formatter

    def languageAttributes(self, lang):
        return ' lang="%s" dir="%s"' % (lang, i18n.getDirection(lang))

    def linkToPage(self, text, query='', **attributes):
        """ Return a link to current page """
        if query:
            url = '%s?%s' % (self.pageURL, query)
        else:
            url = self.pageURL
        return self.formatLink(url, text, **attributes)

    def linkToSlide(self, number, text, **attributes):
        """ Return a link to current page """
        if number == self.slideNumber:
            return self.disabledLink(text, **attributes)

        url = '%s?action=%s&n=%s' % (self.pageURL, self.name, number)
        return self.formatLink(url, text, **attributes)

    def disabledLink(self, text, **attributes):
        return '<span%s>%s</span>' % (self.formatAttributes(attributes), text)

    def formatLink(self, url, text, **attributes):
        return '<a href="%(url)s"%(attributes)s>%(text)s</a>' % {
            'url': wikiutil.escape(url),
            'attributes': self.formatAttributes(attributes),
            'text': wikiutil.escape(text),
            }

    def formatAttributes(self, attributes):
        """ Return formatted attributes string """
        formattedPairs = [' %s="%s"' % (k, v) for k, v in attributes.items()]
        return ''.join(formattedPairs)

    def adaptToLanguage(self, direction):
        # In RTL, directional items should be switched
        if i18n.getDirection(self.request.lang) == 'rtl':
            return not direction
        return direction

    def forwardIcon(self, forward=True):
        return [u'\u2190', u'\u2192'][self.adaptToLanguage(forward)]

    def backIcon(self):
        return self.forwardIcon(False)

    # Key codes constants
    rightArrowKey = 39
    leftArrowKey = 37

    def slideLinksRange(self):
        """ Return range of slides to display, current centered """
        other = self.maxSlideLinks - 1 # other slides except current
        first, last = self.first_slide(), self.last_slide()
        start = max(first, self.slideNumber - other / 2)
        end = min(start + other, last)
        start = max(first, end - other)
        return range(start, end + 1)

    def first_slide(self):
        return 1

    def next_slide(self):
        return min(self.slideNumber + 1, self.last_slide())

    def previous_slide(self):
        return max(self.slideNumber - 1, self.first_slide())

    def last_slide(self):
        return max(len(self.page), 1)

    # Replacing methods ------------------------------------------------------

    def __getitem__(self, name):
        item = getattr(self, 'item_' + name)
        if callable(item):
            return item()
        else:
            return item

    def item_language_attribtues(self):
        return self.languageAttributes(self.request.content_lang)

    def item_theme_url(self):
        return '%s/%s' % (self.request.cfg.url_prefix_static, self.request.theme.name)

    item_action_name = name

    def item_title(self):
        return wikiutil.escape(self.page.page_name)

    def item_slide_title(self):
        return wikiutil.escape(self.page.titleAt(self.slideNumber))

    def item_slide_body(self):
        text = self.page.bodyAt(self.slideNumber)
        format = self.page.pi['format']
        parser = self.createParser(format, text)
        formatter = self.createFormatter('text_html')
        return self.request.redirectedOutput(parser.format, formatter)

    def item_navigation_language_attributes(self):
        return self.languageAttributes(self.request.lang)

    def item_navigation_edit(self):
        _ = self.request.getText
        text = _('Edit')
        if self.request.user.may.write(self.page.page_name):
            return self.linkToPage(text, 'action=edit', title=_('Edit slide show'))
        return self.disabledLink(text, title=_("You are not allowed to edit this page."))

    def item_navigation_quit(self):
        _ = self.request.getText
        return self.linkToPage(_('Quit'), title=_('Quit slide show'))

    def item_navigation_start(self):
        _ = self.request.getText
        number = self.first_slide()
        return self.linkToSlide(number, '|', title=_('Show first slide (up arrow)'))

    def item_navigation_end(self):
        _ = self.request.getText
        number = self.last_slide()
        return self.linkToSlide(number, '|', title=_('Show last slide (down arrow)'))

    def item_navigation_back(self):
        _ = self.request.getText
        number = self.previous_slide()
        return self.linkToSlide(number, text=self.backIcon(), title=_('Show previous slide (left arrow)'))

    def item_navigation_forward(self):
        _ = self.request.getText
        number = self.next_slide()
        return self.linkToSlide(number, self.forwardIcon(), title=_('Show next slide (right arrow)'))

    def item_forward_key(self, forward=True):
        return (self.leftArrowKey, self.rightArrowKey)[self.adaptToLanguage(forward)]

    def item_back_key(self):
        return self.item_forward_key(False)

    def item_navigation_slides(self):
        items = []
        for i in self.slideLinksRange():
            attributes = {'title': self.page.titleAt(i)}
            if i == self.slideNumber:
                attributes = {'class': 'current'}
            items.append(self.linkToSlide(i, i, **attributes))
        items = ['<li>%s</li>' % item for item in items]
        return '\n'.join(items)

    def item_slide_link_base(self):
        return wikiutil.escape(self.pageURL) + '?action=%s&n=' % self.name

    item_next_slide = next_slide
    item_previous_slide = previous_slide
    item_first_slide = first_slide
    item_last_slide = last_slide

    def item_date(self):
        return wikiutil.escape(self.request.getPragma('date', defval=''))

    def item_author(self):
        return wikiutil.escape(self.request.getPragma('author', defval=''))

    def item_counter(self):
        return "%d|%d" % (self.slideNumber, self.last_slide())

# This is quite stupid template, but it cleans most of the code from
# html. With smarter templates, there will be no html in the action code.
template = """
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
    "http://www.w3.org/TR/html4/strict.dtd">

<html%(language_attribtues)s>
<head>
    <meta http-equiv="Content-Type" content="text/html;charset=utf-8">
    <meta name="robots" content="noindex,nofollow">
    <title>%(title)s</title>

    <script type="text/javascript">
        function getKey(e) {
            // Support multiple browsers stupidity
            var key;
            if (e == null) {
                // IE
                key = event.keyCode;
            } else {
                // Standards compliant
                if (e.altKey || e.ctrlKey) {
                    return null;
                }
                key = e.which;
            }
            return key;
        }

        function go(slide) {
            window.location="%(slide_link_base)s" + slide;
        }

        function onkeydown(e) {
            switch(getKey(e)) {
                // presenter maybe rather wants to use up/down for scrolling content!
                // case 38: go('%(first_slide)s'); break; // up arrow
                // case 40: go('%(last_slide)s'); break; // down arrow
                case %(forward_key)s: go('%(next_slide)s'); break;
                case %(back_key)s: go('%(previous_slide)s'); break;
                default: return true; // pass event to browser
            }
            // Return false to consume the event
            return false;
        }

        document.onkeydown = onkeydown
    </script>

    <link rel="stylesheet" type="text/css" charset="utf-8" media="all"
        href="%(theme_url)s/css/%(action_name)s.css">
</head>

<body>
    <h1>%(slide_title)s</h1>

    <div id="content">
        %(slide_body)s
    </div>

    <div id="navigation"%(navigation_language_attributes)s>
        <ul>
            <li>%(navigation_edit)s</li>
            <li>%(navigation_quit)s</li>
            <li>%(navigation_start)s</li>
            <li>%(navigation_back)s</li>
            %(navigation_slides)s
            <li>%(navigation_forward)s</li>
            <li>%(navigation_end)s</li>
        </ul>
    </div>
    <div id="footer">
    <ul id="date">%(date)s</ul>
    <ul id="author">%(author)s</ul>
    <ul id="counter">%(counter)s</ul>
    </div>
<!--
    <p><a href="http://validator.w3.org/check?uri=referer">
        Valid HTML 4.01</a>
    </p>
 -->
</body>
</html>
"""


def execute(pagename, request):
    """ Glue to current plugin system """
    SlideshowAction(request, pagename, template).execute()

