# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - simple text diff (uses difflib)

    @copyright: 2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin.support import difflib

def diff(oldlines, newlines, **kw):
    """
    Find changes between oldlines and newlines.

    @param oldlines: list of old text lines
    @param newlines: list of new text lines
    @keyword ignorews: if 1: ignore whitespace
    @rtype: list
    @return: lines like diff tool does output.
    """
    false = lambda s: None
    if kw.get('ignorews', 0):
        d = difflib.Differ(false)
    else:
        d = difflib.Differ(false, false)

    lines = list(d.compare(oldlines, newlines))

    # return empty list if there were no changes
    changed = 0
    for l in lines:
        if l[0] != ' ':
            changed = 1
            break
    if not changed: return []

#    if not "we want the unchanged lines, too":
#        if "no questionmark lines":
#            lines = [line for line in lines if line[0] != '?']
#        return lines


    # calculate the hunks and remove the unchanged lines between them
    i = 0              # actual index in lines
    count = 0          # number of unchanged lines
    lcount_old = 0     # line count old file
    lcount_new = 0     # line count new file
    while i < len(lines):
        marker = lines[i][0]
        if marker == ' ':
            count = count + 1
            i = i + 1
            lcount_old = lcount_old + 1
            lcount_new = lcount_new + 1
        elif marker in ['-', '+']:
            if (count == i) and count > 3:
                lines[:i-3] = []
                i = 4
                count = 0
            elif count > 6:
                # remove lines and insert new hunk indicator
                lines[i-count+3:i-3] = ['@@ -%i, +%i @@\n' %
                                        (lcount_old, lcount_new)]
                i = i - count + 8
                count = 0
            else:
                count = 0
                i += 1
            if marker == '-': lcount_old = lcount_old + 1
            else: lcount_new = lcount_new + 1
        elif marker == '?':
            lines[i:i+1] = []

    # remove unchanged lines a the end
    if count > 3:
        lines[-count+3:] = []

    return lines

