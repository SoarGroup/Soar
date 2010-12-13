"""
    MoinMoin - AttachInfo Macro

    A macro to produce information about attached pages

    Usage: <<AttachInfo>>

    @copyright: 2004 Jacob Cohen, Nigel Metheringham
    @license: GNU GPL, see COPYING for details
"""

from MoinMoin.action.AttachFile import info

def macro_AttachInfo(macro, pagename=None):
    """ generates info how much attachments stored on a page """
    if not pagename:
        pagename = macro.formatter.page.page_name

    return info(pagename, macro.request)


