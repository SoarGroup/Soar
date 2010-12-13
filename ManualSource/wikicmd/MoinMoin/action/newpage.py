"""
    MoinMoin - newpage action

    Create a new page with optional template. Can be used with NewPage.py macro.

    @copyright: 2004 Vito Miliano (vito_moinnewpagewithtemplate@perilith.com),
                2004 Nir Soffer <nirs@freeshell.org>,
                2004 Alexander Schremmer <alex AT alexanderweb DOT de>
                2008 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""

import time
from MoinMoin.Page import Page

class NewPage:
    """ Open editor for a new page, using template """

    def __init__(self, request, referrer):
        self.request = request
        self.referrer = referrer # The page the user came from
        self.pagename = self.request.values.get('pagename')
        self.nametemplate = self.request.values.get('nametemplate', '%s')
        self.nametemplate = self.nametemplate.replace('\x00', '')

    def checkAndCombineArguments(self):
        """ Check arguments in form, return error msg

        @rtype: unicode
        @return: error message
        """
        _ = self.request.getText
        need_replace = '%s' in self.nametemplate
        if not self.pagename and need_replace:
            return _("Cannot create a new page without a page name."
                     "  Please specify a page name.")
        if need_replace:
        # generate a string that can be safely used as the pagename
        # template variable
            repl = 'A@'
            i = 0
            while repl in self.nametemplate:
                repl += ['#', '&', '$', 'x', 'X', ':', '@'][i]
                i += 1
                i = i % 7
            template = self.nametemplate.replace('%s', repl)
        else:
            template = self.nametemplate
        template = template.encode('utf-8')
        template = time.strftime(template, self.request.user.getTime(time.time()))
        template = template.decode('utf-8')
        if need_replace:
            self.pagename = template.replace(repl, self.pagename)
        else:
            self.pagename = template
        return ''

    def checkPermissions(self):
        """ Check write permission in form, return error msg

        @rtype: unicode
        @return: error message
        """
        _ = self.request.getText
        page = Page(self.request, self.pagename)
        if not (page.isWritable() and self.request.user.may.read(self.pagename)):
            # Same error as the edit page for localization reasons
            return _('You are not allowed to edit this page.')
        return ''

    def render(self):
        """ Redirect to the new page, using edit action and template """
        error = self.checkAndCombineArguments() or self.checkPermissions()
        if error:
            # Send back to the page you came from, with an error msg
            page = Page(self.request, self.referrer)
            self.request.theme.add_msg(error, "error")
            page.send_page()
        else:
            # Redirect to new page using edit action. No error checking
            # is needed because it is done later in new request.
            pagename = self.pagename
            query = {'action': 'edit', 'backto': self.referrer}

            template = self.request.values.get('template', '')
            if template:
                query['template'] = template

            parent = self.request.values.get('parent', '')
            if parent:
                pagename = "%s/%s" % (parent, pagename)

            url = Page(self.request, pagename).url(self.request, query)
            self.request.http_redirect(url)

        return ''

def execute(pagename, request):
    """ Temporary glue code for current moin action system """
    if request.method != 'POST':
        return False, u''

    return NewPage(request, pagename).render()

