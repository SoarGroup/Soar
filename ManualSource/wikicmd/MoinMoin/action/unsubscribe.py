# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - unsubscribe from notifications to a page.

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin.Page import Page

def execute(pagename, request):
    """ Unsubscribe the user from pagename """
    _ = request.getText
    if not request.user.valid:
        actname = __name__.split('.')[-1]
        request.theme.add_msg(_("You must login to use this action: %(action)s.") % {"action": actname}, "error")
        return Page(request, pagename).send_page()

    if request.user.isSubscribedTo([pagename]):
        # Try to unsubscribe
        if request.user.unsubscribe(pagename):
            request.theme.add_msg(_('Your subscription to this page has been removed.'), "info")
        else:
            msg = _("Can't remove regular expression subscription!") + u' ' + \
                  _("Edit the subscription regular expressions in your settings.")
            request.theme.add_msg(msg, "error")
    else:
        # The user is not subscribed
        request.theme.add_msg(_('You need to be subscribed to unsubscribe.'), "info")
    Page(request, pagename).send_page()

