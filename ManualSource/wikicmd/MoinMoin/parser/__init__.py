# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Parser Package

    See "plain.py" for the most simple parser that also
    defines the parser interface.

    @copyright: 2000 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.util import pysupport
from MoinMoin import wikiutil

modules = pysupport.getPackageModules(__file__)


def parse_start_step(request, args):
    """
    Parses common Colorizer parameters start, step, numbers.
    Uses L{wikiutil.parseAttributes} and sanitizes the results.

    Start and step must be a non negative number and default to 1,
    numbers might be on, off, or none and defaults to on. On or off
    means that numbers are switchable via JavaScript (html formatter),
    disabled means that numbers are disabled completely.

    attrdict is returned as last element in the tuple, to enable the
    calling parser to extract further arguments.

    @param request: a request instance
    @param args: the argument string

    @returns: numbers, start, step, attrdict
    """
    nums, start, step = 1, 1, 1
    attrs, msg = wikiutil.parseAttributes(request, args)
    if not msg:
        try:
            start = int(attrs.get('start', '"1"')[1:-1])
        except ValueError:
            pass
        try:
            step = int(attrs.get('step', '"1"')[1:-1])
        except ValueError:
            pass
        if attrs.get('numbers', '"on"')[1:-1].lower() in ('off', 'false', 'no'):
            nums = 0
        elif attrs.get('numbers', '"on"')[1:-1].lower() in ('none', 'disable'):
            nums = -1
    return nums, start, step, attrs

