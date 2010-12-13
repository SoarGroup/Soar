# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - refresh cache of a page

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin.Page import Page

def execute(pagename, request):
    """ Handle refresh action """
    # Without arguments, refresh action will refresh the page text_html cache.
    arena = request.values.get('arena', 'Page.py')
    if arena == 'Page.py':
        arena = Page(request, pagename)
    key = request.values.get('key', 'text_html')

    # Remove cache entry (if exists), and send the page
    from MoinMoin import caching
    caching.CacheEntry(request, arena, key, scope='item').remove()
    caching.CacheEntry(request, arena, "pagelinks", scope='item').remove()
    request.page.send_page()

