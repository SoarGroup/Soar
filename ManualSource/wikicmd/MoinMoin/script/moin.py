#!/usr/bin/env python2
# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - "moin" is the main script command and calls other stuff as
    a sub-command.

    Usage: moin cmdmodule cmdname [options]

    @copyright: 2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

def run():
    from MoinMoin.script import MoinScript
    MoinScript().run(showtime=0)

if __name__ == "__main__":
    # Insert the path to MoinMoin in the start of the path
    import sys, os
    # we use position 1 (not 0) to give a config dir inserted at 0 a chance
    # beware: we have a wikiconfig.py at the toplevel directory in the branch
    sys.path.insert(1, os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), os.pardir, os.pardir)))

    run()

