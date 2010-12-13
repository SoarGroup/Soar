# -*- coding: iso-8859-1 -*-
"""
    Outputs the text verbatimly.

    @copyright: 2005 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details
"""

Dependencies = []

def macro_Verbatim(macro, text=u''):
    return macro.formatter.escapedText(text)
