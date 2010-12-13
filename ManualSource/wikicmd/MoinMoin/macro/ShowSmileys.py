# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - List all defined smileys

    <<ShowSmileys>> will display a table of all the available smileys.

    Based on code by Nick Trout <trout@users.sf.net>

    @copyright: 2003 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import config
from MoinMoin.util.dataset import TupleDataset, Column
from MoinMoin.widget.browser import DataBrowserWidget

COLUMNS = 4

Dependencies = ['user'] # different users have different themes and different user prefs (text/gfx)

def macro_ShowSmileys(macro):
    _ = macro.request.getText
    fmt = macro.formatter

    # create data description
    data = TupleDataset()
    data.columns = []
    for dummy in range(COLUMNS):
        data.columns.extend([
            Column('markup', label=_('Markup')),
            Column('image', label=_('Display'), align='center'),
            Column('', label=''),
        ])
    data.columns[-1].hidden = 1

    # iterate over smileys, in groups of size COLUMNS
    smileys = config.smileys
    for idx in range(0, len(smileys), COLUMNS):
        row = []
        for off in range(COLUMNS):
            if idx+off < len(smileys):
                markup = smileys[idx+off]
                row.extend([fmt.code(1) + fmt.text(markup) + fmt.code(0), fmt.smiley(markup), '', ])
            else:
                row.extend(['&nbsp;'] * 3)
        data.addRow(tuple(row))

    # display table
    if data:
        browser = DataBrowserWidget(macro.request)
        browser.setData(data)
        return browser.render(method="GET")

    return ''

