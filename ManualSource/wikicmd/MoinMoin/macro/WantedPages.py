# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - WantedPages Macro

    @copyright: 2001 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import wikiutil

Dependencies = ["pages"]

def macro_WantedPages(macro):
    request = macro.request
    _ = request.getText

    # prevent recursion
    if request.mode_getpagelinks:
        return ''
    if request.isSpiderAgent: # reduce bot cpu usage
        return ''

    # Get allpages switch from the form
    allpages = int(request.values.get('allpages', 0)) != 0

    # Control bar - filter the list of pages
    # TODO: we should make this a widget and use on all page listing pages
    label = (_('Include system pages'), _('Exclude system pages'))[allpages]
    page = macro.formatter.page
    controlbar = macro.formatter.div(1, css_class="controlbar") + \
                 page.link_to(request, label, querystr={'allpages': '%s' % (allpages and '0' or '1')}) + \
                 macro.formatter.div(0)

    # Get page dict readable by current user
    pages = request.rootpage.getPageDict()

    # build a dict of wanted pages
    wanted = {}
    deprecated_links = []
    for name, page in pages.items():
        # Skip system pages, because missing translations are not wanted pages,
        # unless you are a translator and clicked "Include system pages"
        if not allpages and wikiutil.isSystemPage(request, name):
            continue

        # Add links to pages which do not exist in pages dict
        links = page.getPageLinks(request)
        is_deprecated = page.parse_processing_instructions(
                ).get('deprecated', False)

        for link in links:
            if not link in pages and request.user.may.read(link):
                if is_deprecated:
                    deprecated_links.append(link)
                if link in wanted:
                    wanted[link][name] = 1
                else:
                    wanted[link] = {name: 1}

    for link in deprecated_links:
        if len(wanted[link]) == 1:
            del wanted[link]

    # Check for the extreme case when there are no wanted pages
    if not wanted:
        return u"%s<p>%s</p>" % (controlbar, _("No wanted pages in this wiki."))

    # Return a list of page links
    wantednames = wanted.keys()
    wantednames.sort()
    result = []
    result.append(macro.formatter.number_list(1))
    for name in wantednames:
        if not name:
            continue
        result.append(macro.formatter.listitem(1))
        # Add link to the wanted page
        result.append(macro.formatter.pagelink(1, name, generated=1))
        result.append(macro.formatter.text(name))
        result.append(macro.formatter.pagelink(0, name))

        # Add links to pages that want this page, highliting
        # the link in those pages.
        where = wanted[name].keys()
        where.sort()
        if macro.formatter.page.page_name in where:
            where.remove(macro.formatter.page.page_name)
        wherelinks = [pages[pagename].link_to(request, querystr={'highlight': name}, rel='nofollow')
                      for pagename in where]
        result.append(": " + ', '.join(wherelinks))
        result.append(macro.formatter.listitem(0))
    result.append(macro.formatter.number_list(0))

    return u'%s%s' % (controlbar, u''.join(result))

