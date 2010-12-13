# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - remove a quicklink from the user's quicklinks

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin.Page import Page

def execute(pagename, request):
    """ Remove the current wiki page from the user's quicklinks """
    _ = request.getText
    msg = None

    if not request.user.valid:
        msg = _("You must login to remove a quicklink.")
    elif request.user.isQuickLinkedTo([pagename]):
        if request.user.removeQuicklink(pagename):
            msg = _('Your quicklink to this page has been removed.')
        else: # should not happen
            msg = _('Your quicklink to this page could not be removed.')
    else:
        msg = _('You need to have a quicklink to this page to remove it.')
    if msg:
        request.theme.add_msg(msg)
    Page(request, pagename).send_page()

