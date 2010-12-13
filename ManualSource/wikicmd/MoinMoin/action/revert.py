# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - revert a page to a previous revision

    @copyright: 2000-2004 Juergen Hermann <jh@web.de>,
                2006-2008 MoinMoin:ThomasWaldmann,
                2008 MoinMoin:JohannesBerg,
                2007-2009 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin import wikiutil
from MoinMoin.Page import Page
from MoinMoin.PageEditor import PageEditor
from MoinMoin.action import ActionBase

class revert(ActionBase):
    """ revert page action

    Note: the action name is the class name
    """
    def __init__(self, pagename, request):
        ActionBase.__init__(self, pagename, request)
        self.use_ticket = True
        _ = self._
        self.form_trigger = 'revert'
        self.form_trigger_label = _('Revert')
        self.method = 'POST'

    def is_allowed(self):
        # this is not strictly necessary because the underlying storage code checks
        # as well
        _ = self._
        may = self.request.user.may
        allowed = may.write(self.pagename) and may.revert(self.pagename)
        return allowed, _('You are not allowed to revert this page!')

    def check_condition(self):
        """ checks valid page and rev """
        _ = self._
        if not self.request.rev or Page(self.request, self.pagename).current_rev() == self.request.rev:
            # same string as in PageEditor...
            note = _('You were viewing the current revision of this page when you called the revert action. '
                     'If you want to revert to an older revision, first view that older revision and '
                     'then call revert to this (older) revision again.')
            return note
        else:
            return None

    def do_action(self):
        """ revert pagename """
        form = self.form
        comment = form.get('comment', u'')
        comment = wikiutil.clean_input(comment)

        if self.request.method != 'POST':
            return False, u''

        rev = self.request.rev
        pg = PageEditor(self.request, self.pagename)

        try:
            msg = pg.revertPage(rev, comment)
            # make it show the current version...
            self.request.rev = None
        except PageEditor.RevertError, error:
            msg = unicode(error)
        except PageEditor.Unchanged, error:
            msg = unicode(error)

        return True, msg

    def get_form_html(self, buttons_html):
        """ creates the form """
        _ = self._

        d = {
            'pagename': self.pagename,
            'comment_label': _("Optional reason for reverting this page"),
            'buttons_html': buttons_html,
            'querytext': _('Really revert this page?'),
            'rev': self.request.rev
            }

        return '''
<strong>%(querytext)s</strong>
<table>
    <tr>
        <td class="label"><label>%(comment_label)s</label></td>
        <td class="content">
            <input type="text" name="comment" size="80" maxlength="200">
            <input type="hidden" name="rev" value="%(rev)d">
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
    revert(pagename, request).render()
