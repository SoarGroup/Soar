# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - IRC Log Highlighting Parser

    DEPRECATED compatibility wrapper calling the highlight parser.

    This is to support (deprecated) existing syntax like:
    {{{#!irc ...
    ...
    }}}

    It is equivalent to the new way to highlight code:
    {{{#!highlight irc ...
    ...
    }}}

    @copyright: 2009 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.parser.highlight import Parser as HighlightParser
from MoinMoin.parser.highlight import Dependencies

class Parser(HighlightParser):
    parsername = 'irc'  # Lexer name pygments recognizes
    extensions = [] # this is only a compatibility wrapper, we have declared
                    # support for this extension in the HighlightParser, so
                    # moin will call that directly

