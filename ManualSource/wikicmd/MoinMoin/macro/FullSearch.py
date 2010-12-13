# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - FullSearch Macro

    <<FullSearch>>
        displays a search dialog, as it always did.

    <<FullSearch()>>
        does the same as clicking on the page title, only that
        the result is embedded into the page. note the '()' after
        the macro name, which is an empty argument list.

    <<FullSearch(Help)>>
        embeds a search result into a page, as if you entered
        'Help' into the search box.

    The macro creates a page list without context or match info, just
    like PageList macro. It does not make sense to have context in non
    interactive search, and this kind of search is used usually for
    Category pages, where we don't care about the context.

    TODO: If we need to have context for some cases, either we add a context argument,
          or make another macro that uses context, which may be easier to use.

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006 MoinMoin:FranzPletz
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import wikiutil, search

Dependencies = ["pages"]


def search_box(type, macro):
    """ Make a search box

    Make both Title Search and Full Search boxes, according to type.

    @param type: search box type: 'titlesearch' or 'fullsearch'
    @rtype: unicode
    @return: search box html fragment
    """
    _ = macro._
    if 'value' in macro.request.values:
        default = wikiutil.escape(macro.request.values["value"], quote=1)
    else:
        default = ''

    # Title search settings
    boxes = ''
    button = _("Search Titles")

    # Special code for fullsearch
    if type == "fullsearch":
        boxes = [
            u'<br>',
            u'<input type="checkbox" name="context" value="160" checked="checked">',
            _('Display context of search results'),
            u'<br>',
            u'<input type="checkbox" name="case" value="1">',
            _('Case-sensitive searching'),
            ]
        boxes = u'\n'.join(boxes)
        button = _("Search Text")

    # Format
    type = (type == "titlesearch")
    html = [
        u'<form method="get" action="%s">' % macro.request.href(macro.request.formatter.page.page_name),
        u'<div>',
        u'<input type="hidden" name="action" value="fullsearch">',
        u'<input type="hidden" name="titlesearch" value="%i">' % type,
        u'<input type="text" name="value" size="30" value="%s">' % default,
        u'<input type="submit" value="%s">' % button,
        boxes,
        u'</div>',
        u'</form>',
        ]
    html = u'\n'.join(html)
    return macro.formatter.rawHTML(html)


def execute(macro, needle):
    request = macro.request
    _ = request.getText

    # if no args given, invoke "classic" behavior
    if needle is None:
        return search_box("fullsearch", macro)

    # With empty arguments, simulate title click (backlinks to page)
    elif needle == '':
        needle = '"%s"' % macro.formatter.page.page_name

    # With whitespace argument, show error message like the one used in the search box
    # TODO: search should implement those errors message for clients
    elif needle.isspace():
        err = _('Please use a more selective search term instead of '
                '{{{"%s"}}}', wiki=True) % needle
        return '<span class="error">%s</span>' % err

    needle = needle.strip()

    # Search the pages and return the results
    results = search.searchPages(request, needle, sort='page_name')

    return results.pageList(request, macro.formatter, paging=False)


