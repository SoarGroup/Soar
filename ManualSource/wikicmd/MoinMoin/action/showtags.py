# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - "showtags" action

    This action shows all sync tags related to a specific page.

    @copyright: 2006 MoinMoin:AlexanderSchremmer
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import config
from MoinMoin.Page import Page
from MoinMoin.wikisync import TagStore

def execute(pagename, request):
    request.mimetype = "text/plain"

    page = Page(request, pagename)
    tags = TagStore(page)
    request.write(tags.dump())

