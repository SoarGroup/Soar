# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - login action

    The real login is done in MoinMoin.request.
    Here is only some user notification in case something went wrong.

    @copyright: 2005-2006 Radomirs Cirskis <nad2000@gmail.com>,
                2006 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import userform, wikiutil
from MoinMoin.Page import Page
from MoinMoin.widget import html

def execute(pagename, request):
    return LoginHandler(pagename, request).handle()

class LoginHandler:
    def __init__(self, pagename, request):
        self.request = request
        self._ = request.getText
        self.cfg = request.cfg
        self.pagename = pagename
        self.page = Page(request, pagename)

    def handle_multistage(self):
        """Handle a multistage request.

        If the auth handler wants a multistage request, we
        now set up the login form for that.
        """
        _ = self._
        request = self.request
        form = html.FORM(method='POST', name='logincontinue', action=self.pagename)
        form.append(html.INPUT(type='hidden', name='action', value='login'))
        form.append(html.INPUT(type='hidden', name='login', value='1'))
        form.append(html.INPUT(type='hidden', name='stage',
                               value=request._login_multistage_name))

        request.theme.send_title(_("Login"), pagename=self.pagename)
        # Start content (important for RTL support)
        request.write(request.formatter.startContent("content"))

        extra = request._login_multistage(request, form)
        request.write(unicode(form))
        if extra:
            request.write(extra)

        request.write(request.formatter.endContent())
        request.theme.send_footer(self.pagename)
        request.theme.send_closing_html()

    def handle(self):
        _ = self._
        request = self.request
        form = request.values

        error = None

        islogin = form.get('login', '')

        if islogin: # user pressed login button
            if request._login_multistage:
                return self.handle_multistage()
            if hasattr(request, '_login_messages'):
                for msg in request._login_messages:
                    request.theme.add_msg(wikiutil.escape(msg), "error")
            return self.page.send_page()

        else: # show login form
            request.theme.send_title(_("Login"), pagename=self.pagename)
            # Start content (important for RTL support)
            request.write(request.formatter.startContent("content"))

            request.write(userform.getLogin(request))

            request.write(request.formatter.endContent())
            request.theme.send_footer(self.pagename)
            request.theme.send_closing_html()

