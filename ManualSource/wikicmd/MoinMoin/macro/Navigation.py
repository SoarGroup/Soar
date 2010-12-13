# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Navigation Macro

    @copyright: 2003 Juergen Hermann <jh@web.de>
                2008 MoinMoin:RadomirDopieralski
    @license: GNU GPL, see COPYING for details.
"""

import re
from MoinMoin.Page import Page
from MoinMoin import wikiutil

Dependencies = ["namespace"]

# helpers
#!!! refactor these to an util module?
def _getParent(pagename):
    """ Return parent of pagename.
    """
    pos = pagename.rfind('/')
    if pos >= 0:
        return pagename[:pos]
    return None


def _getPages(request, filter_regex=None):
    """ Return a (filtered) list of pages names.
    """
    filterfn = None
    if filter_regex:
        filterfn = re.compile(filter_regex).match
    pages = request.rootpage.getPageList(filter=filterfn)
    pages.sort()
    return pages


def _getLinks(request, pagename, filter_regex=None):
    """ Return pagename for up, first, prev, next, last; each can be None.
    """
    pos, first, prev, next, last = 0, None, None, None, None

    all_pages = _getPages(request, filter_regex)
    size = len(all_pages)

    if all_pages:
        try:
            pos = all_pages.index(pagename)
        except ValueError:
            # this should never happen in theory, but let's be sure
            pass
        else:
            if pos > 0:
                first = all_pages[0]
                prev = all_pages[pos-1]
            if pos+1 < len(all_pages):
                next = all_pages[pos+1]
                last = all_pages[-1]

    return pos, size, (first, prev, next, last)


class Navigation:
    """ Dispatcher class implementing the navigation schemes.
    """

    # querystring for slideshow links
    PROJECTION = {'action': 'print', 'media': 'projection', }

    def __init__(self, macro, scheme, depth):
        """ Prepare common values used during processing.
        """
        self.macro = macro
        self.scheme = scheme
        self.depth = depth
        self._ = self.macro.request.getText

        self.pagename = self.macro.formatter.page.page_name
        self.print_mode = self.macro.request.action == 'print'
        self.media = self.macro.request.values.get('media')
        self.querystr = self.print_mode and self.PROJECTION or {}


    def dispatch(self):
        """ Return None if in plain print mode (no navigational
            elements in printouts), else the proper HTML code.
        """
        if self.print_mode and self.media != 'projection':
            return None

        return getattr(self, 'do_%s' % self.scheme, self.badscheme)()


    def badscheme(self):
        """ Bad scheme argument.
        """
        _ = self._
        return (self.macro.formatter.sysmsg(1) +
                self.macro.formatter.text(
            _("Unsupported navigation scheme '%(scheme)s'!") %
            {'scheme': self.scheme}) +
                self.macro.formatter.sysmsg(0))


    def do_children(self):
        """ Navigate to subpages from a parent page.
        """
        # delegate to siblings code, setting the parent explicitely
        return self.do_siblings(root=self.pagename)


    def do_siblings(self, root=None):
        """ Navigate from a subpage to its siblings.
        """
        _ = self._
        request = self.macro.request
        # get parent page name
        parent = root or _getParent(self.pagename)
        if not parent:
            return (self.macro.formatter.sysmsg(1) +
                    self.macro.formatter.text(_('No parent page found!'))+
                    self.macro.formatter.sysmsg(0))

        # iterate over children, adding links to all of them
        result = []
        children = _getPages(request, '^%s/' % re.escape(parent))
        for child in children:
            # display short page name, leaving out the parent path
            # (and make sure the name doesn't get wrapped)
            shortname = child[len(parent):]

            # possibly limit depth
            if self.depth and shortname.count('/') > self.depth:
                continue

            if child == self.pagename:
                # do not link to focus
                result.append(self.macro.formatter.text(shortname))
            else:
                # link to sibling / child
                result.append(Page(request, child).link_to(request, text=shortname, querystr=self.querystr))
            result.append(' &nbsp; ')

        return ''.join(result)


    def do_slideshow(self, focus=None):
        """ Slideshow master page links.

            If `focus` is set, it is the name of a slide page; these only
            get the mode toggle and edit links.
        """
        _ = self._
        curpage = focus or self.pagename
        result = []
        request = self.macro.request
        pg = Page(request, curpage)
        if self.print_mode:
            # projection mode
            label = _('Wiki')
            toggle = {}
            result.append(pg.link_to(request, text=_('Edit'), querystr={'action': 'edit'}))
            result.append(' &nbsp; ')
        else:
            # wiki mode
            label = _('Slideshow')
            toggle = self.PROJECTION

        # add mode toggle link
        result.append(pg.link_to(request, text=label, querystr=toggle))

        # leave out the following on slide pages
        if focus is None:
            children = _getPages(request, '^%s/' % re.escape(self.pagename))
            if children:
                # add link to first child if one exists
                result.append(' &nbsp; ')
                result.append(Page(request, children[0]).link_to(request, text=_('Start'), querystr=self.querystr))

        return ''.join(result)


    def do_slides(self, root=None):
        """ Navigate within a slide show.
        """
        _ = self._
        request = self.macro.request
        parent = root or _getParent(self.pagename)
        if not parent:
            return (self.macro.formatter.sysmsg(1) +
                    self.macro.formatter.text(_('No parent page found!')) +
                    self.macro.formatter.sysmsg(0))

        # prepare link generation
        result = []
        labels = ['^', '|<', '<<', '>>', '>|']
        filter_regex = '^%s/' % re.escape(parent)
        pos, size, links = _getLinks(request, self.pagename, filter_regex)
        pos += 1
        links = zip(labels, (parent, ) + links)

        # generate links to neighborhood
        for label, name in links:
            result.append(' ')
            if name:
                # active link
                result.append(Page(request, name).link_to(request, text=label, querystr=self.querystr))
            else:
                # ghosted link
                result.append(self.macro.formatter.text(label))
            result.append(' ')

            # position indicator in the middle
            if label == labels[2]:
                result.append(_('Slide %(pos)d of %(size)d') % {'pos': pos, 'size': size})

        return self.do_slideshow(focus=self.pagename) + ''.join(result)


def macro_Navigation(macro,
                    scheme=wikiutil.required_arg((u'children', u'siblings',
                                                  u'slideshow', u'slides')),
                    depth=0):
    # get HTML code with the links
    navi = Navigation(macro, scheme, depth).dispatch()

    if navi:
        # return links packed into a table
        return u'<table class="navigation"><tr><td>%s</td></tr></table>' % navi

    # navigation disabled in plain print mode
    return u''

