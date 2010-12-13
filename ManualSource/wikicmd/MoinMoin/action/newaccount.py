# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - create account action

    @copyright: 2007 MoinMoin:JohannesBerg
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import user, wikiutil
from MoinMoin.Page import Page
from MoinMoin.widget import html
from MoinMoin.security.textcha import TextCha
from MoinMoin.auth import MoinAuth


def _create_user(request):
    _ = request.getText
    form = request.form

    if request.method != 'POST':
        return

    if not wikiutil.checkTicket(request, form.get('ticket', '')):
        return

    if not TextCha(request).check_answer_from_form():
        return _('TextCha: Wrong answer! Go back and try again...')

    # Create user profile
    theuser = user.User(request, auth_method="new-user")

    # Require non-empty name
    try:
        theuser.name = form['name']
    except KeyError:
        return _("Empty user name. Please enter a user name.")

    # Don't allow creating users with invalid names
    if not user.isValidName(request, theuser.name):
        return _("""Invalid user name {{{'%s'}}}.
Name may contain any Unicode alpha numeric character, with optional one
space between words. Group page name is not allowed.""", wiki=True) % wikiutil.escape(theuser.name)

    # Name required to be unique. Check if name belong to another user.
    if user.getUserId(request, theuser.name):
        return _("This user name already belongs to somebody else.")

    # try to get the password and pw repeat
    password = form.get('password1', '')
    password2 = form.get('password2', '')

    # Check if password is given and matches with password repeat
    if password != password2:
        return _("Passwords don't match!")
    if not password:
        return _("Please specify a password!")

    pw_checker = request.cfg.password_checker
    if pw_checker:
        pw_error = pw_checker(request, theuser.name, password)
        if pw_error:
            return _("Password not acceptable: %s") % wikiutil.escape(pw_error)

    # Encode password
    if password and not password.startswith('{SHA}'):
        try:
            theuser.enc_password = user.encodePassword(password)
        except UnicodeError, err:
            # Should never happen
            return "Can't encode password: %s" % wikiutil.escape(str(err))

    # try to get the email, for new users it is required
    email = wikiutil.clean_input(form.get('email', ''))
    theuser.email = email.strip()
    if not theuser.email and 'email' not in request.cfg.user_form_remove:
        return _("Please provide your email address. If you lose your"
                 " login information, you can get it by email.")

    # Email should be unique - see also MoinMoin/script/accounts/moin_usercheck.py
    if theuser.email and request.cfg.user_email_unique:
        if user.get_by_email_address(request, theuser.email):
            return _("This email already belongs to somebody else.")

    # save data
    theuser.save()

    result = _("User account created! You can use this account to login now...")
    return result


def _create_form(request):
    _ = request.getText
    url = request.page.url(request)
    ret = html.FORM(action=url)
    ret.append(html.INPUT(type='hidden', name='action', value='newaccount'))

    ticket = wikiutil.createTicket(request)
    ret.append(html.INPUT(type="hidden", name="ticket", value="%s" % ticket))

    lang_attr = request.theme.ui_lang_attr()
    ret.append(html.Raw('<div class="userpref"%s>' % lang_attr))
    tbl = html.TABLE(border="0")
    ret.append(tbl)
    ret.append(html.Raw('</div>'))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(
                                  html.Text(_("Name")))))
    cell = html.TD()
    row.append(cell)
    cell.append(html.INPUT(type="text", size="36", name="name"))
    cell.append(html.Text(' ' + _("(Use FirstnameLastname)")))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(
                                  html.Text(_("Password")))))
    row.append(html.TD().append(html.INPUT(type="password", size="36",
                                           name="password1")))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(
                                  html.Text(_("Password repeat")))))
    row.append(html.TD().append(html.INPUT(type="password", size="36",
                                           name="password2")))

    row = html.TR()
    tbl.append(row)
    row.append(html.TD().append(html.STRONG().append(html.Text(_("Email")))))
    row.append(html.TD().append(html.INPUT(type="text", size="36",
                                           name="email")))

    textcha = TextCha(request)
    if textcha.is_enabled():
        row = html.TR()
        tbl.append(row)
        row.append(html.TD().append(html.STRONG().append(
                                      html.Text(_('TextCha (required)')))))
        td = html.TD()
        if textcha:
            td.append(textcha.render())
        row.append(td)

    row = html.TR()
    tbl.append(row)
    row.append(html.TD())
    td = html.TD()
    row.append(td)
    td.append(html.INPUT(type="submit", name="create",
                         value=_('Create Profile')))

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
    form = request.form

    submitted = form.has_key('create')

    if submitted: # user pressed create button
        request.theme.add_msg(_create_user(request), "dialog")
        return page.send_page()
    else: # show create form
        request.theme.send_title(_("Create Account"), pagename=pagename)

        request.write(request.formatter.startContent("content"))

        # THIS IS A BIG HACK. IT NEEDS TO BE CLEANED UP
        request.write(_create_form(request))

        request.write(request.formatter.endContent())

        request.theme.send_footer(pagename)
        request.theme.send_closing_html()

