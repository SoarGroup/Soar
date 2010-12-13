# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Action for saving a page

    @copyright: 2007 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.Page import Page

def execute(pagename, request):
    if not request.user.may.read(pagename):
        Page(request, pagename).send_page()
    else:
        rev = request.rev or 0
        Page(request, pagename, rev=rev).send_raw(content_disposition='attachment')
