# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - System Administration

    Web interface to do MoinMoin system administration tasks.

    @copyright: 2001-2003 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin.userform import do_user_browser
from MoinMoin.action.AttachFile import do_admin_browser

Dependencies = ["time"]

def macro_SystemAdmin(macro):
    _ = macro.request.getText
    request = macro.request

    # do not show system admin to users not in superuser list
    if not request.user.isSuperUser():
        return ''

    _MENU = {
        'attachments': (_("File attachment browser"), do_admin_browser),
        'users': (_("User account browser"), do_user_browser),
    }
    choice = request.values.get('sysadm')

    # create menu
    menuitems = [(label, fnid) for fnid, (label, handler) in _MENU.items()]
    menuitems.sort()
    result = []
    f = macro.formatter
    for label, fnid in menuitems:
        if fnid == choice:
            result.append(f.strong(1))
            result.append(f.text(label))
            result.append(f.strong(0))
        else:
            #result.append(wikiutil.link_tag(request, "%s?sysadm=%s" % (macro.formatter.page.page_name, id), label))
            result.append(f.page.link_to(request, label, querystr={'sysadm': fnid}))
        result.append(f.linebreak())
    result.append(f.linebreak())

    # add chosen content
    if choice in _MENU:
        result.append(f.rawHTML(_MENU[choice][1](request)))

    return ''.join(result)

