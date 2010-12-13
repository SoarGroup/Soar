"""
    MoinMoin - HighlighterList Macro

    A simple macro for displaying a table with list of available Pygments lexers.

    Usage: <<HighlighterList([columns=<list of one of description, names,
             patterns, mimetypes, separated by pipe>,
             sort_column=(description|names|patterns|mimetypes),
             sort=(True|False)], filter_re=<regular expression>)>>

    @param columns: List of columns to display, separated by pipe character.
           Currently supported "description", "names", "patterns", "mimetypes".
           Unknown column names ignored. Spaces should be omitted. If empty,
           all columns are displayed.
    @param sort_column: Name of column by which list should be sorted. Column
           name can be one of "description", "names", "patterns", "mimetypes".
           If column is not "description", item values in this column will be
           split. Has effect in any case (not only when sort is True). If
           sort_column is empty, description column is used.
    @param filter_re: Filtering regular expression which data in sort_column
           should match. If filter_re is empty, no filtering performed.
    @param sort: Boolean value (true values are strings "true", "1", "yes" in
           any case) which determine whether list should be sorted.

    @copyright: 2009 MoinMoin:EugeneSyromyatnikov
    @license: GNU GPL, see COPYING for details.
"""

import re

from MoinMoin.config import multiconfig
from MoinMoin import wikiutil

import pygments.lexers

available_columns = ['description', 'names', 'patterns', 'mimetypes']

def macro_HighlighterList(macro, columns='|'.join(available_columns),
        sort_column=tuple(available_columns),
        sort=True, filter_re=None, _kwargs=None):
    request = macro.request
    _ = request.getText
    f = request.formatter

    column_titles = [_('Lexer description'),
                     _('Lexer names'),
                     _('File patterns'),
                     _('Mimetypes'),
                    ]

    columns = columns and [available_columns.index(column)
                for column
                in columns.split('|')
                if column in available_columns] or range(len(available_columns))
    sort_column = available_columns.index(sort_column) or 0
    do_filter = (filter_re not in (None, ""))
    filter_re = re.compile(filter_re or ".*")

    lexer_list = pygments.lexers.get_all_lexers()
    lexer_data = []

    #expanding tuples if sort_column is not name
    if sort_column != 0:
        for lexer in lexer_list:
            if len(lexer[sort_column]):
                for i in lexer[sort_column]:
                    lexer_item = list(lexer)
                    lexer_item[sort_column] = i
                    lexer_data.append(lexer_item)
            else:
                lexer_item = list(lexer)
                lexer_item[sort_column] = ""
                lexer_data.append(lexer_item)
    else:
        lexer_data.extend(lexer_list)


    #filtering
    if do_filter:
        lexer_data = [lexer for lexer in lexer_data
                       if filter_re.search(lexer[sort_column])]

    #sorting
    if sort:
        lexer_data.sort(cmp=lambda x, y:
          ((x != y)
          and cmp(x[sort_column].lower(), y[sort_column].lower())
          or cmp(x[0].lower(), y[0].lower())))

    #generating output
    ret = []

    #table header
    ret.extend([
        f.table(1),
        f.table_row(1, style="background-color: #ffffcc"),
        ])
    for col in columns:
        ret.extend([
                    f.table_cell(1),
                    f.strong(1),
                    f.text(column_titles[col]), f.strong(0), f.table_cell(0)
                  ])
    ret.append(f.table_row(0))

    #table data
    for parser in lexer_data:
        ret.append(f.table_row(1))

        for col in columns:
            if col:
                ret.extend([
                            f.table_cell(1),
                            f.code(1),
                            isinstance(parser[col], str) and f.text(parser[col])
                              or (f.code(0) + ', ' + f.code(1)).join([f.text(i) for i in parser[col]]),
                            f.code(0),
                            f.table_cell(0),
                          ])
            else:
                ret.extend([f.table_cell(1), f.text(parser[col]), f.table_cell(0)])

        ret.append(f.table_row(0))

    ret.append(f.table(0))

    return ''.join(ret)

