# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - set values in user profile

    @copyright: 2008 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin.Page import Page
from MoinMoin import user, wikiutil

def execute(pagename, request):
    """ set values in user profile """
    _ = request.getText
    cfg = request.cfg
    form = request.form

    if not request.user.isSuperUser():
        request.theme.add_msg(_("Only superuser is allowed to use this action."), "error")
    elif (request.method == 'POST' and
          wikiutil.checkTicket(request, form.get('ticket', ''))):
        user_name = form.get('name', '')
        key = form.get('key', '')
        val = form.get('val', '')
        if key in cfg.user_checkbox_fields:
            val = int(val)
        uid = user.getUserId(request, user_name)
        theuser = user.User(request, uid)
        oldval = getattr(theuser, key)
        setattr(theuser, key, val)
        theuser.save()
        request.theme.add_msg('%s.%s: %s -> %s' % tuple([wikiutil.escape(s) for s in [user_name, key, oldval, val]]), "info")

    Page(request, pagename).send_page()

