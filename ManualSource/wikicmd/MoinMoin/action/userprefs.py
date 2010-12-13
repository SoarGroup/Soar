# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - user settings action

    @copyright: 2006 Radomir Dopieralski
                2007, 2008 MoinMoin:JohannesBerg
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import Page, wikiutil
from MoinMoin.widget import html

def _handle_submission(request):
    """ Handle GET and POST requests of preferences forms.

    Return error msg_class, msg tuple or None, None.
    """
    _ = request.getText
    sub = request.values.get('handler')

    if sub in request.cfg.userprefs_disabled:
        return None, None

    try:
        cls = wikiutil.importPlugin(request.cfg, 'userprefs', sub, 'Settings')
    except wikiutil.PluginMissingError:
        # we never show this plugin to click on so no need to
        # give a message here
        return None, None

    obj = cls(request)
    if not obj.allowed():
        return None, None
    res = obj.handle_form()
    if isinstance(res, tuple):
        return res
    # backward compatibility for userprefs plugins,
    # they just get 'dialog'-style messages.
    return None, res

def _create_prefs_page(request, sel=None):
    _ = request.getText
    plugins = wikiutil.getPlugins('userprefs', request.cfg)
    ret = html.P()
    ret.append(html.Text(_("Please choose:")))
    ret.append(html.BR())
    items = html.UL()
    ret.append(items)
    for sub in plugins:
        if sub in request.cfg.userprefs_disabled:
            continue
        cls = wikiutil.importPlugin(request.cfg, 'userprefs', sub, 'Settings')
        obj = cls(request)
        if not obj.allowed():
            continue
        url = request.page.url(request, {'action': 'userprefs', 'sub': sub})
        lnk = html.LI().append(html.A(href=url).append(html.Text(obj.title)))
        items.append(lnk)
    return unicode(ret)


def _create_page(request, cancel=False):
    # returns text, title, msg_class, msg
    pagename = request.page.page_name

    if 'handler' in request.values:
        msg_class, msg = _handle_submission(request)
    else:
        msg_class, msg = None, None

    sub = request.args.get('sub', '')
    cls = None
    if sub and sub not in request.cfg.userprefs_disabled:
        try:
            cls = wikiutil.importPlugin(request.cfg, 'userprefs', sub, 'Settings')
        except wikiutil.PluginMissingError:
            # cls is already None
            pass

    obj = cls and cls(request)

    if not obj or not obj.allowed():
        return _create_prefs_page(request), None, msg_class, msg

    return obj.create_form(), obj.title, msg_class, msg


def execute(pagename, request):
    _ = request.getText
    if not request.user.valid:
        actname = __name__.split('.')[-1]
        request.theme.add_msg(_("You must login to use this action: %(action)s.") % {"action": actname}, "error")
        return Page.Page(request, pagename).send_page()

    text, title, msg_class, msg = _create_page(request)
    if title:
        # XXX: we would like to make "Settings" here a link back
        #      to the generic userprefs page but that is impossible
        #      due to the way the title is emitted and the theme is
        #      responsible for doing the linking....
        title = _("Settings") + ": " + title
    else:
        title = _("Settings")
    request.theme.add_msg(msg, msg_class)
    request.theme.send_title(title, page=request.page, pagename=pagename)
    # Start content (important for RTL support)
    request.write(request.formatter.startContent("content"))
    request.write(text)
    request.write(request.formatter.endContent())
    request.theme.send_footer(pagename)
    request.theme.send_closing_html()
