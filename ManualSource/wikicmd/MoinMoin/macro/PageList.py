# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - PageList

    print a list of pages whose title matches the search term

    @copyright: @copyright: 2001-2003 Juergen Hermann <jh@web.de>,
                2003-2008 MoinMoin:ThomasWaldmann
                2008 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

Dependencies = ["namespace"]
from MoinMoin import search, wikiutil

def execute(macro, args):
    _ = macro._
    case = 0

    # If called with empty or no argument, default to regex search for .+, the full page list.
    needle = wikiutil.get_unicode(macro.request, args, 'needle', u'regex:.+')

    # With whitespace argument, return same error message as FullSearch
    if not needle.strip():
        err = _('Please use a more selective search term instead of {{{"%s"}}}', wiki=True) % needle
        return '<span class="error">%s</span>' % err

    # Return a title search for needle, sorted by name.
    try:
        results = search.searchPages(macro.request, needle,
                                     titlesearch=1, case=case,
                                     sort='page_name')
        ret = results.pageList(macro.request, macro.formatter, paging=False)
    except ValueError:
        # same error as in MoinMoin/action/fullsearch.py, keep it that way!
        ret = ''.join([macro.formatter.text('<<PageList('),
                      _('Your search query {{{"%s"}}} is invalid. Please refer to '
                        'HelpOnSearching for more information.', wiki=True,
                        percent=True) % wikiutil.escape(needle),
                      macro.formatter.text(')>>')])
    return ret
