# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - New Page macro

    Thanks to Jos Yule's "blogpost" action and his modified Form for
    giving me the pieces I needed to figure all this stuff out: MoinMoin:JosYule

    @copyright: 2004 Vito Miliano (vito_moinnewpagewithtemplate@perilith.com),
                2004 by Nir Soffer <nirs@freeshell.org>,
                2004 Alexander Schremmer <alex AT alexanderweb DOT de>,
                2006-2008 MoinMoin:ReimarBauer
                2008 MoinMoin:RadomirDopieralski
    @license: GNU GPL, see COPYING for details.
"""

from MoinMoin import wikiutil

Dependencies = ["language"]

class NewPage:
    """ NewPage - create new pages

    Let you create new page using optional template, button text
    and parent page (for automatic subpages).

    Usage:

        <<NewPage(template, buttonLabel, parentPage)>>

    Examples:

        <<NewPage>>

            Create an input field with 'Create New Page' button. The new
            page will not use a template.

        <<NewPage(BugTemplate, Create New Bug, MoinMoinBugs)>>

            Create an input field with button labeled 'Create New
            Bug'.  The new page will use the BugTemplate template,
            and create the page as a subpage of MoinMoinBugs.
    """

    def __init__(self, macro, template=u'', button_label=u'',
                 parent_page=u'', name_template=u'%s'):
        self.macro = macro
        self.request = macro.request
        self.formatter = macro.formatter
        self.template = template
        _ = self.request.getText
        if button_label:
            # Try to get a translation, this will probably not work in
            # most cases, but better than nothing.
            self.label = self.request.getText(button_label)
        else:
            self.label = _("Create New Page")
        if parent_page == '@ME' and self.request.user.valid:
            self.parent = self.request.user.name
        elif parent_page == '@SELF':
            self.parent = self.formatter.page.page_name
        else:
            self.parent = parent_page
        self.nametemplate = name_template

    def renderInPage(self):
        """ Render macro in page context

        The parser should decide what to do if this macro is placed in a
        paragraph context.
        """
        f = self.formatter
        _ = self.request.getText

        requires_input = '%s' in self.nametemplate


        # TODO: better abstract this using the formatter
        html = [
            u'<form class="macro" method="POST" action="%s"><div>' % self.request.href(self.formatter.page.page_name),
            u'<input type="hidden" name="action" value="newpage">',
            u'<input type="hidden" name="parent" value="%s">' % wikiutil.escape(self.parent, 1),
            u'<input type="hidden" name="template" value="%s">' % wikiutil.escape(self.template, 1),
            u'<input type="hidden" name="nametemplate" value="%s">' % wikiutil.escape(self.nametemplate, 1),
        ]

        if requires_input:
            html += [
                u'<input type="text" name="pagename" size="30">',
            ]
        html += [
            u'<input type="submit" value="%s">' % wikiutil.escape(self.label, 1),
            u'</div></form>',
            ]
        return self.formatter.rawHTML('\n'.join(html))

def macro_NewPage(macro, template=u'', button_label=u'',
                  parent_page=u'', name_template=u'%s'):
    """ Temporary glue code to use with moin current macro system """
    return NewPage(macro, template, button_label, parent_page, name_template).renderInPage()

