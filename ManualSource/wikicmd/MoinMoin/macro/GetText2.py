# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Load I18N Text and substitute data.

    This macro has the main purpose of being used by extensions that write
    data to wiki pages but want to ensure that it is properly translated.

    @copyright: 2006 MoinMoin:AlexanderSchremmer,
                2009 MoinMoin:EugeneSyromyatnikov,
                2009 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.packages import unpackLine

Dependencies = ["language"]

def execute(macro, args):
    """ args consists of a character specifiying the separator and then a
    packLine sequence describing a list. The first element of it is the message
    and the remaining elements are substituted in the message using string
    substitution.
    """
    msg = u''
    if args:
        sep = args[0]
        args = unpackLine(args[1:], sep)
        if args:
            msg, args = args[0], tuple(args[1:])
            msg = macro.request.getText(msg)
            try:
                msg = msg % args
            except TypeError:
                # % operator will raise TypeError if msg has named placeholders
                msg = msg % dict([arg.split('=', 1) for arg in args if '=' in arg])
    return macro.formatter.text(msg)

