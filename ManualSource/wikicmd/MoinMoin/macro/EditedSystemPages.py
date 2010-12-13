# -*- coding: iso-8859-1 -*-
"""
    EditedSystemPages - list system pages that has been edited in this wiki.

    @copyright: 2004 Nir Soffer <nirs@freeshell.org>
    @license: GNU GPL, see COPYING for details.
"""

class EditedSystemPages:

    def __init__(self, macro):
        self.macro = macro
        self.request = macro.request
        self.formatter = macro.formatter

    def renderInPage(self):
        """ Render macro in page context

        The parser should decide what to do if this macro is placed in a
        paragraph context.
        """
        from MoinMoin.Page import Page

        # Get page list for current user (use this as admin), filter
        # pages that are both underlay and standard pages.
        def filterfn(name):
            page = Page(self.request, name)
            return (page.isStandardPage(includeDeleted=0) and
                    page.isUnderlayPage(includeDeleted=0))

        # Get page filtered page list. We don't need to filter by
        # exists, because our filter check this already.
        pages = self.request.rootpage.getPageList(filter=filterfn, exists=0)

        # Format as numberd list, sorted by page name
        pages.sort()
        result = []
        f = self.formatter
        result.append(f.number_list(1))
        for name in pages:
            result.append(f.listitem(1))
            result.append(f.pagelink(1, name, generated=1))
            result.append(f.text(name))
            result.append(f.pagelink(0, name))
            result.append(f.listitem(0))
        result.append(f.number_list(0))

        return ''.join(result)


def macro_EditedSystemPages(macro):
    """ Temporary glue code to use with moin current macro system """
    return EditedSystemPages(macro).renderInPage()

