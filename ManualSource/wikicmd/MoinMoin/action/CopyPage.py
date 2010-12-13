# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - CopyPage action

    This action allows you to copy a page.

    @copyright: 2007 MoinMoin:ThomasWaldmann,
                2007-2009 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""
import re
from MoinMoin import wikiutil
from MoinMoin.Page import Page
from MoinMoin.PageEditor import PageEditor
from MoinMoin.action import ActionBase
from MoinMoin.security.textcha import TextCha

class CopyPage(ActionBase):
    """ Copy page action

    Note: the action name is the class name
    """
    def __init__(self, pagename, request):
        ActionBase.__init__(self, pagename, request)
        self.use_ticket = True
        _ = self._
        self.form_trigger = 'copy'
        self.form_trigger_label = _('Copy Page')
        filterfn = re.compile(ur"^%s/.*$" % re.escape(pagename), re.U).match
        pages = request.rootpage.getPageList(user='', exists=1, filter=filterfn)
        subpagenames = request.rootpage.getPageList(user='', exists=1, filter=filterfn)
        self.subpages = subpagenames
        self.users_subpages = [pagename for pagename in subpagenames if self.request.user.may.read(pagename)]

    def is_allowed(self):
        may = self.request.user.may
        return may.read(self.pagename)

    def check_condition(self):
        _ = self._
        if not self.page.exists():
            return _('This page is already deleted or was never created!')
        else:
            return None

    def do_action(self):
        """ copy this page to "pagename" """
        _ = self._
        # Currently we only check TextCha for upload (this is what spammers ususally do),
        # but it could be extended to more/all attachment write access
        if not TextCha(self.request).check_answer_from_form():
            return status, _('TextCha: Wrong answer! Go back and try again...')

        form = self.form
        newpagename = form.get('newpagename', u'')
        newpagename = wikiutil.normalize_pagename(newpagename, self.cfg)
        comment = form.get('comment', u'')
        comment = wikiutil.clean_input(comment)

        self.page = PageEditor(self.request, self.pagename)
        success, msgs = self.page.copyPage(newpagename, comment)

        copy_subpages = 0
        try:
            copy_subpages = int(form['copy_subpages'])
        except:
            pass

        if copy_subpages and self.subpages or (not self.users_subpages and self.subpages):
            for name in self.subpages:
                self.page = PageEditor(self.request, name)
                new_subpagename = name.replace(self.pagename, newpagename, 1)
                success_i, msg = self.page.copyPage(new_subpagename, comment)
                msgs = "%s %s" % (msgs, msg)

        self.newpagename = newpagename # keep there for finish
        return success, msgs

    def do_action_finish(self, success):
        if success:
            url = Page(self.request, self.newpagename).url(self.request)
            self.request.http_redirect(url)
        else:
            self.render_msg(self.make_form(), "dialog")

    def get_form_html(self, buttons_html):
        _ = self._
        if self.users_subpages:
            subpages = ' '.join([wikiutil.escape(page) for page in self.users_subpages])

            d = {
                'textcha': TextCha(self.request).render(),
                'subpage': subpages,
                'subpages_checked': ('', 'checked')[self.request.args.get('subpages_checked', '0') == '1'],
                'subpage_label': _('Copy all /subpages too?'),
                'pagename': wikiutil.escape(self.pagename, True),
                'newname_label': _("New name"),
                'comment_label': _("Optional reason for the copying"),
                'buttons_html': buttons_html,
                'querytext': _('Really copy this page?')
                }

            return '''
<strong>%(querytext)s</strong>
<br>
<br>
%(textcha)s
<table>
    <tr>
    <dd>
        %(subpage_label)s<input type="checkbox" name="copy_subpages" value="1" %(subpages_checked)s>
    </dd>
    <dd>
        <class="label"><subpage> %(subpage)s</subpage>
    </dd>
    </tr>
</table>
<table>
    <tr>
        <td class="label"><label>%(newname_label)s</label></td>
        <td class="content">
            <input type="text" name="newpagename" value="%(pagename)s" size="80">
        </td>
    </tr>
    <tr>
        <td class="label"><label>%(comment_label)s</label></td>
        <td class="content">
            <input type="text" name="comment" size="80" maxlength="200">
        </td>
    </tr>
    <tr>
        <td></td>
        <td class="buttons">
            %(buttons_html)s
        </td>
    </tr>
</table>
''' % d

        else:
            d = {
                'textcha': TextCha(self.request).render(),
                'pagename': wikiutil.escape(self.pagename, True),
                'newname_label': _("New name"),
                'comment_label': _("Optional reason for the copying"),
                'buttons_html': buttons_html,
                }
            return '''
%(textcha)s
<table>
    <tr>
        <td class="label"><label>%(newname_label)s</label></td>
        <td class="content">
            <input type="text" name="newpagename" value="%(pagename)s" size="80">
        </td>
    </tr>
    <tr>
        <td class="label"><label>%(comment_label)s</label></td>
        <td class="content">
            <input type="text" name="comment" size="80" maxlength="200">
        </td>
    </tr>
    <tr>
        <td></td>
        <td class="buttons">
            %(buttons_html)s
        </td>
    </tr>
</table>
''' % d

def execute(pagename, request):
    """ Glue code for actions """
    CopyPage(pagename, request).render()

