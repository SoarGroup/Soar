# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - create account action

    @copyright: 2007 MoinMoin:JohannesBerg
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import user, wikiutil
from MoinMoin.Page import Page
from MoinMoin.widget import html
from MoinMoin.auth import MoinAuth

def _do_email(request, u):
    _ = request.getText

    if u and u.valid:
        is_ok, msg = u.mailAccountData()
        if not is_ok:
            return wikiutil.escape(msg)

    return _("If this account exists an email was sent.")


def _do_recover(request):
    _ = request.getText
    form = request.form
    if not request.cfg.mail_enabled:
        return _("""This wiki is not enabled for mail processing.
Contact the owner of the wiki, who can enable email.""")

    try:
        email = wikiutil.clean_input(form['email'].lower())
        if not email:
            # continue if email not given
            raise KeyError

        u = user.get_by_email_address(request, email)

        return _do_email(request, u)
    except KeyError:
        pass

    try:
        username = wikiutil.clean_input(form['name'])
        if not username:
            # continue if name not given
            raise KeyError

        u = user.User(request, user.getUserId(request, username))

        return _do_email(request, u)
    except KeyError:
        pass

    # neither succeeded, give error message
    return _("Please provide a valid email address or a username!")


def _create_form(request):
    _ = request.getText
    url = request.page.url(request)
    ret = html.FORM(action=url)
    ret.append(html.INPUT(type='hidden', name='action', value='recoverpass'))
    lang_attr = request.theme.ui_lang_attr()
    ret.append(html.Raw('<div class="userpref"%s>' % lang_attr))
    tbl = html.TABLE(border="0")
    ret.append(tbl)
    ret.append(html.Raw('</div>'))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(html.Text(_("Username")))))
    row.append(html.TD().append(html.INPUT(type="text", size="36",
                                           name="name")))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(html.Text(_("Email")))))
    row.append(html.TD().append(html.INPUT(type="text", size="36",
                                           name="email")))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD())
    td = html.TD()
    row.append(td)
    td.append(html.INPUT(type="submit", name="account_sendmail",
                         value=_('Mail me my account data')))

    return unicode(ret)


def _create_token_form(request, name=None, token=None):
    _ = request.getText
    url = request.page.url(request)
    ret = html.FORM(action=url)
    ret.append(html.INPUT(type='hidden', name='action', value='recoverpass'))
    lang_attr = request.theme.ui_lang_attr()
    ret.append(html.Raw('<div class="userpref"%s>' % lang_attr))
    tbl = html.TABLE(border="0")
    ret.append(tbl)
    ret.append(html.Raw('</div>'))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(html.Text(_("Username")))))
    value = name or ''
    row.append(html.TD().append(html.INPUT(type='text', size="36",
                                           name="name", value=value)))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(html.Text(_("Recovery token")))))
    value = token or ''
    row.append(html.TD().append(html.INPUT(type='text', size="36",
                                           name="token", value=value)))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(html.Text(_("New password")))))
    row.append(html.TD().append(html.INPUT(type="password", size="36",
                                           name="password")))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(html.Text(_("New password (repeat)")))))
    row.append(html.TD().append(html.INPUT(type="password", size="36",
                                           name="password_repeat")))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD())
    td = html.TD()
    row.append(td)
    td.append(html.INPUT(type="submit", name="recover", value=_('Reset my password')))

    return unicode(ret)


def execute(pagename, request):
    found = False
    for auth in request.cfg.auth:
        if isinstance(auth, MoinAuth):
            found = True
            break

    if not found:
        # we will not have linked, so forbid access
        request.makeForbidden(403, 'No MoinAuth in auth list')
        return

    page = Page(request, pagename)
    _ = request.getText
    form = request.values # link in mail -> GET request

    if not request.cfg.mail_enabled:
        request.theme.add_msg(_("""This wiki is not enabled for mail processing.
Contact the owner of the wiki, who can enable email."""), 'warning')
        page.send_page()
        return

    submitted = form.get('account_sendmail', '')
    token = form.get('token', '')
    newpass = form.get('password', '')
    name = form.get('name', '')

    if token and name and newpass:
        newpass2 = form.get('password_repeat', '')
        msg = _("Passwords don't match!")
        msg_type = 'error'
        if newpass == newpass2:
            pw_checker = request.cfg.password_checker
            pw_error = None
            if pw_checker:
                pw_error = pw_checker(request, name, newpass)
                if pw_error:
                    msg = _("Password not acceptable: %s") % wikiutil.escape(pw_error)
            if not pw_error:
                u = user.User(request, user.getUserId(request, name))
                if u and u.valid and u.apply_recovery_token(token, newpass):
                    msg = _("Your password has been changed, you can log in now.")
                    msg_type = 'info'
                else:
                    msg = _('Your token is invalid!')
        if msg:
            request.theme.add_msg(msg, msg_type)
        if msg_type != 'error':
            page.send_page()
            return

    if token and name:
        request.theme.send_title(_("Password reset"), pagename=pagename)

        request.write(request.formatter.startContent("content"))

        request.write(_("""
== Password reset ==
Enter a new password below.""", wiki=True))
        request.write(_create_token_form(request, name=name, token=token))

        request.write(request.formatter.endContent())

        request.theme.send_footer(pagename)
        request.theme.send_closing_html()
    elif submitted: # user pressed create button
        if request.method != 'POST':
            return
        msg = _do_recover(request)
        request.theme.add_msg(msg, "dialog")
        page.send_page()
    else: # show create form
        request.theme.send_title(_("Lost password"), pagename=pagename)

        request.write(request.formatter.startContent("content"))

        request.write(_("""
== Recovering a lost password ==
If you have forgotten your password, provide your email address or
username and click on '''Mail me my account data'''.
You will receive an email containing a recovery token that can be
used to change your password. The email will also contain further
instructions.""", wiki=True))

        request.write(_create_form(request))

        request.write(request.formatter.rule())

        request.write(_("""
=== Password reset ===
If you already have received the email with the recovery token, enter your
username, the recovery token and a new password (twice) below.""", wiki=True))

        request.write(_create_token_form(request))

        request.write(request.formatter.endContent())

        request.theme.send_footer(pagename)
        request.theme.send_closing_html()
