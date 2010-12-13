"""
    MoinMoin - Render as DocBook action - redirects to the DocBook formatter

    @copyright: 2005 MoinMoin:AlexanderSchremmer
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.Page import Page

def execute(pagename, request):
    url = Page(request, pagename).url(request, {'action': 'show', 'mimetype': 'text/docbook'})
    return request.http_redirect(url)

