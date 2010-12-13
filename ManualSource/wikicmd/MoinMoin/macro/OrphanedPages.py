# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - OrphanedPages Macro

    @copyright: 2001 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

Dependencies = ["pages"]

def macro_OrphanedPages(macro):
    _ = macro.request.getText

    if macro.request.mode_getpagelinks: # prevent recursion
        return ''
    if macro.request.isSpiderAgent: # reduce bot cpu usage
        return ''

    # delete all linked pages from a dict of all pages
    pages = macro.request.rootpage.getPageDict()
    orphaned = {}
    orphaned.update(pages)
    for page in pages.values():
        links = page.getPageLinks(macro.request)
        for link in links:
            if link in orphaned:
                del orphaned[link]

    result = []
    f = macro.formatter
    if not orphaned:
        result.append(f.paragraph(1))
        result.append(f.text(_("No orphaned pages in this wiki.")))
        result.append(f.paragraph(0))
    else:
        # return a list of page links
        orphanednames = orphaned.keys()
        orphanednames.sort()
        result.append(f.number_list(1))
        for name in orphanednames:
            if not name:
                continue
            result.append(f.listitem(1))
            result.append(f.pagelink(1, name, generated=1))
            result.append(f.text(name))
            result.append(f.pagelink(0, name))
            result.append(f.listitem(0))
        result.append(f.number_list(0))

    return ''.join(result)

