# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - PageSize Macro

    @copyright: 2002 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

Dependencies = ["pages"]

def macro_PageSize(macro):
    if macro.request.isSpiderAgent: # reduce bot cpu usage
        return ''

    # get list of pages and their objects
    pages = macro.request.rootpage.getPageDict()

    # get sizes and sort them
    sizes = []
    for name, page in pages.items():
        sizes.append((page.size(), page))
    sizes.sort()
    sizes.reverse()

    # format list
    result = []
    result.append(macro.formatter.number_list(1))
    for size, page in sizes:
        result.append(macro.formatter.listitem(1))
        result.append(macro.formatter.code(1))
        result.append(("%6d" % size).replace(" ", "&nbsp;") + " ")
        result.append(macro.formatter.code(0))
        result.append(macro.formatter.pagelink(1, page.page_name, generated=1))
        result.append(macro.formatter.text(page.page_name))
        result.append(macro.formatter.pagelink(0, page.page_name))
        result.append(macro.formatter.listitem(0))
    result.append(macro.formatter.number_list(0))

    return ''.join(result)

