# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - AdvancedSearch Macro

    <<AdvancedSearch>>
        displays advanced search dialog.

    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import wikiutil
from MoinMoin.i18n import languages
from MoinMoin.widget import html
from MoinMoin.util.web import makeSelection
from MoinMoin.support.python_compatibility import sorted
import mimetypes

Dependencies = ['pages']

def getMimetypes():
    # The types will be listed here, instead of parsing mimetypes.types_map
    types = [
        'application',
        'audio',
        'image',
        'message',
        'text',
        'video',
        ]
    return types


def getCategories(request):
    # This will return all pages with "Category" in the title
    cat_filter = request.cfg.cache.page_category_regexact.search
    pages = request.rootpage.getPageList(filter=cat_filter)
    pages.sort()
    return pages


def form_get(request, name, default='', escaped=False):
    """ Fetches a form field

    @param request: current request
    @param name: name of the field
    @param default: value if not present (default: '')
    @param escaped: if True, escape value so it can be used for html generation (default: False)
    """
    value = request.values.get(name, default)
    if escaped:
        value = wikiutil.escape(value, quote=True)
    return value


def advanced_ui(macro):
    """ Returns the code for the advanced search user interface

    @param macro: current macro instance
    """
    _ = macro._
    f = macro.formatter
    request = macro.request

    disabledIfMoinSearch = not request.cfg.xapian_search and \
            ' disabled="disabled"' or ''

    search_boxes = ''.join([
        f.table_row(1),
        f.table_cell(1, attrs={'rowspan': '5', 'class': 'searchfor'}),
        f.text(_('Search for items')),
        f.table_cell(0),
        ''.join([''.join([
            f.table_row(1),
            f.table_cell(1),
            f.text(txt),
            f.table_cell(0),
            f.table_cell(1),
            f.rawHTML(input_field),
            f.table_cell(0),
            f.table_row(0),
        ]) for txt, input_field in (
            (_('containing all the following terms'),
                '<input type="text" name="and_terms" size="30" value="%s">'
                % (form_get(request, 'and_terms', escaped=True) or form_get(request, 'value', escaped=True))),
            (_('containing one or more of the following terms'),
                '<input type="text" name="or_terms" size="30" value="%s">'
                % form_get(request, 'or_terms', escaped=True)),
            (_('not containing the following terms'),
                '<input type="text" name="not_terms" size="30" value="%s">'
                % form_get(request, 'not_terms', escaped=True)),
            #('containing only one of the following terms',
            #    '<input type="text" name="xor_terms" size="30" value="%s">'
            #    % form_get(request, 'xor_terms', escaped=True)),
            # TODO: dropdown-box?
            (_('last modified since (e.g. 2 weeks before)'),
                '<input type="text" name="mtime" size="30" value="%s">'
                % form_get(request, 'mtime', escaped=True)),
        )])
    ])

    # category selection
    categories = form_get(request, 'categories')
    c_select = makeSelection('categories',
            [('', _('any category'))] +
            [(cat, '%s' % cat) for cat in getCategories(request)],
            categories, 3, True)

    # language selection
    searchedlang = form_get(request, 'language')
    langs = dict([(lang, lmeta['x-language-in-english'])
        for lang, lmeta in languages.items()])
    userlang = macro.request.lang
    lang_select = makeSelection('language',
            [('', _('any language')),
            (userlang, langs[userlang])] + sorted(langs.items(), key=lambda i: i[1]),
            searchedlang, 3, True)

    # mimetype selection
    mimetype = form_get(request, 'mimetype')
    mt_select = makeSelection('mimetype',
            [('', _('any mimetype'))] +
            [(type, 'all %s files' % type) for type in getMimetypes()] +
            [(m[1], '*%s - %s' % m) for m in sorted(mimetypes.types_map.items())],
            mimetype, 3, True)

    # misc search options (checkboxes)
    search_options = ''.join([
        ''.join([
            f.table_row(1),
            f.table_cell(1, attrs={'class': 'searchfor'}),
            txt[0],
            f.table_cell(0),
            f.table_cell(1, colspan=2),
            unicode(txt[1]),
            txt[2],
            f.table_cell(0),
            f.table_row(0),
            ]) for txt in (
                (_('Categories'), unicode(c_select), ''),
                (_('Language'), unicode(lang_select), ''),
                (_('File Type'), unicode(mt_select), ''),
                ('', html.INPUT(type='checkbox', name='titlesearch',
                    value='1', checked=form_get(request, 'titlesearch', escaped=True),
                    id='titlesearch'),
                    '<label for="titlesearch">%s</label>' % _('Search only in titles')),
                ('', html.INPUT(type='checkbox', name='case', value='1',
                    checked=form_get(request, 'case', escaped=True),
                    id='case'),
                    '<label for="case">%s</label>' % _('Case-sensitive search')),
                ('', html.INPUT(type='checkbox', name='excludeunderlay',
                    value='1', checked=form_get(request, 'excludeunderlay', escaped=True),
                    id='excludeunderlay'),
                    '<label for="excludeunderlay">%s</label>' % _('Exclude underlay')),
                ('', html.INPUT(type='checkbox', name='nosystemitems',
                    value='1', checked=form_get(request, 'nosystemitems', escaped=True),
                    id='nosystempages'),
                    '<label for="nosystempages">%s</label>' % _('No system items')),
                ('', html.INPUT(type='checkbox', name='historysearch',
                    value='1', checked=form_get(request, 'historysearch', escaped=True),
                    disabled=(not request.cfg.xapian_search or
                        not request.cfg.xapian_index_history),
                    id='historysearch'),
                    '<label for="historysearch">%s</label>' % _('Search in all page revisions'))
            )
    ])

    # the dialogue
    return f.rawHTML('\n'.join([
        u'<form method="get" action="%s">' % macro.request.href(macro.request.formatter.page.page_name),
        u'<div>',
        u'<input type="hidden" name="action" value="fullsearch">',
        u'<input type="hidden" name="advancedsearch" value="1">',
        f.table(1, attrs={'tableclass': 'advancedsearch'}),
        search_boxes,
        search_options,
        f.table_row(1),
        f.table_cell(1, attrs={'class': 'submit', 'colspan': '3'}),
        u'<input type="submit" value="%s">' % _('Go get it!'),
        f.table_cell(0),
        f.table_row(0),
        f.table(0),
        u'</div>',
        u'</form>',
    ]))


def execute(macro, needle):
    # for now, just show the advanced ui
    return advanced_ui(macro)

