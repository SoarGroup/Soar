# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Create list of LikePages

    @copyright: 2004 Johannes Berg <johannes@sipsolutions.de>
    @license: GNU GPL, see COPYING for details.
"""

Dependencies = ['namespace']

from MoinMoin.action import LikePages

def macro_LikePages(macro, text=u'(none)'):
    request = macro.request
    # we don't want to spend much CPU for spiders requesting nonexisting pages
    if not request.isSpiderAgent:
        pagename = macro.formatter.page.page_name

        # Get matches
        start, end, matches = LikePages.findMatches(pagename, request)

        # Render matches
        if matches and not isinstance(matches, (str, unicode)):
            return request.redirectedOutput(LikePages.showMatches, pagename, request, start, end, matches, False)
        else:
            # if we did not find any similar pages, we just render the text we got as argument:
            return request.formatter.text(text)
    # bots get nothing:
    return ''

