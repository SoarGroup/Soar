# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - BR Macro

    This very complicated macro produces a line break.

    @copyright: 2000 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

Dependencies = []

def macro_BR(macro):
    """
    Creates a linebreak.
    """
    return macro.formatter.linebreak(0)
