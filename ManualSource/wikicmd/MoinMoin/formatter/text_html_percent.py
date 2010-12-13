# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - a special text/html formatter used by the i18n system

    If texts translated by the i18n system are used on the left side of a
    % operator, any markup-generated % char (e.g. in links to non-ASCII page
    names (%XX%XX%XX)) needs to get escaped (%%XX%%XX%%XX).

    Everything else is as in the text/html formatter.

    @copyright: 2007 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.formatter.text_html import Formatter as TextHtmlFormatter

class Formatter(TextHtmlFormatter):

    def _open(self, tag, newline=False, attr=None, allowed_attrs=None, **kw):
        """ Escape % characters in tags, see also text_html.Formatter._open. """
        tagstr = TextHtmlFormatter._open(self, tag, newline, attr, allowed_attrs, **kw)
        return tagstr.replace('%', '%%')

    # override more methods, if needed
