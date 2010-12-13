# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - subscribe to a page to get notified when it changes

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin.Page import Page

def execute(pagename, request):
    """ Subscribe the user to pagename """
    _ = request.getText
    if not request.user.valid:
        actname = __name__.split('.')[-1]
        request.theme.add_msg(_("You must login to use this action: %(action)s.") % {"action": actname}, "error")
        return Page(request, pagename).send_page()

    cfg = request.cfg

    if not request.user.may.read(pagename):
        request.theme.add_msg(_("You are not allowed to subscribe to a page you can't read."), "error")

    # Check if mail is enabled
    elif not cfg.mail_enabled and not cfg.jabber_enabled:
        request.theme.add_msg(_("This wiki is not enabled for mail/Jabber processing."), "error")

    # Suggest visitors to login
    elif not request.user.valid:
        request.theme.add_msg(_("You must log in to use subscriptions."), "error")

    # Suggest users without email to add their email address
    elif not request.user.email and not request.user.jid:
        request.theme.add_msg(_("Add your email address or Jabber ID in your user settings to use subscriptions."),
                              "error")

    elif request.user.isSubscribedTo([pagename]):
        request.theme.add_msg(_('You are already subscribed to this page.'))
    else:
        # Try to subscribe
        if request.user.subscribe(pagename):
            request.theme.add_msg(_('You have been subscribed to this page.'), "info")
        else: # should not happen
            request.theme.add_msg(_('You could not get subscribed to this page.'), "error")

    Page(request, pagename).send_page()
