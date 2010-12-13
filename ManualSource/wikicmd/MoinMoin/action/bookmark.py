# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - set or delete bookmarks (in time) for RecentChanges

    @copyright: 2000-2004 by Juergen Hermann <jh@web.de>,
                2006 by MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
import time

from MoinMoin import wikiutil
from MoinMoin.Page import Page

def execute(pagename, request):
    """ set bookmarks (in time) for RecentChanges or delete them """
    _ = request.getText
    if not request.user.valid:
        actname = __name__.split('.')[-1]
        request.theme.add_msg(_("You must login to use this action: %(action)s.") % {"action": actname}, "error")
        return Page(request, pagename).send_page()

    timestamp = request.values.get('time')
    if timestamp is not None:
        if timestamp == 'del':
            tm = None
        else:
            try:
                tm = int(timestamp)
            except StandardError:
                tm = wikiutil.timestamp2version(time.time())
    else:
        tm = wikiutil.timestamp2version(time.time())

    if tm is None:
        request.user.delBookmark()
    else:
        request.user.setBookmark(tm)
    request.page.send_page()
