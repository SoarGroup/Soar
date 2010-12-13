# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - MyPages - assisting creation of Homepage subpages

    @copyright: 2005-2006 Bastian Blank, Florian Festi, Thomas Waldmann
    @license: GNU GPL, see COPYING for details.
"""

def execute(pagename, request):
    from MoinMoin import wikiutil
    from MoinMoin.Page import Page

    _ = request.getText
    thispage = Page(request, pagename)

    if request.user.valid:
        username = request.user.name
    else:
        username = ''

    if not username:
        request.theme.add_msg(_('Please log in first.'), "error")
        return thispage.send_page()

    userhomewiki = request.cfg.user_homewiki
    if userhomewiki != 'Self' and userhomewiki != request.cfg.interwikiname:
        interwiki = wikiutil.getInterwikiHomePage(request, username=username)
        wikitag, wikiurl, wikitail, wikitag_bad = wikiutil.resolve_interwiki(request, *interwiki)
        wikiurl = wikiutil.mapURL(request, wikiurl)
        homepageurl = wikiutil.join_wiki(wikiurl, wikitail)
        request.http_redirect('%s?action=MyPages' % homepageurl)

    homepage = Page(request, username)
    if not homepage.exists():
        request.theme.add_msg(_('Please first create a homepage before creating additional pages.'), "error")
        return homepage.send_page()

    pagecontent = _("""\
You can add some additional sub pages to your already existing homepage here.

You can choose how open to other readers or writers those pages shall be,
access is controlled by group membership of the corresponding group page.

Just enter the sub page's name and click on the button to create a new page.

Before creating access protected pages, make sure the corresponding group page
exists and has the appropriate members in it. Use HomepageGroupsTemplate for creating
the group pages.

||'''Add a new personal page:'''||'''Related access control list group:'''||
||<<NewPage(HomepageReadWritePageTemplate,read-write page,%(username)s)>>||[[%(username)s/ReadWriteGroup]]||
||<<NewPage(HomepageReadPageTemplate,read-only page,%(username)s)>>||[[%(username)s/ReadGroup]]||
||<<NewPage(HomepagePrivatePageTemplate,private page,%(username)s)>>||%(username)s only||

""")
    pagecontent = pagecontent % locals()

    pagecontent = pagecontent.replace('\n', '\r\n')

    from MoinMoin.parser.text_moin_wiki import Parser as WikiParser

    # This action generate data using the user language
    request.setContentLanguage(request.lang)
    request.theme.send_title(_('MyPages management'), page=homepage)

    parser = WikiParser(pagecontent, request)
    p = Page(request, "$$$")
    request.formatter.setPage(p)
    parser.format(request.formatter)

    # Start content - IMPORTANT - without content div, there is no direction support!
    request.write(request.formatter.startContent("content"))

    request.write(request.formatter.endContent())
    request.theme.send_footer(homepage.page_name)
    request.theme.send_closing_html()

