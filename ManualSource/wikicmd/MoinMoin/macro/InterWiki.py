# -*- coding: iso-8859-1 -*-
"""
    Outputs the interwiki map.

    @copyright: 2007 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details
"""

Dependencies = ["pages"] # if interwikimap is editable

from MoinMoin import wikiutil

def macro_InterWiki(macro):
    interwiki_list = wikiutil.load_wikimap(macro.request)
    iwlist = interwiki_list.items() # this is where we cached it
    iwlist.sort()
    fmt = macro.formatter
    output = []
    output.append(fmt.definition_list(1))
    for tag, url in iwlist:
        output.append(fmt.definition_term(1))
        output.append(fmt.code(1))
        output.append(fmt.url(1, wikiutil.join_wiki(url, 'RecentChanges')))
        output.append(fmt.text(tag))
        output.append(fmt.url(0))
        output.append(fmt.code(0))
        output.append(fmt.definition_term(0))
        output.append(fmt.definition_desc(1))
        output.append(fmt.code(1))
        if '$PAGE' not in url:
            output.append(fmt.url(1, url))
            output.append(fmt.text(url))
            output.append(fmt.url(0))
        else:
            output.append(fmt.text(url))
        output.append(fmt.code(0))
        output.append(fmt.definition_desc(1))
    output.append(fmt.definition_list(0))
    return u''.join(output)

