# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - MoinMoin.formatter.groups

    @copyright: 2009 MoinMoin:DmitrijsMilajevs
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.formatter import FormatterBase
from MoinMoin import wikiutil

class Formatter(FormatterBase):
    """
    Collect members of a group and format nothing.

    Group members are stored in the members attribute.
    """

    def __init__(self, request, **kw):
        FormatterBase.__init__(self, request, **kw)

        self.members = []
        self._bullet_list_level = 0
        self._inside_link = False
        self._new_member = ''

    def bullet_list(self, on, **kw):
        if on:
            self._bullet_list_level += 1
        else:
            self._bullet_list_level -= 1

        assert self._bullet_list_level >= 0

        return self.null()

    def listitem(self, on, **kw):
        if self._bullet_list_level == 1:
            if not on:
                stripped_new_member = self._new_member.strip()
                if stripped_new_member:
                    self.members.append(stripped_new_member)
            self._new_member = ''
        return self.null()

    def text(self, text, **kw):
        if self._bullet_list_level == 1 and not self._inside_link:
            self._new_member += text
        return self.null()

    def pagelink(self, on, pagename='', page=None, **kw):
        if self._bullet_list_level == 1:
            self._inside_link = on
            if not on:
                if not pagename and page:
                    pagename = page.page_name
                pagename = wikiutil.normalize_pagename(pagename, self.request.cfg)
                self._new_member += pagename
        return self.null()

    def null(self, *args, **kw):
        return ''

    # All these must be overriden here because they raise
    # NotImplementedError!@#! or return html?! in the base class.
    set_highlight_re = rawHTML = url = image = smiley = null
    strong = emphasis = underline = highlight = sup = sub = strike = null
    code = preformatted = small = big = code_area = code_line = null
    code_token = linebreak = paragraph = rule = icon = null
    number_list = definition_list = definition_term = definition_desc = null
    heading = table = null
    table_row = table_cell = attachment_link = attachment_image = attachment_drawing = null
    transclusion = transclusion_param = null

