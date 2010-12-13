"""
   MoinMoin - Subscribeuser - Action
   Subscribe (or unsubscribe) a user to a page.

   @copyright: 2003 Daniela Nicklas <nicklas@informatik.uni-stuttgart.de>,
               2005 MoinMoin:AlexanderSchremmer,
               2009 MoinMoin:ThomasWaldmann
   @license: GNU GPL, see COPYING for details.
"""

import sys, os, re

from MoinMoin.Page import Page
from MoinMoin import user
from MoinMoin import wikiutil


def show_form(pagename, request):
    _ = request.getText
    request.theme.send_title(_("Subscribe users to the page %s") % pagename, pagename=pagename)

    request.write("""
<form action="%s" method="POST" enctype="multipart/form-data">
<input type="hidden" name="action" value="SubscribeUser">
%s <input type="text" name="users" size="50">
<input type="submit" value="Subscribe">
</form>
""" % (request.href(pagename),
      _("Enter user names (comma separated):")))
    request.theme.send_footer(pagename)
    request.theme.send_closing_html()


def parse_re(usernames):
    username_regexes = []
    for name in usernames:
        if name.startswith("re:"):
            name = name[3:]
        else:
            name = re.escape(name)
        username_regexes.append(name)
    return username_regexes


def parse_userlist(usernames):
    subscribe = []
    unsubscribe = []
    for name in usernames:
        if name.startswith("-"):
            unsubscribe.append(name[1:])
        elif name.startswith("+"):
            subscribe.append(name[1:])
        else:
            subscribe.append(name)
    return parse_re(subscribe), parse_re(unsubscribe)


def show_result(pagename, request):
    _ = request.getText

    request.theme.send_title(_("Subscribed for %s:") % pagename, pagename=pagename)

    from MoinMoin.formatter.text_html import Formatter
    formatter = Formatter(request)

    usernames = request.form['users'].split(",")
    subscribe, unsubscribe = parse_userlist(usernames)

    result = subscribe_users(request, subscribe, unsubscribe, pagename, formatter)
    request.write(result)

    request.theme.send_footer(pagename)
    request.theme.send_closing_html()


def subscribe_users(request, subscribe, unsubscribe, pagename, formatter):
    _ = request.getText

    if not Page(request, pagename).exists():
        return u"Page does not exist."

    result = []
    did_match = {}

    # get user object - only with IDs!
    for userid in user.getUserList(request):
        userobj = user.User(request, userid)
        name = userobj.name

        matched = subscribed = False

        for name_re in unsubscribe:
            if re.match(name_re, name, re.U):
                matched = did_match[name_re] = True
                if (not userobj.isSubscribedTo([pagename]) or
                    userobj.unsubscribe(pagename)):
                    subscribed = False
                break

        for name_re in subscribe:
            if re.match(name_re, name, re.U):
                matched = did_match[name_re] = True
                if (userobj.isSubscribedTo([pagename]) or
                    (userobj.email or userobj.jid) and userobj.subscribe(pagename)):
                    subscribed = True
                break

        if matched:
            result.extend([formatter.smiley(subscribed and '{*}' or '{o}'),
                           formatter.text(" "),
                           formatter.url(1, Page(request, name).url(request)),
                           formatter.text(name),
                           formatter.url(0),
                           formatter.linebreak(preformatted=0),
                          ])

    result.extend([''.join([formatter.smiley('{X}'),
                            formatter.text(" " + _("Not a user:") + " " + name_re),
                            formatter.linebreak(preformatted=0)])
                   for name_re in subscribe + unsubscribe if name_re not in did_match])

    return ''.join(result)


def execute(pagename, request):
    _ = request.getText
    if not request.user.may.admin(pagename):
        thispage = Page(request, pagename)
        request.theme.add_msg(_("You are not allowed to perform this action."), "error")
        return thispage.send_page()
    elif 'users' not in request.form:
        show_form(pagename, request)
    else:
        show_result(pagename, request)


if __name__ == '__main__':
    args = sys.argv
    if len(args) < 2:
        print >>sys.stderr, """Subscribe users

%(myname)s pagename [+|-][re:]username[,username[,username[,...]]] [URL]

+username: subscribes user <username> to page <pagename>.
-username: unsubscribes user <username> from page <pagename>.
+re:username_re: subscribes users who match <username_re> regex.
-re:username_re: unsubscribes users who match <username_re> regex.

URL is just needed for a farmconfig scenario.

Example:
%(myname)s FrontPage TestUser,MatthewSimpson

""" % {"myname": os.path.basename(args[0])}
        raise SystemExit

    pagename = args[1]
    usernames = args[2].split(",")

    if len(args) > 3:
        request_url = args[3]
    else:
        request_url = None

    # Setup MoinMoin environment
    from MoinMoin.web.contexts import ScriptContext
    request = ScriptContext(url=request_url)

    from MoinMoin.formatter.text_plain import Formatter
    formatter = Formatter(request)

    subscribe, unsubscribe = parse_userlist(usernames)

    print subscribe_users(request, subscribe, unsubscribe, pagename, formatter)

