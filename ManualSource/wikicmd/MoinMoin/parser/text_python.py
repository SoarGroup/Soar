# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Python Source Parser

    DEPRECATED compatibility wrapper calling the highlight parser.

    This is to support (deprecated) existing syntax like:
    {{{#!python ...
    ...
    }}}

    It is equivalent to the new way to highlight code:
    {{{#!highlight python ...
    ...
    }}}

    @copyright: 2008 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.parser.highlight import Parser as HighlightParser
from MoinMoin.parser.highlight import Dependencies

class Parser(HighlightParser):
    parsername = 'python'  # Lexer name pygments recognizes
    extensions = [] # this is only a compatibility wrapper, we have declared
                    # support for this extension in the HighlightParser, so
                    # moin will call that directly

