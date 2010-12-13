# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - "titleindex" action

    This action generates a plain list of pages, so that other wikis
    can implement http://www.usemod.com/cgi-bin/mb.pl?MetaWiki more
    easily.

    @copyright: 2001 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import config, util


def execute(pagename, request):
    # get the MIME type
    mimetype = request.values.get('mimetype', "text/plain")
    request.mimetype = mimetype

    # Get list of user readable pages
    pages = request.rootpage.getPageList()
    pages.sort()

    if mimetype == "text/xml":
        request.write('<?xml version="1.0" encoding="%s"?>\r\n' % (config.charset, ))
        request.write('<TitleIndex>\r\n')
        for name in pages:
            request.write('  <Title>%s</Title>\r\n' % (util.TranslateCDATA(name), ))
        request.write('</TitleIndex>\r\n')
    else:
        for name in pages:
            request.write(name+'\r\n')

