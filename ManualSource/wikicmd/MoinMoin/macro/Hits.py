# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Hits Macro

    This macro is used to show the cumulative hits of the wikipage where the Macro is called from.
    Optionally you could count how much this page or all pages were changed or viewed.

    <<Hits([all=(0,1)],[event_type=(VIEWPAGE,SAVEPAGE)>>

        all: if set to 1/True/yes then cumulative hits over all wiki pages is returned.
             Default is 0/False/no.
        filter: if set to SAVEPAGE then the saved pages are counted. Default is VIEWPAGE.

   @copyright: 2004-2008 MoinMoin:ReimarBauer,
               2005 BenjaminVrolijk
   @license: GNU GPL, see COPYING for details.
"""
Dependencies = ['time'] # do not cache

from MoinMoin.stats import hitcounts

def macro_Hits(macro, all=False, event_type=(u'VIEWPAGE', u'SAVEPAGE')):
    request = macro.request
    pagename = macro.formatter.page.page_name

    if all:
        cache_days, cache_views, cache_edits = hitcounts.get_data(pagename, request, filterpage=None)
    else:
        cache_days, cache_views, cache_edits = hitcounts.get_data(pagename, request, filterpage=pagename)

    if event_type == u'VIEWPAGE':
        return u'%d' % sum(cache_views)
    else:
        return u'%d' % sum(cache_edits)

