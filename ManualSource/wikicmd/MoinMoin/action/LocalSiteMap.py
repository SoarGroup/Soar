# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - LocalSiteMap action

    The LocalSiteMap action gives you a page that shows
    nearby links.  This is an example of what appears on the
    page (names are linkable on the real page):

    MoinMoin
         GarthKidd
              OrphanedPages
              WantedPages
         JoeDoe
              CategoryHomepage
                   CategoryCategory
                   WikiHomePage
              JoeWishes
              WikiWiki
                   OriginalWiki

    @copyright: 2001 Steve Howell <showell@zipcon.com>,
                2001-2004 Juergen Hermann <jh@web.de>
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import wikiutil
from MoinMoin.Page import Page

class MaxNodesReachedException(Exception):
    pass

def execute(pagename, request):
    _ = request.getText

    # This action generate data using the user language
    request.setContentLanguage(request.lang)

    request.theme.send_title(_('Local Site Map for "%s"') % (pagename), pagename=pagename)

    # Start content - IMPORTANT - witout content div, there is no
    # direction support!
    request.write(request.formatter.startContent("content"))

    request.write(LocalSiteMap(pagename).output(request))

    request.write(request.formatter.endContent()) # end content div
    request.theme.send_footer(pagename)
    request.theme.send_closing_html()

class LocalSiteMap:
    def __init__(self, name):
        self.name = name
        self.result = []

    def output(self, request):
        tree = PageTreeBuilder(request).build_tree(self.name)
        #self.append("<small>")
        tree.depth_first_visit(request, self)
        #self.append("</small>")
        return """
<p>
%s
</p>
""" % ''.join(self.result)

    def visit(self, request, name, depth):
        """ Visit a page, i.e. create a link.
        """
        if not name:
            return
        _ = request.getText
        pg = Page(request, name)
        action = __name__.split('.')[-1]
        self.append('&nbsp;' * (5*depth+1))
        self.append(pg.link_to(request, querystr={'action': action}))
        self.append("&nbsp;<small>[")
        self.append(pg.link_to(request, _('view')))
        self.append("</small>]<br>")

    def append(self, text):
        self.result.append(text)


class PageTreeBuilder:
    def __init__(self, request):
        self.request = request
        self.children = {}
        self.numnodes = 0
        self.maxnodes = 35

    def mark_child(self, name):
        self.children[name] = 1

    def child_marked(self, name):
        return name in self.children

    def is_ok(self, child):
        if not self.child_marked(child):
            if not self.request.user.may.read(child):
                return 0
            if Page(self.request, child).exists():
                self.mark_child(child)
                return 1
        return 0

    def new_kids(self, name):
        # does not recurse
        kids = []
        for child in Page(self.request, name).getPageLinks(self.request):
            if self.is_ok(child):
                kids.append(child)
        return kids

    def new_node(self):
        self.numnodes = self.numnodes + 1
        if self.numnodes == self.maxnodes:
            raise MaxNodesReachedException

    def build_tree(self, name):
        self.mark_child(name)
        tree = Tree(name)
        try:
            self.recurse_build([tree], 1)
        except MaxNodesReachedException:
            pass
        return tree

    def recurse_build(self, trees, depth):
        all_kids = []
        for tree in trees:
            kids = self.new_kids(tree.node)
            for kid in kids:
                newTree = Tree(kid)
                tree.append(newTree)
                self.new_node()
                all_kids.append(newTree)
        if len(all_kids):
            self.recurse_build(all_kids, depth+1)

class Tree:
    def __init__(self, node):
        self.node = node
        self.children = []

    def append(self, node):
        self.children.append(node)

    def depth_first_visit(self, request, visitor, depth=0):
        visitor.visit(request, self.node, depth)
        for c in self.children:
            c.depth_first_visit(request, visitor, depth+1)

