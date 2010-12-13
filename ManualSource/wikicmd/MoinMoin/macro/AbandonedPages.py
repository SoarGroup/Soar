# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - AbandonedPages Macro

    This is a list of pages that were not edited for a long time
    according to the edit log; if you shortened the log, the displayed
    information may not be what you expect.

    @copyright: 2001 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.macro import RecentChanges

def macro_AbandonedPages(macro):
    return RecentChanges.macro_RecentChanges(macro, abandoned=True)

