# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - show some statistics chart

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin import wikiutil
from MoinMoin.util import pysupport

def execute(pagename, request):
    """ Show page charts """
    _ = request.getText
    if not request.user.may.read(pagename):
        request.theme.add_msg(_("You are not allowed to view this page."), "error")
        return request.page.send_page()

    if not request.cfg.chart_options:
        request.theme.add_msg(_("Charts are not available!"), "error")
        return request.page.send_page()

    chart_type = request.values.get('type', '').strip()
    if not chart_type:
        request.theme.add_msg(_('You need to provide a chart type!'), "error")
        return request.page.send_page()

    try:
        func = pysupport.importName("MoinMoin.stats.%s" % chart_type, 'draw')
    except (ImportError, AttributeError):
        request.theme.add_msg(_('Bad chart type "%s"!') % wikiutil.escape(chart_type), "error")
        return request.page.send_page()

    func(pagename, request)

