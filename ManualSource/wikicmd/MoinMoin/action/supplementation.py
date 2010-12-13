# -*- coding: iso-8859-1 -*-
"""
    MoinMoin -  This Action is used to create a supplementation subpage e.g. a Discussion page below a comon page

    Note:
    derived from the newpage macro by Vito Miliano (vito_moinnewpagewithtemplate@perilith.com) et al

    @copyright: 2006-2007 MoinMoin:ReimarBauer
    @license: GNU GPL, see COPYING for details.
"""
from MoinMoin.Page import Page
from MoinMoin.wikiutil import quoteWikinameURL

def execute(pagename, request):
    _ = request.getText
    sub_page_name = request.cfg.supplementation_page_name
    sub_page_template = request.cfg.supplementation_page_template
    newpagename = "%s/%s" % (pagename, sub_page_name)
    errormsg = _("You are not allowed to create the supplementation page.")

    if pagename.endswith(sub_page_name): # sub_sub_page redirects to sub_page
        query = {}
        url = Page(request, pagename).url(request, query)
        request.http_redirect(url)
    elif request.user.may.read(newpagename):
        query = {}
        url = Page(request, newpagename).url(request, query)
        test = Page(request, newpagename)
        if test.exists(): # page is defined -> redirect
            request.http_redirect(url)
        elif request.user.may.write(newpagename):  # page will be created from template
            query = {'action': 'edit', 'backto': newpagename, 'template': quoteWikinameURL(sub_page_template)}
            url = Page(request, newpagename).url(request, query)
            request.http_redirect(url)
        else:
            request.theme.add_msg(errormsg, "error")
    else:
        request.theme.add_msg(errormsg, "error")

